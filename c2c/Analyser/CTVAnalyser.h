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

#ifndef ANALYSER_CTV_ANALYSER_H
#define ANALYSER_CTV_ANALYSER_H

#include <llvm/ADT/APSInt.h>
#include "Clang/SourceLocation.h"

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class Expr;
class Decl;
class QualType;
struct Limit;

class CTVAnalyser {
public:
    CTVAnalyser(c2lang::DiagnosticsEngine& Diags_);

    void check(QualType TLeft, const Expr* Right);

    llvm::APSInt checkLiterals(const Expr* Right);

    void checkBitOffset(const Expr* BO, const Expr* Right);
    bool checkRange(QualType T, const Expr* Right, c2lang::SourceLocation Loc, llvm::APSInt Result);
private:
    void checkWidth(int availableWidth, const Limit* L, const Expr* Right, const char* tname);
    bool calcWidth(QualType TLeft, const Expr* Right, int* availableWidth);
    llvm::APSInt checkUnaryLiterals(const Expr* Right);
    llvm::APSInt checkBinaryLiterals(const Expr* Right);
    llvm::APSInt checkArraySubscript(const Expr* Right);
    llvm::APSInt checkDecl(const Decl* D);
    llvm::APSInt truncateLiteral(QualType type, const Expr* Right,llvm::APSInt Result);

    c2lang::DiagnosticsEngine& Diags;

    CTVAnalyser(const CTVAnalyser&);
    CTVAnalyser& operator= (const CTVAnalyser&);
};

}

#endif

