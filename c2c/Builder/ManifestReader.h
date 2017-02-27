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

#ifndef BUILDER_MANIFEST_READER_H
#define BUILDER_MANIFEST_READER_H

#include <string>
#include <vector>
#include "Utils/StringList.h"

namespace C2 {

class ManifestEntry {
public:
    ManifestEntry(const std::string& name_) : name(name_) {}
    std::string name;
    std::string c2File;
};
typedef std::vector<ManifestEntry> Entries;

class ManifestReader {
public:
    ManifestReader(const std::string& name_, const char* filename_)
        : componentName(name_)
        , filename(filename_)
        , _isNative(false)
        , hasStaticLib(false)
        , hasDynamicLib(false)
    {}
    ~ManifestReader() {}

    bool parse();
    const char* getErrorMsg() const { return errorMsg; }

    bool isNative() const { return _isNative; }
    bool hasStatic() const { return hasStaticLib; }
    bool hasDynamic() const { return hasDynamicLib; }
    const std::string& getLinkName() const { return linkName; }
    unsigned numEntries() const { return entries.size(); }
    const ManifestEntry& get(unsigned index) const { return entries[index]; }
    const StringList& getDeps() const { return deps; }
private:
    const std::string& componentName;
    std::string filename;
    std::string linkName;
    char errorMsg[256];
    Entries entries;
    StringList deps;
    bool _isNative;
    bool hasStaticLib;
    bool hasDynamicLib;
};

}

#endif

