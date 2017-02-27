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

#ifndef ANALYSER_TYPE_FINDER_H
#define ANALYSER_TYPE_FINDER_H

#include "AST/Type.h"

namespace C2 {

class Expr;
class BinaryOperator;
class UnaryOperator;
class ConditionalOperator;

/*
 *  TypeFinder tries to find the biggest type in the (sub)Expression,
 *  without Integer Promotions or Usual Arithmetic Conversions
*/
class TypeFinder {
public:
    static QualType findType(const Expr* expr);
private:
    static QualType getBinOpType(const BinaryOperator* binop);
    static QualType getUnaryOpType(const UnaryOperator* unaryop);
    static QualType getCondOpType(const ConditionalOperator* condop);
    static QualType LargestType(const Expr* Left, const Expr* Right);

    TypeFinder(const TypeFinder&);
    TypeFinder& operator= (const TypeFinder&);
};

}

#endif

