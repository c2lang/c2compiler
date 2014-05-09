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
#include "AST/Package.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"
#include "Utils/constants.h"

#include <string.h>
#include <stdio.h>

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

namespace C2 {

class DepFile {
public:
    DepFile(const std::string& name_, const AST& ast_)
        : name(name_), ast(ast_) {}

    std::string name;
    const AST& ast;
};

class PkgInfo {
public:
    PkgInfo(const std::string& name_) : name(name_) {}
    ~PkgInfo() {
        // TODO delete DepFiles
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
    for (unsigned i=0; i<packages.size(); i++) {
        delete packages[i];
    }
}

void DepGenerator::analyse(const AST& ast) {
    const string& pkgName = ast.getPkgName();
    const string& fileName = ast.getFileName();
    //fprintf(stderr, "DEP %s  %s\n", pkgName.c_str(), fileName.c_str());

    PkgInfo* info = getInfo(pkgName);
    info->addFile(fileName, ast);
}

PkgInfo* DepGenerator::getInfo(const std::string& pkgname) {
    for (unsigned i=0; i<packages.size(); i++) {
        PkgInfo* P = packages[i];
        if (P->name == pkgname) return P;
    }
    PkgInfo* P = new PkgInfo(pkgname);
    packages.push_back(P);
    return P;
}

void DepGenerator::write(StringBuilder& output) const {
    int indent = 0;
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    output << "<dsm>\n";
    indent += INDENT;
    output.indent(indent);
    output << "<model>\n";

    indent += INDENT;
    for (unsigned i=0; i<packages.size(); i++) {
        const PkgInfo* P = packages[i];
        output.indent(indent);
        output << "<group name='" << P->name << "' full='package:" << P->name << "' collapsed='1'>\n";
        indent += INDENT;

        for (unsigned j=0; j<P->files.size(); j++) {
            const DepFile* F = P->files[j];
            output.indent(indent);
            // TODO use name / fullName
            const char* fname = getFileName(F->name);
            output << "<group name='" << fname << "' full='file:" << F->name << "' collapsed='1'>\n";
            indent += INDENT;

            writeAST(F->ast, output, indent);

            indent -= INDENT;
            output.indent(indent);
            output << "</group>\n";
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
    const char* type = "";
    switch (D->getKind()) {
    case DECL_FUNC:
        type = "func:";
        break;
    case DECL_VAR:
        type = "var:";
        break;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        type = "type:";
        break;
    default:
        assert(0);
    }
    output.indent(indent);
    output << "<atom name='" << D->getName() << "' full='" << type;
    output << D->getPackage()->getName() << '_' << D->getName() << "'>\n";
    output.indent(indent);
    output << "</atom>\n";
}

