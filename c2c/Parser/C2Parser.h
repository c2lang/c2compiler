/* Copyright 2013-2018 Bas van den Berg
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

#ifndef PARSER_C2PARSER_H
#define PARSER_C2PARSER_H

#include "Clang/Preprocessor.h"
#include "Parser/Ownership.h"
#include "Parser/ParserTypes.h"

//#define PARSER_DEBUG

#ifdef PARSER_DEBUG
#include <iostream>
#include "Utils/color.h"
#endif

// EXCEPTION
using namespace c2lang;

namespace C2 {

class C2Sema;
class Decl;
class Expr;
class FunctionDecl;
class StructTypeDecl;

/// PrecedenceLevels - These have been altered from C99 to C2
/// In particular, addition now comes after bitwise and shifts
/// Bitwise is directly after shift and equality and relational have
/// the same precedence.
namespace prec {
  enum Level {
    Unknown         = 0,    // Not binary operator.
    Comma           = 1,    // ,
    Assignment      = 2,    // =, *=, /=, %=, +=, -=, <<=, >>=, &=, ^=, |=
    Conditional     = 3,    // ?
    LogicalAndOr    = 4,    // &&, ||
    Relational      = 5,    // ==, !=, >=, <=, >, <
    Additive        = 6,    // -, +
    Bitwise         = 7,    // ^, |, &
    Shift           = 8,    // <<, >>
    Multiplicative  = 9,    // *, /, %
    PointerToMember = 10    // .*, ->*
  };
}

class C2Parser {
public:
    friend class BalancedDelimiterTracker;

    C2Parser(c2lang::Preprocessor& pp, C2Sema& sema, bool isInterface_);
    ~C2Parser();

    bool Parse();
private:
    c2lang::Preprocessor& PP;
    Token Tok;
    SourceLocation PrevTokLocation;
    unsigned short ParenCount, BracketCount, BraceCount;
    C2Sema& Actions;
    DiagnosticsEngine& Diags;
    bool isInterface;

    ExprResult ExprError();
    StmtResult StmtError();
    DeclResult DeclError();

    // top level
    void ParseModule();
    void ParseImports();
    bool ParseTopLevel();
    void ParseTypeDef(bool is_public);
    void ParseVarDef(bool is_public);
    void ParseFuncDef(bool is_public);

    // Type def
    void ParseStructType(bool is_struct, const char* id, SourceLocation idLoc, bool is_public);
    void ParseStructBlock(StructTypeDecl* S);
    void ParseEnumType(const char* id, SourceLocation idLoc, bool is_public);
    void ParseAliasType(const char* id, SourceLocation idLoc, bool is_public);
    void ParseFuncType(IdentifierInfo* id, SourceLocation& idLoc, bool is_public);

    // function def
    bool ParseFunctionParams(FunctionDecl* func, bool allow_defaults);
    VarDeclResult ParseParamDecl(FunctionDecl* func, bool allow_defaults);

    // var def
    ExprResult ParseArrayDesignator(bool* need_semi);
    ExprResult ParseFieldDesignator(bool* need_semi);
    ExprResult ParseInitValue(bool* need_semi, bool allow_designator);
    ExprResult ParseInitValues();
    void ParseArrayEntry();
    ExprResult ParseConstantExpression();

    // generic
    ExprResult ParseSingleTypeSpecifier(bool allow_qualifier);
    ExprResult ParseTypeSpecifier(bool allow_qualifier);
    ExprResult ParseArray(ExprResult base);
    ExprResult ParseSizeof();
    ExprResult ParseElemsof();
    ExprResult ParseEnumMinMax(bool isMin);
    ExprResult ParseIdentifier();
    ExprResult ParseFullIdentifier();
    ExprResult ParseAsmStringLiteral();

  //===--------------------------------------------------------------------===//
  // C99 6.8: Statements and Blocks.

  /// A SmallVector of expressions, with stack size 12 (the maximum used.)
  typedef SmallVector<Expr*, 12> ExprVector;

    StmtResult ParseCompoundStatement();
    StmtResult ParseStatement();
    StmtResult ParseAsmStatement();
    StmtResult ParseReturnStatement();
    StmtResult ParseIfStatement();
    StmtResult ParseSwitchStatement();
    StmtResult ParseWhileStatement();
    StmtResult ParseDoStatement();
    StmtResult ParseForStatement();
    StmtResult ParseGotoStatement();
    StmtResult ParseBreakStatement();
    StmtResult ParseContinueStatement();
    StmtResult ParseDeclOrStatement();
    StmtResult ParseDeclaration(bool checkSemi);
    StmtResult ParseCaseStatement();
    StmtResult ParseDefaultStatement();
    StmtResult ParseLabeledStatement();
    StmtResult ParseExprStatement();
    bool ParseCondition(StmtResult& Res);
    bool ParseAttributes(Decl* D);

    // expressions
    /// TypeCastState - State whether an expression is or may be a type cast.
    enum TypeCastState {
        NotTypeCast = 0,
        MaybeTypeCast,
        IsTypeCast
    };


    ExprResult ParseExpression(TypeCastState isTypeCast = NotTypeCast);
    ExprResult ParseAssignmentExpression(TypeCastState isTypeCast = NotTypeCast);
    ExprResult ParseCastExpression(bool isUnaryExpression,
                             bool isAddressOfOperand,
                             bool &NotCastExpr,
                             TypeCastState isTypeCast);
    ExprResult ParseCastExpression(bool isUnaryExpression,
                             bool isAddressOfOperand = false,
                             TypeCastState isTypeCast = NotTypeCast);
    ExprResult ParseExplicitCastExpression();
    ExprResult ParseRHSOfBinaryExpression(ExprResult LHS, prec::Level MinPrec);
    ExprResult ParseStringLiteralExpression(bool AllowUserDefinedLiteral = false);
    ExprResult ParsePostfixExpressionSuffix(ExprResult LHS);
    bool ParseAsmOperandsOpt(SmallVectorImpl<IdentifierInfo*> &Names,
                             SmallVectorImpl<Expr*> &Constraints,
                             SmallVectorImpl<Expr*> &Exprs);

    /// Returns true if the next token would start a postfix-expression
    /// suffix.
    bool isPostfixExpressionSuffixStart() {
        tok::TokenKind K = Tok.getKind();
        return (K == tok::l_square || K == tok::l_paren ||
                K == tok::period || K == tok::arrow ||
                K == tok::plusplus || K == tok::minusminus);
    }

      typedef SmallVector<Expr*, 20> ExprListTy;
      typedef SmallVector<SourceLocation, 20> CommaLocsTy;

    /// ParseExpressionList - Used for C/C++ (argument-)expression-list.
    bool ParseExpressionList(SmallVectorImpl<Expr*> &Exprs,
                             SmallVectorImpl<SourceLocation> &CommaLocs);

    /// ParenParseOption - Control what ParseParenExpression will parse.
    enum ParenParseOption {
        SimpleExpr,      // Only parse '(' expression ')'
        CompoundStmt,    // Also allow '(' compound-statement ')'
        CompoundLiteral, // Also allow '(' type-name ')' '{' ... '}'
        CastExpr         // Also allow '(' type-name ')' <anything>
    };
    ExprResult ParseParenExpression(ParenParseOption &ExprType,
                                    bool stopIfCastExpr,
                                    bool isTypeCast,
                                    SourceLocation &RParenLoc);

    bool isTypeSpec();
    bool isDeclaration();
    int SkipArray(int lookahead);
    unsigned ParseOptionalTypeQualifier();
    bool ParseOptionalAccessSpecifier();

    const Token& getCurToken() const { return Tok; }

    SourceLocation ConsumeToken() {
        PrevTokLocation = Tok.getLocation();
#ifdef PARSER_DEBUG
        if (Tok.isNot(tok::eof)) {
        std::cerr << ANSI_MAGENTA;
        PP.DumpToken(Tok);
        std::cerr << ANSI_NORMAL << std::endl;
        }
#endif
        PP.Lex(Tok);
        return PrevTokLocation;
    }

    bool TryConsumeToken(tok::TokenKind Expected) {
        if (Tok.isNot(Expected))
            return false;
        assert(!isTokenSpecial() &&
                "Should consume special tokens with Consume*Token");
        PrevTokLocation = Tok.getLocation();
        PP.Lex(Tok);
        return true;
    }

    bool TryConsumeToken(tok::TokenKind Expected, SourceLocation &Loc) {
        if (!TryConsumeToken(Expected))
            return false;
        Loc = PrevTokLocation;
        return true;
    }

    // Low-level token peeking and consumption methods (from Parser.h)
    const Token &GetLookAheadToken(unsigned N) {
        if (N == 0 || Tok.is(tok::eof)) return Tok;
        return PP.LookAhead(N-1);
    }

    /// NextToken - This peeks ahead one token and returns it without
    /// consuming it.
    const Token &NextToken() {
        return PP.LookAhead(0);
    }

  /// ConsumeAnyToken - Dispatch to the right Consume* method based on the
  /// current token type.  This should only be used in cases where the type of
  /// the token really isn't known, e.g. in error recovery.
  SourceLocation ConsumeAnyToken() {
    if (isTokenParen())
      return ConsumeParen();
    else if (isTokenBracket())
      return ConsumeBracket();
    else if (isTokenBrace())
      return ConsumeBrace();
    else if (isTokenStringLiteral())
      return ConsumeStringToken();
    else
      return ConsumeToken();
  }

  /// ConsumeParen - This consume method keeps the paren count up-to-date.
  ///
  SourceLocation ConsumeParen() {
    assert(isTokenParen() && "wrong consume method");
    if (Tok.getKind() == tok::l_paren)
      ++ParenCount;
    else if (ParenCount)
      --ParenCount;       // Don't let unbalanced )'s drive the count negative.
    return ConsumeToken();
  }

  /// ConsumeBracket - This consume method keeps the bracket count up-to-date.
  ///
  SourceLocation ConsumeBracket() {
    assert(isTokenBracket() && "wrong consume method");
    if (Tok.getKind() == tok::l_square)
      ++BracketCount;
    else if (BracketCount)
      --BracketCount;     // Don't let unbalanced ]'s drive the count negative.
    return ConsumeToken();
  }
  /// ConsumeBrace - This consume method keeps the brace count up-to-date.
  ///
  SourceLocation ConsumeBrace() {
    assert(isTokenBrace() && "wrong consume method");
    if (Tok.getKind() == tok::l_brace)
      ++BraceCount;
    else if (BraceCount)
      --BraceCount;     // Don't let unbalanced }'s drive the count negative.
    return ConsumeToken();
  }

    /// ConsumeStringToken - Consume the current 'peek token', lexing a new one
    /// and returning the token kind.  This method is specific to strings, as it
    /// handles string literal concatenation, as per C99 5.1.1.2, translation
    /// phase #6.
    SourceLocation ConsumeStringToken() {
        assert(isTokenStringLiteral() &&
               "Should only consume string literals with this method");
        return ConsumeToken();
    }

  /// \brief Abruptly cut off parsing; mainly used when we have reached the
  /// code-completion point.
  void cutOffParsing() {
    if (PP.isCodeCompletionEnabled())
      PP.setCodeCompletionReached();
    // Cut off parsing by acting as if we reached the end-of-file.
    Tok.setKind(tok::eof);
  }

  //===--------------------------------------------------------------------===//
  // Low-Level token peeking and consumption methods.
  //

  /// isTokenParen - Return true if the cur token is '(' or ')'.
  bool isTokenParen() const {
    return Tok.getKind() == tok::l_paren || Tok.getKind() == tok::r_paren;
  }
  /// isTokenBracket - Return true if the cur token is '[' or ']'.
  bool isTokenBracket() const {
    return Tok.getKind() == tok::l_square || Tok.getKind() == tok::r_square;
  }
  /// isTokenBrace - Return true if the cur token is '{' or '}'.
  bool isTokenBrace() const {
    return Tok.getKind() == tok::l_brace || Tok.getKind() == tok::r_brace;
  }

  /// isTokenStringLiteral - True if this token is a string-literal.
  ///
  bool isTokenStringLiteral() const {
    return Tok.getKind() == tok::string_literal ||
           Tok.getKind() == tok::wide_string_literal ||
           Tok.getKind() == tok::utf8_string_literal ||
           Tok.getKind() == tok::utf16_string_literal ||
           Tok.getKind() == tok::utf32_string_literal;
  }

  /// isTokenSpecial - True if this token requires special consumption methods.
  bool isTokenSpecial() const {
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace() || Tok.is(tok::code_completion);
  }

    bool ExpectIdentifier(const char *Msg = "");

    /// ExpectAndConsume - The parser expects that 'ExpectedTok' is next in the
    /// input.  If so, it is consumed and false is returned.
    ///
    /// If a trivial punctuator misspelling is encountered, a FixIt error
    /// diagnostic is issued and false is returned after recovery.
    ///
    /// If the input is malformed, this emits the specified diagnostic and true is
    /// returned.

    bool ExpectAndConsume(tok::TokenKind ExpectedTok,
                          unsigned DiagID = diag::err_expected,
                          const char* DiagMsg = "");

    bool ExpectAndConsumeSemi(unsigned DiagID);

    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
    DiagnosticBuilder Diag(const Token &T, unsigned DiagID);

public:
  /// \brief Control flags for SkipUntil functions.
  enum SkipUntilFlags {
    StopAtSemi = 1 << 0,  ///< Stop skipping at semicolon
    /// \brief Stop skipping at specified token, but don't skip the token itself
    StopBeforeMatch = 1 << 1,
    StopAtCodeCompletion = 1 << 2 ///< Stop at code completion
  };

  friend constexpr SkipUntilFlags operator|(SkipUntilFlags L,
                                                 SkipUntilFlags R) {
    return static_cast<SkipUntilFlags>(static_cast<unsigned>(L) |
                                       static_cast<unsigned>(R));
  }

  /// SkipUntil - Read tokens until we get to the specified token, then consume
  /// it (unless StopBeforeMatch is specified).  Because we cannot guarantee
  /// that the token will ever occur, this skips to the next token, or to some
  /// likely good stopping point.  If Flags has StopAtSemi flag, skipping will
  /// stop at a ';' character.
  ///
  /// If SkipUntil finds the specified token, it returns true, otherwise it
  /// returns false.
  bool SkipUntil(tok::TokenKind T,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0)) {
    return SkipUntil(llvm::makeArrayRef(T), Flags);
  }
  bool SkipUntil(tok::TokenKind T1, tok::TokenKind T2,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0)) {
    tok::TokenKind TokArray[] = {T1, T2};
    return SkipUntil(TokArray, Flags);
  }
  bool SkipUntil(tok::TokenKind T1, tok::TokenKind T2, tok::TokenKind T3,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0)) {
    tok::TokenKind TokArray[] = {T1, T2, T3};
    return SkipUntil(TokArray, Flags);
  }
  bool SkipUntil(ArrayRef<tok::TokenKind> Toks,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0));

private:
    C2Parser(const C2Parser&);
    C2Parser& operator= (const C2Parser&);
};

}

#endif

