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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Decl.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "color.h"
#include "ASTVisitor.h"
#include "AST.h"

//#define SEMA_DEBUG

#define COL_SEMA ANSI_RED

using namespace C2;

AST::~AST() {
    for (unsigned int i=0; i<decls.size(); i++) {
        delete decls[i];
    }
}

void AST::visitAST(ASTVisitor& visitor) {
    for (unsigned int i=0; i<decls.size(); i++) {
        bool stop = visitor.handle(decls[i]);
        if (stop) break;
    }
}

void AST::print(const std::string& filename) const {
    StringBuilder buffer;
    buffer << "---- AST " << "(pkg=" << pkgName << ") " << filename << " ----\n";
    for (DeclListConstIter iter = decls.begin(); iter != decls.end(); ++iter) {
        (*iter)->print(buffer, 0);
        buffer << '\n';
    }
    printf("%s", (const char*)buffer);
}

void AST::addSymbol(Decl* d) {
    symbols[d->getName()] = d;
}


