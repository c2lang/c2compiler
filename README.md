
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
- check AVL trees to have better performance than RB trees
- in a skipFeatures part, still check syntax?

## SourceManager
- close files after use (need indication)
- clear files up to certain index (all except recipe + build-file before each target)

## Parser
- TEST many unterminated things (unexpected EOF)
- move Lookahead feature from Tokenizer to Parser (on lookahead with error, just stop parser also)
- put all output through filter for coloring/not

## AST
- remove Module from Decl, move to sub-classes
    make Decl.getModule() that switches
    VarDecl -> local,param,member dont need Module! (only global)
    Import -> yes for target
    EnumConstant -> no
    StaticAssert -> no

    do need:
    Function,
    StructType,
    EnumType,
    FunctionType,
    AliasType,
    Var,        -> only if Global
    -> LATER
/ name: dont store char* but u32 name_index
    have AST global name_offset that is set to main StringPool
    -> saves 4 bytes -> also needs AST -> u32 otherwise just padded back
- Idea: dont save pointers, but save offsets into global Pool.
    OLD: 43798 allocs (avg 26 bytes), 73 blocks (16384), total 1155 Kb
        takes between 3470 - 3580 usec
    ISSUE: cannot do getLHS2() { return &lhs; } -> need indirection. Would have to
        return ExprIdx with u32* inside, then the whole type system is usefull
    NEW: ??
    -> Pool could be resized then
    -> if size is an issue, we could use offset*4. (QualType would have to be 8 bytes then
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
- C2C: FIX object sizes (Stmt should be 4 bytes, not aligned yet)
    ast_Stmt = 4
    ast_IfStmt = 24
    ast_Expr = 16
    ast_ParenExpr = 24
    -> type must be u32 or i32 (C99)
    -> keep track of how many bits are left. If the field doesn't fit, a next one is created (no straddling)
    -> if followed by a non-bitfield, left = 0;
    -> add a lot of unit tests
    -> ini combination with packed?
    TEMP: try reverting c2c to generate sizeof(x) instead of number
        -> big.c2 -> 309 ipv 488 blocks!
- move SrcLoc to ast_helper?
- Give every Type a PtrType* ptr, since we can re-use efficiently. Since many types will have
    pointers to them, this is not so bad.


## Compiler
- load external components
    - handle nolibc
    - handle use
    Design:
    - load components manifest
        -> change to yaml (keep old for now, add manifest.yml)
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
    - update module sorting



## Analyser
- check if all files of module are in the same dir
    -> check if prefix is the same
- remove check/errors from Component + Main, just do somewhere else?

## Performance
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

## General
-> If fixed size StringPool becomes an issues, users could increase it in the recipe/build file
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
    NEW (without C cast)

Builder
    -> list of Modules (for lookup)
  Component
    Module
      AST

--------------------------------------------
AST Sizes

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



