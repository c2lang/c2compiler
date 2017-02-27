/* Copyright 2013-2017 Bas van den Berg
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

#include "Builder/ManifestWriter.h"
#include "Utils/StringBuilder.h"
#include "FileUtils/FileUtils.h"
#include "AST/Component.h"
#include "AST/Module.h"

using namespace C2;

static const char* type2str(Component::Type type) {
    if (type == Component::SHARED_LIB) return "dynamic";
    if (type == Component::STATIC_LIB) return "static";
    return "";
}

#include <stdio.h>
void ManifestWriter::write(const std::string& dirname) const {
    StringBuilder out;
    out << "[library]\n";
    out << "language = \"C2\"\n";
    out << "type = [ \"" << type2str(component.getType()) << "\" ]\n";
    out << "linkname = \"" << component.getName() << "\"\n";

    const Component::Dependencies& deps = component.getDeps();
    for (unsigned i=0; i<deps.size(); i++) {
        const Component* dep = deps[i];
        out << '\n';
        out << "[[deps]]\n";
        out << "name = \"" << dep->getName() << "\"\n";
        out << "type = \"" << type2str(dep->getType()) << "\"\n";
    }

    const ModuleList& mods = component.getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        const Module* M = mods[m];
        if (!M->isExported()) continue;
        out << '\n';
        out << "[[modules]]\n";
        out << "name = \"" << M->getName() << "\"\n";
    }

    // TODO handle errors
    FileUtils::writeFile(dirname.c_str(), dirname + "manifest", out);
}

