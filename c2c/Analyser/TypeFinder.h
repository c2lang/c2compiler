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

#ifndef TYPE_FINDER_H
#define TYPE_FINDER_H

#include "AST/Type.h"

namespace C2 {

class Expr;
class BinaryOperator;
class UnaryOperator;

/*
 *  TypeFinder tries to find the biggest type in the (sub)Expression,
 *  without Integer Promotions or Usual Arithmetic Conversions
*/
class TypeFinder {
public:
    static QualType findType(const Expr* expr);
private:
    static QualType getBinOpType(const BinaryOperator* binop);
    static QualType  getUnaryOpType(const UnaryOperator* unaryop);
    static QualType LargestType(const Expr* Left, const Expr* Right);
#if 0
    llvm::APSInt checkLiterals(QualType TLeft, Expr* Right);
    llvm::APSInt checkIntegerLiterals(QualType TLeft, Expr* Right);
    llvm::APSInt checkUnaryLiterals(QualType TLeft, Expr* Right);
    llvm::APSInt checkBinaryLiterals(QualType TLeft, Expr* Right);
    llvm::APSInt checkIdentifier(QualType TLeft, Expr* Right);
#endif
    TypeFinder(const TypeFinder&);
    TypeFinder& operator= (const TypeFinder&);
};

}

#endif

