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

#ifndef AST_AST_H
#define AST_AST_H

#include <string>

#include "Clang/SourceLocation.h"

#include "AST/Decl.h"
#include "AST/Type.h"
#include "AST/ASTContext.h"

namespace C2 {

class AST {
public:
    AST(const std::string& filename_, bool isInterface_, bool isExternal_)
        : filename(filename_)
        , m_isInterface(isInterface_)
        , m_isExternal(isExternal_)
    {}
    ~AST() {}

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

    // StaticAssertDecl
    void addStaticAssert(StaticAssertDecl* d) { staticAsserts.push_back(d); }
    unsigned numStaticAsserts() const { return staticAsserts.size(); }
    StaticAssertDecl* getStaticAssert(unsigned i) const { return staticAsserts[i]; }

    void setName(const std::string& name, c2lang::SourceLocation loc) {
        modName = name;
        modLoc = loc;
    }
    const std::string& getModuleName() const { return modName; }
    const std::string& getFileName() const { return filename; }
    ASTContext& getContext() { return astContext; }
    bool isInterface() const { return m_isInterface; }
    bool isExternal() const { return m_isExternal; }
private:
    AST(const AST&);
    void operator=(const AST&);

    const std::string filename;
    std::string modName;
    c2lang::SourceLocation modLoc;
    bool m_isInterface;       // set for .c2i files
    bool m_isExternal;        // if not in main component

    typedef std::vector<ImportDecl*> ImportList;
    ImportList importList;

    typedef std::vector<TypeDecl*> TypeList;
    TypeList typeList;

    typedef std::vector<VarDecl*> VarList;
    VarList varList;

    typedef std::vector<FunctionDecl*> FunctionList;
    FunctionList functionList;

    typedef std::vector<ArrayValueDecl*> ArrayValues;
    ArrayValues arrayValues;

    typedef std::vector<StaticAssertDecl*> StaticAsserts;
    StaticAsserts staticAsserts;

    ASTContext astContext;

    // TEMP for Rewriter
    //FileID fileID;
};

typedef std::vector<AST*> AstList;

}

#endif

