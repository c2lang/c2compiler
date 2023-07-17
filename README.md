
# C2Compiler - Native version

This is the C2 compiler written in C2 itself.

C2 is an evolution of C, please see http://c2lang.org for more info.

The first version was written in C++ and can be found at (https://github.com/c2lang/c2compiler)

This version does not support all the test cases yet, but is catching up fast


## Install Libraries
Please clone the libraries repository in the same parent directory as this repository.

```bash
cd ..
git clone https://github.com/c2lang/c2_libs.git
cd c2c_native
```


## Environment

To find the libraries *c2c* needs an environment variable, please update *env.sh*
to your setting and run

```bash
. ./env.sh
```


## Bootstrap

Since *c2c* is written in C2, a bootstrap is needed. Please run

```bash
make -C bootstrap
```

This will create a boostrap c2c version and use it to build the c2c compiler.
The output will be in the folder output/

Now you can run the regular compiler to compiler (other) C2 projects

```bash
./output/c2c/c2c
```

### Plugins
The C2 compiler has a _plugin_ system, that allows plugins to be loaded during
compilation and the AST to be modified by them.

*c2c* will look for plugins in the environment variable $C2_PLUGINDIR (set by the
env.sh script). Also the path can be set in the _build-file_. To install the plugins
initially, run *./install_plugins.sh*. This will create ~/c2_plugins/ and copy all
plugins there.

To run *c2c* without plugins, use _--noplugins_.




