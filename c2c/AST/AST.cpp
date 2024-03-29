/* Copyright 2013-2023 Bas van den Berg
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

#include <stdio.h>

#include "AST/AST.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

//#define SEMA_DEBUG

using namespace C2;

void AST::print(bool colors) const {
    StringBuilder buffer(4*1024*1024);
    buffer.enableColor(colors);
    print(buffer);
    printf("%s", buffer.c_str());
}

void AST::print(StringBuilder& buffer) const {;
    buffer << "---- AST " << "(module=" << modName << ") " << filename << " ----\n";
    // ImportDecls
    for (unsigned i=1; i<importList.size(); i++) {
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
    // StaticAssertDecls
    for (unsigned i=0; i<staticAsserts.size(); i++) {
        staticAsserts[i]->print(buffer, 0);
        buffer << '\n';
    }

    buffer.setColor(COL_NORM);
    buffer << '\n';
}

void AST::setChecked() {
    for (unsigned i=0; i<numTypes(); i++) {
        TypeDecl* T = getType(i);
        T->setCheckState(CHECK_DONE);
    }
    for (unsigned i=0; i<numVars(); i++) {
        VarDecl* V = getVar(i);
        V->setCheckState(CHECK_DONE);
    }
}

