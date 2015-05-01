/* Copyright 2013,2014,2015 Bas van den Berg
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

#include "Analyser/TargetAnalyser.h"
#include "Analyser/FileAnalyser.h"

using namespace C2;

TargetAnalyser::~TargetAnalyser() {
    for (unsigned i=0; i<analysers.size(); i++) {
        delete analysers[i];
    }
}

void TargetAnalyser::addFile(AST& ast, TypeContext& typeContext) {
    analysers.push_back(new FileAnalyser(modules, Diags, ast, typeContext, verbose));
}

unsigned TargetAnalyser::analyse(bool print1, bool print2, bool print3) {
    unsigned errors = 0;
    const size_t count = analysers.size();

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkImports();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveTypes();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveTypeCanonicals();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveStructMembers();
    }
    if (print1) printASTs();
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveVars();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveEnumConstants();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkArrayValues();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkVarInits();
    }
    if (print2) printASTs();
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkFunctionProtos();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkFunctionBodies();
    }

    for (unsigned i=0; i<count; i++) {
        analysers[i]->checkDeclsForUsed();
    }

    if (print3) printASTs();
    return errors;
}

void TargetAnalyser::printASTs() const {
    for (unsigned i=0; i<analysers.size(); i++) {
        analysers[i]->printAST();
    }
}

