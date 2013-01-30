/* Copyright 2013 Bas van den Berg
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

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "C2Parser.h"
#include "C2Sema.h"
#include "StringBuilder.h"
#include "Decl.h"
#include "Stmt.h"
#include "Expr.h"
#include "color.h"

using namespace C2;
using namespace clang;

#ifdef PARSER_DEBUG
#define LOG_FUNC std::cerr << ANSI_YELLOW << __func__ << "()" << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#endif

/// \brief Return the precedence of the specified binary operator token.
static prec::Level getBinOpPrecedence(tok::TokenKind Kind) {
  switch (Kind) {
  case tok::greater:
      return prec::Relational;
  case tok::greatergreater:
      return prec::Shift;
  default:                        return prec::Unknown;
  case tok::comma:                return prec::Comma;
  case tok::equal:
  case tok::starequal:
  case tok::slashequal:
  case tok::percentequal:
  case tok::plusequal:
  case tok::minusequal:
  case tok::lesslessequal:
  case tok::greatergreaterequal:
  case tok::ampequal:
  case tok::caretequal:
  case tok::pipeequal:            return prec::Assignment;
  case tok::question:             return prec::Conditional;
  case tok::pipepipe:             return prec::LogicalOr;
  case tok::ampamp:               return prec::LogicalAnd;
  case tok::pipe:                 return prec::InclusiveOr;
  case tok::caret:                return prec::ExclusiveOr;
  case tok::amp:                  return prec::And;
  case tok::exclaimequal:
  case tok::equalequal:           return prec::Equality;
  case tok::lessequal:
  case tok::less:
  case tok::greaterequal:         return prec::Relational;
  case tok::lessless:             return prec::Shift;
  case tok::plus:
  case tok::minus:                return prec::Additive;
  case tok::percent:
  case tok::slash:
  case tok::star:                 return prec::Multiplicative;
  case tok::periodstar:
  case tok::arrowstar:            return prec::PointerToMember;
  }
}


static C2Type kw2type(tok::TokenKind Kind) {
    switch (Kind) {
    case tok::kw_u8:    return TYPE_U8;
    case tok::kw_u16:   return TYPE_U16;
    case tok::kw_u32:   return TYPE_U32;
    case tok::kw_s8:    return TYPE_S8;
    case tok::kw_s16:   return TYPE_S16;
    case tok::kw_s32:   return TYPE_S32;
    case tok::kw_int:   return TYPE_INT;
    case tok::kw_uint:  return TYPE_U32;
    case tok::kw_string: return TYPE_STRING;
    case tok::kw_float: return TYPE_FLOAT;
    case tok::kw_char:  return TYPE_CHAR;
    case tok::kw_void:  return TYPE_VOID;
    case tok::kw_uchar: return TYPE_U8;
    default:
        assert(0);
        return TYPE_U8;
    }
}

// TEMP
static SourceRange getExprRange(C2::Expr *E) {
    return E ? E->getSourceRange() : SourceRange();
}


C2Parser::C2Parser(Preprocessor& pp, C2Sema& sema)
    : PP(pp)
    , ParenCount(0)
    , BracketCount(0)
    , BraceCount(0)
    , Actions(sema)
    , Diags(PP.getDiagnostics())
{
    Tok.startToken();
    Tok.setKind(tok::eof);
}

C2Parser::~C2Parser() {}

void C2Parser::Initialize() {
    // Prime the lexer look-ahead.
    ConsumeToken();
}

bool C2Parser::Parse() {
    LOG_FUNC
    ParsePackage();
    if (Diags.hasErrorOccurred()) return false;

    ParseUses();
    if (Diags.hasErrorOccurred()) return false;

    bool done = false;
    while (!done) {
        done = ParseTopLevel();
        if (Diags.hasErrorOccurred()) return false;
    }
    return true;
}

void C2Parser::ParsePackage() {
    LOG_FUNC
    if (ExpectAndConsume(tok::kw_package, diag::err_expected_package)) return;
    if (ExpectIdentifier()) return;

    IdentifierInfo* Pkg = Tok.getIdentifierInfo();
    SourceLocation PkgLoc = ConsumeToken();


    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "package name")) return;

    Actions.ActOnPackage(Pkg->getNameStart(), PkgLoc);
}

void C2Parser::ParseUses() {
    LOG_FUNC
    while (1) {
        if (Tok.isNot(tok::kw_use)) break;
        // Syntax: use <identifier>
        ConsumeToken();
        if (ExpectIdentifier()) return;
        IdentifierInfo* Pkg = Tok.getIdentifierInfo();
        SourceLocation PkgLoc = ConsumeToken();
        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "package name")) return;

        Actions.ActOnUse(Pkg->getNameStart(), PkgLoc);
    }
    // check for 'using' instead of 'use' mistake
    if (Tok.is(tok::kw_using)) {
        Diag(Tok, diag::err_using_not_use);
    }
}

/// ParseTopLevelDef - Parse one top-level declaration, return whatever the
/// action tells us to.  This returns true if the EOF was encountered.
/*
    Syntax:
        <public> type ..
        <public> func ..
        <public> type_qualifier ..
        identifier += init_value
*/
bool C2Parser::ParseTopLevel() {
    LOG_FUNC

    if (Tok.getKind() == tok::identifier && NextToken().getKind() == tok::plusequal) {
        ParseArrayEntry();
        return false;
    }
    bool is_public = ParseOptionalAccessSpecifier();
    switch (Tok.getKind()) {
    case tok::eof:
        return true;
    case tok::kw_type:
        ParseTypeDef(is_public);
        break;
    case tok::kw_func:
        ParseFuncDef(is_public);
        break;
    default:
        ParseVarDef(is_public);
        break;
    }
    return false;
}

/*
type_def ::= TYPE IDENTIFIER type_qualifier type_specifier.
type_def ::= TYPE IDENTIFIER func_type.
type_def ::= TYPE IDENTIFIER STRUCT LBRACE struct_block RBRACE.
type_def ::= TYPE IDENTIFIER UNION LBRACE struct_block RBRACE.
type_def ::= TYPE IDENTIFIER ENUM LBRACE enum_block RBRACE.
*/
void C2Parser::ParseTypeDef(bool is_public) {
    LOG_FUNC
    assert(Tok.is(tok::kw_type) && "Expected type keyword");
    ConsumeToken();

    if (ExpectIdentifier()) return;
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();

    ExprResult type;
    switch (Tok.getKind()) {
    case tok::kw_func:
        ParseFuncType(id, idLoc, is_public);
        break;
    case tok::kw_struct:
        ConsumeToken();
        type = ParseStructBlock(true);
        break;
    case tok::kw_union:
        ConsumeToken();
        type = ParseStructBlock(false);
        break;
    case tok::kw_enum:
        ParseEnumType(id, idLoc, is_public);
        break;
    default:
        type = ParseTypeSpecifier(true);
        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "type definition")) return;
        break;
    }
    if (!type.isUsable()) return;
    Actions.ActOnTypeDef(id->getNameStart(), idLoc, type.release(), is_public);
}

// Syntax: { <struct_block> } etc
C2::ExprResult C2Parser::ParseStructBlock(bool is_struct) {
    LOG_FUNC
    SourceLocation LeftBrace = Tok.getLocation();
    if (ExpectAndConsume(tok::l_brace, diag::err_expected_lbrace)) return ExprError();

    ExprList members;
    // NOTE: memleak on members
    while (1) {
        if (Tok.is(tok::r_brace)) break;
        ExprResult Member = ParseStructMember();
        if (Member.isInvalid()) return ExprError();
        members.push_back(Member.release());
    }
    SourceLocation RightBrace = Tok.getLocation();
    if (ExpectAndConsume(tok::r_brace, diag::err_expected_rbrace)) return ExprError();

    return Actions.ActOnStructType(LeftBrace, RightBrace, members, is_struct);
}

/*
   Syntax:
    struct_member ::= type_qualifier type_specifier IDENTIFIER SEMICOLON.
    struct_member ::= UNION LBRACE struct_block RBRACE IDENTIFIER SEMICOLON.
*/
C2::ExprResult C2Parser::ParseStructMember() {
    LOG_FUNC
    if (Tok.is(tok::kw_union)) {
        ConsumeToken();
        ParseStructBlock(false);
        if (Diags.hasErrorOccurred()) return ExprError();

        if (ExpectIdentifier()) return ExprError();
        IdentifierInfo* id = Tok.getIdentifierInfo();
        SourceLocation idLoc = ConsumeToken();

        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "member")) return ExprError();
        // TODO
        assert(0 && "TODO");
    } else {
        ExprResult type = ParseTypeSpecifier(true);
        if (type.isInvalid()) return ExprError();

        if (ExpectIdentifier()) return ExprError();
        IdentifierInfo* id = Tok.getIdentifierInfo();
        SourceLocation idLoc = ConsumeToken();

        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "member")) return ExprError();
        return Actions.ActOnVarExpr(id->getNameStart(), idLoc, type.release(), 0);
    }
    // TODO handle (anonymous) sub-structs
}

/*
   Syntax:
    type_def ::= access_specifier TYPE IDENTIFIER ENUM LBRACE enum_block RBRACE SEMICOLON.

    enum_block   ::= enum_block COMMA enum_member.
    enum_block   ::= enum_block COMMA.
    enum_block   ::= enum_member.

    enum_member ::= IDENTIFIER.
    enum_member ::= IDENTIFIER EQUALS constant_expression.
*/
void C2Parser::ParseEnumType(IdentifierInfo* id, SourceLocation& idLoc, bool is_public) {
    LOG_FUNC
    assert(Tok.is(tok::kw_enum) && "Expected keyword 'enum'");
    ConsumeToken();
    if (ExpectAndConsume(tok::l_brace, diag::err_expected_lbrace)) return;

    // Syntax: enum_block
    while (1) {
        // Q: handle eof here?
        if (Tok.is(tok::r_brace)) break;

        ParseEnumMember();
        if (Diags.hasErrorOccurred()) return;
        if (Tok.is(tok::comma)) {
            ConsumeToken();
        } else {
            break;
        }
    }
    if (ExpectAndConsume(tok::r_brace, diag::err_expected_rbrace)) return;
    //ConsumeOptionalSemi();
}

