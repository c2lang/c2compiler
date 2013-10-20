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

#include <stdio.h>
#include <assert.h>

#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/Scope.h"
#include "Analyser/TypeResolver.h"
#include "AST/Package.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace clang;

TypeResolver::TypeResolver(Scope& g, clang::DiagnosticsEngine& Diags_, TypeContext& tc_)
    : globals(g)
    , Diags(Diags_)
    , typeContext(tc_)
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
        // will be removed?
        return 0;
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        // ok (TypeDecl will be checked)
        return 0;
    }
}

unsigned TypeResolver::checkUnresolvedType(const UnresolvedType* type, bool used_public) {
    // TODO refactor
    Expr* id = type->getExpr();
    const Package* pkg = 0;
    switch (id->getKind()) {
    case EXPR_IDENTIFIER:   // unqualified
        {
            IdentifierExpr* I = cast<IdentifierExpr>(id);
            ScopeResult res = globals.findSymbol(I->getName(), I->getLocation());
            if (!res.ok) return 1;
            if (res.pkg) {
                Diags.Report(I->getLocation(), diag::err_not_a_typename) << I->getName();
                return 1;
            }
            if (!res.decl) {
                Diags.Report(I->getLocation(), diag::err_unknown_typename) << I->getName();
                return 1;
            }
            TypeDecl* td = dyncast<TypeDecl>(res.decl);
            if (!td) {
                Diags.Report(I->getLocation(), diag::err_not_a_typename) << I->getName();
                return 1;
            }
            bool external = globals.isExternal(res.decl->getPackage());
            if (used_public && !external && !td->isPublic()) {
                Diags.Report(I->getLocation(), diag::err_non_public_type) << I->getName();
                return 1;
            }
            // ok
            I->setDecl(res.decl);
            type->setMatch(td);
        }
        break;
    case EXPR_MEMBER:   // fully qualified
        {
            MemberExpr* M = cast<MemberExpr>(id);
            Expr* base = M->getBase();
            IdentifierExpr* pkg_id = cast<IdentifierExpr>(base);
            const std::string& pName = pkg_id->getName();
            // check if package exists
            pkg = globals.usePackage(pName, pkg_id->getLocation());
            if (!pkg) return 1;
            M->setPkgPrefix(true);
            // check member
            IdentifierExpr* member_id = M->getMember();
            // check Type
            ScopeResult res = globals.findSymbolInPackage(member_id->getName(), member_id->getLocation(), pkg);
            if (!res.ok) return 1;
            TypeDecl* td = dyncast<TypeDecl>(res.decl);
            if (!td) {
                Diags.Report(member_id->getLocation(), diag::err_not_a_typename) << M->getFullName();
                return 1;
            }
            bool external = globals.isExternal(res.decl->getPackage());
            if (used_public && !external && !td->isPublic()) {
                Diags.Report(member_id->getLocation(), diag::err_non_public_type) << M->getFullName();
                return 1;
            }
            // ok
            member_id->setDecl(res.decl);
            type->setMatch(td);
        }
        break;
    default:
        assert(0);
    }
    return 0;
}

QualType TypeResolver::resolveCanonicals(const Decl* D, QualType Q, bool set) const {
    Decls decls;
    if (D != 0) decls.push_back(D);
    return checkCanonicals(decls, Q, set);
}

QualType TypeResolver::checkCanonicals(Decls& decls, QualType Q, bool set) const {
    const Type* T = Q.getTypePtr();
    if (T->hasCanonicalType()) return T->getCanonicalType();

    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return T->getCanonicalType();
    case TC_POINTER:
        {
            const PointerType* P = cast<PointerType>(T);
            QualType t1 = P->getPointeeType();
            // Pointee will always be in same TypeContext (file), since it's either built-in or UnresolvedType
            QualType t2 = checkCanonicals(decls, t1, set);
            if (!t2.isValid()) return t2;
            QualType canonical;
            // create new PointerType if PointeeType has different canonical than itself
            if (t1 == t2) canonical = Q;
            else canonical = typeContext.getPointerType(t2);

            if (set) P->setCanonicalType(canonical);
            return canonical;
        }
    case TC_ARRAY:
        {
            const ArrayType* A = cast<ArrayType>(T);
            QualType t1 = A->getElementType();
            QualType t2 = checkCanonicals(decls, t1, true);
            if (!t2.isValid()) return t2;
            QualType canonical;
            if (t1 == t2) canonical = Q;
            // NOTE: need size Expr, but set ownership to none
            else canonical = typeContext.getArrayType(t2, A->getSize(), false);
            if (set) A->setCanonicalType(canonical);
            return canonical;
        }
    case TC_UNRESOLVED:
        {
            const UnresolvedType* U = cast<UnresolvedType>(T);
            TypeDecl* TD = U->getMatch();
            assert(TD);
            TD->setUsed();
            // check if exists
            if (!checkDecls(decls, TD)) {
                return QualType();
            }
            QualType canonical = checkCanonicals(decls, TD->getType(), false);
            if (set) U->setCanonicalType(canonical);
            return canonical;
        }
    case TC_ALIAS:
        return 0;
    case TC_STRUCT:
        return T->getCanonicalType();
    case TC_ENUM:
        {
            assert(0 && "TODO");
            return 0;
        }
    case TC_FUNCTION:
        return T->getCanonicalType();
    }
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

