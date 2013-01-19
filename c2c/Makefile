all: c2c
CXX := clang++

OBJS = \
    C2Builder.o \
    C2Parser.o \
    C2Sema.o \
    CodeGenerator.o \
    Decl.o \
    DeclCodeGen.o \
    Expr.o \
    ExprCodeGen.o \
    FileAnalyser.o \
    FileMap.o \
    Recipe.o \
    RecipeReader.o \
    Stmt.o \
    StmtCodeGen.o \
    StringBuilder.o \
    Type.o \
    Utils.o \
    main.o

#use warning flags now and then. They give a lot of LLVM/CLANG warnings
#CPPFLAGS = `llvm-config --cxxflags` -Wno-overloaded-virtual -Wall -Wextra
CPPFLAGS = `llvm-config --cxxflags` -Wno-overloaded-virtual
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

