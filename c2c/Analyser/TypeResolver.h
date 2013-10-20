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

#ifndef TYPE_RESOLVER_H
#define TYPE_RESOLVER_H

#include <string>
#include <vector>

#include "AST/Type.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Decl;
class Scope;


class TypeResolver {
public:
    TypeResolver(Scope& g, clang::DiagnosticsEngine& Diags_, TypeContext& tc_);

    unsigned checkType(QualType Q, bool used_public);
    QualType resolveCanonicals(const Decl* D, QualType Q, bool set) const;
private:
    unsigned checkUnresolvedType(const UnresolvedType* type, bool used_public);

    typedef std::vector<const Decl*> Decls;
    typedef Decls::iterator DeclsIter;
    QualType checkCanonicals(Decls& decls, QualType Q, bool set) const;
    bool checkDecls(Decls& decls, const Decl* D) const;

    Scope& globals;
    clang::DiagnosticsEngine& Diags;
    TypeContext& typeContext;
};

}

#endif