/*
   Syntax:
    enum_member ::= IDENTIFIER.
    enum_member ::= IDENTIFIER EQUALS constant_expression.
*/
void C2Parser::ParseEnumMember() {
    LOG_FUNC
    if (ExpectIdentifier()) return;
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();
    if (Tok.is(tok::equal)) {
        ConsumeToken();
        ParseConstantExpression();
        if (Diags.hasErrorOccurred()) return;
    }
}

/*
   Systax:
    func_type ::= FUNC type_qualifier single_type_specifier LPAREN full_param_list RPAREN.
*/
void C2Parser::ParseFuncType(IdentifierInfo* id, SourceLocation& idLoc, bool is_public) {
    LOG_FUNC
    assert(Tok.is(tok::kw_func) && "Expected keyword 'func'");
    ConsumeToken();

    ParseSingleTypeSpecifier(true);
    if (Diags.hasErrorOccurred()) return;

    ExprList params;
    // NOTE: memleak on params
    if (!ParseFullParamList(params, false)) return;
    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "type definition")) return;
}

/*
   Syntax:
    full_param_list ::= .
    full_param_list ::= param_list.
    full_param_list ::= param_list COMMA ELLIPSIS.

    param_list ::= param_declaration.
    param_list ::= param_list COMMA param_declaration.
*/
bool C2Parser::ParseFullParamList(ExprList& results, bool allow_defaults) {
    LOG_FUNC
    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen)) return false;
    // fast path for "()"
    if (Tok.is(tok::r_paren)) {
        ConsumeToken();
        return true;
    }

    while (1) {
        ExprResult Param = ParseParamDecl(allow_defaults);
        if (Param.isInvalid()) return false;
        results.push_back(Param.release());

        // Syntax: param_decl, param_decl
        if (Tok.is(tok::comma)) {
            ConsumeToken();
        } else {
            break;
        }
        // Syntax:: param_decl, ...
        if (Tok.is(tok::ellipsis)) {
            ConsumeToken();
            // TODO add Expr
            break;
        }
    }

    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return false;
    return true;
}

/*
   Syntax:
    param_declaration ::= type_qualifier type_specifier IDENTIFIER param_default.
    param_default ::= EQUALS constant_expression.
*/
C2::ExprResult C2Parser::ParseParamDecl(bool allow_defaults) {
    LOG_FUNC

    ExprResult type = ParseTypeSpecifier(true);
    if (Diags.hasErrorOccurred()) return ExprError();

    if (ExpectIdentifier()) return ExprError();
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();

    ExprResult InitValue;
    if (Tok.is(tok::equal)) {
        if (!allow_defaults) {
            Diag(Tok, diag::err_param_default_argument_nonfunc);
            return ExprError();
        }
        ConsumeToken();
        InitValue = ParseConstantExpression();
        if (InitValue.isInvalid()) return ExprError();
    }
    return Actions.ActOnVarExpr(id->getNameStart(), idLoc, type.release(), InitValue.release());
}

/*
    Syntax:
    type_specifier ::= single_type_specifier.
    type_specifier ::= type_specifier array_specifier.
*/
C2::ExprResult C2Parser::ParseSingleTypeSpecifier(bool allow_qualifier) {
    LOG_FUNC

    unsigned int type_qualifier = 0;
    // TODO also parse if not allowed for error msg
    if (allow_qualifier) type_qualifier = ParseOptionalTypeQualifier();

    ExprResult base;
    // first part is always a base type or identifier(::identifier)
    switch (Tok.getKind()) {
    case tok::kw_u8:
    case tok::kw_u16:
    case tok::kw_u32:
    case tok::kw_s8:
    case tok::kw_s16:
    case tok::kw_s32:
    case tok::kw_int:
    case tok::kw_uint:
    case tok::kw_string:
    case tok::kw_float:
    case tok::kw_char:
    case tok::kw_void:
    case tok::kw_uchar:
        base = Actions.ActOnBuiltinType(kw2type(Tok.getKind()));
        ConsumeToken();
        break;
    case tok::identifier:
        {
            ExprResult Res = ParseIdentifier(true);
            //base = new Type(0, Type::USER);
            //base->setUserType(Res.release());
            base = Actions.ActOnUserType(Res.release());
        }
        break;
    default:
        Diag(Tok, diag::err_expected_type_spec);
        return ExprError();
    }
    // Syntax: pointer type
    while (Tok.is(tok::star)) {
        base = Actions.ActOnPointerType(base.release());
        ConsumeToken();
    }
    return Actions.ActOnTypeQualifier(base, type_qualifier);
}

// Syntax: TODO
C2::ExprResult C2Parser::ParseTypeSpecifier(bool allow_qualifier) {
    LOG_FUNC
    ExprResult type = ParseSingleTypeSpecifier(allow_qualifier);
    if (type.isInvalid()) return ExprError();

    // Syntax: array types
    while (Tok.is(tok::l_square)) {
        type = ParseArray(type);
        if (type.isInvalid()) return ExprError();
    }

    return type;
}

C2::ExprResult C2Parser::ParseExpression(TypeCastState isTypeCast) {
    LOG_FUNC
    ExprResult LHS(ParseAssignmentExpression(isTypeCast));
    return ParseRHSOfBinaryExpression(LHS, prec::Comma);
}

C2::ExprResult C2Parser::ParseAssignmentExpression(TypeCastState isTypeCast) {
    LOG_FUNC
    ExprResult LHS = ParseCastExpression(/*isUnaryExpression=*/false,
                                         /*isAddressOfOperand=*/false,
                                         isTypeCast);
    return ParseRHSOfBinaryExpression(LHS, prec::Assignment);
}

C2::ExprResult C2Parser::ParseRHSOfBinaryExpression(ExprResult LHS, prec::Level MinPrec) {
    LOG_FUNC
    prec::Level NextTokPrec = getBinOpPrecedence(Tok.getKind());

    SourceLocation ColonLoc;

  while (1) {
    // If this token has a lower precedence than we are allowed to parse (e.g.
    // because we are called recursively, or because the token is not a binop),
    // then we are done!
    if (NextTokPrec < MinPrec) return LHS;

    // Consume the operator, saving the operator token for error reporting.
    Token OpToken = Tok;
    ConsumeToken();

    // Bail out when encountering a comma followed by a token which can't
    // possibly be the start of an expression. For instance:
    //   int f() { return 1, }
    // We can't do this before consuming the comma, because
    // isNotExpressionStart() looks at the token stream.
#if 0
    // BB: C2 doesn't have this??
    if (OpToken.is(tok::comma) && isNotExpressionStart()) {
      PP.EnterToken(Tok);
      Tok = OpToken;
      return LHS;
    }
#endif
#if 0
    // Special case handling for the ternary operator.
    ExprResult TernaryMiddle(true);
    if (NextTokPrec == prec::Conditional) {
      if (Tok.isNot(tok::colon)) {
        // Don't parse FOO:BAR as if it were a typo for FOO::BAR.
        ColonProtectionRAIIObject X(*this);

        // Handle this production specially:
        //   logical-OR-expression '?' expression ':' conditional-expression
        // In particular, the RHS of the '?' is 'expression', not
        // 'logical-OR-expression' as we might expect.
        TernaryMiddle = ParseExpression();
        if (TernaryMiddle.isInvalid()) {
          LHS = ExprError();
          TernaryMiddle = 0;
        }
      } else {
        // Special case handling of "X ? Y : Z" where Y is empty:
        //   logical-OR-expression '?' ':' conditional-expression   [GNU]
        TernaryMiddle = 0;
        Diag(Tok, diag::ext_gnu_conditional_expr);
      }

      if (Tok.is(tok::colon)) {
        // Eat the colon.
        ColonLoc = ConsumeToken();
      } else {
        // Otherwise, we're missing a ':'.  Assume that this was a typo that
        // the user forgot. If we're not in a macro expansion, we can suggest
        // a fixit hint. If there were two spaces before the current token,
        // suggest inserting the colon in between them, otherwise insert ": ".
        SourceLocation FILoc = Tok.getLocation();
        const char *FIText = ": ";
        const SourceManager &SM = PP.getSourceManager();
        if (FILoc.isFileID() || PP.isAtStartOfMacroExpansion(FILoc, &FILoc)) {
          assert(FILoc.isFileID());
          bool IsInvalid = false;
          const char *SourcePtr =
            SM.getCharacterData(FILoc.getLocWithOffset(-1), &IsInvalid);
          if (!IsInvalid && *SourcePtr == ' ') {
            SourcePtr =
              SM.getCharacterData(FILoc.getLocWithOffset(-2), &IsInvalid);
            if (!IsInvalid && *SourcePtr == ' ') {
              FILoc = FILoc.getLocWithOffset(-1);
              FIText = ":";
            }
          }
        }

        Diag(Tok, diag::err_expected_colon)
          << FixItHint::CreateInsertion(FILoc, FIText);
        Diag(OpToken, diag::note_matching) << "?";
        ColonLoc = Tok.getLocation();
      }
    }
#endif

#if 0
    // Code completion for the right-hand side of an assignment expression
    // goes through a special hook that takes the left-hand side into account.
    if (Tok.is(tok::code_completion) && NextTokPrec == prec::Assignment) {
      Actions.CodeCompleteAssignmentRHS(getCurScope(), LHS.get());
      cutOffParsing();
      return ExprError();
    }
#endif

    // Parse another leaf here for the RHS of the operator.
    // ParseCastExpression works here because all RHS expressions in C have it
    // as a prefix, at least. However, in C++, an assignment-expression could
    // be a throw-expression, which is not a valid cast-expression.
    // Therefore we need some special-casing here.
    // Also note that the third operand of the conditional operator is
    // an assignment-expression in C++, and in C++11, we can have a
    // braced-init-list on the RHS of an assignment. For better diagnostics,
    // parse as if we were allowed braced-init-lists everywhere, and check that
    // they only appear on the RHS of assignments later.
    ExprResult RHS;
    bool RHSIsInitList = false;
#if 0
    if (getLangOpts().CPlusPlus0x && Tok.is(tok::l_brace)) {
      RHS = ParseBraceInitializer();
      RHSIsInitList = true;
    } else if (getLangOpts().CPlusPlus && NextTokPrec <= prec::Conditional)
      RHS = ParseAssignmentExpression();
    else
#endif
      RHS = ParseCastExpression(false);

    if (RHS.isInvalid())
      LHS = ExprError();

    // Remember the precedence of this operator and get the precedence of the
    // operator immediately to the right of the RHS.
    prec::Level ThisPrec = NextTokPrec;
    NextTokPrec = getBinOpPrecedence(Tok.getKind());

    // Assignment and conditional expressions are right-associative.
    bool isRightAssoc = ThisPrec == prec::Conditional ||
                        ThisPrec == prec::Assignment;

    // Get the precedence of the operator to the right of the RHS.  If it binds
    // more tightly with RHS than we do, evaluate it completely first.
    if (ThisPrec < NextTokPrec ||
        (ThisPrec == NextTokPrec && isRightAssoc)) {
      if (!RHS.isInvalid() && RHSIsInitList) {
        Diag(Tok, diag::err_init_list_bin_op)
          << /*LHS*/0 << PP.getSpelling(Tok) << getExprRange(RHS.get());
        RHS = ExprError();
      }
      // If this is left-associative, only parse things on the RHS that bind
      // more tightly than the current operator.  If it is left-associative, it
      // is okay, to bind exactly as tightly.  For example, compile A=B=C=D as
      // A=(B=(C=D)), where each paren is a level of recursion here.
      // The function takes ownership of the RHS.
      RHS = ParseRHSOfBinaryExpression(RHS,
                            static_cast<prec::Level>(ThisPrec + !isRightAssoc));
      RHSIsInitList = false;

      if (RHS.isInvalid())
        LHS = ExprError();

      NextTokPrec = getBinOpPrecedence(Tok.getKind());
    }
    assert(NextTokPrec <= ThisPrec && "Recursion didn't work!");

    if (!RHS.isInvalid() && RHSIsInitList) {
      if (ThisPrec == prec::Assignment) {
        Diag(OpToken, diag::warn_cxx98_compat_generalized_initializer_lists)
          << getExprRange(RHS.get());
      } else {
        Diag(OpToken, diag::err_init_list_bin_op)
          << /*RHS*/1 << PP.getSpelling(OpToken)
          << getExprRange(RHS.get());
        LHS = ExprError();
      }
    }

    if (!LHS.isInvalid()) {
        LHS = Actions.ActOnBinOp(OpToken.getLocation(), OpToken.getKind(), LHS.take(), RHS.take());
    }
#if 0
    if (!LHS.isInvalid()) {
      // Combine the LHS and RHS into the LHS (e.g. build AST).
      if (TernaryMiddle.isInvalid()) {
        LHS = Actions.ActOnBinOp(getCurScope(), OpToken.getLocation(),
                                 OpToken.getKind(), LHS.take(), RHS.take());
      } else
        LHS = Actions.ActOnConditionalOp(OpToken.getLocation(), ColonLoc,
                                         LHS.take(), TernaryMiddle.take(),
                                         RHS.take());
    }
#endif
  }
  // TODO add return value?
}

