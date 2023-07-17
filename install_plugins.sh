#!/bin/sh
mkdir -p $C2_PLUGINDIR
cp output/deps_generator/libdeps_generator.so $C2_PLUGINDIR
cp output/git_version/libgit_version.so $C2_PLUGINDIR
cp output/load_file/libload_file.so $C2_PLUGINDIR
cp output/refs_generator/librefs_generator.so $C2_PLUGINDIR
cp output/shell_cmd/libshell_cmd.so $C2_PLUGINDIR
cp output/unit_test/libunit_test.so $C2_PLUGINDIR
