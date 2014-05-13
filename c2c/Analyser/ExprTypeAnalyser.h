/* Copyright 2013,2014 Bas van den Berg
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

#ifndef ANALYSER_EXPR_TYPE_ANALYSER_H
#define ANALYSER_EXPR_TYPE_ANALYSER_H

#include <clang/Basic/SourceLocation.h>

#include "AST/Type.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Expr;
class BinaryOperator;
class UnaryOperator;
class ConditionalOperator;

/*
 *  ExprTypeAnalyser checks each sub-expression that is CTC_FULL or
 *  CTC_NONE with the other checkers separately.
 *  It checks Literals for Range and Symbols for Type
 *  For CTC_PARTIAL expressions (eg. 10 + a), each sub-expression
 *  should fit the LHS type.
*/
class ExprTypeAnalyser {
public:
    ExprTypeAnalyser(clang::DiagnosticsEngine& Diags_);

    void check(QualType Tleft, const Expr* expr);
private:
    void error(SourceLocation loc, QualType left, QualType right) const;
    void checkBinOp(QualType TLeft, const BinaryOperator* binop);

    bool checkCompatible(QualType left, const Expr* expr) const;
    bool checkBuiltin(QualType left, QualType right, const Expr* expr, bool first) const;
    bool checkPointer(QualType left, QualType right, const Expr* expr) const;
    bool checkFunction(QualType left, const Expr* expr) const;

    clang::DiagnosticsEngine& Diags;

    ExprTypeAnalyser(const ExprTypeAnalyser&);
    ExprTypeAnalyser& operator= (const ExprTypeAnalyser&);
};

}

#endif

