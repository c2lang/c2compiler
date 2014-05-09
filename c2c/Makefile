all: c2c
# using clang causes segfault somewhere. Bug in clang since g++ works fine
CXX := clang++
#CXX := g++

OBJS = \
    AST/AST.o \
    AST/Decl.o \
    AST/Expr.o \
    AST/Package.o \
    AST/Stmt.o \
    AST/Type.o \
    Parser/C2Parser.o \
    Parser/C2Sema.o \
    Analyser/AnalyserUtils.o \
    Analyser/Scope.o \
    Analyser/TypeResolver.o \
    Analyser/LiteralAnalyser.o \
    Analyser/FileAnalyser.o \
    Analyser/FunctionAnalyser.o \
    Analyser/TypeFinder.o \
    Analyser/ExprTypeAnalyser.o \
    CodeGen/CodeGenFunction.o \
    CodeGen/CodeGenModule.o \
    CGenerator/CCodeGenerator.o \
    Builder/C2Builder.o \
    Builder/Recipe.o \
    Builder/DepVisitor.o \
    Builder/DepGenerator.o \
    Builder/RecipeReader.o \
    FileUtils/FileMap.o \
    FileUtils/FileUtils.o \
    Utils/StringBuilder.o \
    Utils/Utils.o \
    Utils/GenUtils.o \
    main.o

#CPPFLAGS = `llvm-config --cxxflags` -Wno-overloaded-virtual -Wall -Wextra -Wno-unused -Wno-unused-parameter
CPPFLAGS = `llvm-config --cxxflags` -Wno-overloaded-virtual -Wall
LDFLAGS = `llvm-config --ldflags`
LIBS = `llvm-config --libs`

CLANGLIBS = \
				-lclangFrontend\
				-lclangEdit\
				-lclangLex\
				-lclangBasic

.PHONY : clean
clean:
	@rm -rf c2c $(OBJS)

%.o: %.cpp
	@echo building $@
	@$(CXX) -c $(CPPFLAGS) -o $@ $< -I .
#line below is for compiling without asserts
#@$(CXX) -DNDEBUG -c $(CPPFLAGS) -o $@ $< -I .
#line below is needed for Ubuntu 13.04 32 bit
#@$(CXX) -c $(CPPFLAGS) -I /usr/include/i386-linux-gnu/c++/4.8/ -o $@ $< -I .

c2c: $(OBJS)
	@echo linking $@
	@$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(CLANGLIBS) $(LIBS) -ldl -fno-rtti -lrt

.PHONY : test
test:
	@echo running tests
	@../tools/tester/tester test

