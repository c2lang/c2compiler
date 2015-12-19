
## Installation of LLVM/Clang (C2 version)
C2 is based on LLVM 3.7 and some parts of Clang 3.7.
To install C2, follow the steps below. The example shows
how to install in **$HOME/llvm-c2**, but any other dir should work.

To build:
```
git clone git://github.com/llvm-mirror/llvm.git
cd llvm/
git checkout -b release_37 origin/release_37
cd projects
git clone git://github.com/llvm-mirror/compiler-rt.git
cd compiler-rt
git checkout -b release_37 origin/release_37
cd ../../tools
git clone git://github.com/c2lang/clang.git
cd clang
git checkout -b c2master_37 origin/c2master_37
cd ../../..
mkdir llvm_build
cd llvm_build

export CC=gcc (optional)
export CXX=g++ (optional)
# on linux:
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/ --with-python=/usr/bin/python2
# on OS X:
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/
# alternatively, you can use cmake (on OS X you should)
cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DCMAKE_INSTALL_PREFIX=$HOME/llvm-c2 \
    -DLLVM_ENABLE_PEDANTIC=OFF \
    ../llvm

make -j4
make install
```

NOTE:
On Mac OS X (Yosemite/El Capitan) you might need to create a link for your new clang to find the C++ headers.
Since this uses the toolchain that comes with XCode, XCode will have to be installed.
```
mkdir -p ~/llvm-c2/include/c++
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 ~/llvm-c2/include/c++/v1
```

Voila! When adding **$HOME/llvm-c2/bin** to the your $PATH, you should be able
to build C2. Additionally, you need to add the PATH to c2tags and point the
environment variable C2_LIBDIR to point at the directory containing the 'c2libs'
directory. If you place c2tags in $HOME/bin, the following works:
```
export PATH=$HOME/bin:$HOME/llvm-c2/bin:$PATH
export C2_LIBDIR=$HOME/path/to/c2c/c2libs/
```

## Building of C2C
To build C2: (llvm-c2/bin must be in $PATH)
```
git clone git://github.com/c2lang/c2compiler.git
cd c2compiler/c2c
mkdir build
cd build
cmake . ..
make -j4
```
If all goes well, the **c2c** executable should appear in the build directory.

If you get an error with some Clang/C2 errors, try updating your clang C2 archive.

## Getting and Building C2C
To run the unit tests, first build the **tester** tool:
```
cd tools/tester
make
cd ../../c2c/build
make tests
```

## Installing c2tags
```
cd tools/c2tags
make
```
To use **c2tags**, it must be in the $PATH. The easiest is creating a $HOME/bin directory,
copying **c2tags** there. Then add a line to .bashrc (or something similar):
```
export PATH=$PATH:~/bin
cp c2tags ~/bin
```
Additionally, copy the contents of *tools/c2tags/fragment.vim* to your ~/.vimrc.


