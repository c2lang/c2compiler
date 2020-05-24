/* Copyright 2013-2020 Bas van den Berg
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

#ifndef ANALYSER_TYPE_RESOLVER_H
#define ANALYSER_TYPE_RESOLVER_H

#include "AST/Type.h"
#include "Clang/SourceLocation.h"

using c2lang::SourceLocation;

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class Decl;
class Scope;

class TypeResolver {
public:
    TypeResolver(Scope& g, c2lang::DiagnosticsEngine& Diags_);

    bool checkOpaqueType(SourceLocation loc, bool isPublic, QualType Q);
private:
    Scope& globals;
    c2lang::DiagnosticsEngine& Diags;
};

}

#endif

