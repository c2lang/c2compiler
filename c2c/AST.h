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

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <map>
#include <clang/Basic/SourceLocation.h>

namespace C2 {

class Decl;
class ASTVisitor;

class AST {
public:
    AST() {}
    ~AST();

    // debugging
    void print(const std::string& filename) const;

    // parsing
    void addDecl(Decl* d) { decls.push_back(d); }
    void addSymbol(Decl* d);

    // analysis
    void visitAST(ASTVisitor& visitor);

    unsigned getNumDecls() const { return decls.size(); }
    Decl* getDecl(unsigned index) const { return decls[index]; }

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    const Symbols& getSymbols() const { return symbols; }
    Decl* findSymbol(const std::string& name) const {
        SymbolsConstIter iter = symbols.find(name);
        if (iter != symbols.end()) return iter->second;
        return 0;
    }

    void setName(const std::string& name, clang::SourceLocation loc) {
        pkgName = name;
        pkgLoc = loc;
    }
    const std::string& getPkgName() const { return pkgName; }

private:
    std::string pkgName;
    clang::SourceLocation pkgLoc;

    typedef std::vector<Decl*> DeclList;
    typedef DeclList::const_iterator DeclListConstIter;
    typedef DeclList::iterator DeclListIter;
    DeclList decls;

    Symbols symbols;
};

}

#endif

