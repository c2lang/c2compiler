//===- Commit.h - A unit of edits -------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_EDIT_COMMIT_H
#define LLVM_CLANG_EDIT_COMMIT_H

#include "Clang/LLVM.h"
#include "Clang/SourceLocation.h"
#include "Clang/FileOffset.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>

namespace c2lang {

class SourceManager;

namespace edit {

class EditedSource;

class Commit {
public:
  enum EditKind {
    Act_Insert,
    Act_InsertFromRange,
    Act_Remove
  };

  struct Edit {
    EditKind Kind;
    StringRef Text;
    SourceLocation OrigLoc;
    FileOffset Offset;
    FileOffset InsertFromRangeOffs;
    unsigned Length;
    bool BeforePrev;

  };

private:
  const SourceManager &SourceMgr;
  EditedSource *Editor = nullptr;

  bool IsCommitable = true;
  SmallVector<Edit, 8> CachedEdits;

  llvm::BumpPtrAllocator StrAlloc;

public:
  explicit Commit(EditedSource &Editor);
  Commit(const SourceManager &SM)
      : SourceMgr(SM) {}

  bool isCommitable() const { return IsCommitable; }

  bool insert(SourceLocation loc, StringRef text, bool afterToken = false,
              bool beforePreviousInsertions = false);


    bool insertFromRange(SourceLocation loc, CharSourceRange range,
                       bool afterToken = false,
                       bool beforePreviousInsertions = false);

    bool remove(CharSourceRange range);

  bool replace(CharSourceRange range, StringRef text);


    bool remove(SourceRange TokenRange) {
    return remove(CharSourceRange::getTokenRange(TokenRange));
  }


    using edit_iterator = SmallVectorImpl<Edit>::const_iterator;

  edit_iterator edit_begin() const { return CachedEdits.begin(); }
  edit_iterator edit_end() const { return CachedEdits.end(); }

private:
  void addInsert(SourceLocation OrigLoc,
                FileOffset Offs, StringRef text, bool beforePreviousInsertions);
  void addInsertFromRange(SourceLocation OrigLoc, FileOffset Offs,
                          FileOffset RangeOffs, unsigned RangeLen,
                          bool beforePreviousInsertions);
  void addRemove(SourceLocation OrigLoc, FileOffset Offs, unsigned Len);

  bool canInsert(SourceLocation loc, FileOffset &Offset);
  bool canInsertAfterToken(SourceLocation loc, FileOffset &Offset,
                           SourceLocation &AfterLoc);
  bool canInsertInOffset(SourceLocation OrigLoc, FileOffset Offs);
  bool canRemoveRange(CharSourceRange range, FileOffset &Offs, unsigned &Len);
  bool canReplaceText(SourceLocation loc, StringRef text,
                      FileOffset &Offs, unsigned &Len);

  void commitInsert(FileOffset offset, StringRef text,
                    bool beforePreviousInsertions);
  void commitRemove(FileOffset offset, unsigned length);

  bool isAtStartOfMacroExpansion(SourceLocation loc,
                                 SourceLocation *MacroBegin = nullptr) const;
  bool isAtEndOfMacroExpansion(SourceLocation loc,
                               SourceLocation *MacroEnd = nullptr) const;
};

} // namespace edit

} // namespace c2lang

#endif // LLVM_CLANG_EDIT_COMMIT_H
