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
#include <vector>

namespace C2 {

class Package;
class Decl;

class ScopeResult {
public:
    ScopeResult()
        : pkg(0)
        , decl(0)
        , ambiguous(false)
        , external(true)
        , visible(true)
    {}

    const Package* pkg;
    Decl* decl;
    bool ambiguous;     // ambiguous lookup (returns first result)
    bool external;      // package is external
    bool visible;       // symbol is non-public and used externally
};

class Scope {
public:
    Scope(const std::string& name_);

    void addPackage(bool isLocal, const std::string& name_, const Package* pkg);
    const Package* findPackage(const std::string& pkgName) const;
    bool isExternal(const Package* pkg) const;
    ScopeResult findSymbol(const std::string& name) const;

    const std::string& getName() const { return name; }
    void dump();
private:
    const std::string name;

    // locals (or used as local)
    typedef std::vector<const Package*> Locals;
    typedef Locals::const_iterator LocalsConstIter;
    Locals locals;

    // externals
    typedef std::map<std::string, const Package*> Packages;
    typedef Packages::const_iterator PackagesConstIter;
    typedef Packages::iterator PackagesIter;
    Packages packages;
};

}

#endif

