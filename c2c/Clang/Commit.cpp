//===- Commit.cpp - A unit of edits ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Clang/Commit.h"
#include "Clang/LLVM.h"
#include "Clang/SourceLocation.h"
#include "Clang/SourceManager.h"
#include "Clang/EditedSource.h"
#include "Clang/FileOffset.h"
#include "Clang/Lexer.h"
#include <llvm/ADT/StringRef.h>
#include <cassert>
#include <utility>

using namespace c2lang;
using namespace edit;


Commit::Commit(EditedSource &Editor)
    : SourceMgr(Editor.getSourceManager()),
      Editor(&Editor) {}

bool Commit::insert(SourceLocation loc, StringRef text,
                    bool afterToken, bool beforePreviousInsertions) {
  if (text.empty())
    return true;

  FileOffset Offs;
  if ((!afterToken && !canInsert(loc, Offs)) ||
      ( afterToken && !canInsertAfterToken(loc, Offs, loc))) {
    IsCommitable = false;
    return false;
  }

  addInsert(loc, Offs, text, beforePreviousInsertions);
  return true;
}

bool Commit::insertFromRange(SourceLocation loc,
                             CharSourceRange range,
                             bool afterToken, bool beforePreviousInsertions) {
  FileOffset RangeOffs;
  unsigned RangeLen;
  if (!canRemoveRange(range, RangeOffs, RangeLen)) {
    IsCommitable = false;
    return false;
  }

  FileOffset Offs;
  if ((!afterToken && !canInsert(loc, Offs)) ||
      ( afterToken && !canInsertAfterToken(loc, Offs, loc))) {
    IsCommitable = false;
    return false;
  }


  addInsertFromRange(loc, Offs, RangeOffs, RangeLen, beforePreviousInsertions);
  return true;
}

bool Commit::remove(CharSourceRange range) {
  FileOffset Offs;
  unsigned Len;
  if (!canRemoveRange(range, Offs, Len)) {
    IsCommitable = false;
    return false;
  }

  addRemove(range.getBegin(), Offs, Len);
  return true;
}


bool Commit::replace(CharSourceRange range, StringRef text) {
  if (text.empty())
    return remove(range);

  FileOffset Offs;
  unsigned Len;
  if (!canInsert(range.getBegin(), Offs) || !canRemoveRange(range, Offs, Len)) {
    IsCommitable = false;
    return false;
  }

  addRemove(range.getBegin(), Offs, Len);
  addInsert(range.getBegin(), Offs, text, false);
  return true;
}


void Commit::addInsert(SourceLocation OrigLoc, FileOffset Offs, StringRef text,
                       bool beforePreviousInsertions) {
  if (text.empty())
    return;

  Edit data;
  data.Kind = Act_Insert;
  data.OrigLoc = OrigLoc;
  data.Offset = Offs;
  data.Text = text.copy(StrAlloc);
  data.BeforePrev = beforePreviousInsertions;
  CachedEdits.push_back(data);
}

void Commit::addInsertFromRange(SourceLocation OrigLoc, FileOffset Offs,
                                FileOffset RangeOffs, unsigned RangeLen,
                                bool beforePreviousInsertions) {
  if (RangeLen == 0)
    return;

  Edit data;
  data.Kind = Act_InsertFromRange;
  data.OrigLoc = OrigLoc;
  data.Offset = Offs;
  data.InsertFromRangeOffs = RangeOffs;
  data.Length = RangeLen;
  data.BeforePrev = beforePreviousInsertions;
  CachedEdits.push_back(data);
}

void Commit::addRemove(SourceLocation OrigLoc,
                       FileOffset Offs, unsigned Len) {
  if (Len == 0)
    return;

  Edit data;
  data.Kind = Act_Remove;
  data.OrigLoc = OrigLoc;
  data.Offset = Offs;
  data.Length = Len;
  CachedEdits.push_back(data);
}

