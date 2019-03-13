#!/bin/bash

find c2c -name '*.h' | grep -v Clang | xargs clang-format -i
find c2c -name '*.cpp' | grep -v Clang | xargs clang-format -i

