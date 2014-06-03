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

#ifndef AST_MODULE_H
#define AST_MODULE_H

#include <string>
#include <map>

namespace C2 {

class Decl;

class Module {
public:
    Module(const std::string& name_, bool isExternal_, bool isCLib_);

    void addSymbol(Decl* decl);
    Decl* findSymbol(const std::string& name) const;
    const std::string& getName() const { return name; }
    const std::string& getCName() const;
    bool isPlainC() const { return isCLib; }
    bool isExternal() const { return is_External; }

    void dump() const;

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    typedef Symbols::iterator SymbolsIter;
    const Symbols& getSymbols() const { return symbols; }
private:
    const std::string name;
    bool is_External;
    bool isCLib;    // not a C2 module, but used C library

    Symbols symbols;
};

typedef std::map<std::string, Module*> Modules;
typedef Modules::const_iterator ModulesConstIter;
typedef Modules::iterator ModulesIter;

}

#endif

