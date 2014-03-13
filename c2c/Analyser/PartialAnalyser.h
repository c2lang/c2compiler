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

#ifndef PARTIAL_ANALYSER_H
#define PARTIAL_ANALYSER_H

#include "AST/Type.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Expr;
class BinaryOperator;
class UnaryOperator;
class ConditionalOperator;
class TypeChecker;

/*
 *  PartialAnalyser checks each sub-expression that is CTC_FULL or
 *  CTC_NONE with the other checkers separately.
*/
class PartialAnalyser {
public:
    PartialAnalyser(TypeChecker& TC_, clang::DiagnosticsEngine& Diags_);

    void check(QualType Tleft, const Expr* expr);
private:
    void checkBinOp(QualType TLeft, const BinaryOperator* binop);

    TypeChecker& TC;
    clang::DiagnosticsEngine& Diags;

    PartialAnalyser(const PartialAnalyser&);
    PartialAnalyser& operator= (const PartialAnalyser&);
};

}

#endif

