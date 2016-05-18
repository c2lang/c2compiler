# C2Compiler

Please see [C2Lang.org](http://c2lang.org) for more info about C2!

The C2 project attempts to create a new language, strongly based on C.
In a nutshell, the main differences with C are:
* no more header files (too much typing)
* no includes
* packages (needed if we can't have includes)
* compiled per target (not per file)
* more logical keywords (public/local replaces static)
* integrated build system

Below are the instructions for building and using the C2Compiler.

Have fun! (and code..)


## Generic
C2 is based on LLVM 3.8 and some parts of Clang 3.8. The design of C2C's
C2Parser and C2Sema class are heavily based on clang's Parser and Sema class,
so hereby my thanks to the Clang folks!


## What needs to be done
A short list of open items (the full list would probably fill-up GitHub) with
their priority:
* [high] c2c: parse full syntax into AST (ALMOST DONE)
* [high] c2c: generate IR code for more AST elements
* [medium] tool: create graphical refactor tool (c2reto) (IN PROGRESS)
* [medium] c2c: tab completion on targets in recipe / cmdline args
* [medium] tool: create c-parser for parsing C headers.
* [medium] tool: create c2format - astyle for C2.
* [low] tool: c2grep - grep only files in recipe


## Installation
Read the [installation document](INSTALL.md) for installation on Linux or OSX. For
Windows installation, see [windows installation document](INSTALL_WIN.md).

## Using the C2 compiler
By default, c2c will only parse and analyse the targets. Generating C-code
should work on all examples, but generating LLVM's IR code is work in
progress. In the examples directory: (or add -d examples/)
```
c2c multi
c2c hello
c2c puzzle
c2c -I working_ir
```

It's also possible to manually compile a single .c2 file without a recipe
file with:
```
c2c -f <file.c2>
```

To generate ANSI-C code, use:
```
c2c -C <target>
```

The C2 compiler is able to generate a package dependency file in dot format. This
file can be converted into a png as follows:
```
c2c --deps <target>
dot -T png output/target/deps.dot > image.png
```

To see all available options, run:
```
c2c -h
```
##c2tags
**c2tags** is C2's version of ctags. This tool is used by vim (e.a.) to "jump to
definition". See the [installation document](INSTALL.md) on how to install.

How it works is as follows:
* use --refs or add $refs in recipe.txt to generate **refs** file during compilation.
* c2c generates a **refs** file per target. This file contains all references
    and their destination
* c2tags currently doesn't have a full-blown vim-plugin yet, but a small
    code-fragment in you .vimrc suffices.
* when standing on any symbol in C2 code, pressing ctrl-h (configurable)
    will jump to the definition. Pressing ctrl-o (default) will jump back.

Just like **c2c** itself, **c2tags** can be called from any (sub)directory in the
project tree.

