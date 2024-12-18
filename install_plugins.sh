#!/bin/sh
if [ $(uname -s) = 'Darwin' ] ; then
   LIB_EXT='.dylib'
else
   LIB_EXT='.so'
fi
set -e
mkdir -p $C2_PLUGINDIR
cp output/deps_generator/libdeps_generator$LIB_EXT $C2_PLUGINDIR
cp output/git_version/libgit_version$LIB_EXT $C2_PLUGINDIR
cp output/load_file/libload_file$LIB_EXT $C2_PLUGINDIR
cp output/refs_generator/librefs_generator$LIB_EXT $C2_PLUGINDIR
cp output/shell_cmd/libshell_cmd$LIB_EXT $C2_PLUGINDIR
cp output/unit_test/libunit_test$LIB_EXT $C2_PLUGINDIR
if [ $(uname -s) != 'Darwin' ] ; then
   strip $C2_PLUGINDIR/*$LIB_EXT
fi
