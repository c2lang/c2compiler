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

#include "Algo/DepGenerator.h"
#include "Algo/DepVisitor.h"
#include "AST/Module.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "AST/Type.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"
#include "Utils/Utils.h"
#include "FileUtils/FileUtils.h"

#include <string.h>
#include <assert.h>

using namespace C2;
using namespace std;


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
        const Component* C = components[c];
        if (!showExternals && C->isExternal()) continue;

        output.indent(indent);
        output << "<group name='" << C->getName() << "' full='component:" << C->getName() << "' collapsed='0'>\n";

        indent += INDENT;
        const ModuleList& mods = C->getModules();
        for (unsigned m=0; m<mods.size(); m++) {
            writeModule(*mods[m], output, indent);
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

    FileUtils::writeFile(path.c_str(), path + "deps.xml", output);
}

void DepGenerator::writeModule(const Module& M, StringBuilder& output, unsigned indent) const {
    output.indent(indent);
    output << "<group name='" << M.getName() << "' full='module:" << M.getName() << "' collapsed='1'>\n";
    indent += INDENT;

    const AstList& files = M.getFiles();
    for (unsigned j=0; j<files.size(); j++) {
        const AST* A = files[j];
        if (showFiles) {
            output.indent(indent);
            const char* fname = Utils::getFileName(A->getFileName());
            output << "<group name='" << fname << "' full='file:" << A->getFileName() << "' collapsed='1'>\n";
            indent += INDENT;
        }

        writeAST(*A, output, indent, M.isExternal());

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

void DepGenerator::writeAST(const AST& ast, StringBuilder& output, unsigned indent, bool isExternal) const {
    for (unsigned i=0; i<ast.numTypes(); i++) {
        const Decl* D = ast.getType(i);
        if (isExternal && !D->isUsed()) continue;
        writeDecl(D, output, indent);
    }
    for (unsigned i=0; i<ast.numVars(); i++) {
        const Decl* D = ast.getVar(i);
        if (isExternal && !D->isUsed()) continue;
        writeDecl(D, output, indent);
    }
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        const Decl* D = ast.getFunction(i);
        if (isExternal && !D->isUsed()) continue;
        writeDecl(D, output, indent);
    }
}

void DepGenerator::writeDecl(const Decl* D, StringBuilder& output, unsigned indent) const {
    if (!showPrivate && !D->isPublic()) return;
    output.indent(indent);
    output << "<atom name='" << D->getName();
    if (isa<FunctionDecl>(D)) output << "()";
    output << "' full='";
    D->fullName(output);
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
        dep->fullName(output);
        output << "' str='1'/>\n";
    }

    indent -= INDENT;
    output.indent(indent);
    output << "</atom>\n";
}

