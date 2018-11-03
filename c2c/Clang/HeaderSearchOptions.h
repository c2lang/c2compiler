//===- HeaderSearchOptions.h ------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_HEADERSEARCHOPTIONS_H
#define LLVM_CLANG_LEX_HEADERSEARCHOPTIONS_H

#include "Clang/LLVM.h"
#include <llvm/ADT/CachedHashString.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/StringRef.h>
#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace c2lang {

namespace frontend {

/// IncludeDirGroup - Identifies the group an include Entry belongs to,
/// representing its relative positive in the search list.
/// \#include directives whose paths are enclosed by string quotes ("")
/// start searching at the Quoted group (specified by '-iquote'),
/// then search the Angled group, then the System group, etc.
enum IncludeDirGroup {
  /// '\#include ""' paths, added by 'gcc -iquote'.
  Quoted = 0,

  /// Paths for '\#include <>' added by '-I'.
  Angled,

  /// Like Angled, but marks header maps used when building frameworks.
  IndexHeaderMap,

  /// Like Angled, but marks system directories.
  System,

  /// Like System, but headers are implicitly wrapped in extern "C".
  ExternCSystem,

  /// Like System, but only used for C.
  CSystem,

  /// Like System, but only used for C++.
  CXXSystem,

  /// Like System, but only used for ObjC.
  ObjCSystem,

  /// Like System, but only used for ObjC++.
  ObjCXXSystem,

  /// Like System, but searched after the system directories.
  After
};

} // namespace frontend

/// HeaderSearchOptions - Helper class for storing options related to the
/// initialization of the HeaderSearch object.
class HeaderSearchOptions {
public:
  struct Entry {
    std::string Path;
    frontend::IncludeDirGroup Group;
    unsigned IsFramework : 1;

    /// IgnoreSysRoot - This is false if an absolute path should be treated
    /// relative to the sysroot, or true if it should always be the absolute
    /// path.
    unsigned IgnoreSysRoot : 1;

    Entry(StringRef path, frontend::IncludeDirGroup group, bool isFramework,
          bool ignoreSysRoot)
        : Path(path), Group(group), IsFramework(isFramework),
          IgnoreSysRoot(ignoreSysRoot) {}
  };

  struct SystemHeaderPrefix {
    /// A prefix to be matched against paths in \#include directives.
    std::string Prefix;

    /// True if paths beginning with this prefix should be treated as system
    /// headers.
    bool IsSystemHeader;

    SystemHeaderPrefix(StringRef Prefix, bool IsSystemHeader)
        : Prefix(Prefix), IsSystemHeader(IsSystemHeader) {}
  };

  /// If non-empty, the directory to use as a "virtual system root" for include
  /// paths.
  std::string Sysroot;

  /// User specified include entries.
  std::vector<Entry> UserEntries;

  /// User-specified system header prefixes.
  std::vector<SystemHeaderPrefix> SystemHeaderPrefixes;

  /// The directory which holds the compiler resource files (builtin includes,
  /// etc.).
  std::string ResourceDir;


    /// The time in seconds when the build session started.
  ///
  /// This time is used by other optimizations in header search and module
  /// loading.
  uint64_t BuildSessionTimestamp = 0;

    /// Include the compiler builtin includes.
  unsigned UseBuiltinIncludes : 1;

  /// Include the system standard include search directories.
  unsigned UseStandardSystemIncludes : 1;

  /// Include the system standard C++ library include search directories.
  unsigned UseStandardCXXIncludes : 1;

  /// Use libc++ instead of the default libstdc++.
  unsigned UseLibcxx : 1;

  /// Whether header search information should be output as for -v.
  unsigned Verbose : 1;


  /// Whether the module includes debug information (-gmodules).
  unsigned UseDebugInfo : 1;


  HeaderSearchOptions(StringRef _Sysroot = "/")
      : Sysroot(_Sysroot),
        UseBuiltinIncludes(true), UseStandardSystemIncludes(true),
        UseStandardCXXIncludes(true), UseLibcxx(false), Verbose(false), UseDebugInfo(false) {}

  /// AddPath - Add the \p Path path to the specified \p Group list.
  void AddPath(StringRef Path, frontend::IncludeDirGroup Group,
               bool IsFramework, bool IgnoreSysRoot) {
    UserEntries.emplace_back(Path, Group, IsFramework, IgnoreSysRoot);
  }

};

} // namespace c2lang

#endif // LLVM_CLANG_LEX_HEADERSEARCHOPTIONS_H
