
# C2Compiler - Native version

This is the C2 compiler written in C2 itself.

C2 is an evolution of C, please see http://c2lang.org for more info.

The first version was written in C++ and can be found at (https://github.com/c2lang/c2compiler)

This version does not support all the test cases yet, but is catching up fast


## Bootstrap

Since *c2c* is written in C2, a bootstrap is needed. Please run

```bash
make -C bootstrap
./bootstrap/c2c_bootstrap
```

This will create a boostrap c2c version and use it to build the c2c compiler

