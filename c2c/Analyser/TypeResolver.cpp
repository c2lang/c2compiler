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

#include <string>
#include <assert.h>

#include <clang/Basic/SourceLocation.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/Scope.h"
#include "Analyser/TypeResolver.h"
#include "Analyser/AnalyserConstants.h"
#include "Analyser/AnalyserUtils.h"
#include "AST/Module.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/ASTContext.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace clang;

TypeResolver::TypeResolver(Scope& g, clang::DiagnosticsEngine& Diags_, ASTContext& ctx_)
    : globals(g)
    , Diags(Diags_)
    , Context(ctx_)
{}

unsigned TypeResolver::checkType(QualType Q, bool used_public) {
    const Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        // ok
        return 0;
    case TC_POINTER:
        return checkType(cast<PointerType>(T)->getPointeeType(), used_public);
    case TC_ARRAY:
        return checkType(cast<ArrayType>(T)->getElementType(), used_public);
    case TC_UNRESOLVED:
        return checkUnresolvedType(cast<UnresolvedType>(T), used_public);
    case TC_ALIAS:
        return checkType(cast<AliasType>(T)->getRefType(), used_public);
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        // ok (TypeDecl will be checked)
        return 0;
    case TC_MODULE:
        assert(0 && "TBD");
        return 0;
    }
    assert(0);
}

unsigned TypeResolver::checkUnresolvedType(const UnresolvedType* type, bool used_public) {
    IdentifierExpr* moduleName = type->getModuleName();
    IdentifierExpr* typeName = type->getTypeName();
    SourceLocation tLoc = typeName->getLocation();
    const std::string& tName = typeName->getName();

    Decl* D = 0;
    if (moduleName) {   // mod.type
        const std::string& mName = moduleName->getName();
        const Module* mod = globals.findUsedModule(mName, moduleName->getLocation(), used_public);
        if (!mod) return 1;
        Decl* modDecl = globals.findSymbol(mName, moduleName->getLocation(), true, used_public);
        assert(modDecl);
        moduleName->setDecl(modDecl);

        D =  globals.findSymbolInModule(tName, tLoc, mod);
    } else {
        D = globals.findSymbol(tName, tLoc, true, used_public);
    }
    if (!D) return 1;
    TypeDecl* TD = dyncast<TypeDecl>(D);
    if (!TD) {
        StringBuilder name;
        type->printLiteral(name);
        Diags.Report(tLoc, diag::err_not_a_typename) << name.c_str();
        return 1;
    }
    bool external = globals.isExternal(D->getModule());
    if (used_public &&!external && !TD->isPublic()) {
        StringBuilder name;
        type->printLiteral(name);
        Diags.Report(tLoc, diag::err_non_public_type) << AnalyserUtils::fullName(TD->getModule()->getName(), TD->getName());
        //Diags.Report(tLoc, diag::err_non_public_type) << name;
        return 1;
    }
    D->setUsed();
    if (used_public || external) D->setUsedPublic();
    typeName->setDecl(TD);
    return 0;
}

QualType TypeResolver::resolveUnresolved(QualType Q) const {
    const Type* T = Q.getTypePtr();
    switch (Q->getTypeClass()) {
    case TC_BUILTIN:
        return Q;
    case TC_POINTER:
    {
        // Dont return new type if not needed
        const PointerType* P = cast<PointerType>(T);
        QualType t1 = P->getPointeeType();
        QualType Result = resolveUnresolved(t1);
        if (t1 == Result) return Q;
        // TODO qualifiers
        return Context.getPointerType(Result);
    }
    case TC_ARRAY:
    {
        const ArrayType* A = cast<ArrayType>(T);
        QualType t1 = A->getElementType();
        QualType Result = resolveUnresolved(t1);
        if (t1 == Result) return Q;
        // TODO qualifiers
        return Context.getArrayType(Result, A->getSizeExpr(), A->isIncremental());

    }
    case TC_UNRESOLVED:
    {
        const UnresolvedType* U = cast<UnresolvedType>(T);
        TypeDecl* TD = U->getDecl();
        assert(TD);
        QualType result = TD->getType();
        if (Q.isConstQualified()) result.addConst();
        if (Q.isVolatileQualified()) result.addVolatile();
        return result;
    }
    case TC_ALIAS:
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        return Q;
    case TC_MODULE:
        assert(0 && "TBD");
        return Q;
    }
    return Q;
}

QualType TypeResolver::resolveCanonicals(const Decl* D, QualType Q, bool set) const {
    Decls decls;
    if (D != 0 && !isa<AliasTypeDecl>(D)) decls.push_back(D);
    return checkCanonicals(decls, Q, set);
}


QualType TypeResolver::resolveType(QualType Q, bool usedPublic) {
    if (Q->hasCanonicalType()) return Q;    // should be ok already

    // basic resolving of Unresolved
    if (checkType(Q, usedPublic)) return QualType();
    QualType resolved = resolveUnresolved(Q);

    resolveCanonical(resolved);
    return resolved;
}

void TypeResolver::checkOpaqueType(SourceLocation loc, bool isPublic, QualType Q) {
    const StructType* ST = dyncast<StructType>(Q);
    if (!ST) return;

    const StructTypeDecl* S = ST->getDecl();
    if (!S->hasAttribute(ATTR_OPAQUE)) return;

    // if S is not in same Module, give 'used by value' error
    // if S is in same module, and used in public interface, give error: 'public decl with opaque..'
    if (globals.isExternal(S->getModule())) {
        Diags.Report(loc, diag::err_opaque_used_by_value) << S->DiagName();
    } else {
        if (isPublic) {
            Diags.Report(loc, diag::err_opaque_used_by_value_public_decl) << S->DiagName();
        }
    }
}

