all: c2c
# using clang causes segfault somewhere. Bug in clang since g++ works fine
CXX := clang++
#CXX := g++

OBJS = \
    AST.o \
    C2Builder.o \
    C2Parser.o \
    C2Sema.o \
    CCodeGenerator.o \
    CodeGenFunction.o \
    CodeGenModule.o \
    Decl.o \
    Expr.o \
    FileAnalyser.o \
    FileMap.o \
    FileUtils.o \
    FunctionAnalyser.o \
    Package.o \
    Recipe.o \
    RecipeReader.o \
    Scope.o \
    Stmt.o \
    StringBuilder.o \
    Type.o \
    Utils.o \
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
	@$(CXX) -c $(CPPFLAGS) -o $@ $<

c2c: $(OBJS)
	@echo linking $@
	@$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(CLANGLIBS) $(LIBS) -ldl -fno-rtti -lrt
	@strip $@

.PHONY : test
test:
	@echo running tests
	@../tools/tester/tester test

