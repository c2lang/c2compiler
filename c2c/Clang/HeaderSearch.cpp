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

ExternalHeaderFileInfoSource::~ExternalHeaderFileInfoSource() = default;

HeaderSearch::HeaderSearch(std::shared_ptr<HeaderSearchOptions> HSOpts,
                           SourceManager &SourceMgr,
                           DiagnosticsEngine &Diags,
                           const LangOptions &LangOpts)
    : HSOpts(std::move(HSOpts)), Diags(Diags),
      FileMgr(SourceMgr.getFileManager()), FrameworkMap(64) {}

HeaderSearch::~HeaderSearch() {
  // Delete headermaps.
  for (unsigned i = 0, e = HeaderMaps.size(); i != e; ++i)
    delete HeaderMaps[i].second;
}

void HeaderSearch::PrintStats() {
  fprintf(stderr, "\n*** HeaderSearch Stats:\n");
  fprintf(stderr, "%d files tracked.\n", (int)FileInfo.size());
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

  fprintf(stderr, "%d framework lookups.\n", NumFrameworkLookups);
  fprintf(stderr, "%d subframework lookups.\n", NumSubFrameworkLookups);
}

/// CreateHeaderMap - This method returns a HeaderMap for the specified
/// FileEntry, uniquing them through the 'HeaderMaps' datastructure.
const HeaderMap *HeaderSearch::CreateHeaderMap(const FileEntry *FE) {
  // We expect the number of headermaps to be small, and almost always empty.
  // If it ever grows, use of a linear search should be re-evaluated.
  if (!HeaderMaps.empty()) {
    for (unsigned i = 0, e = HeaderMaps.size(); i != e; ++i)
      // Pointer equality comparison of FileEntries works because they are
      // already uniqued by inode.
      if (HeaderMaps[i].first == FE)
        return HeaderMaps[i].second;
  }

  if (const HeaderMap *HM = HeaderMap::Create(FE, FileMgr)) {
    HeaderMaps.push_back(std::make_pair(FE, HM));
    return HM;
  }

  return nullptr;
}


//===----------------------------------------------------------------------===//
// File lookup within a DirectoryLookup scope
//===----------------------------------------------------------------------===//

/// getName - Return the directory or filename corresponding to this lookup
/// object.
StringRef DirectoryLookup::getName() const {
  if (isNormalDir())
    return getDir()->getName();
  if (isFramework())
    return getFrameworkDir()->getName();
  assert(isHeaderMap() && "Unknown DirectoryLookup");
  return getHeaderMap()->getFileName();
}



/// getFileInfo - Return the HeaderFileInfo structure for the specified
/// FileEntry.
HeaderFileInfo &HeaderSearch::getFileInfo(const FileEntry *FE) {
  if (FE->getUID() >= FileInfo.size())
    FileInfo.resize(FE->getUID() + 1);

  HeaderFileInfo *HFI = &FileInfo[FE->getUID()];

  HFI->IsValid = true;
  // We have local information about this header file, so it's no longer
  // strictly external.
  HFI->External = false;
  return *HFI;
}



