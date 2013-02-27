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

#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <map>
#include <vector>

#include "Package.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Decl;
class Type;
class IdentifierExpr;

class ScopeResult {
public:
    ScopeResult()
        : pkg(0)
        , decl(0)
        , ambiguous(false)
        , external(true)
        , visible(true)
    {}

    const Package* pkg; // pkg is only set if Symbol is a global (not stack)
    Decl* decl;
    bool ambiguous;     // ambiguous lookup (returns first result)
    bool external;      // package is external
    bool visible;       // symbol is non-public and used externally
};


class GlobalScope {
public:
    GlobalScope(const std::string& name_, const Pkgs& pkgs_, clang::DiagnosticsEngine& Diags_);

    // TODO rename to findUsedPackade
    const Package* findPackage(const std::string& name) const;
    const Package* findAnyPackage(const std::string& name) const;
    void addPackage(bool isLocal, const std::string& name_, const Package* pkg);

    ScopeResult findSymbol(const std::string& name) const;

    int checkType(Type* type, bool used_public = false);
    bool isExternal(const Package* pkg) const;

    void dump() const;
private:
    int checkStructType(Type* type, bool used_public);
    int checkUserType(IdentifierExpr* id, bool used_public);

    const std::string pkgName;

    // locals (or used local)
    typedef std::vector<const Package*> Locals;
    typedef Locals::const_iterator LocalsConstIter;
    Locals locals;

    // used packages (use <as>)
    typedef std::map<std::string, const Package*> Packages;
    typedef Packages::const_iterator PackagesConstIter;
    typedef Packages::iterator PackagesIter;
    Packages packages;

    // all packages
    const Pkgs& allPackages;

    clang::DiagnosticsEngine& Diags;
};


class Scope {
public:
    Scope(GlobalScope& globals_, Scope* parent_);

    ScopeResult findSymbol(const std::string& name) const;
    ScopeResult findSymbol(const std::string& pkgname, const std::string& name) const;
    void addDecl(Decl* d);

    Scope* getParent() const { return parent; }
private:
    GlobalScope& globals;
    Scope* parent;

    // local decls (in scope)
    typedef std::vector<Decl*> Decls;
    typedef Decls::const_iterator DeclsConstIter;
    Decls decls;
};

}

#endif

