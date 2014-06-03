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

#ifndef AST_AST_H
#define AST_AST_H

#include <string>
#include <vector>
#include <map>

#include <clang/Basic/SourceLocation.h>

#include "AST/Decl.h"
#include "AST/OwningVector.h"

namespace C2 {

class Decl;
class ImportDecl;
class TypeDecl;
class VarDecl;
class FunctionDecl;
class ArrayValueDecl;

class AST {
public:
    AST(const std::string& filename_, unsigned id)
        : filename(filename_)
        , file_id(id)
    {}

    void print(bool colors) const;

    // ImportDecls
    void addImport(ImportDecl* d) { importList.push_back(d); }
    unsigned numImports() const { return importList.size(); }
    ImportDecl* getImport(unsigned i) const { return importList[i]; }

    // TypeDecls
    void addType(TypeDecl* d) { typeList.push_back(d); }
    unsigned numTypes() const { return typeList.size(); }
    TypeDecl* getType(unsigned i) const { return typeList[i]; }

    // VarDecls
    void addVar(VarDecl* d) { varList.push_back(d); }
    unsigned numVars() const { return varList.size(); }
    VarDecl* getVar(unsigned i) const { return varList[i]; }

    // FunctionDecls
    void addFunction(FunctionDecl* d) { functionList.push_back(d); }
    unsigned numFunctions() const { return functionList.size(); }
    FunctionDecl* getFunction(unsigned i) const { return functionList[i]; }

    // ArrayValueDecls
    void addArrayValue(ArrayValueDecl* d) { arrayValues.push_back(d); }
    unsigned numArrayValues() const { return arrayValues.size(); }
    ArrayValueDecl* getArrayValue(unsigned i) const { return arrayValues[i]; }

    void addSymbol(Decl* d);

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    const Symbols& getSymbols() const { return symbols; }
    Decl* findSymbol(const std::string& name) const {
        SymbolsConstIter iter = symbols.find(name);
        if (iter != symbols.end()) return iter->second;
        return 0;
    }

    void setName(const std::string& name, clang::SourceLocation loc) {
        modName = name;
        modLoc = loc;
    }
    const std::string& getModuleName() const { return modName; }
    const std::string& getFileName() const { return filename; }
    unsigned getFileID() const { return file_id; }

private:
    AST(const AST&);
    void operator=(const AST&);

    const std::string filename;
    unsigned file_id;
    std::string modName;
    clang::SourceLocation modLoc;

    typedef OwningVector<ImportDecl> ImportList;
    ImportList importList;

    typedef OwningVector<TypeDecl> TypeList;
    TypeList typeList;

    typedef OwningVector<VarDecl> VarList;
    VarList varList;

    typedef OwningVector<FunctionDecl> FunctionList;
    FunctionList functionList;

    typedef OwningVector<ArrayValueDecl> ArrayValues;
    ArrayValues arrayValues;

    Symbols symbols;
};

}

#endif

