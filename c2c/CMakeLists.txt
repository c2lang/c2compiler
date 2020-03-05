
set(LLVM_CONFIG_EXECUTABLE "llvm-config")

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
#set(LLVM_CXXFLAGS "${LLVM_CXXFLAGS} -fno-exceptions -fno-rtti")

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
  OUTPUT_VARIABLE LLVM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
  OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
#line below is needed for Ubuntu 13.04 32 bit
include_directories("/usr/include/i386-linux-gnu/c++/4.8/")

add_library(c2core
    Clang/CharInfo.cpp
    Clang/Commit.cpp
    Clang/Diagnostic.cpp
    Clang/DiagnosticIDs.cpp
    Clang/DiagnosticOptions.cpp
    Clang/DiagnosticRenderer.cpp
    Clang/EditedSource.cpp
    Clang/FileManager.cpp
    Clang/FileSystemStatCache.cpp
    Clang/HeaderMap.cpp
    Clang/HeaderSearch.cpp
    Clang/IdentifierTable.cpp
    Clang/InitHeaderSearch.cpp
    Clang/Lexer.cpp
    Clang/LiteralSupport.cpp
    Clang/MacroArgs.cpp
    Clang/MacroInfo.cpp
    Clang/PPCaching.cpp
    Clang/PPDirectives.cpp
    Clang/PPExpressions.cpp
    Clang/PPLexerChange.cpp
    Clang/PPMacroExpansion.cpp
    Clang/Preprocessor.cpp
    Clang/PreprocessorLexer.cpp
    Clang/ScratchBuffer.cpp
    Clang/SourceLocation.cpp
    Clang/SourceManager.cpp
    Clang/TextDiagnostic.cpp
    Clang/TextDiagnosticPrinter.cpp
    Clang/TokenConcatenation.cpp
    Clang/TokenKinds.cpp
    Clang/TokenLexer.cpp
    AST/AST.cpp
    AST/Decl.cpp
    AST/Expr.cpp
    AST/Module.cpp
    AST/Stmt.cpp
    AST/Type.cpp
    AST/Attr.cpp
    AST/ASTContext.cpp
    AST/Component.cpp
    Parser/Parser.cpp
    Parser/ParserHelpers.cpp
    Parser/ASTBuilder.cpp
    Analyser/AnalyserUtils.cpp
    Analyser/Scope.cpp
    Analyser/TypeResolver.cpp
    Analyser/LiteralAnalyser.cpp
    Analyser/FileAnalyser.cpp
    Analyser/FunctionAnalyser.cpp
    Analyser/TypeFinder.cpp
    Analyser/ExprAnalyser.cpp
    Analyser/ModuleAnalyser.cpp
    Analyser/ComponentAnalyser.cpp
    Algo/DepVisitor.cpp
    Algo/DepGenerator.cpp
    Algo/ASTVisitor.cpp
    Algo/TagWriter.cpp
    IRGenerator/CodeGenFunction.cpp
    IRGenerator/CodeGenModule.cpp
    IRGenerator/InterfaceGenerator.cpp
    CGenerator/CCodeGenerator.cpp
    CGenerator/MakefileGenerator.cpp
    CGenerator/TypeSorter.cpp
    CGenerator/DepSorter.cpp
    CGenerator/CGenerator.cpp
    Refactor/RefFinder.cpp
    Builder/C2ModuleLoader.cpp
    Builder/ManifestReader.cpp
    Builder/ManifestWriter.cpp
    Builder/BuildFileReader.cpp
    Builder/LibraryLoader.cpp
    Builder/RootFinder.cpp
    Builder/Recipe.cpp
    Builder/RecipeReader.cpp
    Builder/C2Builder.cpp
    FileUtils/FileMap.cpp
    FileUtils/FileUtils.cpp
    FileUtils/TomlReader.cpp
    Utils/StringBuilder.cpp
    Utils/Utils.cpp
    Utils/GenUtils.cpp
    Utils/BuildFile.cpp
    Utils/TargetInfo.cpp
    Utils/ProcessUtils.cpp
)
set_target_properties(c2core PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")

add_executable(c2c
    main.cpp
)
target_link_libraries(c2c c2core ${LLVM_LDFLAGS} ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS})


if(APPLE OR CYGWIN)
    target_link_libraries(c2c ncurses)
else()
# on Ubuntu,  tinfo seems to needed from Feb'15 on Ubuntu 14.04..
    target_link_libraries(c2c tinfo)
endif()

#set_target_properties(c2c PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS} -Wno-covered-switch-default")
set_target_properties(c2c PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")

install(TARGETS c2c DESTINATION ${DESTDIR}/)

