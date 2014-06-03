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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/AST.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

//#define SEMA_DEBUG

using namespace C2;

void AST::print(bool colors) const {
    StringBuilder buffer;
    buffer.enableColor(colors);
    buffer << "---- AST " << "(module=" << modName << ") " << filename << " ----\n";
    // ImportDecls
    for (unsigned i=0; i<importList.size(); i++) {
        importList[i]->print(buffer, 0);
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
    buffer.setColor(COL_NORM);
    printf("%s", (const char*)buffer);
}

void AST::addSymbol(Decl* d) {
    symbols[d->getName()] = d;
}


