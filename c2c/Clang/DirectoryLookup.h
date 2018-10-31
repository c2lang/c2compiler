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
public:
  enum LookupType_t {
    LT_NormalDir,
    LT_Framework,
    LT_HeaderMap
  };
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

  /// LookupType - This indicates whether this DirectoryLookup object is a
  /// normal directory, a framework, or a headermap.
  unsigned LookupType : 2;

  /// Whether this is a header map used when building a framework.
  unsigned IsIndexHeaderMap : 1;


public:
  /// DirectoryLookup ctor - Note that this ctor *does not take ownership* of
  /// 'dir'.
  DirectoryLookup(const DirectoryEntry *dir, SrcMgr::CharacteristicKind DT,
                  bool isFramework)
    : DirCharacteristic(DT),
      LookupType(isFramework ? LT_Framework : LT_NormalDir),
      IsIndexHeaderMap(false) {
    u.Dir = dir;
  }

  /// DirectoryLookup ctor - Note that this ctor *does not take ownership* of
  /// 'map'.
  DirectoryLookup(const HeaderMap *map, SrcMgr::CharacteristicKind DT,
                  bool isIndexHeaderMap)
    : DirCharacteristic(DT), LookupType(LT_HeaderMap),
      IsIndexHeaderMap(isIndexHeaderMap) {
    u.Map = map;
  }

  /// getLookupType - Return the kind of directory lookup that this is: either a
  /// normal directory, a framework path, or a HeaderMap.
  LookupType_t getLookupType() const { return (LookupType_t)LookupType; }

  /// getName - Return the directory or filename corresponding to this lookup
  /// object.
  StringRef getName() const;

  /// getDir - Return the directory that this entry refers to.
  ///
  const DirectoryEntry *getDir() const {
    return isNormalDir() ? u.Dir : nullptr;
  }

  /// getFrameworkDir - Return the directory that this framework refers to.
  ///
  const DirectoryEntry *getFrameworkDir() const {
    return isFramework() ? u.Dir : nullptr;
  }

  /// getHeaderMap - Return the directory that this entry refers to.
  ///
  const HeaderMap *getHeaderMap() const {
    return isHeaderMap() ? u.Map : nullptr;
  }

  /// isNormalDir - Return true if this is a normal directory, not a header map.
  bool isNormalDir() const { return getLookupType() == LT_NormalDir; }

  /// isFramework - True if this is a framework directory.
  ///
  bool isFramework() const { return getLookupType() == LT_Framework; }

  /// isHeaderMap - Return true if this is a header map, not a normal directory.
  bool isHeaderMap() const { return getLookupType() == LT_HeaderMap; }


    /// DirCharacteristic - The type of directory this is, one of the DirType enum
  /// values.
  SrcMgr::CharacteristicKind getDirCharacteristic() const {
    return (SrcMgr::CharacteristicKind)DirCharacteristic;
  }

  /// Whether this describes a system header directory.
  bool isSystemHeaderDirectory() const {
    return getDirCharacteristic() != SrcMgr::C_User;
  }

  /// Whether this header map is building a framework or not.
  bool isIndexHeaderMap() const {
    return isHeaderMap() && IsIndexHeaderMap;
  }


};

}  // end namespace c2lang

#endif
