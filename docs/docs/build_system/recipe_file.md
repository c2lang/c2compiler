## Recipe file

A C2 project requires a `recipe` file (named *recipe.txt*). This file specifies:

* the target type (executable, dynamic library or static library)
* which options are used (compile time flags)
* which source files are used
* external dependencies

The recipe is written by the developers, but for big projects, the file could
be generated (for example by `make menuconfig`), especially if there are many
different configurations.

The exact format of the recipe file is still in progress, but it currently looks like:

```ini
# an example recipe file

config ENABLE_DEBUG
config MAX_BUFFERS 500

plugin deps_generator [all-targets private]
plugin refs_generator [all-targets]
plugin git_version [git options go here]

executable hello
  $warnings no-unused
  sources/hello.c2
end

lib graphics static
  $plugin load_file [load_file.yml]
  $export api
  sources/file1.c2
  file2.c2
  api/file3.c2
end
```
Note that the path to each c2 file is relative to the position of the __recipe file__, not
to that of the c2c binary.

It's possible to let *c2c* show the latest recipe options by: *c2c --help-recipe*

### Global options

Before all executable/lib targets, global configs may be specified. These have
the same syntax as config inside a target (see below), but are applied to all
targets.


### Target options

Inside a target, the following options are available:

 * *$asm [file]* - include ASM file
 * *$backend c* - enable generation of C code
 * *$backend ir* - enable generation of IR (Intermediate Representation) code
 * *$config [name]* <value> - specify definitions for the preprocessor, like #define feature1
 * *$disable-asserts* - disables generation of asserts
 * *$disabled* - disable target
 * *$export [modules]* - export the modules in libraries and generated headers
 * *$nolibc* - dont use libc
 * *$optional* - dont build normally, only when explicitly mentioned or using 'c2c --all'
 * *$plugin* - load a target-specific plugin, or override with target-specific config
 * *$use [libs]* - depend on given libraries
 * *$warnings [list]* - enable/disable specific warnings during compilation


#### Warning options
Different warnings can be disabled by one of more options after _$warnings_:

 * *no-unreachable-code* - silence warnings about unreachable code
 * *no-unused-attribute* - silence warnings about unused attributes
 * *no-unused-deprecated* - silence deprecated warnings
 * *no-unused-enum-assoc-value* - silence warnings about unused enum-associated values
 * *no-unused-enum-constants* - silence warnings about unused enum constants
 * *no-unused-function* - silence warnings about unused functions
 * *no-unused-import* - silence warnings about unused imports
 * *no-unused-label* - silence warnings about unused labels
 * *no-unused-module* - silence warnings about unused modules
 * *no-unused-parameter* - silence warnings about unused parameters
 * *no-unused-public* - silence warnings about unused public keywords
 * *no-unused* - silence all unused warnings
 * *no-unused-struct-member* - silence warnings about unused struct members
 * *no-unused-type* - silence warnings about unused types
 * *no-unused-variable* - silence warnings about unused variables
 * *promote-to-error* - promote  warnings to errors

#### Sets
When using the same group of files in multiple targets, it's possble to use *sets* to avoid
very long recipe files.

```ini
set common
    file1.c2
    file2.c2
end

executable test
    (common)
    main.c2
end
```


