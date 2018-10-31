//===- Version.cpp - Clang Version Number -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines several version-related utility functions for Clang.
//
//===----------------------------------------------------------------------===//

#include "Clang/Version.h"
#include "Clang/LLVM.h"
#include <llvm/Support/raw_ostream.h>
#include <cstdlib>
#include <cstring>

#ifdef HAVE_SVN_VERSION_INC
#  include "SVNVersion.inc"
#endif

namespace c2lang {

std::string getClangRepositoryPath() {
#if defined(CLANG_REPOSITORY_STRING)
  return CLANG_REPOSITORY_STRING;
#else
#ifdef SVN_REPOSITORY
  StringRef URL(SVN_REPOSITORY);
#else
  StringRef URL("");
#endif

  // If the SVN_REPOSITORY is empty, try to use the SVN keyword. This helps us
  // pick up a tag in an SVN export, for example.
  StringRef SVNRepository("$URL$");
  if (URL.empty()) {
    URL = SVNRepository.slice(SVNRepository.find(':'),
                              SVNRepository.find("/lib/Basic"));
  }

  // Strip off version from a build from an integration branch.
  URL = URL.slice(0, URL.find("/src/tools/c2lang"));

  // Trim path prefix off, assuming path came from standard cfe path.
  size_t Start = URL.find("cfe/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start + 4);

  return URL;
#endif
}

std::string getLLVMRepositoryPath() {
#ifdef LLVM_REPOSITORY
  StringRef URL(LLVM_REPOSITORY);
#else
  StringRef URL("");
#endif

  // Trim path prefix off, assuming path came from standard llvm path.
  // Leave "llvm/" prefix to distinguish the following llvm revision from the
  // c2lang revision.
  size_t Start = URL.find("llvm/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start);

  return URL;
}

std::string getClangRevision() {
#ifdef SVN_REVISION
  return SVN_REVISION;
#else
  return "";
#endif
}

std::string getLLVMRevision() {
#ifdef LLVM_REVISION
  return LLVM_REVISION;
#else
  return "";
#endif
}

std::string getClangFullRepositoryVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
  std::string Path = getClangRepositoryPath();
  std::string Revision = getClangRevision();
  if (!Path.empty() || !Revision.empty()) {
    OS << '(';
    if (!Path.empty())
      OS << Path;
    if (!Revision.empty()) {
      if (!Path.empty())
        OS << ' ';
      OS << Revision;
    }
    OS << ')';
  }
  // Support LLVM in a separate repository.
  std::string LLVMRev = getLLVMRevision();
  if (!LLVMRev.empty() && LLVMRev != Revision) {
    OS << " (";
    std::string LLVMRepo = getLLVMRepositoryPath();
    if (!LLVMRepo.empty())
      OS << LLVMRepo << ' ';
    OS << LLVMRev << ')';
  }
  return OS.str();
}

std::string getClangFullVersion() {
  return getClangToolFullVersion("c2lang");
}

std::string getClangToolFullVersion(StringRef ToolName) {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << ToolName << " version " CLANG_VERSION_STRING " "
     << getClangFullRepositoryVersion();

  // If vendor supplied, include the base LLVM version as well.
#ifdef CLANG_VENDOR
  OS << " (based on " << BACKEND_PACKAGE_STRING << ")";
#endif

  return OS.str();
}

std::string getClangFullCPPVersion() {
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << "Clang " CLANG_VERSION_STRING " " << getClangFullRepositoryVersion();
  return OS.str();
}

} // end namespace c2lang
