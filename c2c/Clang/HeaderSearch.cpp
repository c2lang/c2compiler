//===- HeaderSearch.cpp - Resolve Header File Locations -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the DirectoryLookup and HeaderSearch interfaces.
//
//===----------------------------------------------------------------------===//

#include "Clang/HeaderSearch.h"
#include "Clang/Diagnostic.h"
#include "Clang/FileManager.h"
#include "Clang/IdentifierTable.h"
#include "Clang/SourceManager.h"
#include "Clang/VirtualFileSystem.h"
#include "Clang/DirectoryLookup.h"
#include "Clang/HeaderMap.h"
#include "Clang/HeaderSearchOptions.h"
#include "Clang/LexDiagnostic.h"
#include "Clang/Preprocessor.h"
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/Capacity.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>
#include <utility>

using namespace c2lang;

const IdentifierInfo *HeaderFileInfo::getControllingMacro() {
    return ControllingMacro;
}

ExternalHeaderFileInfoSource::~ExternalHeaderFileInfoSource() = default;

HeaderSearch::HeaderSearch(SourceManager &SourceMgr, DiagnosticsEngine &Diags)
        : Diags(Diags),
          FileMgr(SourceMgr.getFileManager()) {}

HeaderSearch::~HeaderSearch() {
}

void HeaderSearch::PrintStats() {
    fprintf(stderr, "\n*** HeaderSearch Stats:\n");
    fprintf(stderr, "%d files tracked.\n", (int) FileInfo.size());
    unsigned NumOnceOnlyFiles = 0, MaxNumIncludes = 0, NumSingleIncludedFiles = 0;
    for (unsigned i = 0, e = FileInfo.size(); i != e; ++i) {
        NumOnceOnlyFiles += FileInfo[i].isImport;
        if (MaxNumIncludes < FileInfo[i].NumIncludes)
            MaxNumIncludes = FileInfo[i].NumIncludes;
        NumSingleIncludedFiles += FileInfo[i].NumIncludes == 1;
    }
    fprintf(stderr, "  %d #import/#pragma once files.\n", NumOnceOnlyFiles);
    fprintf(stderr, "  %d included exactly once.\n", NumSingleIncludedFiles);
    fprintf(stderr, "  %d max times a file is included.\n", MaxNumIncludes);

    fprintf(stderr, "  %d #include/#include_next/#import.\n", NumIncluded);
    fprintf(stderr, "    %d #includes skipped due to"
                    " the multi-include optimization.\n", NumMultiIncludeFileOptzn);

}





//===----------------------------------------------------------------------===//
// File lookup within a DirectoryLookup scope
//===----------------------------------------------------------------------===//

/// getName - Return the directory or filename corresponding to this lookup
/// object.
StringRef DirectoryLookup::getName() const {
    return getDir()->getName();
}

const FileEntry *HeaderSearch::getFile(StringRef FileName,
                                       SourceLocation IncludeLoc,
                                       const DirectoryEntry *Dir,
                                       bool IsSystemHeaderDir) {
    return getFileMgr().getFile(FileName, /*OpenFile=*/true);


}

/// LookupFile - Lookup the specified file in this search path, returning it
/// if it exists or returning null if not.
const FileEntry *DirectoryLookup::LookupFile(StringRef &Filename,
                                             HeaderSearch &HS,
                                             SourceLocation IncludeLoc,
                                             SmallVectorImpl<char> *SearchPath,
                                             SmallVectorImpl<char> *RelativePath,
                                             bool &InUserSpecifiedSystemFramework,
                                             bool &HasBeenMapped,
                                             SmallVectorImpl<char> &MappedName) const {
    InUserSpecifiedSystemFramework = false;
    HasBeenMapped = false;

    // Concatenate the requested file onto the directory.
    SmallString<1024> TmpDir = getDir()->getName();
    llvm::sys::path::append(TmpDir, Filename);
    if (SearchPath) {
        StringRef SearchPathRef(getDir()->getName());
        SearchPath->clear();
        SearchPath->append(SearchPathRef.begin(), SearchPathRef.end());
    }
    if (RelativePath) {
        RelativePath->clear();
        RelativePath->append(Filename.begin(), Filename.end());
    }

    return HS.getFile(TmpDir, IncludeLoc, getDir(), isSystemHeaderDirectory());
}



