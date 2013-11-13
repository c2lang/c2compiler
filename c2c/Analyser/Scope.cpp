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
#include "Analyser/DepAnalyser.h"
#include "Analyser/AnalyserUtils.h"
#include "AST/Package.h"
#include "AST/Decl.h"
#include "AST/Expr.h"

using namespace C2;
using namespace clang;

DynamicScope::DynamicScope() : Flags(0) {}


Scope::Scope(const std::string& name_, const Pkgs& pkgs_, clang::DiagnosticsEngine& Diags_, unsigned id)
    : scopeIndex(0)
    , curScope(0)
    , allPackages(pkgs_)
    , file_id(id)
    , myPkg(0)
    , Diags(Diags_)
{
    // add own package to scope
    myPkg = findAnyPackage(name_);
    assert(myPkg);
    locals.push_back(myPkg);
}

bool Scope::addUsedPackage(UseDecl* useDecl) {
    // NOTE: dont change if Package exists, already done by Builder
#if 0
    // check if aliasname is not a package
    // TODO still needed? Or just search filescope globals?
    const std::string& aliasName = useDecl->getAlias();
    if (aliasName != "") {
        const Package* pkg2 = globals->findAnyPackage(aliasName);
        if (pkg2) {
            Diags.Report(useDecl->getAliasLocation(), diag::err_alias_is_package) << aliasName;
            errors++;
            continue;
        }
    }
#endif

    std::string pkgName = useDecl->getName();
    clang::SourceLocation Loc = useDecl->getLocation();
    if (useDecl->getAlias() != "") {
        pkgName = useDecl->getAlias();
        Loc = useDecl->getAliasLocation();
    }
    // TODO dont use findGlobalSymbol here, since it searches too much and add to cache
    ScopeResult SR = findGlobalSymbol(pkgName, SourceLocation());
    const Decl* Old = SR.getDecl();
    if (Old != 0) {
        Diags.Report(Loc, diag::err_redefinition)
            << pkgName;
        Diags.Report(Old->getLocation(), diag::note_previous_definition);
        return false;
    }
    const Package* pkg = findAnyPackage(useDecl->getName());
    assert(pkg);
    useDecl->setPackage(pkg);
    if (useDecl->isLocal()) locals.push_back(useDecl->getPackage());
    usedPackages[pkgName] = useDecl;
    return true;
}

const Package* Scope::usePackage(const std::string& name, clang::SourceLocation loc) const {
    const Package* P = findPackage(name);
    if (!P) {
        // check if used with alias (then fullname is forbidden)
        for (PackagesConstIter iter = usedPackages.begin(); iter != usedPackages.end(); ++iter) {
            UseDecl* U = iter->second;
            U->setUsed();
            const Package* p = U->getPackage();
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
    fprintf(stderr, "used usedPackages:\n");
    for (PackagesConstIter iter = usedPackages.begin(); iter != usedPackages.end(); ++iter) {
        fprintf(stderr, "  %s (as %s)\n", iter->second->getName().c_str(), iter->first.c_str());
    }
}

void Scope::getExternals(DepAnalyser& dep) const {
    dep.startFile(myPkg, file_id);
    for (unsigned i=0; i<externals.size(); i++) {
        dep.add(file_id, externals[i]);
    }
    for (GlobalsConstIter iter = globalCache.begin(); iter != globalCache.end(); ++iter) {
        const Decl* D = iter->second.getDecl();
        if (!D) continue;
        if (D->getFileID() != file_id) dep.add(file_id, D);
    }
    dep.doneFile();
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
        result.setOK(true);
        result.setPackage(pkg);
        // add to cache
        globalCache[symbol] = result;
        return result;
    }

    bool ambiguous = false;
    bool visible_match = false;
    Decl* D = 0;
    for (LocalsConstIter iter = locals.begin(); iter != locals.end(); ++iter) {
        pkg = *iter;
        Decl* decl = pkg->findSymbol(symbol);
        if (!decl) continue;

        bool external = isExternal(pkg);
        bool visible = !(external && !decl->isPublic());
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
                    result.setOK(false);
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
    if (D && !ambiguous) {
        // find useDecl and set it used
        PackagesConstIter iter = usedPackages.begin();
        while (iter != usedPackages.end()) {
            UseDecl* Use = iter->second;
            if (D->getPackage() == Use->getPackage()) {
                Use->setUsed();
                break;
            }
            ++iter;
        }

        if (!visible_match) {
            Diags.Report(loc, diag::err_not_public) << symbol;
            result.setOK(false);
        }
        if (result.isOK()) {
            result.setDecl(D);
            globalCache[symbol] = result;
        }
    }
    // TODO make suggestion otherwise? (used Packages w/alias, other packages?)
    return result;
}

ScopeResult Scope::findSymbol(const std::string& symbol, clang::SourceLocation loc) const {
    // lookup in local vars (all in cache), return if found
    LocalCacheConstIter iter = localCache.find(symbol);
    if (iter != localCache.end()) {
        ScopeResult result;
        result.setOK(true);
        result.setDecl(iter->second);
        return result;
    }

    // otherwise search globals
    return findGlobalSymbol(symbol, loc);
}

ScopeResult Scope::findSymbolInPackage(const std::string& name, clang::SourceLocation loc, const Package* pkg) const {
    ScopeResult res;
    Decl* D = pkg->findSymbol(name);
    res.setDecl(D);
    if (!D) {
        Diags.Report(loc, diag::err_unknown_package_symbol) << pkg->getName() << name;
        res.setOK(false);
    } else {
        // if external package, check visibility
        if (isExternal(pkg) && !D->isPublic()) {
            Diags.Report(loc, diag::err_not_public) << AnalyserUtils::fullName(pkg->getName(), name);
            res.setDecl(0);
            res.setOK(false);
        } else {
            if (D->getFileID() != file_id) addExternal(D);
        }
    }
    return res;
}

ScopeResult Scope::findSymbolInUsed(const std::string& symbol) const {
    ScopeResult result;
    // symbol can be package name or symbol within package
    const Package* pkg = findPackage(symbol);
    if (pkg) {
        result.setPackage(pkg);
        return result;
    }

    // search in all used usedPackages
    for (PackagesConstIter iter = usedPackages.begin(); iter != usedPackages.end(); ++iter) {
        UseDecl* U = iter->second;
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

bool Scope::checkScopedSymbol(const VarDecl* V) const {
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

void Scope::addScopedSymbol(VarDecl* V) {
    // NOTE: must already be checked with checkScopedSymbol
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

void Scope::addExternal(const Decl* D) const {
    for (unsigned i=0; i<externals.size(); i++) {
        if (externals[i] == D) return;  // already in
    }
    externals.push_back(D);
}

const Package* Scope::findPackage(const std::string& name) const {
    if (name == myPkg->getName()) return myPkg;

    PackagesConstIter iter = usedPackages.find(name);
    if (iter == usedPackages.end()) return 0;
    UseDecl* U = iter->second;
    U->setUsed();
    return U->getPackage();
}

