//===- HeaderSearch.h - Resolve Header File Locations -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HeaderSearch interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_HEADERSEARCH_H
#define LLVM_CLANG_LEX_HEADERSEARCH_H

#include "Clang/SourceLocation.h"
#include "Clang/SourceManager.h"
#include "Clang/DirectoryLookup.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace c2lang {

class DiagnosticsEngine;
class DirectoryEntry;
class FileEntry;
class FileManager;
class HeaderMap;
class HeaderSearchOptions;
class IdentifierInfo;
class Module;
class Preprocessor;

/// The preprocessor keeps track of this information for each
/// file that is \#included.
struct HeaderFileInfo {
  /// True if this is a \#import'd or \#pragma once file.
  unsigned isImport : 1;

  /// True if this is a \#pragma once file.
  unsigned isPragmaOnce : 1;

  /// DirInfo - Keep track of whether this is a system header, and if so,
  /// whether it is C++ clean or not.  This can be set by the include paths or
  /// by \#pragma gcc system_header.  This is an instance of
  /// SrcMgr::CharacteristicKind.
  unsigned DirInfo : 3;

  /// Whether this header file info was supplied by an external source,
  /// and has not changed since.
  unsigned External : 1;


  /// Whether this structure is considered to already have been
  /// "resolved", meaning that it was loaded from the external source.
  unsigned Resolved : 1;


  /// Whether this file has been looked up as a header.
  unsigned IsValid : 1;

  /// The number of times the file has been included already.
  unsigned short NumIncludes = 0;

  /// The ID number of the controlling macro.
  ///
  /// This ID number will be non-zero when there is a controlling
  /// macro whose IdentifierInfo may not yet have been loaded from
  /// external storage.
  unsigned ControllingMacroID = 0;

  /// If this file has a \#ifndef XXX (or equivalent) guard that
  /// protects the entire contents of the file, this is the identifier
  /// for the macro that controls whether or not it has any effect.
  ///
  /// Note: Most clients should use getControllingMacro() to access
  /// the controlling macro of this header, since
  /// getControllingMacro() is able to load a controlling macro from
  /// external storage.
  const IdentifierInfo *ControllingMacro = nullptr;


  HeaderFileInfo()
      : isImport(false), isPragmaOnce(false), DirInfo(SrcMgr::C_User),
        External(false),
        Resolved(false), IsValid(false)  {}

  /// Retrieve the controlling macro for this header file, if
  /// any.
  const IdentifierInfo *
  getControllingMacro();

  /// Determine whether this is a non-default header file info, e.g.,
  /// it corresponds to an actual header we've included or tried to include.
  bool isNonDefault() const {
    return isImport || isPragmaOnce || NumIncludes || ControllingMacro ||
      ControllingMacroID;
  }
};

/// An external source of header file information, which may supply
/// information about header files already included.
class ExternalHeaderFileInfoSource {
public:
  virtual ~ExternalHeaderFileInfoSource();

  /// Retrieve the header file information for the given file entry.
  ///
  /// \returns Header file information for the given file entry, with the
  /// \c External bit set. If the file entry is not known, return a
  /// default-constructed \c HeaderFileInfo.
  virtual HeaderFileInfo GetHeaderFileInfo(const FileEntry *FE) = 0;
};

/// Encapsulates the information needed to find the file referenced
/// by a \#include or \#include_next, (sub-)framework lookup, etc.
class HeaderSearch {
  friend class DirectoryLookup;


  DiagnosticsEngine &Diags;
  FileManager &FileMgr;

  /// \#include search path information.  Requests for \#include "x" search the
  /// directory of the \#including file first, then each directory in SearchDirs
  /// consecutively. Requests for <x> search the current dir first, then each
  /// directory in SearchDirs, starting at AngledDirIdx, consecutively.  If
  /// NoCurDirSearch is true, then the check for the file in the current
  /// directory is suppressed.
  std::vector<DirectoryLookup> SearchDirs;
  unsigned AngledDirIdx = 0;
  unsigned SystemDirIdx = 0;
  bool NoCurDirSearch = false;

  /// \#include prefixes for which the 'system header' property is
  /// overridden.
  ///
  /// For a \#include "x" or \#include \<x> directive, the last string in this
  /// list which is a prefix of 'x' determines whether the file is treated as
  /// a system header.
  std::vector<std::pair<std::string, bool>> SystemHeaderPrefixes;


  /// All of the preprocessor-specific data about files that are
  /// included, indexed by the FileEntry's UID.
  mutable std::vector<HeaderFileInfo> FileInfo;

  /// Keeps track of each lookup performed by LookupFile.
  struct LookupFileCacheInfo {
    /// Starting index in SearchDirs that the cached search was performed from.
    /// If there is a hit and this value doesn't match the current query, the
    /// cache has to be ignored.
    unsigned StartIdx = 0;

    /// The entry in SearchDirs that satisfied the query.
    unsigned HitIdx = 0;

    /// This is non-null if the original filename was mapped to a framework
    /// include via a headermap.
    const char *MappedName = nullptr;

    /// Default constructor -- Initialize all members with zero.
    LookupFileCacheInfo() = default;

