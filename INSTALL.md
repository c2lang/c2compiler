
## Packages for Arch Linux (AUR)
* [llvm-c2](https://aur.archlinux.org/packages/llvm-c2/)
* [c2c-git](https://aur.archlinux.org/packages/c2c-git/)

Howto build directly from git for Arch Linux:
```
$ mkdir llvm-c2
$ cd llvm-c2
$ curl -O https://code.kluisip.nl/pkgbuilds/plain/llvm-c2/PKGBUILD
$ makepkg -is

$ mkdir c2c-git
$ cd c2c-git
$ curl -O https://code.kluisip.nl/pkgbuilds/plain/c2c-git/PKGBUILD
$ makepkg -is
```

* To build from AUR with an AUR helper, for example [packer](https://aur.archlinux.org/packages/packer/):

```$ packer -S c2c-git```



For other Linux distro, a manual build is required. See below.

## Installation of LLVM/Clang (C2 version)
C2 is based on LLVM 3.4 and some parts of Clang 3.4.
To install C2, follow the steps below. The example shows
how to install in **$HOME/llvm-c2**, but any other dir should work.

* download Compiler RT sources (http://llvm.org/releases/3.4/compiler-rt-3.4.src.tar.gz)

To build:
```
git clone git://github.com/llvm-mirror/llvm.git
cd llvm/
git checkout -b release_34 origin/release_34
cd projects
tar -xf <path>/compiler-rt-3.3.src.tar.gz
mv compiler-rt-3.4.src compiler-rt
cd ../tools
git clone git://github.com/c2lang/clang.git
cd clang
git checkout -b c2master_34 origin/c2master_34
cd ../../..
mkdir llvm_build
export CC=gcc (optional)
export CXX=g++ (optional)
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/ --with-python=/usr/bin/python2
make -j4
make install
```

Voila! When adding **$HOME/llvm-c2/bin** to the your $PATH, you should be able
to build C2.
```
export PATH=$HOME/llvm-c2/bin:$PATH
```

## Installation of C2C
To build C2: (llvm-c2/bin must be in PATH)
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
To run the unit tests:
```
cd tools/tester
make
cd ../../c2c/build
make test
```

