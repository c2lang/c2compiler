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

#include "AST/Component.h"
#include "AST/AST.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

using namespace C2;

Component::~Component() {
    for (unsigned i=0; i<modules.size(); i++) {
        delete modules[i];
    }
}

Module* Component::addAST(AST* ast, const std::string& moduleName) {
    Module* M = getModule(moduleName);
    M->addAST(ast);
    return M;
}

Module* Component::getModule(const std::string& name) {
    for (unsigned i=0; i<modules.size(); i++) {
        if (modules[i]->getName() == name) return modules[i];
    }
    Module* module = new Module(name, isExternal, isCLib);
    modules.push_back(module);
    return module;
}

void Component::print(StringBuilder& out) const {
    out << "Component " << name;
    if (isExternal) {
        out.setColor(COL_ATTRIBUTES);
        out << "  external";
        out.setColor(ANSI_NORMAL);
    }
    out << '\n';
    for (unsigned i=0; i<modules.size(); i++) {
        modules[i]->dumpAST(out);
    }
}

