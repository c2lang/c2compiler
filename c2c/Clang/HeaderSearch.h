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
class ExternalPreprocessorSource;
class FileEntry;
class FileManager;
class HeaderMap;
class HeaderSearchOptions;
class IdentifierInfo;
class LangOptions;
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

  /// Whether this is a header inside a framework that is currently
  /// being built.
  ///
  /// When a framework is being built, the headers have not yet been placed
  /// into the appropriate framework subdirectories, and therefore are
  /// provided via a header map. This bit indicates when this is one of
  /// those framework headers.
  unsigned IndexHeaderMapHeader : 1;

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

  /// If this header came from a framework include, this is the name
  /// of the framework.
  StringRef Framework;

  HeaderFileInfo()
      : isImport(false), isPragmaOnce(false), DirInfo(SrcMgr::C_User),
        External(false),
        Resolved(false), IndexHeaderMapHeader(false), IsValid(false)  {}


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

  /// This structure is used to record entries in our framework cache.
  struct FrameworkCacheEntry {
    /// The directory entry which should be used for the cached framework.
    const DirectoryEntry *Directory;

    /// Whether this framework has been "user-specified" to be treated as if it
    /// were a system framework (even if it was found outside a system framework
    /// directory).
    bool IsUserSpecifiedSystemFramework;
  };

  /// Header-search options used to initialize this header search.
  std::shared_ptr<HeaderSearchOptions> HSOpts;

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

  /// Collection mapping a framework or subframework
  /// name like "Carbon" to the Carbon.framework directory.
  llvm::StringMap<FrameworkCacheEntry, llvm::BumpPtrAllocator> FrameworkMap;

  /// IncludeAliases - maps include file names (including the quotes or
  /// angle brackets) to other include file names.  This is used to support the
  /// include_alias pragma for Microsoft compatibility.
  using IncludeAliasMap =
      llvm::StringMap<std::string, llvm::BumpPtrAllocator>;
  std::unique_ptr<IncludeAliasMap> IncludeAliases;

  /// HeaderMaps - This is a mapping from FileEntry -> HeaderMap, uniquing
  /// headermaps.  This vector owns the headermap.
  std::vector<std::pair<const FileEntry *, const HeaderMap *>> HeaderMaps;



  /// Uniqued set of framework names, which is used to track which
  /// headers were included as framework headers.
  llvm::StringSet<llvm::BumpPtrAllocator> FrameworkNames;


  // Various statistics we track for performance analysis.
  unsigned NumIncluded = 0;
  unsigned NumMultiIncludeFileOptzn = 0;
  unsigned NumFrameworkLookups = 0;
  unsigned NumSubFrameworkLookups = 0;

public:
  HeaderSearch(std::shared_ptr<HeaderSearchOptions> HSOpts,
                 SourceManager &SourceMgr,
                 DiagnosticsEngine &Diags,
                 const LangOptions &LangOpts);
  HeaderSearch(const HeaderSearch &) = delete;
  HeaderSearch &operator=(const HeaderSearch &) = delete;
  ~HeaderSearch();

  /// Retrieve the header-search options with which this header search
  /// was initialized.
  HeaderSearchOptions &getHeaderSearchOpts() const { return *HSOpts; }

  FileManager &getFileMgr() const { return FileMgr; }

  DiagnosticsEngine &getDiags() const { return Diags; }

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

  /// Checks whether the map exists or not.
  bool HasIncludeAliasMap() const { return (bool)IncludeAliases; }

  /// Map the source include name to the dest include name.
  ///
  /// The Source should include the angle brackets or quotes, the dest
  /// should not.  This allows for distinction between <> and "" headers.
  void AddIncludeAlias(StringRef Source, StringRef Dest) {
    if (!IncludeAliases)
      IncludeAliases.reset(new IncludeAliasMap);
    (*IncludeAliases)[Source] = Dest;
  }

  /// MapHeaderToIncludeAlias - Maps one header file name to a different header
  /// file name, for use with the include_alias pragma.  Note that the source
  /// file name should include the angle brackets or quotes.  Returns StringRef
  /// as null if the header cannot be mapped.
  StringRef MapHeaderToIncludeAlias(StringRef Source) {
    assert(IncludeAliases && "Trying to map headers when there's no map");

    // Do any filename replacements before anything else
    IncludeAliasMap::const_iterator Iter = IncludeAliases->find(Source);
    if (Iter != IncludeAliases->end())
      return Iter->second;
    return {};
  }


    /// Forget everything we know about headers so far.
  void ClearFileInfo() {
    FileInfo.clear();
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


    /// CreateHeaderMap - This method returns a HeaderMap for the specified
  /// FileEntry, uniquing them through the 'HeaderMaps' datastructure.
  const HeaderMap *CreateHeaderMap(const FileEntry *FE);


    void IncrementFrameworkLookupCount() { ++NumFrameworkLookups; }


public:

    /// Return the HeaderFileInfo structure for the specified FileEntry,
  /// in preparation for updating it in some way.
  HeaderFileInfo &getFileInfo(const FileEntry *FE);

    // Used by external tools
  using search_dir_iterator = std::vector<DirectoryLookup>::const_iterator;

  search_dir_iterator search_dir_begin() const { return SearchDirs.begin(); }
  search_dir_iterator search_dir_end() const { return SearchDirs.end(); }
  unsigned search_dir_size() const { return SearchDirs.size(); }

  search_dir_iterator quoted_dir_begin() const {
    return SearchDirs.begin();
  }

  search_dir_iterator quoted_dir_end() const {
    return SearchDirs.begin() + AngledDirIdx;
  }

  search_dir_iterator angled_dir_begin() const {
    return SearchDirs.begin() + AngledDirIdx;
  }

  search_dir_iterator angled_dir_end() const {
    return SearchDirs.begin() + SystemDirIdx;
  }

  search_dir_iterator system_dir_begin() const {
    return SearchDirs.begin() + SystemDirIdx;
  }

  search_dir_iterator system_dir_end() const { return SearchDirs.end(); }


    void PrintStats();

};

} // namespace c2lang

#endif // LLVM_CLANG_LEX_HEADERSEARCH_H
