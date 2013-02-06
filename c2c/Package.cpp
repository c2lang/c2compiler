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

#include "Package.h"
#include "Decl.h"

using namespace C2;

Package::Package(const std::string& name_) : name(name_) {}

void Package::addSymbol(Decl* decl) {
    symbols[decl->getName()] = decl;
}

Decl* Package::findSymbol(const std::string& name) const {
    SymbolsConstIter iter = symbols.find(name);
    if (iter == symbols.end()) return 0;
    else return iter->second;
}

void Package::dump() {
    printf("symbols of package %s:\n", name.c_str());
    for (SymbolsIter iter = symbols.begin(); iter != symbols.end(); ++iter) {
        printf("  %s\n", iter->second->getName().c_str());
    }
}

