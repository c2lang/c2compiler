an
## Installation of LLVM/Clang (C2 version)
C2 is based on LLVM 4.0 and some parts of Clang 4.0.
To install C2, follow the steps below. The example shows
how to install in **$HOME/llvm-c2**, but any other dir should work.

Note that both the Ubuntu/OS X guides *also* use the Generic part

### Ubuntu 14.04
LLVM/Clang 4.0 needs cmake 3.4.3 or higher, since Ubuntu does not have
this natively, just install it from a binary package:
(installed here in $HOME/progs)
```bash
wget https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.tar.gz
tar -xf cmake-3.5.2-Linux-x86_64.tar.gz -C ~/progs
export PATH=~/progs/cmake-3.5.2-Linux-x86_64/bin:$PATH

sudo apt-get install clang-3.5 cmake ncurses-dev
export CC=clang-3.5
export CXX=clang++-3.5
```

### Ubuntu 16.04
```bash
sudo apt-get install clang cmake ncurses-dev
export CC=clang
export CXX=clang++
```

### OS X (Yosemite/El Capitan)
Homebrew is currently at cmake-3.3, so a binary package for cmake is also needed on OS X
```bash
wget https://cmake.org/files/v3.5/cmake-3.5.2-Darwin-x86_64.tar.gz
tar -xf cmake-3.5.2-Darwin-x86_64.tar.gz ~/progs
export PATH=~/progs/cmake-3.5.2-Darwin-x86_64/CMake.app/Contents/bin:$PATH
```

### Generic
These commands build the C2-modified LLVM/Clang toolchain. The produced clang is still
fully compatibly with the original one (ie. it can just compile C/C++ just as well).

```bash
git clone git://github.com/llvm-mirror/llvm.git
cd llvm/
git checkout -b release_40 origin/release_40
cd projects
git clone git://github.com/llvm-mirror/compiler-rt.git
cd compiler-rt
git checkout -b release_40 origin/release_40
cd ../../tools
git clone git://github.com/c2lang/clang.git
cd clang
git checkout -b c2master_40 origin/c2master_40
cd ../../..

mkdir llvm_build
cd llvm_build

cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DCMAKE_INSTALL_PREFIX=$HOME/llvm-c2 \
    -DLLVM_ENABLE_PEDANTIC=OFF \
    ../llvm

make -j4
make install
```

### OS X (Yosemite/El Capitan) (after building LLVM/Clang)
You might need to create a link for your new clang to find the C++ headers.
Since this uses the toolchain that comes with XCode, XCode will have to be installed.
```
mkdir -p ~/llvm-c2/include/c++
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 ~/llvm-c2/include/c++/v1
```

## Downloading C2C
```bash
git clone git://github.com/c2lang/c2compiler.git
```

## Building C2C
```bash
cd c2compiler
source ./env.sh  (this assumes LLvm/Clang are installed to $HOME/llvm-c2)
mkdir build
cd build
cmake . ..
make -j4
```
If all goes well, the **c2c** executable should appear in the build/c2c/ directory.

If you get an error with some Clang/C2-related errors, try updating your clang C2 archive.

The env.sh script sets some some environment variables that c2c requires to work,
like *C2_LIBDIR*. It will also add $HOME/llvm-c2/bin to the $PATH


## Running the tests
To run the unit tests (from build/)
```bash
make tests
```

or from the base dir:
```bash
./build/tools/tester test
./build/tools/tester test/Functions
```

## Installing c2tags
c2tags is C2's equivalent of ctags; it allows *jumping to definition* inside c2 source files.

To use **c2tags**, it must be in the $PATH. The easiest is creating a $HOME/bin directory,
installing **c2tags** there and adding in to the $PATH:
```bash
make install DESTDIR=$HOME/bin
export PATH=$PATH:~/bin
```

Additionally, copy the contents of *tools/c2tags/fragment.vim* to your ~/.vimrc.

After building your C2 project, you can jump to a definition by moving the cursor anywhere
on a reference and pressing *Ctrl-h*.