C2::ExprResult C2Parser::ParseCastExpression(bool isUnaryExpression,
                         bool isAddressOfOperand,
                         bool &NotCastExpr,
                         TypeCastState isTypeCast) {
    LOG_FUNC
    ExprResult Res;
    tok::TokenKind SavedKind = Tok.getKind();
    NotCastExpr = false;

  // This handles all of cast-expression, unary-expression, postfix-expression,
  // and primary-expression.  We handle them together like this for efficiency
  // and to simplify handling of an expression starting with a '(' token: which
  // may be one of a parenthesized expression, cast-expression, compound literal
  // expression, or statement expression.
  //
  // If the parsed tokens consist of a primary-expression, the cases below
  // break out of the switch;  at the end we call ParsePostfixExpressionSuffix
  // to handle the postfix expression suffixes.  Cases that cannot be followed
  // by postfix exprs should return without invoking
  // ParsePostfixExpressionSuffix.
    switch (SavedKind) {
    case tok::l_paren:
        {
            ParenParseOption ParenExprType = (isUnaryExpression ? CompoundLiteral : CastExpr);
            SourceLocation RParenLoc;
            Res = ParseParenExpression(ParenExprType, false/*stopIfCastExpr*/,
                    isTypeCast == IsTypeCast, RParenLoc);
            switch (ParenExprType) {
            case SimpleExpr: break; // Nothing else to do
            case CompoundStmt: break;   // Nothing else to do
            case CompoundLiteral:
              // We parsed '(' type-name ')' '{' ... '}'.  If any suffixes of
              // postfix-expression exist, parse them now.
              break;
            case CastExpr:
              // We have parsed the cast-expression and no postfix-expr pieces are
              // following.
              return Res;
            }
            break;
        }
    case tok::numeric_constant:
        Res = Actions.ActOnNumericConstant(Tok);
        ConsumeToken();
        break;
    case tok::identifier:
        {                      // primary-expression: identifier
                               // unqualified-id: identifier
                               // constant: enumeration-constant
            Res = ParseIdentifier(true);

            // Make sure to pass down the right value for isAddressOfOperand.
            if (isAddressOfOperand && isPostfixExpressionSuffixStart())
                isAddressOfOperand = false;
            break;
        }
    case tok::string_literal:   // primary-expression: string-literal
        Res = ParseStringLiteralExpression(true);
        break;
    case tok::char_constant:    // constant: character-constant
        //Res = //TODO
        ConsumeToken();
        break;
    case tok::kw___func__:
        ConsumeToken();
        break;
    case tok::kw_NULL:
        ConsumeToken();
        break;
    case tok::kw_sizeof:
        return ParseSizeof();
    case tok::star:          // unary-expression: '*' cast-expression
    case tok::plus:          // unary-expression: '+' cast-expression
    case tok::minus:         // unary-expression: '-' cast-expression
    case tok::tilde:         // unary-expression: '~' cast-expression
    case tok::exclaim:       // unary-expression: '!' cast-expression
    {
        SourceLocation SavedLoc = ConsumeToken();
        Res = ParseCastExpression(false);
        //if (!Res.isInvalid())
        //    Res = Actions.ActOnUnaryOp(getCurScope(), SavedLoc, SavedKind, Res.get());
        return Res;
    }
    default:
        std::cerr << "UNHANDLED TOKEN: ";
        PP.DumpToken(Tok);
        std::cerr << std::endl;
        assert(0 && "TODO");
    }

    return ParsePostfixExpressionSuffix(Res);
}

C2::ExprResult C2Parser::ParseCastExpression(bool isUnaryExpression,
                         bool isAddressOfOperand,
                         TypeCastState isTypeCast) {
    //LOG_FUNC
    bool NotCastExpr = true;
    ExprResult Res = ParseCastExpression(isUnaryExpression,
                                         isAddressOfOperand,
                                         NotCastExpr,
                                         isTypeCast);
    if (NotCastExpr) Diag(Tok, diag::err_expected_expression);
    return Res;
}

/// ParseStringLiteralExpression - This handles the various token types that
/// form string literals, and also handles string concatenation [C99 5.1.1.2,
/// translation phase #6].
///
/// \verbatim
///       primary-expression: [C99 6.5.1]
///         string-literal
/// \verbatim
C2::ExprResult C2Parser::ParseStringLiteralExpression(bool AllowUserDefinedLiteral) {
    LOG_FUNC
  assert(isTokenStringLiteral() && "Not a string literal!");

  // String concat.  Note that keywords like __func__ and __FUNCTION__ are not
  // considered to be strings for concatenation purposes.
  SmallVector<Token, 4> StringToks;

  do {
    StringToks.push_back(Tok);
    ConsumeStringToken();
  } while (isTokenStringLiteral());

  return Actions.ActOnStringLiteral(&StringToks[0], StringToks.size());
}

