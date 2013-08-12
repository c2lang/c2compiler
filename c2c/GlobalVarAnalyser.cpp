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

#include "GlobalVarAnalyser.h"
#include "Decl.h"
#include "Expr.h"
#include "Type.h"
#include "Package.h"
#include "Scope.h"
#include "color.h"

using namespace C2;
using namespace clang;

GlobalVarAnalyser::GlobalVarAnalyser(FileScope& scope_, TypeContext& tc, clang::DiagnosticsEngine& Diags_)
    : globals(scope_)
    , typeContext(tc)
    , Diags(Diags_)
    , errors(0)
{}

GlobalVarAnalyser::~GlobalVarAnalyser() {}

bool GlobalVarAnalyser::handle(Decl* decl) {
    bool is_public = decl->isPublic();
    switch (decl->dtype()) {
    case DECL_FUNC:
        {
            FunctionDecl* FD = cast<FunctionDecl>(decl);
            QualType rtype = FD->getReturnType();
            Type* proto = typeContext.getFunction(rtype);
            for (unsigned i=0; i<FD->numArgs(); i++) {
                DeclExpr* arg = FD->getArg(i);
                QualType argType = arg->getType();
                proto->addArgument(argType);
            }
            FD->setFunctionType(QualType(proto));
            break;
        }
    case DECL_VAR:
        // nothing to do
        break;
    case DECL_ENUMVALUE:
        assert(0 && "TODO");
        break;
    case DECL_TYPE:
        {
            TypeDecl* TD = cast<TypeDecl>(decl);
            // TODO set CanonicalType here?
#if 0
            // not needed anymore?
            Type* T = TD->getType();
            if (T->isStructOrUnionType()) {
                MemberList* members = T->getMembers();
                for (unsigned i=0; i<members->size(); i++) {
                    DeclExpr* mem = (*members)[i];
                    Type* canonicalType = mem->getType()->getCanonical(typeContext);
                    mem->setCanonicalType(canonicalType);
                }
            }
#endif
        }
        break;
    case DECL_ARRAYVALUE:
    case DECL_USE:
        // nothing to do
        break;
    }
    return false;
}

