
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
- advanced basic feature-selection:
    - NAME=<value> -> value always treated as text
    - OR AND NOT in condition
    - () to indicate order

## SourceManager
+ reserve location 0
+ remove hardcoded file max
- close files after use (need indication)

## Parser
- put all output through filter for coloring/not
- parse Types
- parse Functions
- parse Varables

## AST
+ create structure
+ create Context (for allocation)

## AST-Builder
- fill AST
- remove c2_ast_builder, let Parser call creators directly?
    -> KEEP for now (does some parse-time checks)

## Performance
- profile application to see where time is spent