//===----------------------------------------------------------------------===//
// Header File Location.
//===----------------------------------------------------------------------===//

/// Return true with a diagnostic if the file that MSVC would have found
/// fails to match the one that Clang would have found with MSVC header search
/// disabled.
static bool checkMSVCHeaderSearch(DiagnosticsEngine &Diags,
                                  const FileEntry *MSFE, const FileEntry *FE,
                                  SourceLocation IncludeLoc) {
    if (MSFE && FE != MSFE) {
        Diags.Report(IncludeLoc, diag::ext_pp_include_search_ms) << MSFE->getName();
        return true;
    }
    return false;
}

static const char *copyString(StringRef Str, llvm::BumpPtrAllocator &Alloc) {
    assert(!Str.empty());
    char *CopyStr = Alloc.Allocate<char>(Str.size() + 1);
    std::copy(Str.begin(), Str.end(), CopyStr);
    CopyStr[Str.size()] = '\0';
    return CopyStr;
}

static bool isFrameworkStylePath(StringRef Path, bool &IsPrivateHeader,
                                 SmallVectorImpl<char> &FrameworkName) {
    using namespace llvm::sys;
    path::const_iterator I = path::begin(Path);
    path::const_iterator E = path::end(Path);
    IsPrivateHeader = false;

    // Detect different types of framework style paths:
    //
    //   ...Foo.framework/{Headers,PrivateHeaders}
    //   ...Foo.framework/Versions/{A,Current}/{Headers,PrivateHeaders}
    //   ...Foo.framework/Frameworks/Nested.framework/{Headers,PrivateHeaders}
    //   ...<other variations with 'Versions' like in the above path>
    //
    // and some other variations among these lines.
    int FoundComp = 0;
    while (I != E) {
        if (*I == "Headers")
            ++FoundComp;
        if (I->endswith(".framework")) {
            FrameworkName.append(I->begin(), I->end());
            ++FoundComp;
        }
        if (*I == "PrivateHeaders") {
            ++FoundComp;
            IsPrivateHeader = true;
        }
        ++I;
    }

    return FoundComp >= 2;
}

static void
diagnoseFrameworkInclude(DiagnosticsEngine &Diags, SourceLocation IncludeLoc,
                         StringRef Includer, StringRef IncludeFilename,
                         const FileEntry *IncludeFE, bool isAngled = false,
                         bool FoundByHeaderMap = false) {
    bool IsIncluderPrivateHeader = false;
    SmallString<128> FromFramework, ToFramework;
    if (!isFrameworkStylePath(Includer, IsIncluderPrivateHeader, FromFramework))
        return;
    bool IsIncludeePrivateHeader = false;
    bool IsIncludeeInFramework = isFrameworkStylePath(
            IncludeFE->getName(), IsIncludeePrivateHeader, ToFramework);

    if (!isAngled && !FoundByHeaderMap) {
        SmallString<128> NewInclude("<");
        if (IsIncludeeInFramework) {
            NewInclude += StringRef(ToFramework).drop_back(10); // drop .framework
            NewInclude += "/";
        }
        NewInclude += IncludeFilename;
        NewInclude += ">";
        Diags.Report(IncludeLoc, diag::warn_quoted_include_in_framework_header)
                << IncludeFilename
                << FixItHint::CreateReplacement(IncludeLoc, NewInclude);
    }

    // Headers in Foo.framework/Headers should not include headers
    // from Foo.framework/PrivateHeaders, since this violates public/private
    // API boundaries and can cause modular dependency cycles.
    if (!IsIncluderPrivateHeader && IsIncludeeInFramework &&
        IsIncludeePrivateHeader && FromFramework == ToFramework)
        Diags.Report(IncludeLoc, diag::warn_framework_include_private_from_public)
                << IncludeFilename;
}

