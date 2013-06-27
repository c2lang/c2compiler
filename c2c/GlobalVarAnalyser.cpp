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
            // TODO extract to function to create CanonicalType for FunctionDecl
            FunctionDecl* FD = DeclCaster<FunctionDecl>::getType(decl);
            Type* rtypeCanon = FD->getReturnType()->getCanonical(typeContext);
            Type* proto = typeContext.getFunction(rtypeCanon);
            for (unsigned i=0; i<FD->numArgs(); i++) {
                DeclExpr* arg = FD->getArg(i);
                Type* canonicalType = arg->getType()->getCanonical(typeContext);
                arg->setCanonicalType(canonicalType);
                proto->addArgument(canonicalType);
            }
            FD->setCanonicalType(proto);
            break;
        }
    case DECL_VAR:
        {
            VarDecl* VD = DeclCaster<VarDecl>::getType(decl);
            Type* canonical = VD->getCanonicalType();
            // TODO assert canonical and return it.
            // Don't set here, because VD can be in different file!
            if (!canonical) {
                canonical = VD->getType()->getCanonical(typeContext);
                VD->setCanonicalType(canonical);
            }
            break;
        }
    case DECL_ENUMVALUE:
        assert(0 && "TODO");
        break;
    case DECL_TYPE:
        {
            // set canonical type for struct members
            TypeDecl* TD = DeclCaster<TypeDecl>::getType(decl);
            Type* T = TD->getType();
            if (T->isStructOrUnionType()) {
                MemberList* members = T->getMembers();
                for (unsigned i=0; i<members->size(); i++) {
                    DeclExpr* mem = (*members)[i];
                    Type* canonicalType = mem->getType()->getCanonical(typeContext);
                    mem->setCanonicalType(canonicalType);
                }
            }
        }
        break;
    case DECL_ARRAYVALUE:
    case DECL_USE:
        // nothing to do
        break;
    }
    return false;
}

