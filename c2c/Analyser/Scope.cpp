/* Copyright 2013,2014 Bas van den Berg
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
    assert(myPkg);
}

bool Scope::addImportDecl(ImportDecl* useDecl) {
    clang::SourceLocation Loc = useDecl->getLocation();
    if (useDecl->getAliasLocation().isValid()) {
        Loc = useDecl->getAliasLocation();
    }
    const Package* pkg = findAnyPackage(useDecl->getPkgName());
    assert(pkg);
    useDecl->setPackage(pkg);
    if (useDecl->isLocal()) locals.push_back(pkg);
    usedPackages[useDecl->getName()] = useDecl;
    symbolCache[useDecl->getName()] = useDecl;
    return true;
}

bool Scope::checkScopedSymbol(const VarDecl* V) const {
    const Decl* Old = 0;

    CacheConstIter iter = symbolCache.find(V->getName());
    if (iter != symbolCache.end()) {
        Old = iter->second;
        goto err;
    }

    // lookup in all used local (= also own)
    for (LocalsConstIter iter = locals.begin(); iter != locals.end(); ++iter) {
        Old = (*iter)->findSymbol(V->getName());
        if (Old) goto err;
    }
    return true;
err:
        Diags.Report(V->getLocation(), diag::err_redefinition)
            << V->getName();
        Diags.Report(Old->getLocation(), diag::note_previous_definition);
    return false;
}

void Scope::addScopedSymbol(VarDecl* V) {
    // NOTE: must already be checked with checkScopedSymbol
    assert(curScope);
    curScope->decls.push_back(V);
    symbolCache[V->getName()] = V;
}

const Package* Scope::findUsedPackage(const std::string& name, clang::SourceLocation loc) const {
    PackagesConstIter iter = usedPackages.find(name);
    if (iter != usedPackages.end()) {
        ImportDecl* U = iter->second;
        U->setUsed();
        return U->getPackage();
    }

    // check if used with alias (then fullname is forbidden)
    for (PackagesConstIter iter = usedPackages.begin(); iter != usedPackages.end(); ++iter) {
        ImportDecl* U = iter->second;
        U->setUsed();
        const Package* p = U->getPackage();
        if (p->getName() == name) {
            Diags.Report(loc, diag::err_package_has_alias) << name << iter->first;
            return 0;
        }
    }
    const Package* P2 = findAnyPackage(name);
    if (P2) {
        Diags.Report(loc, diag::err_package_not_used) << name;
    } else {
        Diags.Report(loc, diag::err_unknown_package) << name;
    }
    return 0;
}

Decl* Scope::findSymbol(const std::string& symbol, clang::SourceLocation loc, bool isType) const {
    // lookup in global cache first, return if found
    CacheConstIter iter = symbolCache.find(symbol);
    if (iter != symbolCache.end()) {
        return iter->second;
    }

    // lookup in used package list
    bool ambiguous = false;
    bool visible_match = false;
    Decl* D = 0;
    for (LocalsConstIter iter = locals.begin(); iter != locals.end(); ++iter) {
        const Package* pkg = *iter;
        Decl* decl = pkg->findSymbol(symbol);
        if (!decl) continue;

        bool visible = !(isExternal(pkg) && !decl->isPublic());
        if (D) {
            // if previous result was non-visible, replace with new one
            if (visible_match == visible) {
                if (!ambiguous) {
                    Diags.Report(loc, diag::err_ambiguous_symbol) << symbol;
                    // NASTY: are different FileManagers!
                    // TEMP just use 0 location
                    Diags.Report(SourceLocation(), diag::note_function_suggestion)
                        << AnalyserUtils::fullName(D->getPackage()->getName(), D->getName());

                    ambiguous = true;
                }
                Diags.Report(SourceLocation(), diag::note_function_suggestion)
                    << AnalyserUtils::fullName(pkg->getName(), decl->getName());

                continue;
            }
            if (!visible_match) { // replace with visible symbol
                D = decl;
                visible_match = visible;
            }
        } else {
            D = decl;
            visible_match = visible;
        }
    }
    if (ambiguous) return 0;

    if (D) {
        // mark ImportDecl as used
        PackagesConstIter iter = usedPackages.begin();
        while (iter != usedPackages.end()) {
            ImportDecl* Use = iter->second;
            if (D->getPackage() == Use->getPackage()) {
                Use->setUsed();
                break;
            }
            ++iter;
        }

        if (!visible_match) {
            Diags.Report(loc, diag::err_not_public) << symbol;
            return 0;
        }
        if (isExternal(D->getPackage())) D->setUsedPublic();
        symbolCache[symbol] = D;
        return D;
    } else {
        if (isType) {
            Diags.Report(loc, diag::err_unknown_typename) << symbol;
        } else {
            Diags.Report(loc, diag::err_undeclared_var_use) << symbol;
        }
/*
        ScopeResult res2 = scope.findSymbolInUsed(symbol);
        Decl* D2 = res2.getDecl();
        if (D2) {
            assert(D2->getPackage());
            // Crashes if ambiguous
            Diags.Report(D->getLocation(), diag::note_function_suggestion)
                << AnalyserUtils::fullName(D2->getPackage()->getName(), id->getName());
        }

*/
    }
    // TODO make suggestion otherwise? (used Packages w/alias, other packages?)
    return 0;
}

Decl* Scope::findSymbolInPackage(const std::string& name, clang::SourceLocation loc, const Package* pkg) const {
    Decl* D = pkg->findSymbol(name);
    if (!D) {
        Diags.Report(loc, diag::err_unknown_package_symbol) << pkg->getName() << name;
        return 0;
    }
    // if external package, check visibility
    if (isExternal(pkg)) {
        if (!D->isPublic()) {
            Diags.Report(loc, diag::err_not_public) << AnalyserUtils::fullName(pkg->getName(), name);
            return 0;
        }
        D->setUsedPublic();
    }
    return D;
}

#if 0
// TODO rename to makeSuggestion() and use in findSymbol
Decl* Scope::findSymbolInUsed(const std::string& symbol) const {
    // symbol can be package name or symbol within package
    const Package* pkg = findPackage(symbol);
    if (pkg) {
        result.setPackage(pkg);
        return result;
    }

    // search in all used usedPackages
    for (PackagesConstIter iter = usedPackages.begin(); iter != usedPackages.end(); ++iter) {
        ImportDecl* U = iter->second;
        U->setUsed();
        const Package* pkg2 = U->getPackage();
        Decl* decl = pkg2->findSymbol(symbol);
        if (!decl) continue;

        // NOTE: dont check ambiguity here (just return first match)
        result.setDecl(decl);
        //bool external = isExternal(pkg2);
        //bool visible = !(external && !decl->isPublic());
        // Q: check visibility and set ok?
    }
    return result;
}
#endif

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
        // remove from symbol cache
        CacheIter iter = symbolCache.find(D->getName());
        assert(iter != symbolCache.end());
        symbolCache.erase(iter);
    }
    curScope->decls.clear();
    curScope->Flags = 0;

    scopeIndex--;
    if (scopeIndex == 0) curScope = 0;
    else curScope = &scopes[scopeIndex-1];
}

const Package* Scope::findAnyPackage(const std::string& name) const {
    PkgsConstIter iter = allPackages.find(name);
    if (iter == allPackages.end()) return 0;
    return iter->second;
}

