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

// TEMP
#include <stdio.h>

#include "Builder/DepVisitor.h"
#include "AST/Decl.h"

using namespace C2;

void DepVisitor::run() {
    fprintf(stderr, "CHECKING %s\n", decl->getName().c_str());
    switch (decl->getKind()) {
    case DECL_FUNC:
        checkFunctionDecl(cast<FunctionDecl>(decl));
        break;
    case DECL_VAR:
        checkVarDecl(cast<VarDecl>(decl));
        break;
    case DECL_ENUMVALUE:
        assert(0);
        break;
    case DECL_ALIASTYPE:
        checkType(cast<AliasTypeDecl>(decl)->getRefType());
        break;
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        assert(0 && "TODO");
        break;
    case DECL_ARRAYVALUE:
        assert(0 && "TODO");
        break;
    case DECL_USE:
        break;
    }
}

void DepVisitor::checkFunctionDecl(const FunctionDecl* F) {
    // return Type
    checkType(F->getReturnType());

    // args
    for (unsigned i=0; i<F->numArgs(); i++) {
        checkVarDecl(F->getArg(i));
        // TODO default value
    }

    // check body
    // checkCompoundStmt(F->getBody());
    // TODO
}

void DepVisitor::checkVarDecl(const VarDecl* V) {
    checkType(V->getType());

    // TODO check init
}

void DepVisitor::checkType(QualType Q) {
    const Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return;
    case TC_POINTER:
        checkType(cast<PointerType>(T)->getPointeeType());
        break;
    case TC_ARRAY:
        // TODO size expr
        checkType(cast<ArrayType>(T)->getElementType());
        break;
    case TC_UNRESOLVED:
        addDep(cast<UnresolvedType>(T)->getDecl());
        break;
    case TC_ALIAS:
        addDep(cast<AliasType>(T)->getDecl());
        break;
    case TC_STRUCT:
        break;
    case TC_ENUM:
        break;
    case TC_FUNCTION:
        break;
    case TC_PACKAGE:
        assert(0);
        break;
    }
}

void DepVisitor::addDep(const Decl* D) {
    assert(D);
    if (decl == D) return;
    for (unsigned i=0; i<deps.size(); i++) {
        if (deps[i] == D) return;
    }
    deps.push_back(D);
    fprintf(stderr, "  %s -> %s\n", decl->getName().c_str(), D->getName().c_str());
}

