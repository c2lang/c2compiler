
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

## SourceManager
- close files after use (need indication)
- clear files up to certain index (all except recipe + build-file before each target)

## Parser
- TEST many unterminated things (unexpected EOF)
- use lookup tables to speedup
- move Lookahead feature from Tokenizer to Parser (on lookahead with error, just stop parser also)
- put all output through filter for coloring/not

## AST
- improve AST print, like c2c
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
- use common/pointer-map, or put ptr* in each Type? (pointers are very common, so could be efficient)
    - or use RB-tree to put all Type* to get pointer-types etc, program-wide?

## Builder

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
- make string_buffer auto resizable
- rename string_buffer.StringBuffer -> string_buffer.Buf
- TypePtr
-> If fixed size StringPool becomes an issues, users could increase it in the recipe/build file
- convert files to relative path in findProjectDir(), to Vim understands errors
- add option to print all configs in code
- CHECK  all DeclList / StmtList etc to see if they are freed
- add Trace option to instrument all functions?

Parse Expr:
clang
    TODO

cproc
    TODO

c2c
    NEW (without C cast)