    void reset(unsigned StartIdx) {
      this->StartIdx = StartIdx;
      this->MappedName = nullptr;
    }
  };
  llvm::StringMap<LookupFileCacheInfo, llvm::BumpPtrAllocator> LookupFileCache;






  /// Entity used to look up stored header file information.
  ExternalHeaderFileInfoSource *ExternalSource = nullptr;

  // Various statistics we track for performance analysis.
  unsigned NumIncluded = 0;
  unsigned NumMultiIncludeFileOptzn = 0;

public:
  HeaderSearch(SourceManager &SourceMgr, DiagnosticsEngine &Diags);
  HeaderSearch(const HeaderSearch &) = delete;
  HeaderSearch &operator=(const HeaderSearch &) = delete;
  ~HeaderSearch();


  FileManager &getFileMgr() const { return FileMgr; }


  /// Interface for setting the file search paths.
  void SetSearchPaths(const std::vector<DirectoryLookup> &dirs,
                      unsigned angledDirIdx, unsigned systemDirIdx,
                      bool noCurDirSearch) {
    assert(angledDirIdx <= systemDirIdx && systemDirIdx <= dirs.size() &&
        "Directory indices are unordered");
    SearchDirs = dirs;
    AngledDirIdx = angledDirIdx;
    SystemDirIdx = systemDirIdx;
    NoCurDirSearch = noCurDirSearch;
    //LookupFileCache.clear();
  }



  /// Set the list of system header prefixes.
  void SetSystemHeaderPrefixes(ArrayRef<std::pair<std::string, bool>> P) {
    SystemHeaderPrefixes.assign(P.begin(), P.end());
  }


    /// Given a "foo" or \<foo> reference, look up the indicated file,
  /// return null on failure.
  ///
  /// \returns If successful, this returns 'UsedDir', the DirectoryLookup member
  /// the file was found in, or null if not applicable.
  ///
  /// \param IncludeLoc Used for diagnostics if valid.
  ///
  /// \param isAngled indicates whether the file reference is a <> reference.
  ///
  /// \param CurDir If non-null, the file was found in the specified directory
  /// search location.  This is used to implement \#include_next.
  ///
  /// \param Includers Indicates where the \#including file(s) are, in case
  /// relative searches are needed. In reverse order of inclusion.
  ///
  /// \param SearchPath If non-null, will be set to the search path relative
  /// to which the file was found. If the include path is absolute, SearchPath
  /// will be set to an empty string.
  ///
  /// \param RelativePath If non-null, will be set to the path relative to
  /// SearchPath at which the file was found. This only differs from the
  /// Filename for framework includes.
  ///
  /// \param SuggestedModule If non-null, and the file found is semantically
  /// part of a known module, this will be set to the module that should
  /// be imported instead of preprocessing/parsing the file found.
  ///
  /// \param IsMapped If non-null, and the search involved header maps, set to
  /// true.
    const FileEntry *LookupFile(StringRef Filename,
                                  SourceLocation IncludeLoc,
                                  bool isAngled,
                                  const DirectoryLookup *FromDir,
                                  const DirectoryLookup *&CurDir,
                                  ArrayRef <std::pair<const FileEntry *, const DirectoryEntry *>> Includers,
                                  SmallVectorImpl<char> *SearchPath,
                                  SmallVectorImpl<char> *RelativePath,
                                  bool *IsMapped,
                                  bool SkipCache);




  /// Mark the specified file as a target of a \#include,
  /// \#include_next, or \#import directive.
  ///
  /// \return false if \#including the file will have no effect or true
  /// if we should include it.
  bool ShouldEnterIncludeFile(Preprocessor &PP, const FileEntry *File,
                              bool isImport);

  /// Return whether the specified file is a normal header,
  /// a system header, or a C++ friendly system header.
  SrcMgr::CharacteristicKind getFileDirFlavor(const FileEntry *File) {
    return (SrcMgr::CharacteristicKind)getFileInfo(File).DirInfo;
  }



    /// Increment the count for the number of times the specified
  /// FileEntry has been entered.
  void IncrementIncludeCount(const FileEntry *File) {
    ++getFileInfo(File).NumIncludes;
  }

  /// Mark the specified file as having a controlling macro.
  ///
  /// This is used by the multiple-include optimization to eliminate
  /// no-op \#includes.
  void SetFileControllingMacro(const FileEntry *File,
                               const IdentifierInfo *ControllingMacro) {
    getFileInfo(File).ControllingMacro = ControllingMacro;
  }

  /// Return true if this is the first time encountering this header.
  bool FirstTimeLexingFile(const FileEntry *File) {
    return getFileInfo(File).NumIncludes == 1;
  }



private:

    /// Look up the file with the specified name and determine its owning
  /// module.
  const FileEntry *getFile(StringRef FileName,
                           SourceLocation IncludeLoc,
                           const DirectoryEntry *Dir,
                           bool IsSystemHeaderDir);

public:


  /// Return the HeaderFileInfo structure for the specified FileEntry,
  /// in preparation for updating it in some way.
  HeaderFileInfo &getFileInfo(const FileEntry *FE);


  // Used by external tools
  using search_dir_iterator = std::vector<DirectoryLookup>::const_iterator;


    void PrintStats();


};

} // namespace c2lang

#endif // LLVM_CLANG_LEX_HEADERSEARCH_H
