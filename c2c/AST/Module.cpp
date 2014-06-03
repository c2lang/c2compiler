/* Copyright 2013,2014 Bas van den Berg
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

#include "AST/Module.h"
#include "AST/Decl.h"

using namespace C2;

static std::string empty = "";

Module::Module(const std::string& name_, bool isExternal_, bool isCLib_)
    : name(name_)
    , is_External(isExternal_)
    , isCLib(isCLib_)
{}

const std::string& Module::getCName() const {
    if (isCLib) return empty;
    return name;
}

void Module::addSymbol(Decl* decl) {
    symbols[decl->getName()] = decl;
    decl->setModule(this);
}

Decl* Module::findSymbol(const std::string& name_) const {
    SymbolsConstIter iter = symbols.find(name_);
    if (iter == symbols.end()) return 0;
    else return iter->second;
}

void Module::dump() const {
    printf("symbols of module %s (clib=%d, external=%d)\n", name.c_str(), isCLib, is_External);
    for (SymbolsConstIter iter = symbols.begin(); iter != symbols.end(); ++iter) {
        printf("  %s\n", iter->second->getName().c_str());
    }
}

