This guide is for compiling C2C on Windows with the Cygwin compatibility layer.
The Cygwin installer can be obtained [here](https://cygwin.com/install.html).
You will need an installation with at least the base packages along with:
* make
* cmake
* gcc
* g++
* libncurses-dev
* ncurses
* Python
* git
* clang

Once your Cygwin is ready, start its shell where you will enter the commands
written below.

## Installation of LLVM/Clang (C2 version)
C2 is based on LLVM 10.0 and some parts of a modified Clang 10.0,
so we will need to build it first.

As this installation is counting with installation into **$HOME/llvm-10**, you can start by creating
the folder and navigating to it:

```bash
cd $HOME
mkdir llvm-10
cd llvm-10
```

The git provided with Cygwin automatically changes LF line-endings (standard on Unix-based systems)
to CRLF line-endings (standard on Windows). This however breaks some of LLVM's scripts, because the
CR character is recognized as a part of words at the end of the line or as a syntax error. Therefore
before you clone the following repositories, we need to disable that with:

```bash
git config --global core.autocrlf false
```

If you want, you can re-enable it once you are finished with the installation.

For instructions on how to build LLVM, see INSTALL.md.

## Building of C2C

First, start by cloning the repo and navigating to the folder
```bash
git clone git://github.com/c2lang/c2compiler.git
cd c2compiler/
```

In order to be able to build C2C, you need to have some enviroment variables set, namely
C2_LIBDIR and PATH. There is a script called env.sh in the folder which can do it
for you automatically. This script needs to be executed like this:
```bash
. env.sh # mind the dot
```
or this:
```bash
source env.sh
```

This is because otherwise the script would be ran in a subshell and it wouldn't have been
able to change, albeit temporarily, the environment variables. Alternatively, if you don't use
a bash-compatible shell or if you installed LLVM into a different directory than **$HOME/llvm-10**
you can do it by hand:
```bash
alias c2c=$(pwd)/build/c2c/c2c
export C2_LIBDIR=$(pwd)/c2libs
export PATH=$PATH:/path/to/llvm/installation/bin
```

Then you should be able to compile C2C with
```bash
mkdir build
cd build
cmake . ..
make -j4
```
If all goes well, the **c2c** executable should appear in the build directory and be ready for use.

If you get an error with some Clang/C2 errors, try updating your clang C2 archive. If you have problems
compiling LLVM, make sure that you have everything in the correct directory and the aforementioned packages
installed.
