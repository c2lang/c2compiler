//===--- DirectoryLookup.h - Info for searching for headers -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DirectoryLookup interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_DIRECTORYLOOKUP_H
#define LLVM_CLANG_LEX_DIRECTORYLOOKUP_H

#include "Clang/LLVM.h"
#include "Clang/SourceManager.h"

namespace c2lang {
class HeaderMap;
class DirectoryEntry;
class FileEntry;
class HeaderSearch;

/// DirectoryLookup - This class represents one entry in the search list that
/// specifies the search order for directories in \#include directives.  It
/// represents either a directory, a framework, or a headermap.
///
class DirectoryLookup {

private:
  union {  // This union is discriminated by isHeaderMap.
    /// Dir - This is the actual directory that we're referring to for a normal
    /// directory or a framework.
    const DirectoryEntry *Dir;

    /// Map - This is the HeaderMap if this is a headermap lookup.
    ///
    const HeaderMap *Map;
  } u;

  /// DirCharacteristic - The type of directory this is: this is an instance of
  /// SrcMgr::CharacteristicKind.
  unsigned DirCharacteristic : 2;



public:
  /// DirectoryLookup ctor - Note that this ctor *does not take ownership* of
  /// 'dir'.
  DirectoryLookup(const DirectoryEntry *dir, SrcMgr::CharacteristicKind DT)
    : DirCharacteristic(DT) {
    u.Dir = dir;
  }



  /// getName - Return the directory or filename corresponding to this lookup
  /// object.
  StringRef getName() const;

  /// getDir - Return the directory that this entry refers to.
  ///
  const DirectoryEntry *getDir() const {
    return u.Dir;
  }


    /// DirCharacteristic - The type of directory this is, one of the DirType enum
  /// values.
  SrcMgr::CharacteristicKind getDirCharacteristic() const {
    return (SrcMgr::CharacteristicKind)DirCharacteristic;
  }

  /// Whether this describes a system header directory.
  bool isSystemHeaderDirectory() const {
    return getDirCharacteristic() != SrcMgr::C_User;
  }



  /// LookupFile - Lookup the specified file in this search path, returning it
  /// if it exists or returning null if not.
  ///
  /// \param Filename The file to look up relative to the search paths.
  ///
  /// \param HS The header search instance to search with.
  ///
  /// \param IncludeLoc the source location of the #include or #import
  /// directive.
  ///
  /// \param SearchPath If not NULL, will be set to the search path relative
  /// to which the file was found.
  ///
  /// \param RelativePath If not NULL, will be set to the path relative to
  /// SearchPath at which the file was found. This only differs from the
  /// Filename for framework includes.
  ///
  /// \param RequestingModule The module in which the lookup was performed.
  ///
  /// \param SuggestedModule If non-null, and the file found is semantically
  /// part of a known module, this will be set to the module that should
  /// be imported instead of preprocessing/parsing the file found.
  ///
  /// \param [out] InUserSpecifiedSystemFramework If the file is found,
  /// set to true if the file is located in a framework that has been
  /// user-specified to be treated as a system framework.
  ///
  /// \param [out] MappedName if this is a headermap which maps the filename to
  /// a framework include ("Foo.h" -> "Foo/Foo.h"), set the new name to this
  /// vector and point Filename to it.
  const FileEntry *LookupFile(StringRef &Filename,
                              HeaderSearch &HS,
                              SourceLocation IncludeLoc,
                              SmallVectorImpl<char> *SearchPath,
                              SmallVectorImpl<char> *RelativePath,
                              bool &InUserSpecifiedSystemFramework,
                              bool &HasBeenMapped,
                              SmallVectorImpl<char> &MappedName) const;

};

}  // end namespace c2lang

#endif
