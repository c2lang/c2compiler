/* Copyright 2013-2022 Bas van den Berg
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
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

using namespace C2;

Component::~Component() {
    for (unsigned i=0; i<modules.size(); i++) {
        delete modules[i];
    }
}

Module* Component::getModule(const std::string& name_) {
    for (unsigned i=0; i<modules.size(); i++) {
        if (modules[i]->getName() == name_) { return modules[i]; }
    }
    Module* module = new Module(name_, is_external, is_interface, is_clib, type == PLUGIN);
    if (isExported(name_)) module->setExported();
    modules.push_back(module);
    return module;
}

Module* Component::findModule(const std::string& name_) const {
    for (unsigned i=0; i<modules.size(); i++) {
        if (modules[i]->getName() == name_) { return modules[i]; }
    }
    return 0;
}

void Component::print(StringBuilder& out) const {
    out << "Component " << name << ' ';
    out.setColor(COL_ATTRIBUTES);
    out << Str(type);
    if (is_external) {
        out.setColor(COL_ATTRIBUTES);
        out << " external";
        out.setColor(ANSI_NORMAL);
    }
    if (!deps.empty()) {
        out.setColor(COL_EXPR);
        out << ' ' << '(';
        for (unsigned i=0; i<deps.size(); i++) {
            if (i != 0) out << ", ";
            out << deps[i]->name << ' ' << Str(deps[i]->type);
        }
        out << ')';
        out.setColor(ANSI_NORMAL);
    }
    out.setColor(ANSI_NORMAL);
    out << '\n';
    for (unsigned i=0; i<modules.size(); i++) {
        bool brief = false;
        if (isExternal() || isInternalOrPlugin()) brief = true;
        modules[i]->printFiles(out, brief);
    }
}

void Component::printSymbols(StringBuilder& out, bool printNonPublic) const {
    out << "Component " << name;
    if (is_external) {
        out.setColor(COL_ATTRIBUTES);
        out << " external";
        out.setColor(ANSI_NORMAL);
    }
    out << '\n';
    for (unsigned i=0; i<modules.size(); i++) {
        if (!modules[i]->isLoaded()) continue;
        modules[i]->printSymbols(out, printNonPublic);
    }
}

bool Component::isExported(const std::string& moduleName) const {
    for (unsigned i=0; i<exportList.size(); ++i) {
        if (exportList[i] == moduleName) return true;
    }
    return false;
}

bool Component::hasDep(const Component* other) const {
    for (unsigned i=0; i<deps.size(); i++) {
        if (deps[i] == other) return true;
    }
    return false;
}

const char* C2::Str(Component::Type type) {
    switch (type) {
    case Component::MAIN_EXECUTABLE:    return "executable";
    case Component::MAIN_SHARED_LIB:    return "main_shared_lib";
    case Component::MAIN_STATIC_LIB:    return "main_static_lib";
    case Component::MAIN_SOURCE_LIB:    return "main_source_lib";
    case Component::EXT_SHARED_LIB:     return "external_shared_lib";
    case Component::EXT_STATIC_LIB:     return "external_static_lib";
    case Component::EXT_SOURCE_LIB:     return "external_source_lib";
    case Component::INTERNAL:      return "internal";
    case Component::PLUGIN:        return "plugin";
    }
    return "";
}

