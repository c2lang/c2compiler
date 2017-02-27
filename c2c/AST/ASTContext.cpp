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

#include "AST/ASTContext.h"
#include "AST/Expr.h"

#include <stdlib.h>
#include <assert.h>

using namespace C2;

void ASTContext::dump() const {
    BumpAlloc.PrintStats();
    stringPool.PrintStats();
}

QualType ASTContext::getPointerType(QualType ref) {
    assert(ref.isValid());
    for (unsigned i=0; i<types.size(); i++) {
        Type* t = types[i];
        if (isa<PointerType>(t)) {
            PointerType* P = cast<PointerType>(t);
            if (P->getPointeeType() == ref) return t;
        }
    }
    Type* N = new (*this) PointerType(ref);
    if (ref->hasCanonicalType()) N->setCanonicalType(N);
    return add(N);
}

QualType ASTContext::getArrayType(QualType element, Expr* size, bool isIncremental) {
    Type* N = new (*this) ArrayType(element, size, isIncremental);
    if (element->hasCanonicalType()) N->setCanonicalType(N);
    return add(N);
}

QualType ASTContext::getUnresolvedType(IdentifierExpr* moduleName, IdentifierExpr* typeName) {
    return add(new (*this) UnresolvedType(moduleName, typeName));
}

QualType ASTContext::getAliasType(AliasTypeDecl* A, QualType refType) {
    return add(new (*this) AliasType(A, refType));
}

QualType ASTContext::getStructType() {
    return add(new (*this) StructType());
}

QualType ASTContext::getEnumType() {
    return add(new (*this) EnumType());
}

QualType ASTContext::getFunctionType(FunctionDecl* F) {
    return add(new (*this) FunctionType(F));
}

QualType ASTContext::getModuleType(ImportDecl* D) {
    return add(new (*this) ModuleType(D));
}

QualType ASTContext::add(Type* T) {
    types.push_back(T);
    return QualType(T);
}

void* ASTContext::allocTypeExpr() {
    void* result;
    if (typeExprCache.empty()) {
        result = Allocate(sizeof(TypeExpr));
    } else {
        result = typeExprCache[typeExprCache.size()-1];
        typeExprCache.pop_back();
    }
    return result;
}

void ASTContext::freeTypeExpr(TypeExpr* t) {
    typeExprCache.push_back(t);
}