/// \brief Once the leading part of a postfix-expression is parsed, this
/// method parses any suffixes that apply.
///
/// \verbatim
///       postfix-expression: [C99 6.5.2]
///         primary-expression
///         postfix-expression '[' expression ']'
///         postfix-expression '[' braced-init-list ']'
///         postfix-expression '(' argument-expression-list[opt] ')'
///         postfix-expression '.' identifier
///         postfix-expression '->' identifier
///         postfix-expression '++'
///         postfix-expression '--'
///         '(' type-name ')' '{' initializer-list '}'
///         '(' type-name ')' '{' initializer-list ',' '}'
///
///       argument-expression-list: [C99 6.5.2]
///         argument-expression ...[opt]
///         argument-expression-list ',' assignment-expression ...[opt]
/// \endverbatim
C2::ExprResult C2Parser::ParsePostfixExpressionSuffix(ExprResult LHS) {
    LOG_FUNC
  // Now that the primary-expression piece of the postfix-expression has been
  // parsed, see if there are any postfix-expression pieces here.
  SourceLocation Loc;
  while (1) {
    switch (Tok.getKind()) {
    case tok::code_completion:
        assert(0);
#if 0
      if (InMessageExpression)
        return LHS;

      Actions.CodeCompletePostfixExpression(getCurScope(), LHS);
      cutOffParsing();
      return ExprError();
#endif
    case tok::identifier:
      // Fall through; this isn't a message send.
    default:  // Not a postfix-expression suffix.
      return LHS;
    case tok::l_square: {  // postfix-expression: p-e '[' expression ']'
        // Basic version
        ConsumeToken();
        ExprResult Idx = ParseExpression();
        if (Idx.isInvalid()) return ExprError();
        if (Tok.isNot(tok::r_square)) {
            Diag(Tok, diag::err_expected_rsquare);
            return ExprError();
        }
        SourceLocation RLoc = ConsumeToken();
        return Actions.ActOnArraySubScriptExpr(RLoc, LHS.release(), Idx.release());
#if 0
      // Reject array indices starting with a lambda-expression. '[[' is
      // reserved for attributes.
      if (CheckProhibitedCXX11Attribute())
        return ExprError();

      BalancedDelimiterTracker T(*this, tok::l_square);
      T.consumeOpen();
      Loc = T.getOpenLocation();
      ExprResult Idx;
      if (getLangOpts().CPlusPlus0x && Tok.is(tok::l_brace)) {
        Diag(Tok, diag::warn_cxx98_compat_generalized_initializer_lists);
        Idx = ParseBraceInitializer();
      } else
        Idx = ParseExpression();

      SourceLocation RLoc = Tok.getLocation();

      if (!LHS.isInvalid() && !Idx.isInvalid() && Tok.is(tok::r_square)) {
        LHS = Actions.ActOnArraySubscriptExpr(getCurScope(), LHS.take(), Loc,
                                              Idx.take(), RLoc);
      } else
        LHS = ExprError();

      // Match the ']'.
      T.consumeClose();
#endif
      break;
    }

    case tok::l_paren:         // p-e: p-e '(' argument-expression-list[opt] ')'
    //case tok::lesslessless:  // p-e: p-e '<<<' argument-expression-list '>>>'
                               //   '(' argument-expression-list[opt] ')'
    {
      tok::TokenKind OpKind = Tok.getKind();
      //InMessageExpressionRAIIObject InMessage(*this, false);

        // TODO ParseExpr.cpp:2562
      //BalancedDelimiterTracker PT(*this, tok::l_paren);
      //PT.consumeOpen();
      ///Loc = PT.getOpenLocation();
      Loc = ConsumeToken();

      ExprVector ArgExprs;
      CommaLocsTy CommaLocs;

      if (OpKind == tok::l_paren || !LHS.isInvalid()) {
        if (Tok.isNot(tok::r_paren)) {
          if (ParseExpressionList(ArgExprs, CommaLocs)) {
            LHS = ExprError();
          }
        }
      }

      // Match the ')'.
      if (LHS.isInvalid()) {
        SkipUntil(tok::r_paren);
      } else if (Tok.isNot(tok::r_paren)) {
        //PT.consumeClose();
        LHS = ExprError();
      } else {
        assert((ArgExprs.size() == 0 ||
                ArgExprs.size()-1 == CommaLocs.size())&&
               "Unexpected number of commas!");
        if (ArgExprs.size() == 0)
            LHS = Actions.ActOnCallExpr(LHS.take(), 0, 0, Tok.getLocation());
        else
            LHS = Actions.ActOnCallExpr(LHS.take(), &ArgExprs[0], ArgExprs.size(), Tok.getLocation());
        //LHS = Actions.ActOnCallExpr(getCurScope(), LHS.take(), Loc,
        //                            ArgExprs, Tok.getLocation(),
        //                            ExecConfig);
        //PT.consumeClose();
        if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return ExprError();
      }

      break;
    }
    case tok::arrow:
    case tok::period: {
      // postfix-expression: p-e '->' template[opt] id-expression
      // postfix-expression: p-e '.' template[opt] id-expression
      tok::TokenKind OpKind = Tok.getKind();
      SourceLocation OpLoc = ConsumeToken();  // Eat the "." or "->" token.

      CXXScopeSpec SS;
      ParsedType ObjectType;

      if (Tok.is(tok::code_completion)) {
        // Code completion for a member access expression.
        //Actions.CodeCompleteMemberReferenceExpr(getCurScope(), LHS.get(),
        //                                        OpLoc, OpKind == tok::arrow);
        //cutOffParsing();
        return ExprError();
      }

      // Either the action has told is that this cannot be a
      // pseudo-destructor expression (based on the type of base
      // expression), or we didn't see a '~' in the right place. We
      // can still parse a destructor name here, but in that case it
      // names a real destructor.
      // Allow explicit constructor calls in Microsoft mode.
      SourceLocation TemplateKWLoc;
      UnqualifiedId Name;
      // TODO handle return value
      ParseUnqualifiedId(SS,
                            /*EnteringContext=*/false,
                            /*AllowDestructorName=*/true,
                            /*AllowConstructorName=*/false,
                            ObjectType, TemplateKWLoc, Name);
        LHS = ExprError();

#if 0
      if (!LHS.isInvalid())
        LHS = Actions.ActOnMemberAccessExpr(getCurScope(), LHS.take(), OpLoc,
                                            OpKind, SS, TemplateKWLoc, Name,
                                 CurParsedObjCImpl ? CurParsedObjCImpl->Dcl : 0,
                                            Tok.is(tok::l_paren));
#endif
      break;
    }
    case tok::plusplus:    // postfix-expression: postfix-expression '++'
    case tok::minusminus:  // postfix-expression: postfix-expression '--'
#if 0
      if (!LHS.isInvalid()) {
        LHS = Actions.ActOnPostfixUnaryOp(getCurScope(), Tok.getLocation(),
                                          Tok.getKind(), LHS.take());
      }
#endif
      ConsumeToken();
      break;
    }
  }
}

/// For C2, only identifiers are allowed
/// \returns true if parsing fails, false otherwise.
bool C2Parser::ParseUnqualifiedId(CXXScopeSpec &SS, bool EnteringContext,
                                bool AllowDestructorName,
                                bool AllowConstructorName,
                                ParsedType ObjectType,
                                SourceLocation& TemplateKWLoc,
                                UnqualifiedId &Result) {
    LOG_FUNC

  // unqualified-id:
  //   identifier
  //   template-id (when it hasn't already been annotated)
  if (Tok.is(tok::identifier)) {
    // Consume the identifier.
    IdentifierInfo *Id = Tok.getIdentifierInfo();
    SourceLocation IdLoc = ConsumeToken();

    // Only allow identifier
      Result.setIdentifier(Id, IdLoc);
      return false;
  }

  Diag(Tok, diag::err_expected_unqualified_id);
  return true;
}


/// ParseExpressionList - Used for C/C++ (argument-)expression-list.
///
/// \verbatim
///       argument-expression-list:
///         assignment-expression
///         argument-expression-list , assignment-expression
///
/// [C++] expression-list:
/// [C++]   assignment-expression
/// [C++]   expression-list , assignment-expression
///
/// [C++0x] expression-list:
/// [C++0x]   initializer-list
///
/// [C++0x] initializer-list
/// [C++0x]   initializer-clause ...[opt]
/// [C++0x]   initializer-list , initializer-clause ...[opt]
///
/// [C++0x] initializer-clause:
/// [C++0x]   assignment-expression
/// [C++0x]   braced-init-list
/// \endverbatim
bool C2Parser::ParseExpressionList(SmallVectorImpl<Expr*> &Exprs,
                                   SmallVectorImpl<SourceLocation> &CommaLocs)
{
    LOG_FUNC
  while (1) {
    ExprResult Expr;
    Expr = ParseAssignmentExpression();

    if (Tok.is(tok::ellipsis)) {
        ConsumeToken();
        //Expr = Actions.ActOnPackExpansion(Expr.get(), ConsumeToken());
    }
    if (Expr.isInvalid())
      return true;

    Exprs.push_back(Expr.release());

    if (Tok.isNot(tok::comma))
      return false;
    // Move to the next argument, remember where the comma was.
    CommaLocs.push_back(ConsumeToken());
  }
}

/// ParseParenExpression - This parses the unit that starts with a '(' token,
/// based on what is allowed by ExprType.  The actual thing parsed is returned
/// in ExprType. If stopIfCastExpr is true, it will only return the parsed type,
/// not the parsed cast-expression.
///
/// \verbatim
///       primary-expression: [C99 6.5.1]
///         '(' expression ')'
/// [GNU]   '(' compound-statement ')'      (if !ParenExprOnly)
///       postfix-expression: [C99 6.5.2]
///         '(' type-name ')' '{' initializer-list '}'
///         '(' type-name ')' '{' initializer-list ',' '}'
///       cast-expression: [C99 6.5.4]
///         '(' type-name ')' cast-expression
/// \endverbatim
C2::ExprResult
C2Parser::ParseParenExpression(ParenParseOption &ExprType, bool stopIfCastExpr,
                             bool isTypeCast, SourceLocation &RParenLoc)
{
    LOG_FUNC
    assert(Tok.is(tok::l_paren) && "Not a paren expr!");
    ConsumeToken();

    ExprResult Result(true);

    // None of these cases should fall through with an invalid Result
    // unless they've already reported an error.
    if (ExprType >= CompoundStmt && Tok.is(tok::l_brace)) {
        assert(0 && "TODO");
#if 0
        Diag(Tok, diag::ext_gnu_statement_expr);
        Actions.ActOnStartStmtExpr();

        StmtResult Stmt(ParseCompoundStatement(true));
        ExprType = CompoundStmt;

        // If the substmt parsed correctly, build the AST node.
        if (!Stmt.isInvalid()) {
            Result = Actions.ActOnStmtExpr(OpenLoc, Stmt.take(), Tok.getLocation());
        } else {
            Actions.ActOnStmtExprError();
        }
#endif
    } else if (ExprType >= CompoundLiteral && isDeclaration()) {
        // Otherwise, this is a compound literal expression or cast expression.
        ParseTypeSpecifier(true);
        if (Diags.hasErrorOccurred()) return ExprError();

        RParenLoc = Tok.getLocation();
        if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return ExprError();

        if (Tok.is(tok::l_brace)) {
            assert(0 && "TODO");
        }
        if (ExprType == CastExpr) {
            // We parsed '(' type-name ')' and the thing after it wasn't a '{'.

            if (stopIfCastExpr) {
                assert(0 && "TODO");
            }

            // Parse the cast-expression that follows it next.
            // TODO: For cast expression with CastTy.
            Result = ParseCastExpression(/*isUnaryExpression=*/false,
                                         /*isAddressOfOperand=*/false,
                                         /*isTypeCast=*/IsTypeCast);
            return Result;
        }
        Diag(Tok, diag::err_expected_lbrace_in_compound_literal);
        return ExprError();
    } else if (isTypeCast) {
        assert(0 && "TODO");
    } else {
        assert(0 && "TODO");
    }

    // Match the ')'.
    if (Result.isInvalid()) {
        SkipUntil(tok::r_paren);
        return ExprError();
    }

    RParenLoc = Tok.getLocation();
    ExpectAndConsume(tok::r_paren, diag::err_expected_rparen);

    return Result;
}

