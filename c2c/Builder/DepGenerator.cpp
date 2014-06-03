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

#include "Builder/DepGenerator.h"
#include "Builder/DepVisitor.h"
#include "AST/Module.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "AST/Type.h"
#include "Utils/StringBuilder.h"
#include "Utils/constants.h"

#include <string.h>

using namespace C2;
using namespace std;

// return pointer to filename after last '/'
static const char* getFileName(const std::string& s) {
    const char* input = s.c_str();
    const char* cp = input + strlen(input) - 1;
    while (cp != input) {
        if (*cp == '/') return cp+1;
        cp--;
    }
    return cp;
}

static void fullName(const Decl* D, StringBuilder& output) {
    const Module* P = D->getModule();
    assert(P);
    output << P->getName() << '_' << D->getName();
}

namespace C2 {

class DepFile {
public:
    DepFile(const std::string& name_, const AST& ast_)
        : name(name_), ast(ast_) {}

    std::string name;
    const AST& ast;
};

class ModInfo {
public:
    ModInfo(const std::string& name_) : name(name_) {}
    ~ModInfo() {
        for (unsigned i=0; i<files.size(); i++) {
            delete files[i];
        }
    }

    void addFile(const std::string& name_, const AST& ast_) {
        files.push_back(new DepFile(name_, ast_));
    }

    std::string name;
    typedef std::vector<DepFile*> Files;
    Files files;
};

}

DepGenerator::~DepGenerator() {
    for (unsigned i=0; i<modules.size(); i++) {
        delete modules[i];
    }
}

void DepGenerator::analyse(const AST& ast) {
    const string& modName = ast.getModuleName();
    const string& fileName = ast.getFileName();

    ModInfo* info = getInfo(modName);
    info->addFile(fileName, ast);
}

ModInfo* DepGenerator::getInfo(const std::string& modName) {
    for (unsigned i=0; i<modules.size(); i++) {
        ModInfo* P = modules[i];
        if (P->name == modName) return P;
    }
    ModInfo* P = new ModInfo(modName);
    modules.push_back(P);
    return P;
}

void DepGenerator::addExternal(const Module* P) const {
    for (unsigned i=0; i<externals.size(); i++) {
        if (externals[i] == P) return;
    }
    externals.push_back(P);
}

void DepGenerator::write(StringBuilder& output, const std::string& title) const {
    int indent = 0;
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    output << "<dsm name='" << title << "'>\n";
    indent += INDENT;
    output.indent(indent);
    output << "<model>\n";

    indent += INDENT;
    for (unsigned i=0; i<modules.size(); i++) {
        const ModInfo* P = modules[i];
        output.indent(indent);
        output << "<group name='" << P->name << "' full='module:" << P->name << "' collapsed='1'>\n";
        indent += INDENT;

        for (unsigned j=0; j<P->files.size(); j++) {
            const DepFile* F = P->files[j];
            if (showFiles) {
                output.indent(indent);
                const char* fname = getFileName(F->name);
                output << "<group name='" << fname << "' full='file:" << F->name << "' collapsed='1'>\n";
                indent += INDENT;
            }

            writeAST(F->ast, output, indent);

            if (showFiles) {
                indent -= INDENT;
                output.indent(indent);
                output << "</group>\n";
            }
        }

        indent -= INDENT;
        output.indent(indent);
        output << "</group>\n";
    }
    if (showExternals) {
        output.indent(indent);
        output << "<group name='Externals' full='Externals' collapsed='1'>\n";
        indent += INDENT;
        for (unsigned i=0; i<externals.size(); i++) {
            writeExternal(externals[i], output, indent);
        }
        indent -= INDENT;
        output.indent(indent);
        output << "</group>\n";
    }
    indent -= INDENT;

    output.indent(indent);
    output << "</model>\n";
    indent -= INDENT;
    output << "</dsm>\n";
}

void DepGenerator::writeAST(const AST& ast, StringBuilder& output, unsigned indent) const {
    if (showExternals) {
        for (unsigned i=0; i<ast.numImports(); i++) {
            const ImportDecl* U = ast.getImport(i);
            QualType Q = U->getType();
            const ModuleType* T = cast<ModuleType>(Q.getTypePtr());
            const Module* P = T->getModule();
            assert(P);
            if (P->isExternal()) addExternal(P);
        }
    }
    for (unsigned i=0; i<ast.numTypes(); i++) {
        writeDecl(ast.getType(i), output, indent);
    }
    for (unsigned i=0; i<ast.numVars(); i++) {
       writeDecl(ast.getVar(i), output, indent);
    }
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        writeDecl(ast.getFunction(i), output, indent);
    }
}

void DepGenerator::writeDecl(const Decl* D, StringBuilder& output, unsigned indent) const {
    if (!showPrivate && !D->isPublic()) return;
    output.indent(indent);
    output << "<atom name='" << D->getName();
    if (isa<FunctionDecl>(D)) output << "()";
    output << "' full='";
    fullName(D, output);
    output << "'>\n";
    indent += INDENT;

    DepVisitor visitor(D);
    visitor.run();

    for (unsigned i=0; i<visitor.getNumDeps(); i++) {
        // syntax: <dep dest='G1/B' str='1'/>
        const Decl* dep = visitor.getDep(i);
        if (!showPrivate && !dep->isPublic()) continue;
        if (!showExternals && dep->getModule()->isExternal()) continue;
        output.indent(indent);
        output << "<dep dest='";
        fullName(dep, output);
        output << "' str='1'/>\n";
    }

    indent -= INDENT;
    output.indent(indent);
    output << "</atom>\n";
}

void DepGenerator::writeExternal(const Module* P, StringBuilder& output, unsigned indent) const {
    output.indent(indent);
    output << "<group name='" << P->getName() << "' full='module:" << P->getName() << "' collapsed='1'>\n";
    indent += INDENT;

    const Module::Symbols& symbols = P->getSymbols();
    for (Module::SymbolsConstIter iter=symbols.begin(); iter!=symbols.end(); ++iter) {
        const Decl* D = iter->second;
        output.indent(indent);
        output << "<atom name='" << D->getName();
        if (isa<FunctionDecl>(D)) output << "()";
        output << "' full='";
        fullName(D, output);
        output << "' />\n";
    }

    indent -= INDENT;
    output.indent(indent);
    output << "</group>\n";
}

