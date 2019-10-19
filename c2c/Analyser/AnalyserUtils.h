/* Copyright 2013-2019 Bas van den Berg
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

#ifndef ANALYSER_UTILS_H
#define ANALYSER_UTILS_H

#include <string>
#include "AST/Type.h"
#include "AST/Expr.h"

namespace C2 {

class Decl;
class StringBuilder;

class AnalyserUtils {
public:
    static const char* fullName(const std::string& modName, const char* symname);

    static QualType getStructType(QualType Q);
    static bool exprIsType(const Expr* E);
    static QualType UsualUnaryConversions(Expr* expr);
    static void SetConstantFlags(Decl* D, Expr* expr);
    static ExprCTC combineCtc(Expr* Result, const Expr* L, const Expr* R);
    static bool isConstantBitOffset(const Expr* E);
    static StringBuilder& quotedField(StringBuilder &builder, IdentifierExpr *field);

    static uint64_t sizeOfStruct(StructTypeDecl* S, uint32_t* align);
    static uint64_t sizeOfUnion(StructTypeDecl* S, uint32_t* align);
    static uint64_t sizeOfType(QualType type, unsigned* alignment);
};

}

#endif

