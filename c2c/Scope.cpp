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

#include "Scope.h"
#include "Package.h"

using namespace C2;

Scope::Scope(const std::string& name_) : name(name_) {}

void Scope::addPackage(const std::string& name_, const Package* pkg) {
    packages[name_] = pkg;
}

Decl* Scope::findSymbol(const char* pkgName, const char* symbolName) const {
    if (pkgName) {
        const Package* pkg = findPackage(pkgName);
        if (!pkg) return 0;
        return pkg->findSymbol(symbolName);
    } else {
        // TODO check for duplicates
        for (PackagesConstIter iter = packages.begin(); iter != packages.end(); ++iter) {
            const Package* pkg = iter->second;
            Decl* decl = pkg->findSymbol(symbolName);
            if (decl) return decl;
        }
        return 0;
    }
}

const Package* Scope::findPackage(const std::string& pkgName) const {
    PackagesConstIter iter = packages.find(pkgName);
    if (iter == packages.end()) return 0;
    return iter->second;
}

void Scope::dump() {
    printf("Scoped packages:\n");
    for (PackagesIter iter = packages.begin(); iter != packages.end(); ++iter) {
        printf("  %s\n", iter->first.c_str());
    }
}

