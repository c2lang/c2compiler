
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
    - ()

## SourceManager
+ reserve location 0
+ remove hardcoded file max
- close files after use (need indication)

## Performance
- profile application to see where time is spent

