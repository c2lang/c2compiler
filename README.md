
## Tokenizer
+ keywords
+ print number of each TokenKind
+ use lookup table per char
+ faster keyword lookup
+ support block comment
+ let error generate line-nr
+ TODO escaped chars in strings (especially \")
+ SourceLoc of EOF is 0
+ check max identifier Length (31 bytes)
+ support basic feature-selection: (#if [cond], #else, #endif)
+ have error() be like printf
+ error/warn feature
+ named feature-selection
- support nested /* */ comments
- replace __file__ and __line__ with string-literal / number?
    Q: how to do __func__??
        -> emit FuncNameToken? (let analyser fill it in)
- give error on un-terminated #if/#else -> on EOF
+ Numbers: hex, octal, pass radix
- parse floating points
    - parse hexadecimal
    + parse octal
- pass Numbers as number? (no need to alloc str then) -> just pass as u64?
- Move context to common?
- advanced basic feature-selection:
    - NAME=<value> -> value always treated as text
    - OR AND NOT in condition
    - () to indicate order

## SourceManager
+ reserve location 0
+ remove hardcoded file max
+ alias-type for SrcLoc
- close files after use (need indication)

## Parser
+ parse Types
+ parse Functions
+ parse Varables
- TEST many unterminated things (unexpected EOF)
- use lookup tables to speedup
- move Lookahead feature from Tokenizer to Parser (on lookahead with error, just stop parser also)
- put all output through filter for coloring/not

## AST
+ create structure
+ create Context (for allocation)
- FIX object sizes (Stmt should be 4 bytes, not aligned yet)
- move SrcLoc to ast_helper?
- use common/pointer-map, or put ptr* in each Type? (pointers are very common, so could be efficient)

## AST-Builder
+ fill AST
+ print content
- use RB-tree to put all Type* to get pointer-types etc, program-wide!

## Performance
- profile application to see where time is spent
- try building as 32-bit application, measure mem + speed
    add -m32 to CFLAGS + LDFLAGS
- check how other Compilers (easier ones) parse Expr
    cproc -> see expr.c:1311 -> expr()

## Language
- Dont allow Declaration in Condition of if stmt?

## General
- remove unused import
- dont import local, (for single printf)
- CHECK  all DeclList / StmtList etc to see if they are freed
- add Trace option to instrument all functions?
- BIG LOOKUP TABLES: per TokenKind
    -> different tables, better for cache
    Q: put function-pointers in table? (no need to jump after) -> try speedup
    - top level
    - stmt level
    - expr level
    - struct body?


Parse Expr:
clang
    TODO

cproc
    TODO

c2c
    NEW (without C cast)
