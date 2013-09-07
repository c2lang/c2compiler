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

#include "Decl.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "color.h"
#include "AST.h"

//#define SEMA_DEBUG

#define COL_SEMA ANSI_RED

using namespace C2;

void AST::print() const {
    StringBuilder buffer;
    buffer << "---- AST " << "(pkg=" << pkgName << ") " << filename << " ----\n";
    // UseDecls
    for (unsigned i=0; i<useList.size(); i++) {
        useList[i]->print(buffer, 0);
        buffer << '\n';
    }
    // TypeDecls
    for (unsigned i=0; i<typeList.size(); i++) {
        typeList[i]->print(buffer, 0);
        buffer << '\n';
    }
    // VarDecls
    for (unsigned i=0; i<varList.size(); i++) {
        varList[i]->print(buffer, 0);
        buffer << '\n';
    }
    // ArrayValueDecls
    for (unsigned i=0; i<arrayValues.size(); i++) {
        arrayValues[i]->print(buffer, 0);
        buffer << '\n';
    }
    // FunctionDecls
    for (unsigned i=0; i<functionList.size(); i++) {
        functionList[i]->print(buffer, 0);
        buffer << '\n';
    }
    printf("%s", (const char*)buffer);
}

void AST::addSymbol(Decl* d) {
    symbols[d->getName()] = d;
}


