# C2Compiler

Please see [C2Lang.org](http://c2lang.org) for more info about C2!

NOTE:
This archive will host the c2 compiler after the Fosdem '13 presentation!
Please check back then..

**TODO**
* Clang
* LLVM

## Installation
C2 is based on LLVM 3.2 and some parts of Clang 3.2.
To install C2, you first need to build llvm+clang normally, then
patch clang with the clang-c2.patch and rebuild it.
Installation below shows how to install in t$HOME/llvm-c2, but any
other dir should work.
```
download LLVM sources (http://llvm.org/releases/3.2/llvm-3.2.src.tar.gz)
download Clang sources (http://llvm.org/releases/3.2/clang-3.2.src.tar.gz)
download Compiler RT sources (http://llvm.org/releases/3.2/compiler-rt-3.2.src.tar.gz)
tar -xf llvm-3.2.src.tar.gz
mv llvm-3.2.src llvm
cd llvm/tools
tar -xf ../../clang-3.2.src.tar.gz
mv clang-3.2.src clang
cd ../projects
tar -xf compiler-rt-3.2.src.tar.gz
mv compiler-rt-3.2.src compiler-rt
cd ../..
mkdir llvm_build
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/
make
make install
```
Now patch and re-build
```
cd ../llvm/tools/clang
patch -p0 < clang-c2.patch
cd ../../../llvm_build/tools/clang
make
make install
```
Voila! When adding $HOME/llvm-c2/bin to the your $PATH, you should be able
to build C2.


