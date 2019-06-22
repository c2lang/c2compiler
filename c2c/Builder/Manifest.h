/* Copyright 2013-2019 Bas van den Berg
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

#ifndef BUILDER_MANIFEST_H
#define BUILDER_MANIFEST_H

#include <string>
#include <vector>
#include "Utils/StringList.h"

namespace C2 {

class Manifest {
public:
    Manifest(const char* filename_)
        : filename(filename_)
        , isNative(false)
        , hasStaticLib(false)
        , hasDynamicLib(false)
    {}
    ~Manifest() {}

    std::string filename;
    std::string linkName;
    bool isNative;
    bool hasStaticLib;
    bool hasDynamicLib;
    StringList deps;
    StringList modules;

};

}

#endif

