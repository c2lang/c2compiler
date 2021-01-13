/* Copyright 2013-2021 Bas van den Berg
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

#include "Builder/Recipe.h"

using namespace C2;

void Recipe::addLibrary(const std::string& lib_, Component::Type type_) {
    libraries.push_back(Dependency(lib_, type_));
}

bool Recipe::hasLibrary(const std::string& lib_) const {
    for (unsigned i=0; i<libraries.size(); i++) {
        if (libraries[i].name == lib_) return true;
    }
    return false;
}

const std::string& Recipe::get(int i) const {
    return files[i];
}

bool Recipe::hasExported(const std::string& mod) const {
    for (unsigned i=0; i<exported.size(); ++i) {
        if (exported[i] == mod) return true;
    }
    return false;
}

