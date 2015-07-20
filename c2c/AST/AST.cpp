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

#include <stdio.h>

#include "AST/Decl.h"
#include "AST/AST.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

//#define SEMA_DEBUG

using namespace C2;

AST::~AST() {
    for (AttrMapIter iter = declAttrs.begin(); iter != declAttrs.end(); ++iter) {
        AttrList& AL = iter->second;
        for (AttrListIter ai = AL.begin(); ai != AL.end(); ++ai) {
            delete *ai;
        }
    }
}

void AST::print(bool colors, bool showAttrs) const {
    StringBuilder buffer(4*1024*1024);
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
    // Decl Attributes
    if (showAttrs) {
        buffer.setColor(ANSI_YELLOW);
        if (!declAttrs.empty()) buffer << "Attributes:\n";
        for (AttrMapConstIter iter = declAttrs.begin(); iter != declAttrs.end(); ++iter) {
            const AttrList& AL = iter->second;
            const Decl* D = iter->first;
            buffer << "  " << D->getName() << ": ";
            for (unsigned i=0; i<AL.size(); i++) {
                const Attr* A = AL[i];
                if (i != 0) buffer << ", ";
                A->print(buffer);
            }
            buffer << '\n';
        }
    }

    buffer.setColor(COL_NORM);
    printf("%s", (const char*)buffer);
}

void AST::addAttribute(const Decl* d, Attr* attr) {
    AttrMapIter iter = declAttrs.find(d);
    if (iter == declAttrs.end()) {
        declAttrs[d] = AttrList();
        iter = declAttrs.find(d);
    }
    iter->second.push_back(attr);
}

bool AST::hasAttribute(const Decl* d, AttrKind k) const {
    AttrMapConstIter iter = declAttrs.find(d);
    if (iter == declAttrs.end()) return false;

    const AttrList& AL = iter->second;
    for (AttrListConstIter ai = AL.begin(); ai != AL.end(); ++ai) {
        if ((*ai)->getKind() == k) return true;
    }

    return false;
}

void AST::addSymbol(Decl* d) {
    symbols[d->getName()] = d;
}


