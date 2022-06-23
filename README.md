
## Tokenizer
+ keywords
+ print number of each TokenKind
+ use lookup table per char
+ faster keyword lookup
+ support block comment
+ let error generate line-nr
+ TODO escaped chars in strings (especially \")
+ SourceLoc of EOF is 0

## SourceManager
+ reserve location 0
+ remove hardcoded file max
- close files after use (need indication)

## Performance
- profile application to see where time is spent

