/* Copyright 2013-2023 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PARSER_HELPERS_H
#define PARSER_HELPERS_H

#include "Parser/Parser.h"

namespace C2 {

class Parser;

class BalancedDelimiterTracker {
private:
    Parser& P;
    tok::TokenKind Kind, Close, FinalToken;
    SourceLocation (Parser::*Consumer)();
    SourceLocation LOpen, LClose;

    unsigned short &getDepth() {
        switch (Kind) {
        case tok::l_brace: return P.BraceCount;
        case tok::l_square: return P.BracketCount;
        case tok::l_paren: return P.ParenCount;
        default: llvm_unreachable("Wrong token kind");
        }
    }

    enum { MaxDepth = 256 };

    bool diagnoseOverflow();
    bool diagnoseMissingClose();

    public:
    BalancedDelimiterTracker(Parser& p, tok::TokenKind k,
                             tok::TokenKind FinalToken_ = tok::semi)
        :P(p), Kind(k), FinalToken(FinalToken_)
    {
        switch (Kind) {
        default: llvm_unreachable("Unexpected balanced token");
        case tok::l_brace:
            Close = tok::r_brace;
            Consumer = &Parser::ConsumeBrace;
            break;
        case tok::l_paren:
            Close = tok::r_paren;
            Consumer = &Parser::ConsumeParen;
            break;
        case tok::l_square:
            Close = tok::r_square;
            Consumer = &Parser::ConsumeBracket;
            break;
        }
    }

    SourceLocation getOpenLocation() const { return LOpen; }
    SourceLocation getCloseLocation() const { return LClose; }
    SourceRange getRange() const { return SourceRange(LOpen, LClose); }

    bool consumeOpen() {
        if (!P.Tok.is(Kind))
        return true;

        if (getDepth() < MaxDepth) {
            LOpen = (P.*Consumer)();
            return false;
        }

        return diagnoseOverflow();
    }

    bool expectAndConsume(unsigned DiagID = diag::err_expected,
                          const char *Msg = "",
                          tok::TokenKind SkipToTok = tok::unknown);
                          bool consumeClose()
    {
        if (P.Tok.is(Close)) {
        LClose = (P.*Consumer)();
        return false;
        }

        return diagnoseMissingClose();
    }
    void skipToEnd();
};

}

#endif

