
## Installation of LLVM/Clang (C2 version)
C2 is based on LLVM 3.7 and some parts of Clang 3.7.
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
```
cd $HOME
mkdir llvm-c2
cd llvm-c2
```

Now, to build LLVM:
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
said files from ```llvm_install_dir/llvm_build/tools/clang/include/clang/Basic/``` to
```llvm_install_dir/include/clang/Basic```


Voila! When adding **$HOME/llvm-c2/bin** to the your $PATH, you should be able
to build C2. Additionally, you need to add the PATH to c2tags and point the
environment variable C2_LIBDIR to point at the directory containing the 'c2libs'
directory. If you place c2tags in $HOME/bin, the following works:
```
export PATH=$HOME/bin:$HOME/llvm-c2/bin:$PATH
export C2_LIBDIR=$HOME/c2compiler/c2c/
```

## Building of C2C
Note: In order for this to succeed, $PATH needs to contain $HOME/llvm-c2/bin

First, start by cloning the repo and navigating to the folder
```
git clone git://github.com/c2lang/c2compiler.git
cd c2compiler/c2c
```
Now open the CMakeLists.txt file that is found there (you can use any editor that supports LF endings,
this shows nano)
```
nano CMakeLists.txt
``` 
and change these two lines:
``` 
# SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_COMPILER "clang++")
``` 
to this
``` 
SET(CMAKE_CXX_COMPILER "g++")
#SET(CMAKE_CXX_COMPILER "clang++")
```
Save and exit nano with Ctrl+O and Ctrl+X.

Then the compilation can be finished with
```  
mkdir build
cd build
cmake . ..
make -j4
```
If all goes well, the **c2c** executable should appear in the build directory.

If you get an error with some Clang/C2 errors, try updating your clang C2 archive.

## Testing C2C
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