/// LookupFile - Given a "foo" or \<foo> reference, look up the indicated file,
/// return null on failure.  isAngled indicates whether the file reference is
/// for system \#include's or not (i.e. using <> instead of ""). Includers, if
/// non-empty, indicates where the \#including file(s) are, in case a relative
/// search is needed. Microsoft mode will pass all \#including files.
const FileEntry *HeaderSearch::LookupFile(StringRef Filename,
                                          SourceLocation IncludeLoc,
                                          bool isAngled,
                                          const DirectoryLookup *FromDir,
                                          const DirectoryLookup *&CurDir,
                                          ArrayRef<std::pair<const FileEntry *, const DirectoryEntry *>> Includers,
                                          SmallVectorImpl<char> *SearchPath,
                                          SmallVectorImpl<char> *RelativePath,
                                          bool *IsMapped,
                                          bool SkipCache) {
    if (IsMapped)
        *IsMapped = false;


    // If 'Filename' is absolute, check to see if it exists and no searching.
    if (llvm::sys::path::is_absolute(Filename)) {
        CurDir = nullptr;

        // If this was an #include_next "/absolute/file", fail.
        if (FromDir) return nullptr;

        if (SearchPath)
            SearchPath->clear();
        if (RelativePath) {
            RelativePath->clear();
            RelativePath->append(Filename.begin(), Filename.end());
        }
        // Otherwise, just return the file.
        return getFile(Filename, IncludeLoc, nullptr, false);
    }

    // This is the header that MSVC's header search would have found.
    const FileEntry *MSFE = nullptr;

    // Unless disabled, check to see if the file is in the #includer's
    // directory.  This cannot be based on CurDir, because each includer could be
    // a #include of a subdirectory (#include "foo/bar.h") and a subsequent
    // include of "baz.h" should resolve to "whatever/foo/baz.h".
    // This search is not done for <> headers.
    if (!Includers.empty() && !isAngled && !NoCurDirSearch) {
        SmallString<1024> TmpDir;
        bool First = true;
        for (const auto &IncluderAndDir : Includers) {
            const FileEntry *Includer = IncluderAndDir.first;

            // Concatenate the requested file onto the directory.
            // FIXME: Portability.  Filename concatenation should be in sys::Path.
            TmpDir = IncluderAndDir.second->getName();
            TmpDir.push_back('/');
            TmpDir.append(Filename.begin(), Filename.end());

            // FIXME: We don't cache the result of getFileInfo across the call to
            // getFile, because it's a reference to an element of
            // a container that could be reallocated across this call.
            //
            // If we have no includer, that means we're processing a #include
            // from a module build. We should treat this as a system header if we're
            // building a [system] module.
            bool IncluderIsSystemHeader =
                    Includer ? getFileInfo(Includer).DirInfo != SrcMgr::C_User : false;
            if (const FileEntry *FE = getFile(TmpDir, IncludeLoc, IncluderAndDir.second, IncluderIsSystemHeader)) {
                if (!Includer) {
                    assert(First && "only first includer can have no file");
                    return FE;
                }

                // Leave CurDir unset.
                // This file is a system header or C++ unfriendly if the old file is.
                //
                // Note that we only use one of FromHFI/ToHFI at once, due to potential
                // reallocation of the underlying vector potentially making the first
                // reference binding dangling.
                HeaderFileInfo &FromHFI = getFileInfo(Includer);
                unsigned DirInfo = FromHFI.DirInfo;

                HeaderFileInfo &ToHFI = getFileInfo(FE);
                ToHFI.DirInfo = DirInfo;

                if (SearchPath) {
                    StringRef SearchPathRef(IncluderAndDir.second->getName());
                    SearchPath->clear();
                    SearchPath->append(SearchPathRef.begin(), SearchPathRef.end());
                }
                if (RelativePath) {
                    RelativePath->clear();
                    RelativePath->append(Filename.begin(), Filename.end());
                }
                if (First) {
                    diagnoseFrameworkInclude(Diags, IncludeLoc,
                                             IncluderAndDir.second->getName(), Filename,
                                             FE);
                    return FE;
                }

                // Otherwise, we found the path via MSVC header search rules.  If
                // -Wmsvc-include is enabled, we have to keep searching to see if we
                // would've found this header in -I or -isystem directories.
                if (Diags.isIgnored(diag::ext_pp_include_search_ms, IncludeLoc)) {
                    return FE;
                } else {
                    MSFE = FE;
                    break;
                }
            }
            First = false;
        }
    }

    CurDir = nullptr;

    // If this is a system #include, ignore the user #include locs.
    unsigned i = isAngled ? AngledDirIdx : 0;

    // If this is a #include_next request, start searching after the directory the
    // file was found in.
    if (FromDir)
        i = FromDir - &SearchDirs[0];

    // Cache all of the lookups performed by this method.  Many headers are
    // multiply included, and the "pragma once" optimization prevents them from
    // being relex/pp'd, but they would still have to search through a
    // (potentially huge) series of SearchDirs to find it.
    LookupFileCacheInfo &CacheLookup = LookupFileCache[Filename];

    // If the entry has been previously looked up, the first value will be
    // non-zero.  If the value is equal to i (the start point of our search), then
    // this is a matching hit.
    if (!SkipCache && CacheLookup.StartIdx == i + 1) {
        // Skip querying potentially lots of directories for this lookup.
        i = CacheLookup.HitIdx;
        if (CacheLookup.MappedName) {
            Filename = CacheLookup.MappedName;
            if (IsMapped)
                *IsMapped = true;
        }
    } else {
        // Otherwise, this is the first query, or the previous query didn't match
        // our search start.  We will fill in our found location below, so prime the
        // start point value.
        CacheLookup.reset(/*StartIdx=*/i + 1);
    }

    SmallString<64> MappedName;

    // Check each directory in sequence to see if it contains this file.
    for (; i != SearchDirs.size(); ++i) {
        bool InUserSpecifiedSystemFramework = false;
        bool HasBeenMapped = false;
        const FileEntry *FE = SearchDirs[i].LookupFile(Filename,
                                                       *this,
                                                       IncludeLoc,
                                                       SearchPath,
                                                       RelativePath,
                                                       InUserSpecifiedSystemFramework,
                                                       HasBeenMapped,
                                                       MappedName);
        if (HasBeenMapped) {
            CacheLookup.MappedName =
                    copyString(Filename, LookupFileCache.getAllocator());
            if (IsMapped)
                *IsMapped = true;
        }
        if (!FE) continue;

        CurDir = &SearchDirs[i];

        // This file is a system header or C++ unfriendly if the dir is.
        HeaderFileInfo &HFI = getFileInfo(FE);
        HFI.DirInfo = CurDir->getDirCharacteristic();

        // If the directory characteristic is User but this framework was
        // user-specified to be treated as a system framework, promote the
        // characteristic.
        if (HFI.DirInfo == SrcMgr::C_User && InUserSpecifiedSystemFramework)
            HFI.DirInfo = SrcMgr::C_System;

        // If the filename matches a known system header prefix, override
        // whether the file is a system header.
        for (unsigned j = SystemHeaderPrefixes.size(); j; --j) {
            if (Filename.startswith(SystemHeaderPrefixes[j - 1].first)) {
                HFI.DirInfo = SystemHeaderPrefixes[j - 1].second ? SrcMgr::C_System
                                                                 : SrcMgr::C_User;
                break;
            }
        }


        if (checkMSVCHeaderSearch(Diags, MSFE, FE, IncludeLoc)) {
            return MSFE;
        }

        bool FoundByHeaderMap = !IsMapped ? false : *IsMapped;
        if (!Includers.empty())
            diagnoseFrameworkInclude(Diags, IncludeLoc,
                                     Includers.front().second->getName(), Filename,
                                     FE, isAngled, FoundByHeaderMap);

        // Remember this location for the next lookup we do.
        CacheLookup.HitIdx = i;
        return FE;
    }

    if (checkMSVCHeaderSearch(Diags, MSFE, nullptr, IncludeLoc)) {
        return MSFE;
    }

    // Otherwise, didn't find it. Remember we didn't find this.
    CacheLookup.HitIdx = SearchDirs.size();
    return nullptr;
}


