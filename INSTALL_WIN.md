
## Installation of LLVM/Clang (C2 version)
C2 is based on LLVM 4.0 and some parts of Clang 4.0.
To install C2 on Windows, follow the steps below. The example shows
how to install with Cygwin in **$HOME/llvm-c2**, but any other dir should work.

First, you will need Cygwin installation. You can get it [here](https://cygwin.com/install.html).
During the installation, select the following packages along with all that are selected
by default:

* OCaml
* Ruby
* Python
* make
* cmake
* g++
* git
* ncurses (get both libncurses and ncurses)
* nano (if you don't have any other text editor supporting LF line endings)

After the installation is finished, start Cygwin bash, where you will enter following commands.
As this installation is counting with installation into $HOME/llvm-c2, you can start by creating
the folder and navigating to it:
```bash
cd $HOME
mkdir llvm-c2
cd llvm-c2
```

NOTE: UNTESTED for 4.0 yet

Now, to build LLVM:
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

export CC=gcc
export CXX=g++
../llvm/configure --enable-optimized --prefix=$HOME/llvm-c2/ --with-python=/usr/bin/python2

make -j4 #can be raised a bit on faster computers
make install
```
NOTE:
On some devices a bug in LLVM's make install command may occur and the following files are not put in correct directories:
* DiagnosticCommonKinds.inc
* DiagnosticSemaKinds.inc
* DiagnosticParseKinds.inc
It appears to be a recurring bug which is not 100% reproduceable. The solution is to simply copy
said files from 
```bash
llvm_install_dir/llvm_build/tools/clang/include/clang/Basic/
``` 
to
```bash
llvm_install_dir/include/clang/Basic
```


Voila! When adding **$HOME/llvm-c2/bin** to the your $PATH, you should be able
to build C2. Additionally, you need to add the PATH to c2tags and point the
environment variable C2_LIBDIR to point at the directory containing the 'c2libs'
directory. If you place c2tags in $HOME/bin, the following works:
```bash
export PATH=$HOME/bin:$HOME/llvm-c2/bin:$PATH
export C2_LIBDIR=$HOME/c2compiler/c2c/
```

## Building of C2C
Note: In order for this to succeed, $PATH needs to contain $HOME/llvm-c2/bin

First, start by cloning the repo and navigating to the folder
```bash
git clone git://github.com/c2lang/c2compiler.git
cd c2compiler/
```bash
Now open the CMakeLists.txt file that is found there (you can use any editor that supports LF endings,
this shows nano)
```bash
nano CMakeLists.txt
```
and change this line:
```bash
SET(CMAKE_CXX_COMPILER "clang++")
```
to this
```bash
SET(CMAKE_CXX_COMPILER "g++")
```
Save and exit nano with Ctrl+O and Ctrl+X.

Then the compilation can be finished with
```bash
mkdir build
cd build
cmake . ..
make -j4
```
If all goes well, the **c2c** executable should appear in the build directory.

If you get an error with some Clang/C2 errors, try updating your clang C2 archive.

