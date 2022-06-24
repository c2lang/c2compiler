
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
- named feature-selection
    #if NAME
- advanced basic feature-selection: (#if [cond], #else, #endif)
- error/warn feature
    #error "text"
    #warn "text" -> hmm how to get warnings out tokenizer? (Warning token)
    [cond] can be 0 | 1 | literal (must be known) | literal=<value>
        -> also allow OR AND NOT
        -> LATER: () to specify precedence
        -> literal=<value>   value is treated as text (numbers also)
        -> ignore extra spaces around
    -> pass selection to tokenizer at creation


## SourceManager
+ reserve location 0
+ remove hardcoded file max
- close files after use (need indication)

## Performance
- profile application to see where time is spent

