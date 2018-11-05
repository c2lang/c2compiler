/* Copyright 2013-2018 Bas van den Berg
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

#include <vector>

#include "AST/Type.h"
#include "Clang/SourceLocation.h"

using c2lang::SourceLocation;

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class Decl;
class Scope;
class ASTContext;

class TypeResolver {
public:
    TypeResolver(Scope& g, c2lang::DiagnosticsEngine& Diags_, ASTContext& ctx_);

    // resolving of TypeDecls (during Type Analysis phase)
    unsigned checkType(QualType Q, bool used_public);
    QualType resolveUnresolved(QualType Q) const;
    QualType resolveCanonicals(const Decl* D, QualType Q, bool set) const;

    // resolving of other Types (after Type Analysis phase)
    QualType resolveType(QualType Q, bool usedPublic);

    void checkOpaqueType(SourceLocation loc, bool isPublic, QualType Q);

    bool requireCompleteType(SourceLocation loc, QualType Q, int msg);
private:
    QualType resolveCanonical(QualType Q) const;
    unsigned checkUnresolvedType(const UnresolvedType* type, bool used_public);

    typedef std::vector<const Decl*> Decls;
    typedef Decls::iterator DeclsIter;
    QualType checkCanonicals(Decls& decls, QualType Q, bool set) const;
    bool checkDecls(Decls& decls, const Decl* D) const;

    Scope& globals;
    c2lang::DiagnosticsEngine& Diags;
    ASTContext& Context;
};

}

#endif

