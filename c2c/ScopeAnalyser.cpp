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

#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "ScopeAnalyser.h"
#include "Decl.h"
#include "Expr.h"
#include "Package.h"
#include "Scope.h"
#include "color.h"

using namespace C2;
using namespace clang;

ScopeAnalyser::ScopeAnalyser(FileScope& scope_, clang::DiagnosticsEngine& Diags_)
    : globals(scope_)
    , Diags(Diags_)
    , errors(0)
{}

ScopeAnalyser::~ScopeAnalyser() {}

bool ScopeAnalyser::handle(Decl* decl) {
    bool is_public = decl->isPublic();
    switch (decl->getKind()) {
    case DECL_FUNC:
        {
            FunctionDecl* func = cast<FunctionDecl>(decl);
            // check return type
            checkType(func->getReturnType(), is_public);
            // check argument types
            for (unsigned i=0; i<func->numArgs(); i++) {
                VarDecl* V = func->getArg(i);
                checkType(V->getType(), is_public);
            }
            // TEMP (do elsewhere?)
            if (!is_public && func->getName() == "main") {
                Diags.Report(decl->getLocation(), diag::err_main_non_public);
            }
        }
        break;
    case DECL_VAR:
        {
            VarDecl* vd = cast<VarDecl>(decl);
            checkType(vd->getType(), is_public);
        }
        break;
    case DECL_ENUMVALUE:
        assert(0 && "TODO");
        break;
    case DECL_TYPE:
        {
            TypeDecl* td = cast<TypeDecl>(decl);
            checkType(td->getType(), is_public);
        }
        break;
    case DECL_STRUCTTYPE:
        checkStructType(cast<StructTypeDecl>(decl), is_public);
        break;
    case DECL_FUNCTIONTYPE:
        {
            FunctionTypeDecl* FTD = cast<FunctionTypeDecl>(decl);
            handle(FTD->getDecl());
        }
        break;
    case DECL_ARRAYVALUE:
        {
            ArrayValueDecl* A = cast<ArrayValueDecl>(decl);
            ScopeResult SR = globals.findSymbol(A->getName());
            if (!SR.ok) break;
            if (!SR.decl) {
                Diags.Report(A->getLocation(), diag::err_undeclared_var_use)
                << A->getName();
                break;
            }
            if (SR.external) {
                // TODO proper error
                fprintf(stderr, "Incremental Array Value for %s cannot be for external symbol\n", A->getName().c_str());
                break;
            }
            if (!isa<VarDecl>(SR.decl)) {
                fprintf(stderr, "TODO symbol '%s' is not a variable\n", A->getName().c_str());
                break;
            }
            VarDecl* VD = cast<VarDecl>(SR.decl);
            QualType T = VD->getType();
            if (!T.isArrayType()) {
                Diags.Report(A->getLocation(), diag::err_typecheck_subscript);
                break;
            }
            // TODO add to VarDecl
            VD->addInitValue(A);
        }
        break;
    case DECL_USE:
        checkUse(decl);
        break;
    }
    return false;
}

void ScopeAnalyser::checkType(QualType type, bool used_public) {
    errors += globals.checkType(type, used_public);
}

void ScopeAnalyser::checkStructType(StructTypeDecl* S, bool used_public) {
    for (unsigned i=0; i<S->getNumMembers(); i++) {
        Decl* M = S->getMember(i);
        handle(M);
    }
}

void ScopeAnalyser::checkUse(Decl* decl) {
    std::string pkgName = decl->getName();
    UseDecl* useDecl = cast<UseDecl>(decl);

    // check if package exists
    const Package* pkg = globals.findAnyPackage(pkgName);
    if (pkg == 0) {
        Diags.Report(decl->getLocation(), diag::err_unknown_package) << pkgName;
        errors++;
        return;
    }

    // check if aliasname is not a package
    const std::string& aliasName = useDecl->getAlias();
    if (aliasName != "") {
        const Package* pkg2 = globals.findAnyPackage(aliasName);
        if (pkg2) {
            Diags.Report(useDecl->getAliasLocation(), diag::err_alias_is_package) << aliasName;
            errors++;
            return;
        }
        pkgName = aliasName;
    }

    // add to Scope
    globals.addPackage(useDecl->isLocal(), pkgName, pkg);
}

