
## TODO list

Analysis of:
* un-initialized variables
* Type comparisons
* string parsing (\n\0 etc)
* labels (redefinition)
* ...

IR generation of:
* control flow statements (while, for, etc)
* global strings

Generic:
* saving package info to cache
* saving recipe info to cache
* parsing ansi-C headers into Package

Tooling:
* create c2format, astyle for C2
* create graphical refactor tool to view files/packages/target symbols
    and allows project-wide renaming and drag-n-drop var/type/function definition
    reordering within files or within package.
    <IN PROGRESS>
* create c2find that only greps in files in recipe/target

