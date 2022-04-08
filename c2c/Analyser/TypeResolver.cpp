/* Copyright 2013-2022 Bas van den Berg
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

#include "Clang/SourceLocation.h"
#include "Clang/ParseDiagnostic.h"
#include "Clang/SemaDiagnostic.h"

#include "Analyser/Scope.h"
#include "Analyser/TypeResolver.h"
#include "AST/Module.h"

using namespace C2;
using namespace c2lang;

TypeResolver::TypeResolver(Scope& g, c2lang::DiagnosticsEngine& Diags_)
    : globals(g)
    , Diags(Diags_)
{}

bool TypeResolver::checkOpaqueType(SourceLocation loc, bool isPublic, QualType Q) {
    const StructType* ST = dyncast<StructType>(Q);
    if (!ST) return true;

    const StructTypeDecl* S = ST->getDecl();
    if (!S->hasAttribute(ATTR_OPAQUE)) return true;

    // if S is not in same Module, give 'used by value' error
    // if S is in same module, and used in public interface, give error: 'public decl with opaque..'
    if (globals.isExternal(S->getModule())) {
        Diags.Report(loc, diag::err_opaque_used_by_value) << S->DiagName();
        return false;
    } else {
        if (isPublic) {
            Diags.Report(loc, diag::err_opaque_used_by_value_public_decl) << S->DiagName();
            return false;
        }
    }
    return true;
}

