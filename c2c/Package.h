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

#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <map>

namespace C2 {

class Decl;

class Package {
public:
    Package(const std::string& name_);

    void addSymbol(Decl* decl);
    Decl* findSymbol(const std::string& name) const;
    const std::string& getName() const { return name; }

    void dump();
private:
    const std::string name;

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    typedef Symbols::iterator SymbolsIter;
    Symbols symbols;
};

typedef std::map<std::string, Package*> Pkgs;
typedef Pkgs::const_iterator PkgsConstIter;
typedef Pkgs::iterator PkgsIter;

}

#endif

