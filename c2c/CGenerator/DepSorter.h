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

#ifndef CGENERATOR_DEP_SORTER_H
#define CGENERATOR_DEP_SORTER_H

#include <vector>

namespace C2 {

class Component;

class DepSorter {
public:
    DepSorter(const Component& comp);

    unsigned numComps() const { return deps.size(); }
    const Component* getComp(unsigned pos) const { return deps[pos].comp; }
private:
    void handle(const Component* comp, int depth);
    int find(const Component* comp) const;

    struct Dep {
        Dep(const Component* comp_, int depth_) : comp(comp_), depth(depth_) {}

        bool operator<(const Dep &rhs) const { return depth < rhs.depth; }

        const Component* comp;
        int depth;
    };

    typedef std::vector<Dep> Deps;
    Deps deps;
};

}

#endif

