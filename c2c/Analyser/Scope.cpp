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
#include <assert.h>

#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/Scope.h"
#include "Analyser/AnalyserUtils.h"
#include "AST/Package.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "Utils/color.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace clang;

DynamicScope::DynamicScope() : Flags(0) {}


Scope::Scope(const std::string& name_, const Pkgs& pkgs_, clang::DiagnosticsEngine& Diags_)
    : scopeIndex(0)
    , curScope(0)
    , allPackages(pkgs_)
    , myPkg(0)
    , Diags(Diags_)
{
    // add own package to scope
    myPkg = findAnyPackage(name_);
    addPackage(true, name_, myPkg);
}

void Scope::addPackage(bool isLocal, const std::string& name_, const Package* pkg) {
    assert(pkg);
    if (isLocal) locals.push_back(pkg);
    packages[name_] = pkg;
}

const Package* Scope::findPackage(const std::string& name) const {
    PackagesConstIter iter = packages.find(name);
    if (iter == packages.end()) return 0;
    return iter->second;
}

const Package* Scope::usePackage(const std::string& name, clang::SourceLocation loc) const {
    const Package* P = findPackage(name);
    if (!P) {
        // check if used with alias (then fullname is forbidden)
        for (PackagesConstIter iter = packages.begin(); iter != packages.end(); ++iter) {
            const Package* p = iter->second;
            if (p->getName() == name) {
                Diags.Report(loc, diag::err_package_has_alias) << name << iter->first;
                return 0;
            }
        }
        PkgsConstIter iter = allPackages.find(name);
        if (iter == allPackages.end()) {
            Diags.Report(loc, diag::err_unknown_package) << name;
        } else {
            Diags.Report(loc, diag::err_package_not_used) << name;
        }
    }
    return P;
}

const Package* Scope::findAnyPackage(const std::string& name) const {
    PkgsConstIter iter = allPackages.find(name);
    if (iter == allPackages.end()) return 0;
    return iter->second;
}

void Scope::dump() const {
    fprintf(stderr, "used packages:\n");
    for (PackagesConstIter iter = packages.begin(); iter != packages.end(); ++iter) {
        fprintf(stderr, "  %s (as %s)\n", iter->second->getName().c_str(), iter->first.c_str());
    }
    fprintf(stderr, "External symbol cache:\n");
    for (GlobalsConstIter iter = globalCache.begin(); iter != globalCache.end(); ++iter) {
        fprintf(stderr, "  %s\n", iter->first.c_str());
    }
}

ScopeResult Scope::findGlobalSymbol(const std::string& symbol, clang::SourceLocation loc) const {
    // lookup in global cache first, return if found
    GlobalsConstIter iter = globalCache.find(symbol);
    if (iter != globalCache.end()) {
        return iter->second;
    }

    ScopeResult result;
    // lookup in used package list
    const Package* pkg = findPackage(symbol);
    if (pkg) {
        result.ok = true;
        result.pkg = pkg;
        // add to cache
        globalCache[symbol] = result;
        return result;
    }

    // TODO cleanup below
    bool ambiguous = false;
    bool visible_match = false;
    // return private symbol only if no public symbol is found
    for (LocalsConstIter iter = locals.begin(); iter != locals.end(); ++iter) {
        pkg = *iter;
        Decl* decl = pkg->findSymbol(symbol);
        if (!decl) continue;

        bool external = isExternal(pkg);
        bool visible = !(external && !decl->isPublic());
        if (result.decl) {
            // if previous result was non-visible, replace with new one
            if (visible_match == visible) {
                if (!ambiguous) {
                    Diags.Report(loc, diag::err_ambiguous_symbol) << symbol;
                    // NASTY: are different FileManagers!
                    // TEMP just use 0 location
                    assert(result.decl->getPackage());
                    Diags.Report(SourceLocation(), diag::note_function_suggestion)
                        << AnalyserUtils::fullName(result.decl->getPackage()->getName(), result.decl->getName());

                    ambiguous = true;
                    result.ok = false;
                }
                Diags.Report(SourceLocation(), diag::note_function_suggestion)
                    << AnalyserUtils::fullName(pkg->getName(), decl->getName());

                continue;
            }
            if (!visible_match) { // replace with visible symbol
                result.decl = decl;
                visible_match = visible;
            }
        } else {
            result.decl = decl;
            visible_match = visible;
        }
    }
    if (result.decl) {
        if (!visible_match) {
            Diags.Report(loc, diag::err_not_public) << symbol;
            result.decl = 0;
            result.ok = false;
        }
        if (result.ok) globalCache[symbol] = result;
    }
    return result;
}