/*
    Declarations (type + name)
        bla* bliep <init> -> yes
        bla[] bliep <init> -> yes
        bla*[] bliep <init> -> yes
        bla bliep <init> -> yes
    Assignments/Function calls
        bla = ..     -> no
        bla *= .. etc -> no
        bla() -> no
        bla[10] = .. -> no
    // NOTE: Tok is first identifier
*/

bool C2Parser::isTypeSpec() {
    assert(Tok.is(tok::identifier) && "Not an identifier!");

    int lookahead = 1;  // skip first identifier
    const Token& tok2 = GetLookAheadToken(lookahead);
    if (tok2.is(tok::coloncolon)) {
        lookahead++;
        const Token& tok3 = GetLookAheadToken(lookahead);
        if (tok3.isNot(tok::identifier)) {
            // syntax error
            assert(0 && "TODO handle syntax error");
            return false;
        }
        lookahead++;
    }

    int state = 0;  // 0 = Number, 1 = pointers, 2 = arrays
    while (1) {
        const Token& t2 = GetLookAheadToken(lookahead);
        switch (t2.getKind()) {
        case tok::identifier:
            if (state == 0) return true;    // Number num
            return true;        // Number num
        case tok::star:     // Number* num
            if (state == 2) return false; // syntax error?
            state = 1;
            lookahead++;
            break;
        case tok::l_square:     // Number[] num
        {
            lookahead = SkipArray(lookahead);
            if (Diags.hasErrorOccurred()) return false;
            state = 2;
            break;
        }
        case tok::equal:
            if (state == 0) return false;     // 'foo =' -> assignment
            if (state == 1) {
                Diag(t2, diag::err_expected_expression);
                return false;            // 'foo*' -> error
            }
            if (state == 2) return false;            // 'foo[..] =' -> assignment
        default:
            return false;
        }
    }
    return false;
}

bool C2Parser::isDeclaration() {
    switch (Tok.getKind()) {
    case tok::identifier:
        return isTypeSpec();
    // all basic types
    case tok::kw_u8:
    case tok::kw_u16:
    case tok::kw_u32:
    case tok::kw_s8:
    case tok::kw_s16:
    case tok::kw_s32:
    case tok::kw_int:
    case tok::kw_uint:
    case tok::kw_string:
    case tok::kw_float:
    case tok::kw_void:
    case tok::kw_char:
    case tok::kw_uchar:
    case tok::kw_const:
    case tok::kw_volatile:
    case tok::kw_local:
        return true;
    default:
        break;
    }
    return false;
}

// NOTE: current token is '['
// will skip until token is after ']'
// skip until ']' (keep track of other braces)
int C2Parser::SkipArray(int lookahead) {
    const Token& tok2 = GetLookAheadToken(lookahead);
    assert(tok2.is(tok::l_square) && "Expected '['");
    lookahead++;

    int count = 1;
    while (count) {
        const Token& t2 = GetLookAheadToken(lookahead);
        switch (t2.getKind()) {
        case tok::l_square:
            count++;
            break;
        case tok::r_square:
            count--;
            break;
        default:
            break;
        }
        lookahead++;
    }
    return lookahead;
}

// Syntax: TODO
C2::ExprResult C2Parser::ParseConstantExpression() {
    LOG_FUNC
    // TODO implement properly
    if (Tok.is(tok::numeric_constant)) {
        ExprResult E = Actions.ActOnNumericConstant(Tok);
        ConsumeToken();
        return E;
    }
    if (Tok.is(tok::string_literal)) {
        ExprResult Res = ParseStringLiteralExpression(true);
        ConsumeStringToken();
        return Res;
    }
    if (Tok.is(tok::kw_NULL)) {
        ConsumeToken();
        return ExprError();
    }
    std::cerr << "UNHANDLED TOKEN: ";
    PP.DumpToken(Tok);
    std::cerr << std::endl;
    assert(0 && "unhandled token type");
}

// Syntax: [],  [<numeric_constant>]
C2::ExprResult C2Parser::ParseArray(ExprResult base) {
    LOG_FUNC
    assert(Tok.is(tok::l_square) && "Expected '['");
    ConsumeToken();
    // fast path for "[]"
    if (Tok.is(tok::r_square)) {
        ConsumeToken();
        return Actions.ActOnArrayType(base.release(), 0);
    }
    // fast path for "[10]"
    if (Tok.is(tok::numeric_constant) && NextToken().is(tok::r_square)) {
        ExprResult E = Actions.ActOnNumericConstant(Tok);
        ConsumeToken(); // consume number
        ConsumeToken(); // consume ']'
        return Actions.ActOnArrayType(base.release(), E.release());
    }
    ExprResult E = ParseConstantExpression();
    if (E.isInvalid()) return ExprError();
    ExpectAndConsume(tok::r_square, diag::err_expected_rsquare);
    return Actions.ActOnArrayType(base.release(), E.release());
}

/// Syntax:
///  'sizeof' '(' var-name ')'
///  'sizeof' '(' type-name ')'
C2::ExprResult C2Parser::ParseSizeof()
{
    LOG_FUNC
    assert(Tok.is(tok::kw_sizeof) && "Not sizeof keyword!");
    SourceLocation Loc = ConsumeToken();

    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen)) return ExprError();
    ExprResult Res;
    // TEMP only support base types and identifier (no struct members etc)
    switch (Tok.getKind()) {
    case tok::identifier:
        Res = ParseIdentifier(true);
        break;
    // all basic types
    case tok::kw_u8:
    case tok::kw_u16:
    case tok::kw_u32:
    case tok::kw_s8:
    case tok::kw_s16:
    case tok::kw_s32:
    case tok::kw_int:
    case tok::kw_uint:
    case tok::kw_string:
    case tok::kw_float:
    case tok::kw_void:
    case tok::kw_char:
    case tok::kw_uchar:
        Res = ParseTypeSpecifier(false);
        break;
    case tok::kw_const:
    case tok::kw_volatile:
    case tok::kw_local:
        //Diag(Tok, diag::err_no_qualifier_allowed_here);
        fprintf(stderr, "Not type qualifier allowed here\n");
        return ExprError();
    default:
        //Diag(Tok, diag::err_expected type or symbol name);
        fprintf(stderr, "Expected Type or Symbol name\n");
        return ExprError();
    }
    if (Res.isInvalid()) return ExprError();

    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return ExprError();
    return Actions.ActOnSizeofExpression(Loc, Res.release());
}

// Syntax:
// identifier
// identifier :: identifier
C2::ExprResult C2Parser::ParseIdentifier(bool allow_package) {
    LOG_FUNC
    assert(Tok.is(tok::identifier) && "Not an identifier!");

    IdentifierInfo* pkgII = 0;
    SourceLocation pkgLoc;
    IdentifierInfo* symII = Tok.getIdentifierInfo();
    SourceLocation symLoc = ConsumeToken();
    if (Tok.is(tok::coloncolon)) {
        ConsumeToken();
        if (ExpectIdentifier()) return ExprError();
        pkgLoc = symLoc;
        pkgII = symII;
        symII = Tok.getIdentifierInfo();
        symLoc = ConsumeToken();
        if (!allow_package) {
            // TODO add error msg, no package specifier is allowed here
            fprintf(stderr, "NOT PACKAGE SPECIFIER ALLOWED HERE\n");
            return ExprError();
        }
    }
    return Actions.ActOnIdExpression(pkgII, pkgLoc, *symII, symLoc);
}

/*
   Syntax:
    func_def ::= FUNC type_qualifier single_type_specifier IDENTIFIER LPAREN full_param_list RPAREN compound_statement SEMICOLON.
*/
void C2Parser::ParseFuncDef(bool is_public) {
    LOG_FUNC
    assert(Tok.is(tok::kw_func) && "Expected func keyword");
    ConsumeToken();

    ExprResult rtype = ParseSingleTypeSpecifier(true);
    if (rtype.isInvalid()) return;

    if (ExpectIdentifier()) return;
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();

    // TODO use ParseIdentifier(false)
    FunctionDecl* func = Actions.ActOnFuncDef(id->getNameStart(), idLoc, is_public, rtype.release());

    ParseFullParamList(func->getArgs(), true);
    if (Diags.hasErrorOccurred()) return;

    StmtResult FnBody = ParseCompoundStatement();
    Actions.ActOnFinishFunctionBody(func, FnBody.release());
}