//===----------------------------------------------------------------------===//
// File Info Management.
//===----------------------------------------------------------------------===//

/// Merge the header file info provided by \p OtherHFI into the current
/// header file info (\p HFI)
static void mergeHeaderFileInfo(HeaderFileInfo &HFI,
                                const HeaderFileInfo &OtherHFI) {
    assert(OtherHFI.External && "expected to merge external HFI");

    HFI.isImport |= OtherHFI.isImport;
    HFI.isPragmaOnce |= OtherHFI.isPragmaOnce;
    HFI.NumIncludes += OtherHFI.NumIncludes;

    if (!HFI.ControllingMacro && !HFI.ControllingMacroID) {
        HFI.ControllingMacro = OtherHFI.ControllingMacro;
        HFI.ControllingMacroID = OtherHFI.ControllingMacroID;
    }

    HFI.DirInfo = OtherHFI.DirInfo;
    HFI.External = (!HFI.IsValid || HFI.External);
    HFI.IsValid = true;

}

/// getFileInfo - Return the HeaderFileInfo structure for the specified
/// FileEntry.
HeaderFileInfo &HeaderSearch::getFileInfo(const FileEntry *FE) {
    if (FE->getUID() >= FileInfo.size())
        FileInfo.resize(FE->getUID() + 1);

    HeaderFileInfo *HFI = &FileInfo[FE->getUID()];
    // FIXME: Use a generation count to check whether this is really up to date.
    if (ExternalSource && !HFI->Resolved) {
        HFI->Resolved = true;
        auto ExternalHFI = ExternalSource->GetHeaderFileInfo(FE);

        HFI = &FileInfo[FE->getUID()];
        if (ExternalHFI.External)
            mergeHeaderFileInfo(*HFI, ExternalHFI);
    }

    HFI->IsValid = true;
    // We have local information about this header file, so it's no longer
    // strictly external.
    HFI->External = false;
    return *HFI;
}




