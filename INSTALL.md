## Installation of LLVM/Clang (C2 version)
The latest C2 version is based on LLVM 11.0 (but older versions
have been based on version 3.3, 4, 5, 6, 7, 8 and 9).
To install C2, follow the steps below. The example shows
how to install in **$HOME/llvm-11**, but any other dir should work.

Note that all OS-specific instructions include the Generic section

### Ubuntu 14.04
LLVM 11.0 needs cmake 3.4.3 or higher, since Ubuntu does not have
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

### Ubuntu 16.04 / 18.04 / 20.04
```bash
sudo apt-get install clang cmake ncurses-dev
export CC=clang
export CXX=clang++
```

### Ubuntu 20.10
This version already has clang/llvm 11 via apt tooling, so it's possible
to skip building LLVM yourself and use the system packages. The exact steps
how to do this have not yet been documented.


### Arch
```bash
pacman -S cmake ncurses-dev clang
```

### OS X (Yosemite/El Capitan)
Homebrew seems to have updated to a sufficient cmake version, so installation is easy:
```bash
brew install cmake
```

### Generic
These commands build the C2-modified LLVM/Clang toolchain. The produced clang is still
fully compatible with the original one (ie. it can compile C/C++ just as well).

There is a convenience script: *clang\_build.sh* that runs the commands below.

NOTE: C2 now uses the new LLVM mono-repository, not the separate repos!

```bash
https://github.com/c2lang/llvm-project.git
cd llvm-project/
git checkout -b c2master llvmorg-11.0.0
cd ..
mkdir llvm_build
cd llvm_build

cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DCMAKE_INSTALL_PREFIX=$HOME/llvm-11 \
    -DLLVM_ENABLE_PEDANTIC=OFF \
    ../llvm-project/llvm

make -j16
make install
```

Or if you want to build with Ninja replace 'Unix makefiles' with 'Ninja'


### OS X (Yosemite/El Capitan) (after building LLVM/Clang)
You might need to create a link for your new clang to find the C++ headers.
Since this uses the toolchain that comes with XCode, XCode will have to be installed.
```
mkdir -p ~/llvm-11/include/c++
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 ~/llvm-11/include/c++/v1
```

## Downloading C2C
```bash
git clone git://github.com/c2lang/c2compiler.git
```

## Building C2C
```bash
cd c2compiler
source ./env.sh  (this assumes LLVM is installed to $HOME/llvm-11)
mkdir build
cd build
cmake . ..
make -j4
```

alternatively, if you want to you Ninja build, replace last two commands with:
```bash
cmake -G "Ninja" . ..
ninja
```

If all goes well, the **c2c** executable should appear in the build/c2c/ directory.

If you get an error with some Clang/C2-related errors, try updating your clang C2 archive.

The env.sh script sets some some environment variables that c2c requires to work
like *C2_LIBDIR*. It will also add $HOME/llvm-11/bin to the $PATH


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

