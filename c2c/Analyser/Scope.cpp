/* Copyright 2013-2017 Bas van den Berg
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
#include "AST/Decl.h"

using namespace C2;
using namespace clang;

DynamicScope::DynamicScope() : Flags(0) {}


Scope::Scope(const std::string& name_, const Modules& modules_, clang::DiagnosticsEngine& Diags_)
    : scopeIndex(0)
    , curScope(0)
    , allModules(modules_)
    , myModule(0)
    , Diags(Diags_)
{
    // add own module to scope
    myModule = findAnyModule(name_.c_str());
    assert(myModule);
}

void Scope::addImportDecl(ImportDecl* importDecl) {
    assert(importDecl->getModule());
    if (importDecl->isLocal()) locals.push_back(importDecl->getModule());
    importedModules[importDecl->getName()] = importDecl;
    symbolCache[importDecl->getName()] = importDecl;
}

bool Scope::checkScopedSymbol(const VarDecl* V) const {
    const Decl* Old = 0;

    CacheConstIter iter = symbolCache.find(V->getName());
    if (iter != symbolCache.end()) {
        Old = iter->second;
        goto err;
    }

    // lookup in all used local (= also own)
    for (LocalsConstIter iter2 = locals.begin(); iter2 != locals.end(); ++iter2) {
        Old = (*iter2)->findSymbol(V->getName());
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

const Module* Scope::findUsedModule(const std::string& name, clang::SourceLocation loc, bool usedPublic) const {
    ImportsConstIter iter = importedModules.find(name);
    if (iter != importedModules.end()) {
        ImportDecl* I = iter->second;
        I->setUsed();
        if (usedPublic) {
            // TODO refactor common code
            // TODO check if using non-exported module from exported one
            I->setUsedPublic();
        }
        return I->getModule();
    }

    // check if used with alias (then fullname is forbidden)
    for (ImportsConstIter iter2 = importedModules.begin(); iter2 != importedModules.end(); ++iter2) {
        ImportDecl* I = iter2->second;
        I->setUsed();
        if (usedPublic) {
            // TODO refactor common code
            // TODO check if using non-exported module from exported one
            I->setUsedPublic();
        }
        const Module* p = I->getModule();
        if (p->getName() == name) {
            Diags.Report(loc, diag::err_module_has_alias) << name << iter2->first;
            return 0;
        }
    }
    const Module* P2 = findAnyModule(name.c_str());
    if (P2) {
        Diags.Report(loc, diag::err_module_not_used) << name;
    } else {
        Diags.Report(loc, diag::err_unknown_module) << name;
    }
    return 0;
}

Decl* Scope::findSymbol(const std::string& symbol, clang::SourceLocation loc, bool isType, bool usedPublic) const {
    // lookup in global cache first, return if found
    Decl* D = 0;
    {
        CacheConstIter iter = symbolCache.find(symbol);
        if (iter != symbolCache.end()) {
            D = iter->second;
            // update usedPublic if needed
            // TODO also cache this part?
            if (usedPublic && D->getModule() != myModule) {
                ImportsConstIter iter = importedModules.begin();
                while (iter != importedModules.end()) {
                    ImportDecl* I = iter->second;
                    if (D->getModule() == I->getModule()) {
                        I->setUsedPublic();
                        break;
                    }
                    ++iter;
                }
            }
            return D;
        }
    }

    // lookup in used module list
    bool ambiguous = false;
    bool visible_match = false;
    for (LocalsConstIter iter = locals.begin(); iter != locals.end(); ++iter) {
        const Module* mod = *iter;
        Decl* decl = mod->findSymbol(symbol);
        if (!decl) continue;

        bool visible = !(isExternal(mod) && !decl->isPublic());
        if (D) {
            // if previous result was non-visible, replace with new one
            if (visible_match == visible) {
                if (!ambiguous) {
                    Diags.Report(loc, diag::err_ambiguous_symbol) << symbol;
                    // NASTY: are different FileManagers!
                    // TEMP just use 0 location
                    Diags.Report(SourceLocation(), diag::note_function_suggestion)
                            << AnalyserUtils::fullName(D->getModule()->getName(), D->getName());

                    ambiguous = true;
                }
                Diags.Report(SourceLocation(), diag::note_function_suggestion)
                        << AnalyserUtils::fullName(mod->getName(), decl->getName());

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
        if (D->getModule() != myModule) {
            ImportsConstIter iter = importedModules.begin();
            while (iter != importedModules.end()) {
                ImportDecl* I = iter->second;
                if (D->getModule() == I->getModule()) {
                    I->setUsed();
                    if (usedPublic) {
                        I->setUsedPublic();
                        // TODO refactor common code
                        // TODO check if using non-exported module from exported one
                    }
                    break;
                }
                ++iter;
            }
        }

        if (!visible_match) {
            Diags.Report(loc, diag::err_not_public) << AnalyserUtils::fullName(D->getModule()->getName(), D->getName());
            return 0;
        }
        if (isExternal(D->getModule())) D->setUsedPublic();
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
                    assert(D2->getModule());
                    // Crashes if ambiguous
                    Diags.Report(D->getLocation(), diag::note_function_suggestion)
                        << AnalyserUtils::fullName(D2->getModule()->getName(), id->getName());
                }

        */
    }
    // TODO make suggestion otherwise? (used Modules w/alias, other modules?)
    return 0;
}

Decl* Scope::findSymbolInModule(const std::string& name, clang::SourceLocation loc, const Module* mod) const {
    Decl* D = mod->findSymbol(name);
    if (!D) {
        Diags.Report(loc, diag::err_unknown_module_symbol) << mod->getName() << name;
        return 0;
    }
    // if external module, check visibility
    if (isExternal(mod)) {
        if (!D->isPublic()) {
            Diags.Report(loc, diag::err_not_public) << AnalyserUtils::fullName(mod->getName(), name.c_str());
            return 0;
        }
        D->setUsedPublic();
    }
    return D;
}

void Scope::checkAccess(Decl* D, clang::SourceLocation loc) const {
    const Module* mod = D->getModule();
    assert(mod);
    // if external module, check visibility
    if (isExternal(mod)) {
        if (!D->isPublic()) {
            Diags.Report(loc, diag::err_not_public) << AnalyserUtils::fullName(mod->getName(), D->getName());
            return;
        }
        D->setUsedPublic();
    }
}

#if 0
// TODO rename to makeSuggestion() and use in findSymbol
Decl* Scope::findSymbolInUsed(const std::string& symbol) const {
    // symbol can be module name or symbol within module
    const Module* mod = findModule(symbol);
    if (mod) {
        result.setModule(mod);
        return result;
    }

    // search in all used importedModules
    for (ModulesConstIter iter = importedModules.begin(); iter != importedModules.end(); ++iter) {
        ImportDecl* U = iter->second;
        U->setUsed();
        const Module* mod2 = U->getModule();
        Decl* decl = mod2->findSymbol(symbol);
        if (!decl) continue;

        // NOTE: dont check ambiguity here (just return first match)
        result.setDecl(decl);
        //bool external = isExternal(mod2);
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
            Diags.Report(D->getLocation(), msg) << D->DiagName();
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

const Module* Scope::findAnyModule(const char* name_) const {
    ModulesConstIter iter = allModules.find(name_);
    if (iter == allModules.end()) return 0;
    return iter->second;
}

