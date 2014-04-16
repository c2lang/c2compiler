#!/bin/sh

# script to compile .ll -> a.out using clang tooling
echo "compiling $1"
rm -f a.out /tmp/output.s
llc -march=x86 $1 -o /tmp/output.s
clang /tmp/output.s -o a.out

# to compile to .o
#llc -march=x86 -cppgen=module foo.ll
#clang -c foo.s -o foo.o

