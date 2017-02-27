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

#include <map>
#include <list>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "CGenerator/TypeSorter.h"
#include "Algo/DepVisitor.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"

using namespace C2;

namespace C2 {

class DeclDep {
public:
    DeclDep(const Decl* decl_) : decl(decl_) {}
    void addDep(const Decl* d, bool isFull) {
        deps.push_back((uintptr_t)d | isFull);
    }
    unsigned numDeps() const {
        return deps.size();
    }
    inline const Decl* getDep(unsigned i) const {
        return reinterpret_cast<const Decl*>(deps[i] & ~0x1);
    }
    inline bool isFull(unsigned i) const {
        return  deps[i] & 0x1;
    }

    const Decl* decl;
    typedef std::vector<uintptr_t> Decls;
    Decls deps;
};

}

TypeSorter::~TypeSorter() {
    for (DeclDepListIter iter=depmap.begin(); iter != depmap.end(); ++iter) {
        delete *iter;
    }
}

void TypeSorter::add(const Decl* decl) {
    // For FunctionTypeDecl's record the FunctionDecl, not the FunctionTypeDecl!
    if (isa<FunctionTypeDecl>(decl)) {
        const FunctionTypeDecl* ftd = cast<FunctionTypeDecl>(decl);
        decl = ftd->getDecl();
        functions[decl] = ftd;
    }
    DeclDep* dd = new DeclDep(decl);
    DepVisitor visitor(decl, false);
    visitor.run();
    // TODO just swap visitor.deps..
    for (unsigned i=0; i<visitor.getNumDeps(); ++i) {
        // TODO only add Type Deps (not constants in enums)
        dd->addDep(visitor.getDep(i), visitor.isFull(i));
    }
    depmap.push_back(dd);
}

void TypeSorter::write(CTypeWriter& writer) {
    enum State { FORWARD_DECLARED, DEFINED };
    typedef std::map<const Decl*, State> States;
    typedef States::iterator StatesIter;
    States states;

    // forward declare all struct types
    for (DeclDepListIter iter=depmap.begin(); iter != depmap.end(); ++iter) {
        DeclDep* dd = *iter;
        const Decl* d = dd->decl;
        if (isa<StructTypeDecl>(d)) {
            writer.forwardDecl(d);
            states[d] = FORWARD_DECLARED;
        }
    }

    // keep checking list and writing decls that are complete, until done
    DeclDepList current = depmap;
    DeclDepList next;
    while (!current.empty()) {
        for (DeclDepListIter iter=current.begin(); iter != current.end(); ++iter) {
            DeclDep* dd = *iter;
            const Decl* d = dd->decl;
            if (isa<FunctionDecl>(d)) {
                FunctionMapConstIter iter2 = functions.find(d);
                assert(iter2 != functions.end());
                d = iter2->second;
            }
            //printf("  checking %s\n", d->getName());
            bool complete = true;
            for (unsigned i=0; i<dd->numDeps(); ++i) {
                const Decl* dep = dd->getDep(i);
                // only care about other types for now
                if (!isa<TypeDecl>(dep)) continue;

                //printf("    dep %s  %s\n", dep->getName(), dd->isFull(i) ? "full" : "ptr");
                StatesIter iter = states.find(dep);
                if (iter == states.end() || (dd->isFull(i) && iter->second == FORWARD_DECLARED)) {
                    complete = false;
                    break;
                }
            }
            if (complete) {
                //printf("  -> writing\n");
                writer.fullDecl(d);
                states[d] = DEFINED;
            } else {
                //printf("  -> skipping\n");
                next.push_back(dd);
            }
        }
        current = next;
        next.clear();
    }
}

