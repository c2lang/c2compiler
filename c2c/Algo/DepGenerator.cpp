/* Copyright 2013-2015 Bas van den Berg
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

#include "Algo/DepGenerator.h"
#include "Algo/DepVisitor.h"
#include "AST/Module.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "AST/Type.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"
#include "FileUtils/FileUtils.h"

#include <string.h>
#include <assert.h>

using namespace C2;
using namespace std;

// TODO move to Utils..
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

// TODO move to Utils..
static void fullName(const Decl* D, StringBuilder& output) {
    const Module* P = D->getModule();
    assert(P);
    output << P->getName() << '_' << D->getName();
}


void DepGenerator::write(const Components& components, const std::string& title, const std::string& path) const {

    StringBuilder output;
    int indent = 0;
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    output << "<dsm name='" << title << "'>\n";
    indent += INDENT;
    output.indent(indent);
    output << "<model>\n";

    indent += INDENT;
    for (unsigned c=0; c<components.size(); c++) {
        // BBB also show components
        const Component* C = components[c];
        if (!showExternals && C->isExternal) continue;

        const ModuleList& mods = C->getModules();
        for (unsigned m=0; m<mods.size(); m++) {
            writeModule(*mods[m], output, indent);
        }
    }
    indent -= INDENT;

    output.indent(indent);
    output << "</model>\n";
    indent -= INDENT;
    output << "</dsm>\n";

    FileUtils::writeFile(path.c_str(), path + "deps.xml", output);
}

void DepGenerator::writeModule(const Module& M, StringBuilder& output, unsigned indent) const {
    output.indent(indent);
    output << "<group name='" << M.getName() << "' full='module:" << M.getName() << "' collapsed='1'>\n";
    indent += INDENT;

    const Files& files = M.getFiles();
    for (unsigned j=0; j<files.size(); j++) {
        const AST* A = files[j];
        if (showFiles) {
            output.indent(indent);
            const char* fname = getFileName(A->getFileName());
            output << "<group name='" << fname << "' full='file:" << A->getFileName() << "' collapsed='1'>\n";
            indent += INDENT;
        }

        writeAST(*A, output, indent);

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

void DepGenerator::writeAST(const AST& ast, StringBuilder& output, unsigned indent) const {
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

    DepVisitor visitor(D, true);
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

