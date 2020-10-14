echo 'setting C2 environment'
alias c2c=$PWD/build/c2c/c2c
export C2_LIBDIR=$PWD/c2libs
export CC=clang
export CXX=clang++
export PATH=$HOME/llvm-10/bin:$PATH
