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

#include <stdio.h>
#include <ctype.h>

#include "Analyser/AnalyserUtils.h"

using namespace C2;

const char* AnalyserUtils::fullName(const std::string& modName, const char* symname) {
    static char buffer[128];
    sprintf(buffer, "%s.%s", modName.c_str(), symname);
    return buffer;
}

const char* AnalyserUtils::splitStructFunctionName(char* structName, const char* funcName) {
    // foo_bar -> typename='Foo', return 'bar' (or NULL if no _)
    const char* cp = funcName;
    char* op = structName;
    const char* memberName = 0;
    while (*cp != 0) {
        if (*cp == '_') {
            cp++;
            memberName = cp;
            break;
        }
        *op = *cp;
        ++cp;
        ++op;
    }
    *op = 0;
    structName[0] = toupper(structName[0]);

    return memberName;
}

