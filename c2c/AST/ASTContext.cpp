#include "AST/ASTContext.h"
#include "AST/Expr.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>  // for printf

using namespace C2;

void ASTContext::dump() const {
    BumpAlloc.PrintStats();
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

