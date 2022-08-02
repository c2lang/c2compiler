echo 'setting C2 environment'
alias c2c=$PWD/build/c2c/c2c
export C2_LIBDIR=$PWD/c2libs
export C2_SRCDIR=$PWD/c2c
export C2_PLUGINDIR=$HOME/c2c/plugins
export CC=clang
export CXX=clang++
export PATH=$HOME/llvm-11/bin:$PATH
