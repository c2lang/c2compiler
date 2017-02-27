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

#ifndef ANALYSER_LITERAL_ANALYSER_H
#define ANALYSER_LITERAL_ANALYSER_H

#include <llvm/ADT/APSInt.h>
#include <clang/Basic/SourceLocation.h>

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Expr;
class Decl;
class QualType;
struct Limit;

class LiteralAnalyser {
public:
    LiteralAnalyser(clang::DiagnosticsEngine& Diags_);

    void check(QualType TLeft, const Expr* Right);
    llvm::APSInt checkLiterals(const Expr* Right);
    void checkBitOffset(const Expr* BO, const Expr* Right);
    bool checkRange(QualType T, const Expr* Right, clang::SourceLocation Loc, llvm::APSInt Result);
private:
    void checkWidth(int availableWidth, const Limit* L, const Expr* Right, const char* tname);
    bool calcWidth(QualType TLeft, const Expr* Right, int* availableWidth);
    llvm::APSInt checkIntegerLiterals(const Expr* Right);
    llvm::APSInt checkUnaryLiterals(const Expr* Right);
    llvm::APSInt checkBinaryLiterals(const Expr* Right);
    llvm::APSInt checkArraySubscript(const Expr* Right);
    llvm::APSInt checkDecl(const Decl* D);
    llvm::APSInt truncateLiteral(QualType type, const Expr* Right,llvm::APSInt Result);

    clang::DiagnosticsEngine& Diags;

    LiteralAnalyser(const LiteralAnalyser&);
    LiteralAnalyser& operator= (const LiteralAnalyser&);
};

}

#endif

