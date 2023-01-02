/* Copyright 2013-2023 Bas van den Berg
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

#include <algorithm>

#include "CGenerator/DepSorter.h"
#include "AST/Component.h"

using namespace C2;

DepSorter::DepSorter(const Component& comp)
{
    handle(&comp, 1);
    std::sort(deps.begin(), deps.end());
}

void DepSorter::handle(const Component* comp, int depth) {
    int pos = find(comp);
    if (pos == -1) {
        deps.push_back(Dep(comp, depth));
    } else {
        int oldDepth = deps[pos].depth;
        if (depth <= oldDepth) return;
        deps[pos].depth = depth;
    }
    const Component::Dependencies& subdeps = comp->getDeps();
    for (unsigned i=0; i<subdeps.size(); ++i) {
        handle(subdeps[i], depth+1);
    }
}

int DepSorter::find(const Component* comp) const {
    for (int i=0; i<(int)deps.size(); ++i) {
        if (deps[i].comp == comp) return i;
    }
    return -1;
}

