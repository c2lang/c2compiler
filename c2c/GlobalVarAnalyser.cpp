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
    switch (decl->getKind()) {
    case DECL_FUNC:
    case DECL_VAR:
        // nothing to do
        break;
    case DECL_ENUMVALUE:
        assert(0 && "TODO");
        break;
    case DECL_TYPE:
        // need to set CanonicalTypes?
        {
            TypeDecl* T = cast<TypeDecl>(decl);
            QualType qt = T->getType();
            if (qt->isEnumType()) {
                if (!qt->getConstants()) break;
                ConstantList* clist = qt->getConstants();
                // TEMP use unsigned only
                unsigned lastValue = 0;
                for (unsigned i=0; i<clist->size(); i++) {
                    EnumConstantDecl* C = (*clist)[i];
                    if (C->getInitValue()) {
#warning "TODO evaluate expr as number for enums"
                        //C->setValue(lastValue);
                        //lastValue = val;
                        // TEMP just ignore
                        C->setValue(lastValue);
                        lastValue++;
                    } else {
                        C->setValue(lastValue);
                        lastValue++;
                    }
                    // TODO check for duplicates
                }
            }
        }
        break;
    case DECL_STRUCTTYPE:
    case DECL_FUNCTIONTYPE:
        // nothing to do?
        break;
    case DECL_ARRAYVALUE:
    case DECL_USE:
        // nothing to do
        break;
    }
    return false;
}

