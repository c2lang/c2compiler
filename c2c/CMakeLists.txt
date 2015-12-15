CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(c2compiler)

# SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_COMPILER "clang++")

#SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_VERBOSE_MAKEFILE true)
ADD_DEFINITIONS(-Wall)
#ADD_DEFINITIONS(-Wall -Wextra -Wno-overloaded-virtual -Wno-unused -Wno-unused-parameter)

SET(LLVM_CONFIG_EXECUTABLE "llvm-config")

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
#set(LLVM_CXXFLAGS "${LLVM_CXXFLAGS} -fno-exceptions -fno-rtti")

execute_process(
#  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs bitreader bitwriter instrumentation ipo irreader linker mc mcparser objcarcopts option scalaropts transformutils
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
  OUTPUT_VARIABLE LLVM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

SET(CLANG_LIBS "-lclangFrontend -lclangEdit -lclangLex -lclangBasic -lclangRewrite")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
#line below is needed for Ubuntu 13.04 32 bit
INCLUDE_DIRECTORIES("/usr/include/i386-linux-gnu/c++/4.8/")

ADD_LIBRARY(c2core
    AST/AST.cpp
    AST/Decl.cpp
    AST/Expr.cpp
    AST/Module.cpp
    AST/Stmt.cpp
    AST/Type.cpp
    AST/Attr.cpp
    AST/Component.cpp
    Parser/C2Parser.cpp
    Parser/ParserHelpers.cpp
    Parser/C2Sema.cpp
    Analyser/AnalyserUtils.cpp
    Analyser/Scope.cpp
    Analyser/TypeResolver.cpp
    Analyser/LiteralAnalyser.cpp
    Analyser/FileAnalyser.cpp
    Analyser/FunctionAnalyser.cpp
    Analyser/TypeFinder.cpp
    Analyser/ExprTypeAnalyser.cpp
    Analyser/TargetAnalyser.cpp
    Algo/DepVisitor.cpp
    Algo/DepGenerator.cpp
    Algo/ASTVisitor.cpp
    Algo/TagWriter.cpp
    CodeGen/CodeGenFunction.cpp
    CodeGen/CodeGenModule.cpp
    CGenerator/CCodeGenerator.cpp
    CGenerator/MakefileGenerator.cpp
    CGenerator/TypeSorter.cpp
    CGenerator/CGenerator.cpp
    Refactor/RefFinder.cpp
    Builder/C2ModuleLoader.cpp
    Builder/ManifestReader.cpp
    Builder/LibraryLoader.cpp
    Builder/RootFinder.cpp
    Builder/Recipe.cpp
    Builder/RecipeReader.cpp
    Builder/C2Builder.cpp
    FileUtils/FileMap.cpp
    FileUtils/FileUtils.cpp
    Utils/StringBuilder.cpp
    Utils/Utils.cpp
    Utils/GenUtils.cpp
    Utils/ProcessUtils.cpp
)
SET_TARGET_PROPERTIES(c2core PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")

ADD_EXECUTABLE(c2c
    main.cpp
)
TARGET_LINK_LIBRARIES(c2c c2core ${LLVM_LDFLAGS} ${CLANG_LIBS} ${LLVM_LIBS} dl pthread)


if (APPLE OR CYGWIN)
    TARGET_LINK_LIBRARIES(c2c ncurses)
else()
# on Ubuntu,  tinfo seems to needed from Feb'15 on Ubuntu 14.04..
    TARGET_LINK_LIBRARIES(c2c tinfo)
endif()

#SET_TARGET_PROPERTIES(c2c PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS} -Wno-covered-switch-default")
SET_TARGET_PROPERTIES(c2c PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")

ADD_CUSTOM_TARGET(tests COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../tools/tester/tester test DEPENDS c2c WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

