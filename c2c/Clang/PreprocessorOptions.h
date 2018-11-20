//===- PreprocessorOptions.h ------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_PREPROCESSOROPTIONS_H_
#define LLVM_CLANG_LEX_PREPROCESSOROPTIONS_H_

#include "Clang/LLVM.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace llvm {

class MemoryBuffer;

} // namespace llvm

namespace c2lang {


/// PreprocessorOptions - This class is used for passing the various options
/// used in preprocessor initialization to InitializePreprocessor().
class PreprocessorOptions {
public:
  std::vector<std::pair<std::string, bool/*isUndef*/>> Macros;


    /// When enabled, preprocessor is in a mode for parsing a single file only.
  ///
  /// Disables #includes of other files and if there are unresolved identifiers
  /// in preprocessor directive conditions it causes all blocks to be parsed so
  /// that the client can get the maximum amount of information from the parser.
  bool SingleFileParseMode = false;

  /// When enabled, the preprocessor will construct editor placeholder tokens.
  bool LexEditorPlaceholders = true;

  /// True if the SourceManager should report the original file name for
  /// contents of files that were remapped to other files. Defaults to true.
  bool RemappedFilesKeepOriginalName = true;

  /// The set of file remappings, which take existing files on
  /// the system (the first part of each pair) and gives them the
  /// contents of other files on the system (the second part of each
  /// pair).
  std::vector<std::pair<std::string, std::string>> RemappedFiles;

  /// The set of file-to-buffer remappings, which take existing files
  /// on the system (the first part of each pair) and gives them the contents
  /// of the specified memory buffer (the second part of each pair).
  std::vector<std::pair<std::string, llvm::MemoryBuffer *>> RemappedFileBuffers;

  /// Whether the compiler instance should retain (i.e., not free)
  /// the buffers associated with remapped files.
  ///
  /// This flag defaults to false; it can be set true only through direct
  /// manipulation of the compiler invocation object, in cases where the
  /// compiler invocation and its buffers will be reused.
  bool RetainRemappedFileBuffers = false;



  public:

public:
  PreprocessorOptions()  {}

  void addMacroDef(StringRef Name) { Macros.emplace_back(Name, false); }
  void addMacroUndef(StringRef Name) { Macros.emplace_back(Name, true); }

  void addRemappedFile(StringRef From, StringRef To) {
    RemappedFiles.emplace_back(From, To);
  }

  void addRemappedFile(StringRef From, llvm::MemoryBuffer *To) {
    RemappedFileBuffers.emplace_back(From, To);
  }

  void clearRemappedFiles() {
    RemappedFiles.clear();
    RemappedFileBuffers.clear();
  }

};

} // namespace c2lang

#endif // LLVM_CLANG_LEX_PREPROCESSOROPTIONS_H_
