/* Copyright 2013 Bas van den Berg
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

#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <map>

namespace C2 {

class Package;
class Decl;

class Scope {
public:
    Scope(const std::string& name_);

    void addPackage(const std::string& name_, const Package* pkg);
    Decl* findSymbol(const char* pkgName, const char* symbolName) const;
    const Package* findPackage(const std::string& pkgName) const;

    const std::string& getName() const { return name; }
    void dump();
private:
    const std::string name;

    typedef std::map<std::string, const Package*> Packages;
    typedef Packages::const_iterator PackagesConstIter;
    typedef Packages::iterator PackagesIter;
    Packages packages;
};

}

#endif

