# C2Compiler

Please see [C2Lang.org](http://c2lang.org) for more info about C2!

## Installation of LLVM/Clang
C2 is based on LLVM 3.2 and some parts of Clang 3.2.
To install C2, follow the steps below. The example shows
how to install in t$HOME/llvm-c2, but any other dir should work.

* download LLVM sources (http://llvm.org/releases/3.2/llvm-3.2.src.tar.gz)
* download Clang sources (http://llvm.org/releases/3.2/clang-3.2.src.tar.gz)
* download Compiler RT sources (http://llvm.org/releases/3.2/compiler-rt-3.2.src.tar.gz)
```
tar -xf <path>/llvm-3.2.src.tar.gz
mv llvm-3.2.src llvm
cd llvm/projects
tar -xf <path>/compiler-rt-3.2.src.tar.gz
mv compiler-rt-3.2.src compiler-rt
cd ../tools
tar -xf <path>/clang-3.2.src.tar.gz
mv clang-3.2.src clang
cd clang
patch -p1 < <path>/clang-c2.patch
cd ../../..
mkdir llvm_build
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/
make -j4
make install
```
Voila! When adding $HOME/llvm-c2/bin to the your $PATH, you should be able
to build C2.
```
export PATH=$HOME/llvm-c2/bin:$PATH
```


## Building C2
**NOTE** This part will be up real soon..