/*
   Syntax:
    compound_statement ::= LBRACE RBRACE.
    compound_statement ::= LBRACE statement_list RBRACE.

    statement_list ::= statement.
    statement_list ::= statement_list statement.
}
*/
C2::StmtResult C2Parser::ParseCompoundStatement() {
    LOG_FUNC
    if (Tok.isNot(tok::l_brace)) {
        Diag(Tok, diag::err_expected_lbrace);
        return StmtError();
    }
    SourceLocation OpenLoc = ConsumeToken();

    StmtList Stmts;
    while (1) {
        if (Tok.is(tok::r_brace)) break;

        StmtResult R = ParseStatement();
        if (R.isUsable()) Stmts.push_back(R.release());
        else fprintf(stderr, "COMPOUND: skipping invalid statement\n");
    }

    if (Tok.isNot(tok::r_brace)) {
        Diag(Tok, diag::err_expected_rbrace);
        return StmtError();
    }

    SourceLocation CloseLoc = ConsumeToken();
    return Actions.ActOnCompoundStmt(OpenLoc, CloseLoc, Stmts);
}

// TODO see Parser::ParseStatementOrDeclaration
C2::StmtResult C2Parser::ParseStatement() {
    LOG_FUNC
    switch (Tok.getKind()) {
    case tok::kw_if:
        return ParseIfStatement();
    case tok::kw_switch:
        return ParseSwitchStatement();
    case tok::kw_while:
        return ParseWhileStatement();
    case tok::kw_do:
        return ParseDoStatement();
    case tok::kw_for:
        return ParseForStatement();
    case tok::kw_goto:
        return ParseGotoStatement();
    case tok::kw_continue:
        return ParseContinueStatement();
    case tok::kw_break:
        return ParseBreakStatement();
    case tok::kw_return:
        return ParseReturnStatement();
    case tok::l_brace:
        return ParseCompoundStatement();
    case tok::identifier:
        return ParseDeclOrStatement();
    case tok::kw_case:
        Diag(Tok, diag::err_case_not_in_switch);
        return StmtError();
    case tok::kw_default:
        Diag(Tok, diag::err_default_not_in_switch);
        return StmtError();
    // all basic types
    case tok::kw_u8:
    case tok::kw_u16:
    case tok::kw_u32:
    case tok::kw_s8:
    case tok::kw_s16:
    case tok::kw_s32:
    case tok::kw_int:
    case tok::kw_uint:
    case tok::kw_string:
    case tok::kw_float:
    case tok::kw_void:
    case tok::kw_char:
    case tok::kw_uchar:
    case tok::kw_const:
    case tok::kw_volatile:
    case tok::kw_local:
        return ParseDeclaration();
    case tok::star:
        ParseExprStatement();
        return StmtError(); // TODO
    default:
        if (Tok.is(tok::r_brace)) {
            Diag(Tok, diag::err_expected_statement);
            return StmtError();
        }
        // TEMP
        std::cerr << "UNHANDLED TOKEN: ";
        PP.DumpToken(Tok);
        std::cerr << std::endl;
        assert(0 && "unhandled token");
        return StmtError(); // TODO
    }
}

// Syntax: return <expression>
C2::StmtResult C2Parser::ParseReturnStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_return) && "Not a return stmt!");
    SourceLocation loc = ConsumeToken();

    ExprResult result;
    // fast path for 'return;'
    if (Tok.isNot(tok::semi)) {
        result = ParseExpression();
        if (Diags.hasErrorOccurred()) return StmtError();
    }

    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "return")) return StmtError();
    return Actions.ActOnReturnStmt(loc, result.release());
}

/// ParseIfStatement
///       if-statement: [C99 6.8.4.1]
///         'if' '(' expression ')' statement
///         'if' '(' expression ')' statement 'else' statement
C2::StmtResult C2Parser::ParseIfStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_if) && "Not an if stmt!");

    SourceLocation IfLoc = ConsumeToken();

    if (Tok.isNot(tok::l_paren)) {
        Diag(Tok, diag::err_expected_lparen_after) << "if";
        SkipUntil(tok::semi);
        return StmtError();
    }

  // Parse the condition.
  //ExprResult CondExp;
  Decl *CondVar = 0;
// TODO import ParseParenExprOrCondition function (with BalancedTracker)
  //if (ParseParenExprOrCondition(CondExp, CondVar, IfLoc, true))
  //  return StmtError();
    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen)) return StmtError();
    ExprResult CondExp = ParseExpression();
    if (CondExp.isInvalid()) return StmtError();
    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return StmtError();

    // Read the 'then' stmt.
    SourceLocation ThenStmtLoc = Tok.getLocation();

    StmtResult ThenStmt(ParseStatement());

    // If it has an else, parse it.
    SourceLocation ElseLoc;
    SourceLocation ElseStmtLoc;
    StmtResult ElseStmt;
    if (Tok.is(tok::kw_else)) {
        // TODO import TrailingElseLoc (ParseStmt.cpp)
        //if (TrailingElseLoc) *TrailingElseLoc = Tok.getLocation();

        ElseLoc = ConsumeToken();
        ElseStmtLoc = Tok.getLocation();
        ElseStmt = ParseStatement();
    }

    // If the condition was invalid, discard the if statement.  We could recover
    // better by replacing it with a valid expr, but don't do that yet.
    if (CondExp.isInvalid() && !CondVar)
        return StmtError();

    // If the then or else stmt is invalid and the other is valid (and present),
    // make turn the invalid one into a null stmt to avoid dropping the other
    // part.  If both are invalid, return error.
    if ((ThenStmt.isInvalid() && ElseStmt.isInvalid()) ||
        (ThenStmt.isInvalid() && ElseStmt.get() == 0) ||
        (ThenStmt.get() == 0  && ElseStmt.isInvalid()))
    {
        // Both invalid, or one is invalid and other is non-present: return error.
        return StmtError();
    }
    // Now if either are invalid, replace with a ';'.
    // TODO FIX
/*
    if (ThenStmt.isInvalid())
        ThenStmt = Actions.ActOnNullStmt(ThenStmtLoc);
    if (ElseStmt.isInvalid())
        ElseStmt = Actions.ActOnNullStmt(ElseStmtLoc);
*/
    return Actions.ActOnIfStmt(IfLoc, CondExp, ThenStmt.get(),
                               ElseLoc, ElseStmt.get());
    //return Actions.ActOnIfStmt(IfLoc, FullCondExp, CondVar, ThenStmt.get(),
    //                           ElseLoc, ElseStmt.get());
}

/// ParseSwitchStatement
///       switch-statement:
///         'switch' '(' expression ')' statement
C2::StmtResult C2Parser::ParseSwitchStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_switch) && "Not a switch stmt!");
    SourceLocation Loc = ConsumeToken();

    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen)) return StmtError();

    ExprResult Cond = ParseExpression();
    if (Cond.isInvalid()) return StmtError();

    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return StmtError();
    if (ExpectAndConsume(tok::l_brace, diag::err_expected_lbrace)) return StmtError();

    StmtList Cases;
    bool done = false;
    while (!done) {
        StmtResult Res;
        switch (Tok.getKind()) {
        case tok::kw_case:
            Res = ParseCaseStatement();
            break;
        case tok::kw_default:
            Res = ParseDefaultStatement();
            break;
        case tok::r_brace:
            done = true;
            continue;
        default:
            // TODO Diag expected case/default statement
            fprintf(stderr, "UNEXPECTED TOKEN IN SWITCH\n");
            return StmtError();
        }
        if (Res.isUsable()) Cases.push_back(Res.release());
        else return StmtError();
        // TODO check for multiple default statements here?
    }

    if (ExpectAndConsume(tok::r_brace, diag::err_expected_rbrace)) return StmtError();

    return Actions.ActOnSwitchStmt(Loc, Cond.release(), Cases);
}

/// ParseWhileStatement
///       while-statement: [C99 6.8.5.1]
///         'while' '(' expression ')' statement
C2::StmtResult C2Parser::ParseWhileStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_while) && "Not a while stmt!");
    SourceLocation Loc = ConsumeToken();

    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen)) return StmtError();

    ExprResult Cond = ParseExpression();
    if (Cond.isInvalid()) return StmtError();

    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return StmtError();

    StmtResult Then = ParseStatement();
    if (Then.isInvalid()) return StmtError();

    return Actions.ActOnWhileStmt(Loc, Cond, Then);
}

/// ParseDoStatement
///       do-statement: [C99 6.8.5.2]
///         'do' statement 'while' '(' expression ')' ';'
/// Note: this lets the caller parse the end ';'.
C2::StmtResult C2Parser::ParseDoStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_do) && "Not a do stmt!");
    SourceLocation Loc = ConsumeToken();

    StmtResult Then = ParseStatement();
    if (Then.isInvalid()) return StmtError();

    if (ExpectAndConsume(tok::kw_while, diag::err_expected_while)) return StmtError();

    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen)) return StmtError();

    ExprResult Cond = ParseExpression();
    if (Cond.isInvalid()) return StmtError();

    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return StmtError();
    StmtResult Res = Actions.ActOnDoStmt(Loc, Cond, Then);
    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "while")) return StmtError();
    return Res;
}

