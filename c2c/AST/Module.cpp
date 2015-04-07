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

#include <stdio.h>

#include "AST/Module.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

using namespace C2;

static std::string empty = "";

Module::Module(const std::string& name_, bool isExternal_, bool isCLib_)
    : name(name_)
    , m_isExternal(isExternal_)
    , m_isCLib(isCLib_)
    , m_isExported(false)
{}

const std::string& Module::getCName() const {
    if (m_isCLib) return empty;
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

void Module::addAttributes(AttrMap& am) {
    declAttrs.insert(am.begin(), am.end());
}

void Module::dump() const {
    StringBuilder out;
    out.enableColor(true);
    out << "symbols of module " << name << "(clib=" << m_isCLib
        <<", external=" << m_isExternal << ", exported=" << m_isExported << ")\n";
    for (SymbolsConstIter iter = symbols.begin(); iter != symbols.end(); ++iter) {
        const Decl* D = iter->second;
        out.indent(3);
        out << D->getName() << "    ";// << '\n';
        out.setColor(COL_ATTRIBUTES);
        switch (D->getKind()) {
        case DECL_FUNC:
            out << "function";
            break;
        case DECL_VAR:
            out << "variable";
            break;
        case DECL_ALIASTYPE:
        case DECL_STRUCTTYPE:
        case DECL_ENUMTYPE:
        case DECL_FUNCTIONTYPE:
            out << "type";
            break;
        case DECL_ENUMVALUE:
            out << "constant";
            break;
        case DECL_ARRAYVALUE:
        case DECL_IMPORT:
            // never symbol
            break;
        }
        if (D->isPublic()) out << " public";
        if (D->isExported()) out << " exported";
        out.setColor(COL_NORM);
        out << '\n';
    }
    printf("%s", (const char*)out);
}

const AttrList& Module::getAttributes(const Decl* d) const {
    assert(d->hasAttributes());
    AttrMapConstIter iter = declAttrs.find(d);
    assert(iter != declAttrs.end() && "called before Decl is linked to module!");
    return iter->second;
}