ScopeResult Scope::findSymbol(const std::string& symbol, clang::SourceLocation loc) const {
    // lookup in local vars (all in cache), return if found
    LocalCacheConstIter iter = localCache.find(symbol);
    if (iter != localCache.end()) {
        ScopeResult result;
        result.ok = true;
        result.decl = iter->second;
        return result;
    }

    // otherwise search globals
    return findGlobalSymbol(symbol, loc);
}

ScopeResult Scope::findSymbolInPackage(const std::string& name, clang::SourceLocation loc, const Package* pkg) const {
    ScopeResult res;
    res.decl = pkg->findSymbol(name);
    if (!res.decl) {
        Diags.Report(loc, diag::err_unknown_package_symbol) << pkg->getName() << name;
        res.ok = false;
    } else {
        // if external package, check visibility
        if (isExternal(pkg) && !res.decl->isPublic()) {
            Diags.Report(loc, diag::err_not_public) << AnalyserUtils::fullName(pkg->getName(), name);
            res.decl = 0;
            res.ok = false;
        }
    }
    return res;
}

ScopeResult Scope::findSymbolInUsed(const std::string& symbol) const {
    ScopeResult result;
    // symbol can be package name or symbol within package
    const Package* pkg = findPackage(symbol);
    if (pkg) {
        result.pkg = pkg;
        return result;
    }

    // search in all used packages
    for (PackagesConstIter iter = packages.begin(); iter != packages.end(); ++iter) {
        const Package* pkg2 = iter->second;
        Decl* decl = pkg2->findSymbol(symbol);
        if (!decl) continue;

        // NOTE: dont check ambiguity here (just return first match)
        result.decl = decl;
        //bool external = isExternal(pkg2);
        //bool visible = !(external && !decl->isPublic());
        // Q: check visibility and set ok?
    }
    return result;
}

bool Scope::checkSymbol(const VarDecl* V) const {
    // lookup in local scopes, error if found
    LocalCacheConstIter iter = localCache.find(V->getName());
    if (iter != localCache.end()) {
        const VarDecl* Old = iter->second;
        Diags.Report(V->getLocation(), diag::err_redefinition)
            << V->getName();
        Diags.Report(Old->getLocation(), diag::note_previous_definition);
        return false;
    }

    // check if symbol is a package, error is so
    const Package* Pkg = findPackage(V->getName());
    if (Pkg) {
        fprintf(stderr, "TODO ERROR symbol is package\n");
        return false;
    }

    // lookup in own package, error if found
    Decl* Old= myPkg->findSymbol(V->getName());
    if (Old) {
        Diags.Report(V->getLocation(), diag::err_redefinition)
            << V->getName();
        // NASTY, loc might be other file!!
        Diags.Report(Old->getLocation(), diag::note_previous_definition);
        return false;
    }
    return true;
}

void Scope::addSymbol(VarDecl* V) {
    // NOTE: must already be checked with checkSymbol
    assert(curScope);
    curScope->decls.push_back(V);
    localCache[V->getName()] = V;
}

void Scope::EnterScope(unsigned flags) {
    assert (scopeIndex < MAX_SCOPE_DEPTH && "out of scopes");
    DynamicScope* parent = curScope;
    curScope = &scopes[scopeIndex];
    curScope->Flags = flags;

    if (parent) {
        if (parent->Flags & BreakScope) curScope->Flags |= BreakScope;
        if (parent->Flags & ContinueScope) curScope->Flags |= ContinueScope;
    }

    scopeIndex++;
}

void Scope::ExitScope() {
    for (unsigned i=0; i<curScope->decls.size(); i++) {
        VarDecl* D = curScope->decls[i];
        if (!D->isUsed()) {
            unsigned msg = diag::warn_unused_variable;
            if (D->isParameter()) msg = diag::warn_unused_parameter;
            Diags.Report(D->getLocation(), msg) << D->getName();
        }
        // remove from localCache
        LocalCacheIter iter = localCache.find(D->getName());
        localCache.erase(iter);
    }
    curScope->decls.clear();
    curScope->Flags = 0;

    scopeIndex--;
    if (scopeIndex == 0) curScope = 0;
    else curScope = &scopes[scopeIndex-1];
}

