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

#include "Utils/GenUtils.h"
#include "Utils/StringBuilder.h"

using namespace C2;

void GenUtils::addName(const std::string& modName, const std::string& name, StringBuilder& buffer) {
    if (name == "main" || modName == "") {
        buffer << name;
    } else {
        buffer << "__" << modName << '_' << name;
    }
}

void GenUtils::toCapital(const std::string& input, StringBuilder& output) {
    for (unsigned i=0; i<input.size(); i++) {
        char c = input[i];
        if (c >= 'a' && c <= 'z') c &= ~0x20;   // in ascii a and A differ only 1 bit, etc
        output << c;
    }
}

