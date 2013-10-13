all: c2c
# using clang causes segfault somewhere. Bug in clang since g++ works fine
CXX := clang++
#CXX := g++

OBJS = \
    Analyser/FileAnalyser.o \
    Analyser/FunctionAnalyser.o \
    Analyser/Scope.o \
    Analyser/AnalyserUtils.o \
    AST/AST.o \
    AST/Decl.o \
    AST/Expr.o \
    AST/Package.o \
    AST/Stmt.o \
    AST/Type.o \
    Builder/C2Builder.o \
    Builder/Recipe.o \
    Builder/RecipeReader.o \
    FileUtils/FileMap.o \
    FileUtils/FileUtils.o \
    Generators/CCodeGenerator.o \
    Generators/CodeGenFunction.o \
    Generators/CodeGenModule.o \
    Generators/GenUtils.o \
    Parser/C2Parser.o \
    Parser/C2Sema.o \
    Utils/StringBuilder.o \
    Utils/Utils.o \
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

c2c: $(OBJS)
	@echo linking $@
	@$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(CLANGLIBS) $(LIBS) -ldl -fno-rtti -lrt
	@strip $@

.PHONY : test
test:
	@echo running tests
	@../tools/tester/tester test

