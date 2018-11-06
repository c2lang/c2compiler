#!/bin/sh

set -e

git clone git://github.com/c2lang/llvm.git
cd llvm/
git checkout -b release_70 origin/release_70
cd ..

mkdir llvm_build
cd llvm_build

cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DCMAKE_INSTALL_PREFIX=$HOME/llvm-c2 \
    -DLLVM_ENABLE_PEDANTIC=OFF \
    ../llvm

make -j4
make install

