/* Copyright 2013,2014 Bas van den Berg
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
#include <stdio.h>
#include <assert.h>

#include <clang/Basic/SourceLocation.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/Scope.h"
#include "Analyser/TypeChecker.h"
#include "Analyser/TypeFinder.h"
#include "Analyser/constants.h"
#include "AST/Package.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace clang;

// 0 = ok,
// 1 = loss of integer precision,
// 2 = sign-conversion,
// 3 = float->integer,
// 4 = incompatible,
// 5 = loss of FP precision
static int type_conversions[14][14] = {
    // I8  I16  I32  I64   U8  U16  U32  U64  F32  F64  Bool  Void
    // I8 ->
    {   0,   0,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I16 ->
    {   1,   0,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I32 ->
    {   1,   1,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I64 ->
    {   1,   1,   1,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // U8 ->
    {   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   4},
    // U16 ->
    {   1,   2,   0,   0,   1,   0,   0,   0,   0,   0,    0,   4},
    // U32 ->
    {   1,   1,   2,   0,   1,   1,   0,   0,   0,   0,    0,   4},
    // U64 ->
    {   1,   1,   1,   2,   1,   1,   1,   0,   0,   0,    0,   4},
    // F32 ->
    {   3,   3,   3,   3,   3,   3,   3,   3,   0,   0,    4,   4},
    // F64 ->
    {   3,   3,   3,   3,   3,   3,   3,   3,   5,   0,    4,   4},
    // BOOL ->
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   4},
    // VOID ->
    {  4,    4,   4,   4,  4,   4,   4,   4,   4,   4,    4,    0},
};


TypeChecker::TypeChecker(Scope& g, clang::DiagnosticsEngine& Diags_, TypeContext& tc_)
    : globals(g)
    , Diags(Diags_)
    , typeContext(tc_)
{}

unsigned TypeChecker::checkType(QualType Q, bool used_public) {
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
    }
}

unsigned TypeChecker::checkUnresolvedType(const UnresolvedType* type, bool used_public) {
    const std::string& pName = type->getPName();
    const std::string& tName = type->getTName();
    SourceLocation tLoc = type->getTLoc();
    Decl* D = 0;
    if (!pName.empty()) {   // pkg.type
        // check if package exists
        const Package* pkg = globals.usePackage(pName, type->getPLoc());
        if (!pkg) return 1;
        // check type
        ScopeResult res = globals.findSymbolInPackage(tName, tLoc, pkg);
        if (!res.isOK()) return 1;
        D = res.getDecl();
    } else {
        ScopeResult res = globals.findSymbol(tName, tLoc);
        if (!res.isOK()) return 1;
        if (res.getPackage()) {
            Diags.Report(tLoc, diag::err_not_a_typename) << tName;
            return 1;
        }
        D = res.getDecl();
        if (!D) {
            Diags.Report(tLoc, diag::err_unknown_typename) << tName;
            return 1;
        }
    }
    TypeDecl* TD = dyncast<TypeDecl>(D);
    if (!TD) {
        StringBuilder name;
        type->printLiteral(name);
        Diags.Report(tLoc, diag::err_not_a_typename) << name;
        return 1;
    }
    bool external = globals.isExternal(D->getPackage());
    if (used_public &&!external && !TD->isPublic()) {
        StringBuilder name;
        type->printLiteral(name);
        Diags.Report(tLoc, diag::err_non_public_type) << name;
        return 1;
    }
    D->setUsed();
    if (used_public || external) D->setUsedPublic();
    type->setDecl(TD);
    return 0;
}

QualType TypeChecker::resolveCanonicals(const Decl* D, QualType Q, bool set) const {
    Decls decls;
    if (D != 0 && !isa<AliasTypeDecl>(D)) decls.push_back(D);
    return checkCanonicals(decls, Q, set);
}

QualType TypeChecker::resolveUnresolved(QualType Q) const {
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
            return typeContext.getPointerType(Result);
        }
    case TC_ARRAY:
        {
            const ArrayType* A = cast<ArrayType>(T);
            QualType t1 = A->getElementType();
            QualType Result = resolveUnresolved(t1);
            if (t1 == Result) return Q;
            // TODO qualifiers
            return typeContext.getArrayType(Result, A->getSizeExpr(), false);

        }
    case TC_UNRESOLVED:
        {
            const UnresolvedType* U = cast<UnresolvedType>(T);
            TypeDecl* TD = U->getDecl();
            assert(TD);
            return TD->getType();
        }
    case TC_ALIAS:
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        return Q;
    }
    return Q;
}

QualType TypeChecker::resolveType(QualType Q, bool usedPublic) {
    if (Q->hasCanonicalType()) return Q;    // should be ok already

    // basic resolving of Unresolved
    if (checkType(Q, usedPublic)) return QualType();
    QualType resolved = resolveUnresolved(Q);

    resolveCanonical(resolved);
    return resolved;
}

QualType TypeChecker::checkCanonicals(Decls& decls, QualType Q, bool set) const {
    if (Q->hasCanonicalType()) return Q.getCanonicalType();

    const Type* T = Q.getTypePtr();
    switch (Q->getTypeClass()) {
    case TC_BUILTIN:
        return Q;
    case TC_POINTER:
        {
            // TODO Helper to get PointerType (can be aliased?)
            const PointerType* P = cast<PointerType>(T);
            QualType t1 = P->getPointeeType();
            // Pointee will always be in same TypeContext (file), since it's either built-in or UnresolvedType
            QualType t2 = checkCanonicals(decls, t1, set);
            if (!t2.isValid()) return t2;
            QualType canon;
            if (t1 == t2) canon = Q;
            else {
                canon = typeContext.getPointerType(t2);
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
                canon = typeContext.getArrayType(t2, A->getSizeExpr(), false);
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
    }
}

QualType TypeChecker::resolveCanonical(QualType Q) const {
    if (Q->hasCanonicalType()) return Q.getCanonicalType();

    const Type* T = Q.getTypePtr();
    switch (Q->getTypeClass()) {
    case TC_BUILTIN:
        return Q;
    case TC_POINTER:
        {
            const PointerType* P = cast<PointerType>(T);
            QualType t1 = P->getPointeeType();
            // Pointee will always be in same TypeContext (file), since it's either built-in or UnresolvedType
            QualType t2 = resolveCanonical(t1);
            assert(t2.isValid());
            if (t1 == t2) {
                Q->setCanonicalType(Q);
                return Q;
            } else {
                // TODO qualifiers
                QualType Canon = typeContext.getPointerType(t2);
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
                QualType Canon = typeContext.getArrayType(t2, A->getSizeExpr(), false);
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
    }
}
bool TypeChecker::checkDecls(Decls& decls, const Decl* D) const {
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

bool TypeChecker::checkCompatible(QualType left, const Expr* expr) const {
    QualType right = expr->getType();
    //right = TypeFinder::findType(expr);
    assert(left.isValid());
    const Type* canon = left.getCanonicalType();
    switch (canon->getTypeClass()) {
    case TC_BUILTIN:
        return checkBuiltin(left, right, expr, true);
    case TC_POINTER:
        return checkPointer(left, right, expr);
    case TC_ARRAY:
        break;
    case TC_UNRESOLVED:
        break;
    case TC_ALIAS:
        break;
    case TC_STRUCT:
        break;
    case TC_ENUM:
        break;
    case TC_FUNCTION:
        break;
    }
    return false;
}

// Convert smaller types to int, others remain the same
// This function should only be called if Expr's type is ok for unary operator
QualType TypeChecker::UsualUnaryConversions(Expr* expr) const {
    const Type* canon = expr->getType().getCanonicalType();
    assert(canon->isBuiltinType());
    const BuiltinType* BI = cast<BuiltinType>(canon);
    if (BI->isPromotableIntegerType()) {
        // TODO keep flags (const, etc)?
        expr->setImpCast(BuiltinType::Int32);
        return Type::Int32();
    }
    return expr->getType();
}

bool TypeChecker::checkBuiltin(QualType left, QualType right, const Expr* expr, bool first) const {
    if (right->isBuiltinType()) {
        // NOTE: canonical is builtin, var itself my be UnresolvedType etc
        const BuiltinType* Right = cast<BuiltinType>(right.getCanonicalType());
        const BuiltinType* Left = cast<BuiltinType>(left.getCanonicalType());
        int rule = type_conversions[Right->getKind()][Left->getKind()];
        // 0 = ok, 1 = loss of precision, 2 sign-conversion, 3=float->integer, 4 incompatible, 5 loss of FP prec.
        // TODO use matrix with allowed conversions: 3 options: ok, error, warn
        int errorMsg = 0;

        if (first) {
            if (Right->getKind() != Left->getKind()) {
                // add Implicit Cast
                // TODO remove const cast
                Expr* E = const_cast<Expr*>(expr);
                E->setImpCast(Left->getKind());
            }
            if (rule == 1) {
                QualType Q = TypeFinder::findType(expr);
                return checkBuiltin(left, Q, expr, false);
            }
        }

        switch (rule) {
        case 0:
            return true;
        case 1: // loss of precision
            errorMsg = diag::warn_impcast_integer_precision;
            break;
        case 2: // sign-conversion
            errorMsg = diag::warn_impcast_integer_sign;
            break;
        case 3: // float->integer
            errorMsg = diag::warn_impcast_float_integer;
            break;
        case 4: // incompatible
            errorMsg = diag::err_illegal_type_conversion;
            break;
        case 5: // loss of fp-precision
            errorMsg = diag::warn_impcast_float_precision;
            break;
        default:
            assert(0 && "should not come here");
        }
        StringBuilder buf1(MAX_LEN_TYPENAME);
        StringBuilder buf2(MAX_LEN_TYPENAME);
        right.DiagName(buf1);
        left.DiagName(buf2);
        // TODO error msg depends on conv type (see clang errors)
        Diags.Report(expr->getLocation(), errorMsg) << buf1 << buf2
            << expr->getSourceRange();
        return false;
    }

    StringBuilder buf1(MAX_LEN_TYPENAME);
    StringBuilder buf2(MAX_LEN_TYPENAME);
    right.DiagName(buf1);
    left.DiagName(buf2);
    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(expr->getLocation(), diag::err_illegal_type_conversion) << buf1 << buf2;
    return false;
}

bool TypeChecker::checkPointer(QualType left, QualType right, const Expr* expr) const {
    if (right->isPointerType()) {
        // TODO
        return true;
    }
    if (right->isArrayType()) {
        // TODO
        return true;
    }
    StringBuilder buf1(MAX_LEN_TYPENAME);
    StringBuilder buf2(MAX_LEN_TYPENAME);
    right.DiagName(buf1);
    left.DiagName(buf2);
    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(expr->getLocation(), diag::err_illegal_type_conversion)
            << buf1 << buf2;
    return false;
}

