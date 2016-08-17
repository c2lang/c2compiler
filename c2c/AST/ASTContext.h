#ifndef AST_AST_CONTEXT_H
#define AST_AST_CONTEXT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "llvm/Support/Allocator.h"
#include "AST/Type.h"

namespace C2 {

class TypeExpr;

class ASTContext {
public:
    ASTContext() {}
    ~ASTContext() {}

    void* Allocate(size_t Size, unsigned Align = 8) const {
        return BumpAlloc.Allocate(Size, Align);
    }
    void Deallocate(void* Ptr) const {}

    void dump() const;

    // For Types
    QualType getPointerType(QualType ref);
    QualType getArrayType(QualType element, Expr* size, bool isIncremental);
    QualType getUnresolvedType(IdentifierExpr* moduleName, IdentifierExpr* typeName);
    QualType getAliasType(AliasTypeDecl* A, QualType ref);
    QualType getStructType();
    QualType getEnumType();
    QualType getFunctionType(FunctionDecl* F);
    QualType getModuleType(ImportDecl* D);

    void* allocTypeExpr();
    void freeTypeExpr(TypeExpr* t);

    const char* addIdentifier(const char* name, unsigned len) {
        len++;  // include 0
        char* mem = (char*)stringPool.Allocate(len, 1);
        memcpy(mem, name, len);
        return mem;
    }
#if 0
    void clearStrings() {
        stringPool.Reset();
    }
#endif
private:
    QualType add(Type* T);

    mutable llvm::BumpPtrAllocator BumpAlloc;
    llvm::BumpPtrAllocator stringPool;

    typedef std::vector<Type*> Types;
    Types types;

    typedef std::vector<TypeExpr*> TypeExprCache;
    TypeExprCache typeExprCache;

    ASTContext(const ASTContext&);
    ASTContext& operator= (const ASTContext&);
};

}

// TODO add operator new[] and delete[]

inline void *operator new(size_t Bytes, const C2::ASTContext &C, size_t Alignment) {
    return C.Allocate(Bytes, Alignment);
}

inline void operator delete(void *Ptr, const C2::ASTContext &C, size_t) {
    C.Deallocate(Ptr);
}

#endif

