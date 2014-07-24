
## TODO list

Parsing of:
* Balanced brackets/braces (Convert clang::BalancedDelimiterTracker)
* cast expression
* bitfields
* attributes

AST Generation for:
* cast expression
* ...

Analysis of:
* un-initialized variables
* Type comparisons
* string parsing (\n\0 etc)
* labels (redefinition)
* package+file dependencies
* ...

IR generation of:
* control flow statements (while, for, etc)
* global strings

C-code generation:
* creating build-script/Makefile

Generic:
* saving package info to cache
* saving recipe info to cache
* parsing ansi-C headers into Package

Tooling:
* create c2find that only greps in files in recipe/target
* create c2style, astyle for C2
* create c2tags/c2scope that only includes files from recipe/target
* create graphical refactor tool to view files/packages/target symbols
    and allows project-wide renaming and drag-n-drop var/type/function definition
    reordering within files or within package.

