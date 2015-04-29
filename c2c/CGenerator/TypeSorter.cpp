/* Copyright 2013,2014,2015 Bas van den Berg
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
#include <assert.h>
#include <stdint.h>

#include "CGenerator/TypeSorter.h"
#include "Algo/DepVisitor.h"
#include "AST/Decl.h"

using namespace C2;

namespace C2 {

class DeclDep {
public:
    DeclDep(const Decl* decl_) : decl(decl_) {}
    void addDep(const Decl* d, bool isFull) {
        deps.push_back((uintptr_t)d | isFull);
    }
    unsigned numDeps() const { return deps.size(); }
    inline const Decl* getDep(unsigned i) const {
        return reinterpret_cast<const Decl*>(deps[i] & ~0x1);
    }
    inline bool isFull(unsigned i) const { return  deps[i] & 0x1; }
    bool dependsOn(const DeclDep* dd, bool* full) const {
        const Decl* d = dd->decl;
        for (unsigned i=0; i<numDeps(); ++i) {
            if (d == getDep(i)) {
                *full = isFull(i);
                return true;
            }
        }
        return false;
    }

    static bool compare(const DeclDep* A, const DeclDep* B) {
        const Decl* a = A->decl;
        const Decl* b = B->decl;
        //printf("COMPARE %s vs %s\n", a->getName().c_str(), b->getName().c_str());
        // public before non-public
        if (a->isPublic() != b->isPublic()) return a->isPublic();

        // no-deps before deps
        if (A->numDeps() == 0 && B->numDeps() != 0) return true;
        if (B->numDeps() == 0 && A->numDeps() != 0) return false;

        // if A depends on B, but not B on A, swap
        bool abFull = false;
        bool baFull = false;
        bool a_on_b = A->dependsOn(B, &abFull);
        bool b_on_a = B->dependsOn(A, &baFull);
        if (a_on_b && !b_on_a) return false;
        if (b_on_a && !a_on_b) return true;
        if (!a_on_b && !b_on_a) return false;

        // circular dep, check full/ptr, pointer goes first
        if (abFull != baFull) return !abFull;
        assert(!abFull);    // analyser should have checked
        return false;
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
    depmap.clear();
}

void TypeSorter::add(const Decl* decl) {
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
    // sort
    //dump("ORIGINAL", true);
    std::sort(depmap.begin(), depmap.end(), DeclDep::compare);
    //dump("AFTER", false);

    enum State { FORWARD_DECLARED, DEFINED };
    typedef std::map<const Decl*, State> States;
    typedef States::iterator StatesIter;
    States states;

    // write
    for (DeclDepListConstIter iter=depmap.begin(); iter != depmap.end(); ++iter) {
        const DeclDep* dd = *iter;
        const Decl* d = dd->decl;
        for (unsigned i=0; i<dd->numDeps(); ++i) {
            const Decl* dep = dd->getDep(i);
            // only care about other types for now
            if (!isa<TypeDecl>(dep)) continue;

            StatesIter iter = states.find(dep);
            if (iter == states.end()) {
                writer.forwardDecl(dep);
                states[dep] = FORWARD_DECLARED;
            }
        }
        // always output typedef for struct types
        if (isa<StructTypeDecl>(d)) {
            StatesIter iter = states.find(d);
            if (iter == states.end()) {
                writer.forwardDecl(d);
                states[d] = FORWARD_DECLARED;
            }
        }
        writer.fullDecl(d);
        states[d] = DEFINED;
    }
}

void TypeSorter::dump(const char* msg, bool showDeps) const {
    printf("%s:\n", msg);
    for (DeclDepListConstIter iter=depmap.begin(); iter != depmap.end(); ++iter) {
        const DeclDep* dd = *iter;
        printf(" %s\n", dd->decl->getName().c_str());
        if (showDeps) {
            for (unsigned i=0; i<dd->numDeps(); ++i) {
                printf("  -> %s  %s\n", dd->getDep(i)->getName().c_str(), dd->isFull(i) ? "full" : "ptr");
            }
        }
    }
}

