/* Copyright 2013-2019 Bas van den Berg
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

#include <assert.h>

#include "Analyser/ComponentAnalyser.h"
#include "Analyser/ModuleAnalyser.h"
#include "AST/Component.h"

#include "Clang/ParseDiagnostic.h"
#include "Clang/Diagnostic.h"
#include "Clang/SemaDiagnostic.h"

using namespace C2;
using namespace c2lang;

struct ModElem {
    ModElem(Module* m_)
        : m(m_), depth(0), next(0), prev(0) {}

    void init() {
        next = this;
        prev = this;
    }
    bool isEmpty() const {
        return next == this;
    }
    void addFront(ModElem* e) {
        ModElem* old_head = next;
        old_head->prev = e;
        e->next = old_head;
        e->prev = this;
        next = e;
    }
    void addTail(ModElem* e) {
        ModElem* old_tail = prev;
        prev = e;
        e->next = this;
        e->prev = old_tail;
        old_tail->next = e;
    }
    ModElem* popFront() {
        ModElem* node = next;
        node->remove();
        return node;
    }
    void remove() {
        ModElem* prev_ = prev;
        ModElem* next_ = next;
        prev_->next = next_;
        next_->prev = prev_;
    }
    void moveList(ModElem* src) {
        ModElem* node = src->next;
        while (node != src) {
            ModElem* next = node->next;
            node->remove();
            addTail(node);
            node = next;
        }
    }
    void swapList(ModElem* src) {
        ModElem* tmp_next = next;
        ModElem* tmp_prev = prev;
        if (src->isEmpty()) {
            init();
        } else {
            next = src->next;
            prev = src->prev;
            next->prev = this;
            prev->next = this;
        }
        if (tmp_next == this) { // dest list was empty
            src->init();
        } else {
            src->next = tmp_next;
            src->prev = tmp_prev;
            tmp_next->prev = src;
            tmp_prev->next = src;
        }
    }
    void freeList() {
        ModElem* cur = next;
        while (cur != this) {
            ModElem* next = cur->next;
            free(cur);
            cur = next;
        }
    }
    bool hasDep(const ModElem* dep) const {
        for (unsigned i=0; i<deps.size(); i++) {
            if (deps[i] == dep) return true;
        }
        return false;
    }
    void addDep(ModElem* dep) {
        deps.push_back(dep);
    }
    bool isImported(const ModElem* list) const {
        ModElem* cur = list->next;
        while (cur != list) {
            ModElem* next = cur->next;
            if (cur->hasDep(this)) return true;
            cur = next;
        }
        return false;
    }
    void printList(const char* name) const {
        printf("%s\n", name);
        ModElem* cur = next;
        while (cur != this) {
            printf("  %s  depth %d\n", cur->m->getName().c_str(), cur->depth);
            cur = cur->next;
        }
    }
    void printDeps() const {
        for (unsigned i=0; i<deps.size(); i++) {
            printf("  %s\n", deps[i]->m->getName().c_str());
        }
    }

    Module* m;
    int32_t depth;
    ModElem* next;
    ModElem* prev;

    typedef std::vector<ModElem*> Deps;
    Deps deps;
};

ModElem* mod2elem(ModElem* list, const Module* m) {
    ModElem* cur = list->next;
    while (cur != list) {
        if (cur->m == m) return cur;
        cur = cur->next;
    }
    return 0;   // not in this component
}

ComponentAnalyser::ComponentAnalyser(Component& C,
                                     const Modules& allModules,
                                     c2lang::DiagnosticsEngine& Diags,
                                     const TargetInfo& target_,
                                     ASTContext& context_,
                                     bool verbose,
                                     bool testMode)
    : component(C)
{
    if (!component.isExternal()) checkMainFunction(testMode, Diags);

    // sort Modules bottom-up
    ModElem all(0);
    ModElem todo(0);
    ModElem done(0);
    all.init();
    todo.init();
    done.init();

    // create temp datastructure to sort more easily
    const ModuleList& mods = component.getModules();
    int max_depth = 0;
    for (unsigned i=0; i<mods.size(); i++) {
        Module* mod = mods[i];
        if (mod->isLoaded()) {
            all.addTail(new ModElem(mod));
            max_depth++;
        }
    }

    // fill dependencies, only those within this component
    ModElem* cur = all.next;
    while (cur != &all) {
        const AstList& files = cur->m->getFiles();
        for (unsigned f=0; f<files.size(); f++) {
            const AST* ast = files[f];
            for (unsigned d=1; d<ast->numImports(); d++) {
                const Module* depM = ast->getImport(d)->getModule();
                ModElem* depE = mod2elem(&all, depM);
                if (depE && !cur->hasDep(depE)) cur->addDep(depE);
            }
        }
        cur = cur->next;
    }

    todo.moveList(&all);

    // sort nodes, top to bottom
    while (!todo.isEmpty()) {
        ModElem* node = todo.popFront();
        if (node->depth > max_depth) {
            todo.addFront(node);
            Diags.Report(SourceLocation(), diag::err_circular_module_dependency);
            return;
        }
        done.addTail(node);

        for (unsigned i=0; i<node->deps.size(); i++) {
            ModElem* dep = node->deps[i];
            if (node->depth +1 > dep->depth) {
                dep->depth = node->depth + 1;
                dep->remove();
                todo.addTail(dep);
            }
        }
    }

    all.swapList(&done);

    if (!component.isExternal()) {
        ModElem* cur = all.next;
        while (cur != &all) {
            const Module* m = cur->m;
            if (!m->isExported() && !m->hasMain() && !cur->isImported(&all)) {
                // give warning on all files of that module
                const AstList& files = m->getFiles();
                for (unsigned i=0; i<files.size(); i++) {
                    const AST* ast = files[i];
                    const ImportDecl* modDecl = ast->getImport(0);
                    Diags.Report(modDecl->getLocation(), diag::warn_unused_module) << cur->m->getName();
                }
            }
            cur = cur->next;
        }
    }

    // remove walk Modules and create ModuleAnalysers (bottom up, unused last)
    // also update list in component to be in same order
    ModuleList sorted_mods;
    cur = all.prev;
    while (cur != &all) {
        if (cur->m->isLoaded()) {
            ModuleAnalyser* ma = new ModuleAnalyser(*cur->m, allModules, Diags, target_, context_, verbose);
            analysers.push_back(ma);
            sorted_mods.push_back(cur->m);
        }
        cur = cur->prev;
    }
    if (!sorted_mods.empty()) {
        assert(mods.size() == sorted_mods.size());
        component.updateModules(sorted_mods);
    }

    all.freeList();
}

ComponentAnalyser::~ComponentAnalyser() {
    for (auto &analyser : analysers) delete analyser;
}

bool ComponentAnalyser::checkMainFunction(bool testMode, c2lang::DiagnosticsEngine& Diags)
{
    Decl* mainDecl = 0;
    const ModuleList& mods = component.getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        Module* M = mods[m];
        Decl* decl = M->findSymbol("main");
        if (decl) {
            decl->setExported();
            M->setMain();
            if (mainDecl) {
                // TODO multiple main functions
                TODO;
            } else {
                mainDecl = decl;
            }
        }
    }

    if (component.getType() == Component::EXECUTABLE) {
        // bin: must have main
        if (testMode) return true;
        if (!mainDecl) {
            Diags.Report(diag::err_main_missing);
            return false;
        }
    } else {
        // lib: cannot have main
        if (mainDecl) {
            Diags.Report(mainDecl->getLocation(), diag::err_lib_has_main);
            return false;
        }
    }
    return true;
}

unsigned ComponentAnalyser::analyse(bool print1, bool print2, bool print3, bool printLib) {
    for (unsigned i=0; i<analysers.size(); i++) {
        if (analysers[i]->analyse(print1, print2, print3, printLib)) return 1;
    }

    if (!component.isExternal()) {
        for (unsigned i=0; i<analysers.size(); i++) {
            analysers[i]->checkUnused();
        }
    }
    return 0;
}