bool TypeResolver::requireCompleteType(SourceLocation loc, QualType Q, int msg) {
    if (Q.isIncompleteType()) {
        StringBuilder name;
        Q.DiagName(name);
        Diags.Report(loc, msg) << name;
        return false;
    }
    return true;
}

QualType TypeResolver::checkCanonicals(Decls& decls, QualType Q, bool set) const {
    if (Q->hasCanonicalType()) return Q.getCanonicalType();

    const Type* T = Q.getTypePtr();
    switch (Q->getTypeClass()) {
    case TC_BUILTIN:
        return Q;
    case TC_POINTER:
    {
        const PointerType* P = cast<PointerType>(T);
        QualType t1 = P->getPointeeType();
        // Pointee will always be in same ASTContext (file), since it's either built-in or UnresolvedType
        QualType t2 = checkCanonicals(decls, t1, set);
        if (!t2.isValid()) return t2;
        QualType canon;
        if (t1 == t2) canon = Q;
        else {
            canon = Context.getPointerType(t2);
            if (!canon->hasCanonicalType()) canon->setCanonicalType(canon);
        }
        assert(Q.isValid());
        if (set) P->setCanonicalType(canon);
        return canon;
    }
    case TC_ARRAY:
    {
        const ArrayType* A = cast<ArrayType>(T);
        QualType t1 = A->getElementType();
        // NOTE: qualifiers are lost here!
        QualType t2 = checkCanonicals(decls, t1, set);
        if (!t2.isValid()) return t2;
        QualType canon;
        if (t1 == t2) canon = Q;
        // NOTE: need size Expr, but set ownership to none
        else {
            canon = Context.getArrayType(t2, A->getSizeExpr(), A->isIncremental());
            if (!canon->hasCanonicalType()) canon->setCanonicalType(canon);
        }

        if (set) A->setCanonicalType(canon);
        return canon;
    }
    case TC_UNRESOLVED:
    {
        const UnresolvedType* U = cast<UnresolvedType>(T);
        TypeDecl* TD = U->getDecl();
        assert(TD);
        // check if exists
        if (!checkDecls(decls, TD)) {
            return QualType();
        }
        QualType canonical = checkCanonicals(decls, TD->getType(), false);
        if (set) U->setCanonicalType(canonical);
        return canonical;
    }
    case TC_ALIAS:
    {
        const AliasType* A = cast<AliasType>(T);
        if (!checkDecls(decls, A->getDecl())) {
            return QualType();
        }
        QualType canonical = checkCanonicals(decls, A->getRefType(), set);
        assert(Q.isValid());
        if (set) A->setCanonicalType(canonical);
        return canonical;
    }
    case TC_STRUCT:
        return Q.getCanonicalType();
    case TC_ENUM:
    {
        assert(0 && "TODO");
        return 0;
    }
    case TC_FUNCTION:
        return Q.getCanonicalType();
    case TC_MODULE:
        assert(0 && "TBD");
        return 0;
    }
    assert(0);
}

QualType TypeResolver::resolveCanonical(QualType Q) const {
    if (Q->hasCanonicalType()) return Q.getCanonicalType();

    const Type* T = Q.getTypePtr();
    switch (Q->getTypeClass()) {
    case TC_BUILTIN:
        return Q;
    case TC_POINTER:
    {
        const PointerType* P = cast<PointerType>(T);
        QualType t1 = P->getPointeeType();
        // Pointee will always be in same ASTContext (file), since it's either built-in or UnresolvedType
        QualType t2 = resolveCanonical(t1);
        assert(t2.isValid());
        if (t1 == t2) {
            Q->setCanonicalType(Q);
            return Q;
        } else {
            // TODO qualifiers
            QualType Canon = Context.getPointerType(t2);
            if (!Canon->hasCanonicalType()) Canon->setCanonicalType(Canon);
            Q->setCanonicalType(Canon);
            return Canon;
        }
    }
    case TC_ARRAY:
    {
        const ArrayType* A = cast<ArrayType>(T);
        QualType t1 = A->getElementType();
        // NOTE: qualifiers are lost here!
        QualType t2 = resolveCanonical(t1);
        if (t1 == t2) {
            Q->setCanonicalType(Q);
            return Q;
        } else  {
            // NOTE: need size Expr, but set ownership to none
            QualType Canon = Context.getArrayType(t2, A->getSizeExpr(), A->isIncremental());
            if (!Canon->hasCanonicalType()) Canon->setCanonicalType(Canon);
            Q->setCanonicalType(Canon);
            return Canon;
        }
    }
    case TC_UNRESOLVED:
        assert(0 && "should not get here");
        return QualType();
    case TC_ALIAS:
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        return Q.getCanonicalType();
    case TC_MODULE:
        assert(0 && "TBD");
        return Q;
    }
    assert(0);
}

bool TypeResolver::checkDecls(Decls& decls, const Decl* D) const {
    for (DeclsIter iter = decls.begin(); iter != decls.end(); ++iter) {
        if (*iter == D) {
            bool first = true;
            StringBuilder buf;
            for (DeclsIter I = decls.begin(); I != decls.end(); ++I) {
                if (first) first = false;
                else {
                    buf << " -> ";
                }
                buf << (*I)->getName();
            }
            buf << " -> " << D->getName();
            Diags.Report(D->getLocation(), diag::err_circular_typedef) << buf;
            return false;
        }
    }
    decls.push_back(D);
    return true;
}