bool HeaderSearch::ShouldEnterIncludeFile(Preprocessor &PP,
                                          const FileEntry *File, bool isImport) {
    ++NumIncluded; // Count # of attempted #includes.

    // Get information about this file.
    HeaderFileInfo &FileInfo = getFileInfo(File);


    // If this is a #import directive, check that we have not already imported
    // this header.
    if (isImport) {
        // If this has already been imported, don't import it again.
        FileInfo.isImport = true;

        // Has this already been #import'ed or #include'd?
        if (FileInfo.NumIncludes) return false;
    } else {
        // Otherwise, if this is a #include of a file that was previously #import'd
        // or if this is the second #include of a #pragma once file, ignore it.
        if (FileInfo.isImport) return false;
    }

    // Next, check to see if the file is wrapped with #ifndef guards.  If so, and
    // if the macro that guards it is defined, we know the #include has no effect.
    if (const IdentifierInfo *ControllingMacro
            = FileInfo.getControllingMacro()) {
        // If the header corresponds to a module, check whether the macro is already
        // defined in that module rather than checking in the current set of visible
        // modules.
        if (PP.isMacroDefined(ControllingMacro)) {
            ++NumMultiIncludeFileOptzn;
            return false;
        }
    }

    // Increment the number of times this file has been included.
    ++FileInfo.NumIncludes;

    return true;
}



