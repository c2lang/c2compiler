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

#include "Analyser/AnalyserUtils.h"
#include "Utils/StringBuilder.h"

using namespace C2;

const char* AnalyserUtils::fullName(const std::string& modName, const std::string& symname) {
    static char buffer[128];
    sprintf(buffer, "%s.%s", modName.c_str(), symname.c_str());
    return buffer;
}

