
## 2-phase parsing
- 1st pass:
    - reads module + imports
    - filters #ifdefs
    - stores types/funcs/vars + name + loc
    - store all tokens? (of all files in main module, around 1.3 Mb for c2c, sources are 396Kb)
        -> only need to parse and filter ifdefs once, but takes more memory
        -> would this work for a Big project (ie Linux kernel)?
- 2nd pass: (in order of modules)
    - also filters ifdefs (inside functions)
        Note: global ifdefs already filtered out
    - parses all types (+ dependent vars), then vars, then funcs
    - just parse a single TopLevel from offset
-> no need to store module prefixes / func prefixes
-> allows all RefTypes and IdentifierExpr to be resolved immediately
-> allows caching of string -> Decl (no need to check/search every time)
-> this costs more parsing time, but saves a lot on resolving and AST
- Q: how much AST would be saved?
- Q: cannot tail-allocate struct members anymore? (or function params)

## TODO
- MemberExpr: setKind during analysis, also finish (for global)
- PointerType: filter duplicates (set ptr in Type of inner)
- FIX unit tests (global only, not functions)
- add type to Component
- When analysing a file, cache symbol lookup, so we dont have to check stuff multiple times
- when parsing a file all Type* will be the same! So re-use pointers.
    -> only need to resolve once then
    -> the source location is not the same! (different TypeExpr, same QualType)
    -> between modules, re-use same QualType during analysis, so we can do ptr-compare on Type
    -> RefType is different, since it has a different IdentifierExpr (with loc)
    IDEA: or dont create more ptr-types, but just check if they point to the same decl!
        -> sames space + origType ptr
        -> canonicalType must be the same!
- Types:
    - Canonical types
        + enum
        + struct
        + builtin
        + ref
        + pointer
            Enum*
                Parse:
                    PointerType -> RefType
                Analysis:
                    PointerType -> EnumType
                      ------         u8
                                     --
                -> VarDecl has origType (used for getting refs)
                -> TypeExpr also has an origType!
        + array
            Parse:
                ArrayType -> RefType
            Analyses:
                ArrayType -> StructType
                ---------      ---
        - function
        - alias? (does not exist yet)
- analyse types:
    - array size expr -> must be CTV, must be number
    - init expr
- load external components
    - for main component: walk imports
        - for all imports to external component, mark module as used
        - store component_idx in module? (fast way to find component for module)
    - for all external components:
        - parse all used modules, check imports, add more module to queue
    - analyse external components (bottom up)
    - handle nolibc
    - handle use
    Design:
    - load components manifest
        -> change Yaml parser to use SourceMgr, add locs
            -> LATER, for now just use
        DONT CHANGE RECIPE YET, FIRST JUST PARSE manifest.yml
    - re-use components between targets? (libc)

    - give component names ("libc", "main")
    - check duplicate module names between components (on every new module)
        -> components get/add to list of all Modules
            -> if they create a new one, they check
    - fill in all imports
        -> mark all external modules if used (later only parse those needed)
- hardcode/create c2 module - add extra symbols
- create DiagnosticsEngine, return true if error, false if warning
    - user should still be able to use -Werror like
    - add info() for 2nd info like 'older decl is here'
    - use enum with diags.id (must be short, so not two namespaces (diags.Id.Bla)
        -> only needed if we use same diag in multiple places
    - print error location nicely (line indicating error)
- automatically add feature _target_32bit_ (otherwise 32-bit)
- try longjump in ModuleAnalyser -> get memleaks, or register lists (make linked list)
- load other external components (recursively)
    parse used sources (only)
- SourceMgr, close after use (and re-open)
    - takes 200 usec
    - try not mapping, but malloc + read file? -> measure
- Parser: try to merge tokens '-' and '<number>' as single IntegerLiteral
    if parseExpr() starts with '-', check if next is IntegerLiteral, merge then.
    -> on bb_combine.. branch
- use c2recipe.yml

## Tokenizer
- parse floating points
- replace __file__ and __line__ with string-literal / number?
    Q: how to do __func__??
        -> emit FuncNameToken? (let analyser fill it in)
- pass Numbers as number? (no need to alloc str then) -> just pass as u64?
- Move context to common?
- advanced basic feature-selection:
    - NAME=<value> -> value always treated as text
    - OR AND NOT in condition
    - () to indicate order
- in a skipFeatures part, still check syntax? -> no

## Parser
- TEST many unterminated things (unexpected EOF)
- move Lookahead feature from Tokenizer to Parser (on lookahead with error, just stop parser also)
- put all output through filter for coloring/not

## Analyser
- mark Module with main() as used
- remove check/errors from Component + Main, just do somewhere else?
- analyser: allow enum switches without prefix (also fix in C2C)
    -> need analyseStmt first

## AST
- Idea: use same data dat build new tree for Scope?
    or make orderned copy then use flat trees to walk? Also instead of pointers, could be u32's
    comparing is just comparing/checking u32's
- store Attributes
    2 prints:
        -> all types are printed on single line
        - one for AST print - print()
            '(struct)Block*'
            'u8[a]' -> after analysis 'u8[3]'
            'Point => u8'
            'u8[3] => u8*'
        - one for diagnostics - diagPrint()
            -> has (enum)/(struct) etc prepends
            const Block*
            'Point* => u8'
            'u8[3] => u8*'
        -> rename debugPrint -> diagPrint
    -> ALSO need expr.printLiteral()
        -> printLiteral does NOT print colors
- move SrcLoc to ast_helper?
- Give every Type a PtrType* ptr, since we can re-use efficiently. Since many types will have
    pointers to them, this is not so bad.

## Compiler
## Performance
- check memleaks: valgrind --leak-check=full -s ./output/c2c/c2c
- profile application to see where time is spent
    add -pg to CFLAGS + LDFLAGS
    run c2c
    -> gmon.out is created
- try building as 32-bit application, measure mem + speed
    add -m32 to CFLAGS + LDFLAGS
- check how other Compilers (easier ones) parse Expr
    cproc -> see expr.c:1311 -> expr()

## Language
- Dont allow Declaration in Condition of if stmt?

## Generate-C
- assert:
    generate original source location in assert (not generate C)
    -> so generate own if stmt with source loc included

## General
- convert files to relative path in findProjectDir(), to Vim understands errors
- add option to print all configs in code
- CHECK all DeclList / StmtList etc to see if they are freed
- add Trace option to instrument all functions?

Parse Expr:
clang
    TODO

cproc
    TODO

c2c

--------------------------------------------
AST Sizes

NOTE: NOT possible due to Expr** getLHS2() { ..}

Name: char* -> u32
mod:  Module* -> u32
AST: Expr* -> u32

                    Now     Name u32        AST u32
Decl                32      32              20
FunctionDecl        48      48              28
VarDecl             same as Decl

Stmt                 4      -               -
IfStmt              24      -              12
ReturnStmt           8      -               -

Expr                16      -               12
CharLiteral         same as Expr
NilExpr
IdentifierExpr      24      -               16
CallExpr            24      -               20
MemberExpr          32      -               24
BinaryOperator      32      -               24

Type                 4      -               -
PointerType         16      -               8
RefType             16/24   -               8/12

QualType             8      -               4/8 (depending on offset granularity)