bool Commit::canInsert(SourceLocation loc, FileOffset &offs) {
  if (loc.isInvalid())
    return false;

  if (loc.isMacroID())
    isAtStartOfMacroExpansion(loc, &loc);

  const SourceManager &SM = SourceMgr;
  loc = SM.getTopMacroCallerLoc(loc);

  if (loc.isMacroID())
    if (!isAtStartOfMacroExpansion(loc, &loc))
      return false;

  if (SM.isInSystemHeader(loc))
    return false;

  std::pair<FileID, unsigned> locInfo = SM.getDecomposedLoc(loc);
  if (locInfo.first.isInvalid())
    return false;
  offs = FileOffset(locInfo.first, locInfo.second);
  return canInsertInOffset(loc, offs);
}

bool Commit::canInsertAfterToken(SourceLocation loc, FileOffset &offs,
                                 SourceLocation &AfterLoc) {
  if (loc.isInvalid())

    return false;

  SourceLocation spellLoc = SourceMgr.getSpellingLoc(loc);
  unsigned tokLen = Lexer::MeasureTokenLength(spellLoc, SourceMgr);
  AfterLoc = loc.getLocWithOffset(tokLen);

  if (loc.isMacroID())
    isAtEndOfMacroExpansion(loc, &loc);

  const SourceManager &SM = SourceMgr;
  loc = SM.getTopMacroCallerLoc(loc);

  if (loc.isMacroID())
    if (!isAtEndOfMacroExpansion(loc, &loc))
      return false;

  if (SM.isInSystemHeader(loc))
    return false;

  loc = Lexer::getLocForEndOfToken(loc, 0, SourceMgr);
  if (loc.isInvalid())
    return false;

  std::pair<FileID, unsigned> locInfo = SM.getDecomposedLoc(loc);
  if (locInfo.first.isInvalid())
    return false;
  offs = FileOffset(locInfo.first, locInfo.second);
  return canInsertInOffset(loc, offs);
}

bool Commit::canInsertInOffset(SourceLocation OrigLoc, FileOffset Offs) {
  for (const auto &act : CachedEdits)
    if (act.Kind == Act_Remove) {
      if (act.Offset.getFID() == Offs.getFID() &&
          Offs > act.Offset && Offs < act.Offset.getWithOffset(act.Length))
        return false; // position has been removed.
    }

  if (!Editor)
    return true;
  return Editor->canInsertInOffset(OrigLoc, Offs);
}

bool Commit::canRemoveRange(CharSourceRange range,
                            FileOffset &Offs, unsigned &Len) {
  const SourceManager &SM = SourceMgr;
  range = Lexer::makeFileCharRange(range, SM);
  if (range.isInvalid())
    return false;

  if (range.getBegin().isMacroID() || range.getEnd().isMacroID())
    return false;
  if (SM.isInSystemHeader(range.getBegin()) ||
      SM.isInSystemHeader(range.getEnd()))
    return false;


  std::pair<FileID, unsigned> beginInfo = SM.getDecomposedLoc(range.getBegin());
  std::pair<FileID, unsigned> endInfo = SM.getDecomposedLoc(range.getEnd());
  if (beginInfo.first != endInfo.first ||
      beginInfo.second > endInfo.second)
    return false;

  Offs = FileOffset(beginInfo.first, beginInfo.second);
  Len = endInfo.second - beginInfo.second;
  return true;
}

bool Commit::canReplaceText(SourceLocation loc, StringRef text,
                            FileOffset &Offs, unsigned &Len) {
  assert(!text.empty());

  if (!canInsert(loc, Offs))
    return false;

  // Try to load the file buffer.
  bool invalidTemp = false;
  StringRef file = SourceMgr.getBufferData(Offs.getFID(), &invalidTemp);
  if (invalidTemp)
    return false;

  Len = text.size();
  return file.substr(Offs.getOffset()).startswith(text);
}

bool Commit::isAtStartOfMacroExpansion(SourceLocation loc,
                                       SourceLocation *MacroBegin) const {
  return Lexer::isAtStartOfMacroExpansion(loc, SourceMgr, MacroBegin);
}

bool Commit::isAtEndOfMacroExpansion(SourceLocation loc,
                                     SourceLocation *MacroEnd) const {
  return Lexer::isAtEndOfMacroExpansion(loc, SourceMgr, MacroEnd);
}