/// ParseForStatement
///       for-statement: [C99 6.8.5.3]
///         'for' '(' expr[opt] ';' expr[opt] ';' expr[opt] ')' statement
///         'for' '(' declaration expr[opt] ';' expr[opt] ')' statement
/// [C++]   'for' '(' for-init-statement condition[opt] ';' expression[opt] ')'
/// [C++]       statement
/// [C++0x] 'for' '(' for-range-declaration : for-range-initializer ) statement
/// [OBJC2] 'for' '(' declaration 'in' expr ')' statement
/// [OBJC2] 'for' '(' expr 'in' expr ')' statement
///
/// [C++] for-init-statement:
/// [C++]   expression-statement
/// [C++]   simple-declaration
//statement ::= FOR LPAREN expression_statement expression_statement RPAREN statement.
//statement ::= FOR LPAREN expression_statement expression_statement expression RPAREN statement.
C2::StmtResult C2Parser::ParseForStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_for) && "Not a for stmt!");
    SourceLocation Loc = ConsumeToken();

    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen_after, "for")) return StmtError();

    // first substmt
    StmtResult Init;
    if (Tok.is(tok::semi)) {    // for (;
        ConsumeToken();
    } else {
        bool isDecl = isDeclaration();
        if (Diags.hasErrorOccurred()) return StmtError();
        if (isDecl) {
            Init = ParseDeclaration();
        } else {
            Init = ParseExprStatement();
        }
        if (Init.isInvalid()) return StmtError();

        //if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "statement")) return;
    }

    // second substmt
    ExprResult Cond;
    if (Tok.isNot(tok::semi)) {
        Cond = ParseExpression();
        if (Cond.isInvalid()) return StmtError();
    }
    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "statement")) return StmtError();

    // third substmt
    ExprResult Incr;
    if (Tok.isNot(tok::r_paren)) {
        Incr = ParseExpression();
        if (Incr.isInvalid()) return StmtError();
    }
    if (ExpectAndConsume(tok::r_paren, diag::err_expected_rparen)) return StmtError();

    StmtResult Body = ParseStatement();
    if (Body.isInvalid()) return StmtError();

    return Actions.ActOnForStmt(Loc, Init.release(), Cond.release(), Incr.release(), Body.release());
}

/// ParseGotoStatement
///       jump-statement:
///         'goto' identifier ';'
C2::StmtResult C2Parser::ParseGotoStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_goto) && "Not a goto stmt!");
    SourceLocation GotoLoc = ConsumeToken();  // eat the 'goto'.

    if (Tok.isNot(tok::identifier)) {
        Diag(Tok, diag::err_expected_ident);
        return StmtError();
    }
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation LabelLoc = ConsumeToken();

    StmtResult Res = Actions.ActOnGotoStmt(id->getNameStart(), GotoLoc, LabelLoc);

    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "goto")) return StmtError();
    return Res;
}

/// ParseContinueStatement
///       jump-statement:
///         'continue' ';'
C2::StmtResult C2Parser::ParseContinueStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_continue) && "Not a continue stmt!");
    SourceLocation Loc = ConsumeToken();

    StmtResult Res = Actions.ActOnContinueStmt(Loc);
    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "continue")) return StmtError();
    return Res;
}

/// ParseBreakStatement
///       jump-statement:
///         'break' ';'
C2::StmtResult C2Parser::ParseBreakStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_break) && "Not a break stmt!");
    SourceLocation Loc = ConsumeToken();

    StmtResult Res = Actions.ActOnBreakStmt(Loc);
    if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "break")) return StmtError();
    return Res;
}

/*
  Syntax:
    Number num = .     // id = type
    Utils::Type t = .  // id = pkg::type
    myfunc()        // id = func
    Pkg::func()     // id = pkg::func
    count =         // id = var
    Pkg::var =      // id = pkg::var
    id:             // id = label
*/
// TODO see Parser::ParseStatementOrDeclarationAfterAttributes()
C2::StmtResult C2Parser::ParseDeclOrStatement() {
    LOG_FUNC
    assert(Tok.is(tok::identifier) && "Not an identifier!");

    bool isDecl = isTypeSpec();
    if (Diags.hasErrorOccurred()) return StmtError();
    // case 1: declaration
    if (isDecl) return ParseDeclaration();

    int lookahead = 1;
    // Note: making copies otherwise const error
    Token afterIdent = GetLookAheadToken(lookahead);
    // TODO extract to function
    // check if full_id here
    bool is_fullname = false;
    if (afterIdent.is(tok::coloncolon)) {
        lookahead++;
        afterIdent = GetLookAheadToken(lookahead);
        if (afterIdent.isNot(tok::identifier)) {
            Diag(afterIdent, diag::err_expected_ident);
            return StmtError();
        }
        lookahead++;
        is_fullname = true;
    }

    StmtResult Res;
    afterIdent = GetLookAheadToken(lookahead);
    switch (afterIdent.getKind()) {
    case tok::coloncolon:
        // syntax error
        assert(0 && "double package id");
        return StmtError();
    case tok::colon:
        // TODO move this to ParseLabeledStatement
        if (is_fullname) {
            Diag(afterIdent, diag::err_invalid_label);
            return StmtError();
        }
        return ParseLabeledStatement();
/*
    case tok::l_paren:
        Res = ParseFunctionCall();
        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "function call")) return StmtError();
        break;
*/
    default:
        Res = ParseExprStatement();
        break;
    }
    return Res;
}

//Syntax: declaration ::= type_qualifier type_specifier IDENTIFIER var_initialization.
C2::StmtResult C2Parser::ParseDeclaration() {
    LOG_FUNC

    ExprResult type = ParseTypeSpecifier(true);
    if (type.isInvalid()) return StmtError();

    // TODO use ParseIdentifier(false)
    if (ExpectIdentifier()) return StmtError();
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();

    // NOTE: same as ParseVarDef(), TODO refactor?
    bool need_semi = true;
    ExprResult InitValue;
    if (Tok.is(tok::equal)) {
        ConsumeToken();
        InitValue = ParseInitValue(&need_semi);
        if (InitValue.isInvalid()) return StmtError();
    }
    StmtResult Res = Actions.ActOnDeclaration(id->getNameStart(), idLoc, type.release(), InitValue.release());

    if (need_semi) {
        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "declaration")) return StmtError();
    }
    return Res;
}

/// ParseCaseStatement
///       labeled-statement:
///         'case' constant-expression ':' statement
/// [GNU]   'case' constant-expression '...' constant-expression ':' statement
C2::StmtResult C2Parser::ParseCaseStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_case) && "Not a case stmt!");
    SourceLocation Loc = ConsumeToken();

    ExprResult Cond = ParseConstantExpression();
    if (Cond.isInvalid()) return StmtError();

    if (ExpectAndConsume(tok::colon, diag::err_expected_colon_after, "case")) return StmtError();

    StmtList Stmts;
    bool done = false;
    while (!done) {
        switch (Tok.getKind()) {
        case tok::kw_case:
        case tok::kw_default:
        case tok::r_brace:
            done = true;
             break;
        default:
            {
                StmtResult Res = ParseStatement();
                if (Res.isUsable()) Stmts.push_back(Res.release());
                else return StmtError();
            }
        }
    }

    return Actions.ActOnCaseStmt(Loc, Cond.release(), Stmts);
}

/// ParseDefaultStatement
///       labeled-statement:
///         'default' ':' statement
/// Note that this does not parse the 'statement' at the end.
C2::StmtResult C2Parser::ParseDefaultStatement() {
    LOG_FUNC
    assert(Tok.is(tok::kw_default) && "Not a default stmt!");
    SourceLocation Loc = ConsumeToken();

    if (ExpectAndConsume(tok::colon, diag::err_expected_colon_after, "default")) return StmtError();

    StmtList Stmts;
    bool done = false;
    while (!done) {
        switch (Tok.getKind()) {
        case tok::kw_case:
        case tok::kw_default:
        case tok::r_brace:
            done = true;
             break;
        default:
            {
                StmtResult Res = ParseStatement();
                if (Res.isUsable()) Stmts.push_back(Res.release());
                else return StmtError();
            }
        }
    }
    return Actions.ActOnDefaultStmt(Loc, Stmts);
}

/// ParseLabeledStatement - We have an identifier and a ':' after it.
///
///       labeled-statement:
///         identifier ':' statement
C2::StmtResult C2Parser::ParseLabeledStatement() {
    LOG_FUNC
    assert(Tok.is(tok::identifier) && Tok.getIdentifierInfo() && "Not an identifier!");

    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation LabelLoc = ConsumeToken();

    assert(Tok.is(tok::colon) && "Not a label!");

    // identifier ':' statement
    SourceLocation ColonLoc = ConsumeToken();

    StmtResult SubStmt(ParseStatement());
/*
    // TODO
 // Broken substmt shouldn't prevent the label from being added to the AST.
  if (SubStmt.isInvalid())
    SubStmt = Actions.ActOnNullStmt(ColonLoc);
*/

    //LabelDecl *LD = Actions.LookupOrCreateLabel(id, LabelLoc);
    return Actions.ActOnLabelStmt(id->getNameStart(), LabelLoc, SubStmt.get());
}

C2::StmtResult C2Parser::ParseExprStatement() {
    LOG_FUNC
    ExprResult Expr(ParseExpression());
    if (Expr.isInvalid()) {
        // If the expression is invalid, skip ahead to the next semicolon or '}'.
        // Not doing this opens us up to the possibility of infinite loops if
        // ParseExpression does not consume any tokens.
        SkipUntil(tok::r_brace, /*StopAtSemi=*/true, /*DontConsume=*/true);
        if (Tok.is(tok::semi)) ConsumeToken();
        return StmtError();
    }

    ExpectAndConsumeSemi(diag::err_expected_semi_after_expr);
    Expr.get()->setStatementFlag();
    return StmtResult(Expr.release());
}

/*
   Syntax:
    var_def ::= type_qualifier type_specifier IDENTIFIER var_initialization SEMICOLON.
*/
void C2Parser::ParseVarDef(bool is_public) {
    LOG_FUNC

    // TODO dont allow local keyword (check in actions)
    ExprResult type = ParseTypeSpecifier(true);
    if (type.isInvalid()) return;

    if (ExpectIdentifier()) return;
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();

    bool need_semi = true;
    ExprResult InitValue;
    if (Tok.is(tok::equal)) {
        ConsumeToken();
        InitValue = ParseInitValue(&need_semi);
        if (InitValue.isInvalid()) return;
    }
    // TODO use ParseIdentifier(false)
    Actions.ActOnVarDef(id->getNameStart(), idLoc, is_public, type.release(), InitValue.release());

    if (need_semi) {
        ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "variable definition");
    }
}

