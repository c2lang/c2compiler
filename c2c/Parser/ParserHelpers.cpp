/* Copyright 2013-2017 Bas van den Berg
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

#include <assert.h>
#include <clang/AST/Expr.h>
#include <clang/Parse/ParseDiagnostic.h>

#include "Parser/ParserHelpers.h"

using namespace C2;

bool BalancedDelimiterTracker::diagnoseOverflow() {
    P.Diag(P.Tok, diag::err_bracket_depth_exceeded)
            << MaxDepth;
    P.Diag(P.Tok, diag::note_bracket_depth);
    P.cutOffParsing();
    return true;
}

bool BalancedDelimiterTracker::expectAndConsume(unsigned DiagID,
        const char *Msg,
        tok::TokenKind SkipToTok) {
    LOpen = P.Tok.getLocation();
    if (P.ExpectAndConsume(Kind, DiagID, Msg)) {
        if (SkipToTok != tok::unknown)
            P.SkipUntil(SkipToTok, C2Parser::StopAtSemi);
        return true;
    }

    if (getDepth() < MaxDepth)
        return false;

    return diagnoseOverflow();
}

bool BalancedDelimiterTracker::diagnoseMissingClose() {
    assert(!P.Tok.is(Close) && "Should have consumed closing delimiter");

    P.Diag(P.Tok, diag::err_expected) << Close;
    P.Diag(LOpen, diag::note_matching) << Kind;

    // If we're not already at some kind of closing bracket, skip to our closing
    // token.
    if (P.Tok.isNot(tok::r_paren) && P.Tok.isNot(tok::r_brace) &&
            P.Tok.isNot(tok::r_square) &&
            P.SkipUntil(Close, FinalToken,
                        C2Parser::StopAtSemi | C2Parser::StopBeforeMatch) &&
            P.Tok.is(Close))
        LClose = P.ConsumeAnyToken();
    return true;
}

void BalancedDelimiterTracker::skipToEnd() {
    P.SkipUntil(Close, C2Parser::StopBeforeMatch);
    consumeClose();
}

