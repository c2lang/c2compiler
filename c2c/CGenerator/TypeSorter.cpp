/* Copyright 2013-2015 Bas van den Berg
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

    // return true if B should go before A (A depends on B)
    static bool goesBefore(const DeclDep* A, const DeclDep* B) {
        const Decl* a = A->decl;
        const Decl* b = B->decl;
        //printf("COMPARE %s vs %s\n", a->getName().c_str(), b->getName().c_str());
        // public before non-public
        if (a->isPublic() != b->isPublic()) return b->isPublic();

        // no-deps before deps
        if (A->numDeps() == 0) return false;

        // if A depends on B, but not B on A, swap
        bool abFull = false;
        bool baFull = false;
        bool a_on_b = A->dependsOn(B, &abFull);
        bool b_on_a = B->dependsOn(A, &baFull);

        if (a_on_b) {
            if (b_on_a) {
                // circular dep, check full/ptr, pointer goes first
                if (abFull != baFull) return abFull;
                assert(!abFull);    // analyser should have checked
                return false;
            } else {
                return true;
            }
        } else {
            return false;
        }
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
    //dump("ORIGINAL", true);
    // sort Types, basic algorithms, O(N*N), so optimization is very well possible
    typedef std::list<DeclDep*> SortedDecls;
    typedef SortedDecls::iterator SortedDeclsIter;
    SortedDecls sorted;
    for (DeclDepListIter it1=depmap.begin(); it1 != depmap.end(); ++it1) {
        DeclDep* newDD = *it1;
        bool added = false;
        // check if element in new list depends on new one, otherwise add at tail
        for (SortedDeclsIter it2=sorted.begin(); it2 != sorted.end(); ++it2) {
            const DeclDep* curDD = *it2;
            bool before = DeclDep::goesBefore(curDD, newDD);
            if (before) {
                // insert before it2
                //printf("adding %s before %s\n", newDD->decl->getName().c_str(), curDD->decl->getName().c_str());
                sorted.insert(it2, newDD);
                added = true;
                break;
            }
        }
        if (!added) {
            //printf("adding %s at end\n", newDD->decl->getName().c_str());
            sorted.push_back(newDD);
        }
    }
    depmap.clear();
    for (SortedDeclsIter it=sorted.begin(); it != sorted.end(); ++it) {
        depmap.push_back(*it);
    }
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
    StringBuilder output;
    output << msg << ":\n";
    for (DeclDepListConstIter iter=depmap.begin(); iter != depmap.end(); ++iter) {
        const DeclDep* dd = *iter;
        output << "  " << dd->decl->getName();
        if (showDeps) {
            for (unsigned i=0; i<dd->numDeps(); ++i) {
                output << "  -> " << dd->getDep(i)->getName() << "  " << (dd->isFull(i) ? "full" : "ptr");
            }
        }
        output << '\n';
    }
    printf("%s", (const char*)output);
}

