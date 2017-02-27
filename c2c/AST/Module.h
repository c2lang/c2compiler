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

#ifndef AST_MODULE_H
#define AST_MODULE_H

#include <string>
#include <map>

#include <clang/Basic/SourceLocation.h>

#include "AST/Attr.h"
#include "AST/AST.h"

namespace clang {
class SourceLocation;
}

namespace C2 {

class Decl;
class StringBuilder;
class AST;

class Module {
public:
    Module(const std::string& name_, bool isExternal_, bool isCLib_);
    ~Module();

    void addAST(AST* ast) { files.push_back(ast); }

    void addSymbol(Decl* decl);
    Decl* findSymbol(const std::string& name) const;
    const std::string& getName() const { return name; }
    const std::string& getCName() const;
    bool isPlainC() const { return m_isCLib; }
    bool isExternal() const { return m_isExternal; }
    bool isExported() const { return m_isExported; }
    void setExported() { m_isExported = true; }
    bool isLoaded() const { return files.size() != 0; }

    void dump() const;
    void printFiles(StringBuilder& output) const;
    void printSymbols(StringBuilder& output) const;
    void print(StringBuilder& output) const;

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    typedef Symbols::iterator SymbolsIter;
    const Symbols& getSymbols() const { return symbols; }

    void addAttribute(const Decl* d, Attr* attr);
    bool hasAttribute(const Decl* d, AttrKind k) const;
    AttrMap& getAttributes() { return declAttrs; }
    const AttrList& getAttributes(const Decl* d) const;
    void printAttributes(bool colors) const;

    const AstList& getFiles() const { return files; }
private:
    const std::string name;
    bool m_isExternal;       // not a module in current target
    bool m_isCLib;           // not a C2 module, but used C library
    bool m_isExported;       // symbols should be exported (in recipe)

    Symbols symbols;
    AttrMap declAttrs;

    AstList files;

    Module(const Module&);
    Module& operator= (const Module&);
};

typedef std::vector<Module*> ModuleList;

typedef std::map<std::string, Module*> Modules;
typedef Modules::const_iterator ModulesConstIter;
typedef Modules::iterator ModulesIter;

}

#endif

