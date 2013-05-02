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
        // nothing to do
        break;
    case DECL_VAR:
        {
            VarDecl* VD = DeclCaster<VarDecl>::getType(decl);
            assert(VD);
            Type* canonical = VD->getCanonicalType();
            // TODO assert canonical and return it.
            // Don't set here, because VD can be in different file!
            if (!canonical) {
                canonical = VD->getType()->getCanonical(typeContext);
                VD->setCanonicalType(canonical);
            }
            break;
        }
    case DECL_TYPE:
    case DECL_ARRAYVALUE:
    case DECL_USE:
        // nothing to do
        break;
    }
    return false;
}

