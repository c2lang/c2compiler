
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


## Bootstrap

Since *c2c* is written in C2, a bootstrap is needed. Please run

```bash
make -C bootstrap
./bootstrap/c2c_bootstrap -c
```

This will create a boostrap c2c version and use it to build the c2c compiler.
The output will be in the folder c2_output/

