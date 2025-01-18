## Installation of C2C

### Requirements

To bootstrap/install C2C the following tools are needed:

* gcc or clang
* make (gmake on FreeBSD/OpenBSD)

### Basic installation

The fastest way to just get c2c up and running is just following the
next steps. The explanation of what that does is below. This installs
everything in a code/ subdirectory. Once you understand what is happening,
you can deviate.

Make sure ~/bin is in your $PATH

```bash
cd
mkdir ~/bin
ln -s ~/c2compiler/output/c2c/c2c ~/bin/c2c
ln -s ~/c2compiler/output/c2tags/c2tags ~/bin/c2tags
ln -s ~/c2compiler/libs ~/c2_libs
mkdir code
git clone --recurse-submodules git@github.com:c2lang/c2compiler.git
cd c2compiler
. ./env.sh
make -C bootstrap
./install_plugins.sh
./output/tester/tester test
```

### Explanation

To be usable, *c2c* needs to be in your $PATH. This is done above by adding it to ~/bin. This
directory should be in your $PATH, otherwise *c2c* and *c2tags* cannot be found.

c2c uses the following _enviroment variables_:

* _C2_LIBDIR_ - used to find libraries for compilation. Currently a subdir 'libs' of c2compiler/
* _C2_PLUGINDIR_ - used to find plugins for the compiler

The easiest way to set these is to source *env.sh* in the c2compiler archive top-level directory.

The _install_plugins.sh_ script simply copies the compiled plugins to ~/c2_plugins.


### Bootstrap description

the _bootstrap/_ directory contain the C sources of the bootstrap C2 compiler. This must be
used because c2c itself is written in C2.

