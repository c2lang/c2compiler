# C2Compiler

Please see [C2Lang.org](http://c2lang.org) for more info about C2!

The C2 project attempts to create a new language, strongly based on C.
In a nutshell, the main differences with C are:
* no more header files (too much typing)
* no includes
* packages (needed if we can't have includes)
* compiled per target (not per file)
* more logical keywords (public/local replaces static)

Below are the instructions for building and using the C2Compiler.

Have fun! (and code..)


## What needs to be done
A short list of open items (the full list would probably fill-up GitHub) with
their priority:
* [high] c2c: parse full syntax into AST
* [high] Makefile: fixed header dependencies and more .o files to .objs dir
* [high] c2c: add proper analysis for AST
* [high] c2c: unit test system
* [high] c2c: generate IR code for more AST elements
* [medium] c2c: improve C generation by generating forward declarations in C code.
* [medium] c2c: tab completion on targets in recipe / cmdline args
* [medium] tool: c2grep - grep only files in recipe
* [medium] tool: create c-parser for parsing C headers.
* [medium] tool: create c2style - astyle for C2.
* [low] c2tags/c2scope - only add files in recipe


## Installation of LLVM/Clang
C2 is based on LLVM 3.2 and some parts of Clang 3.2.
To install C2, follow the steps below. The example shows
how to install in **$HOME/llvm-c2**, but any other dir should work.

* download LLVM sources (http://llvm.org/releases/3.2/llvm-3.2.src.tar.gz)
* download Compiler RT sources (http://llvm.org/releases/3.2/compiler-rt-3.2.src.tar.gz)

To build:
```
tar -xf <path>/llvm-3.2.src.tar.gz
mv llvm-3.2.src llvm
cd llvm/projects
tar -xf <path>/compiler-rt-3.2.src.tar.gz
mv compiler-rt-3.2.src compiler-rt
cd ../tools
git clone git://github.com/c2lang/clang.git
cd clang
git co -b c2master origin/c2master
cd ../../..
mkdir llvm_build
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/
make -j4
make install
```

Voila! When adding **$HOME/llvm-c2/bin** to the your $PATH, you should be able
to build C2.
```
export PATH=$HOME/llvm-c2/bin:$PATH
```

## Getting and Building C2C
To build C2: (llvm-c2/bin must be in PATH)
```
git clone git://github.com/c2lang/c2compiler.git
cd c2compiler/c2c
make -j4
```
If all goes well, the **c2c** executable should appear.

If you get an error with some Clang/C2 errors, try updating your clang C2 archive.


## Using the C2 compiler
Not all examples (in the examples/ dir) work yet. Some still hit an assert.
Some examples that work better than others:
```
./c2c multi
./c2c hello
./c2c iter
./c2c switch
./c2c working_ir
```

It's also possible to manually compile a single .c2 file without a recipe
file with:
```
./c2c -single <file.c2>
```