/*
   Syntax:
    init_value ::= constant_expression.
    init_value ::= LBRACE init_values RBRACE.
    Return value semi-colon needed
*/
C2::ExprResult C2Parser::ParseInitValue(bool* need_semi) {
    LOG_FUNC
    if (Tok.is(tok::l_brace)) {
        // Syntax: { <init_values> }
        *need_semi = false;
        return ParseInitValues();
    } else {
        // Syntax: <constant expr>
        *need_semi = true;
        ExprResult Res = ParseAssignmentExpression();
        return Res;
    }
}

/*
   Syntax:
    init_values ::= init_values COMMA init_value.
    init_values ::= init_values COMMA.
    init_values ::= init_value.
*/
C2::ExprResult C2Parser::ParseInitValues() {
    LOG_FUNC
    assert(Tok.is(tok::l_brace) && "Expected '{'");
    SourceLocation left = ConsumeToken();

    ExprList vals;
    // NOTE memleak on vals
    while (1) {
        if (Tok.is(tok::r_brace)) break;
        bool unused;
        ExprResult R = ParseInitValue(&unused);
        if (R.isInvalid()) return ExprError();
        vals.push_back(R.release());
        if (Tok.is(tok::comma)) {
            ConsumeToken();
        } else {
            break;
        }
    }
    if (Tok.isNot(tok::r_brace)) {
        Diag(Tok, diag::err_expected_rbrace) << PP.getSpelling(Tok);
        return ExprError();
    }
    SourceLocation right = ConsumeToken();
    return Actions.ActOnInitList(left, right, vals);
}

// Syntax: identifier += <init_value>
void C2Parser::ParseArrayEntry() {
    LOG_FUNC
    assert(Tok.is(tok::identifier) && "Not an identifier!");
    IdentifierInfo* id = Tok.getIdentifierInfo();
    SourceLocation idLoc = ConsumeToken();

    assert(Tok.is(tok::plusequal) && "Not a plusequal!");
    ConsumeToken();

    bool need_semi = true;
    ExprResult Value = ParseInitValue(&need_semi);
    if (Value.isInvalid()) return;

    Actions.ActOnArrayValue(id->getNameStart(), idLoc, Value.release());

    if (need_semi) {
        if (ExpectAndConsume(tok::semi, diag::err_expected_semi_after, "array entry")) return;
    }
}

// Syntax: const | volatile | local | local const
unsigned int C2Parser::ParseOptionalTypeQualifier() {
    // TODO consume all const/volatile/local tokens (can give errors)
    LOG_FUNC
    switch (Tok.getKind()) {
    case tok::kw_const:
        ConsumeToken();
        return TYPE_CONST;
    case tok::kw_volatile:
        ConsumeToken();
        return TYPE_VOLATILE;
    case tok::kw_local:
        ConsumeToken();
        if (Tok.is(tok::kw_const)) {
            ConsumeToken();
            return TYPE_LOCAL | TYPE_CONST;
        }
        else return TYPE_LOCAL;
    default:
        break;
    }
    return 0;
}

// Syntax: public
bool C2Parser::ParseOptionalAccessSpecifier() {
    LOG_FUNC
    if (Tok.is(tok::kw_public)) {
        ConsumeToken();
        return true;
    }
    return false;
}

// Syntax foo, bar10
bool C2Parser::ExpectIdentifier(const char *Msg) {
    const unsigned DiagID = diag::err_expected_ident;
    if (Tok.is(tok::identifier)) return false;

    const char *Spelling = 0;
    SourceLocation EndLoc = PP.getLocForEndOfToken(PrevTokLocation);
    if (EndLoc.isValid() && (Spelling = tok::getTokenSimpleSpelling(tok::identifier))) {
        // Show what code to insert to fix this problem.
        Diag(EndLoc, DiagID)
        << Msg
        << FixItHint::CreateInsertion(EndLoc, Spelling);
    } else
        Diag(Tok, DiagID) << Msg;

    return true;
}

/// ExpectAndConsume - The parser expects that 'ExpectedTok' is next in the
/// input.  If so, it is consumed and false is returned.
///
/// If the input is malformed, this emits the specified diagnostic.  Next, if
/// SkipToTok is specified, it calls SkipUntil(SkipToTok).  Finally, true is
/// returned.
bool C2Parser::ExpectAndConsume(tok::TokenKind ExpectedTok, unsigned DiagID,
                              const char *Msg, tok::TokenKind SkipToTok) {
  if (Tok.is(ExpectedTok) || Tok.is(tok::code_completion)) {
    ConsumeAnyToken();
    return false;
  }

#if 0
  // Detect common single-character typos and resume.
  if (IsCommonTypo(ExpectedTok, Tok)) {
    SourceLocation Loc = Tok.getLocation();
    Diag(Loc, DiagID)
      << Msg
      << FixItHint::CreateReplacement(SourceRange(Loc),
                                      getTokenSimpleSpelling(ExpectedTok));
    ConsumeAnyToken();

    // Pretend there wasn't a problem.
    return false;
  }
#endif

  const char *Spelling = 0;
  SourceLocation EndLoc = PP.getLocForEndOfToken(PrevTokLocation);
  if (EndLoc.isValid() &&
      (Spelling = tok::getTokenSimpleSpelling(ExpectedTok))) {
    // Show what code to insert to fix this problem.
    Diag(EndLoc, DiagID)
      << Msg
      << FixItHint::CreateInsertion(EndLoc, Spelling);
  } else
    Diag(Tok, DiagID) << Msg;

  //if (SkipToTok != tok::unknown)
  //  SkipUntil(SkipToTok);
  return true;
}

bool C2Parser::ExpectAndConsumeSemi(unsigned DiagID) {
    if (Tok.is(tok::semi) || Tok.is(tok::code_completion)) {
        ConsumeToken();
        return false;
    }
  if ((Tok.is(tok::r_paren) || Tok.is(tok::r_square)) &&
      NextToken().is(tok::semi)) {
    Diag(Tok, diag::err_extraneous_token_before_semi)
      << PP.getSpelling(Tok)
      << FixItHint::CreateRemoval(Tok.getLocation());
    ConsumeAnyToken(); // The ')' or ']'.
    ConsumeToken(); // The ';'.
    return false;
  }

  return ExpectAndConsume(tok::semi, DiagID);
}

DiagnosticBuilder C2Parser::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder C2Parser::Diag(const Token &Tok, unsigned DiagID) {
    return Diag(Tok.getLocation(), DiagID);
}

//===----------------------------------------------------------------------===//
// Error recovery.
//===----------------------------------------------------------------------===//

/// SkipUntil - Read tokens until we get to the specified token, then consume
/// it (unless DontConsume is true).  Because we cannot guarantee that the
/// token will ever occur, this skips to the next token, or to some likely
/// good stopping point.  If StopAtSemi is true, skipping will stop at a ';'
/// character.
///
/// If SkipUntil finds the specified token, it returns true, otherwise it
/// returns false.
bool C2Parser::SkipUntil(ArrayRef<tok::TokenKind> Toks, bool StopAtSemi,
                       bool DontConsume, bool StopAtCodeCompletion) {
  // We always want this function to skip at least one token if the first token
  // isn't T and if not at EOF.
  bool isFirstTokenSkipped = true;
  while (1) {
    // If we found one of the tokens, stop and return true.
    for (unsigned i = 0, NumToks = Toks.size(); i != NumToks; ++i) {
      if (Tok.is(Toks[i])) {
        if (DontConsume) {
          // Noop, don't consume the token.
        } else {
          ConsumeAnyToken();
        }
        return true;
      }
    }

    switch (Tok.getKind()) {
    case tok::eof:
      // Ran out of tokens.
      return false;

    case tok::code_completion:
      if (!StopAtCodeCompletion)
        ConsumeToken();
      return false;

   case tok::l_paren:
      // Recursively skip properly-nested parens.
      ConsumeParen();
      SkipUntil(tok::r_paren, false, false, StopAtCodeCompletion);
      break;
    case tok::l_square:
      // Recursively skip properly-nested square brackets.
      ConsumeBracket();
      SkipUntil(tok::r_square, false, false, StopAtCodeCompletion);
      break;
    case tok::l_brace:
      // Recursively skip properly-nested braces.
      ConsumeBrace();
      SkipUntil(tok::r_brace, false, false, StopAtCodeCompletion);
      break;

    // Okay, we found a ']' or '}' or ')', which we think should be balanced.
    // Since the user wasn't looking for this token (if they were, it would
    // already be handled), this isn't balanced.  If there is a LHS token at a
    // higher level, we will assume that this matches the unbalanced token
    // and return it.  Otherwise, this is a spurious RHS token, which we skip.
    case tok::r_paren:
      if (ParenCount && !isFirstTokenSkipped)
        return false;  // Matches something.
      ConsumeParen();
      break;
    case tok::r_square:
      if (BracketCount && !isFirstTokenSkipped)
        return false;  // Matches something.
      ConsumeBracket();
      break;
    case tok::r_brace:
      if (BraceCount && !isFirstTokenSkipped)
        return false;  // Matches something.
      ConsumeBrace();
      break;

    case tok::string_literal:
    case tok::wide_string_literal:
    case tok::utf8_string_literal:
    case tok::utf16_string_literal:
    case tok::utf32_string_literal:
      ConsumeStringToken();
      break;

    case tok::semi:
      if (StopAtSemi)
        return false;
      // FALL THROUGH.
    default:
      // Skip this token.
      ConsumeToken();
      break;
    }
    isFirstTokenSkipped = false;
  }
}

C2::ExprResult C2Parser::ExprError() {
    fprintf(stderr, "EXPR_ERROR()\n");
    return C2::ExprResult(true);
}

C2::StmtResult C2Parser::StmtError() {
    fprintf(stderr, "STMT_ERROR()\n");
    return C2::StmtResult(true);
}

