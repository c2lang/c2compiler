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

#include "Analyser/ComponentAnalyser.h"
#include "Analyser/FileAnalyser.h"
#include "AST/Component.h"
#include "AST/Module.h"
#include "AST/ASTContext.h"

#include <clang/Basic/Diagnostic.h>

using namespace C2;

ComponentAnalyser::ComponentAnalyser(Component& C,
                                     const Modules& modules_,
                                     clang::DiagnosticsEngine& Diags_,
                                     ASTContext& context_,
                                     bool verbose_)
    : Diags(Diags_)
    , context(context_)
    , verbose(verbose_)
{
    const ModuleList& mods = C.getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        const Module* M = mods[m];
        const AstList& files = M->getFiles();
        for (unsigned f=0; f<files.size(); f++) {
            analysers.push_back(new FileAnalyser(*M, modules_, Diags, *files[f], verbose));
        }
    }
}

ComponentAnalyser::~ComponentAnalyser() {
    for (unsigned i=0; i<analysers.size(); i++) {
        delete analysers[i];
    }
}

unsigned ComponentAnalyser::analyse(bool print1, bool print2, bool print3, bool printLib) {
    unsigned errors = 0;
    const size_t count = analysers.size();

    for (unsigned i=0; i<count; i++) {
        analysers[i]->addImports();
    }

    for (unsigned i=0; i<count; i++) {
        analysers[i]->resolveTypes();
    }
    if (Diags.hasErrorOccurred()) return 1;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveTypeCanonicals();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveStructMembers();
    }
    if (print1) printASTs(printLib);
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveVars();
    }
    if (errors) return errors;

    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->resolveEnumConstants();
    }
    if (errors) return errors;

    IncrementalArrayVals ia_values;
    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkArrayValues(ia_values);
    }
    if (errors) return errors;

    // Set ArrayValues
    for (IncrementalArrayValsIter iter = ia_values.begin(); iter != ia_values.end(); ++iter) {
        VarDecl* D = iter->first;
        unsigned numValues = iter->second.size();
        assert(numValues);
        // NOTE: incremenal array is given InitListExpr in resolveVars()
        Expr* I = D->getInitValue();
        assert(I);
        assert(dyncast<InitListExpr>(I));
        InitListExpr* ILE = cast<InitListExpr>(I);
        Expr** values = (Expr**)context.Allocate(sizeof(Expr*)*numValues);
        memcpy(values, &iter->second[0], sizeof(Expr*)*numValues);
        ILE->setValues(values, numValues);
    }
    ia_values.clear();

    StructFunctionList structFuncs;
    for (unsigned i=0; i<count; i++) {
        errors += analysers[i]->checkFunctionProtos(structFuncs);
    }
    if (errors) return errors;
    // Set StructFunctions
    // NOTE: since these are linked anyways, just use special ASTContext from Builder
    for (StructFunctionListIter iter = structFuncs.begin(); iter != structFuncs.end(); ++iter) {
        StructTypeDecl* S = iter->first;
        const StructFunctionEntries& entries = iter->second;
        unsigned numFuncs = entries.size();
        FunctionDecl** funcs = (FunctionDecl**)context.Allocate(sizeof(FunctionDecl*)*numFuncs);
        memcpy(funcs, &entries[0], sizeof(FunctionDecl*)*numFuncs);
        S->setStructFuncs(funcs, numFuncs);
    }

    for (unsigned i=0; i<count; i++) {
        analysers[i]->checkVarInits();
    }
    if (print2) printASTs(printLib);
    if (Diags.hasErrorOccurred()) return 1;

    for (unsigned i=0; i<count; i++) {
        analysers[i]->checkFunctionBodies();
    }
    if (Diags.hasErrorOccurred()) return 1;

    for (unsigned i=0; i<count; i++) {
        analysers[i]->checkDeclsForUsed();
    }

    if (print3) printASTs(printLib);
    return errors;
}

void ComponentAnalyser::printASTs(bool printLib) const {
    for (unsigned i=0; i<analysers.size(); i++) {
        analysers[i]->printAST(printLib);
    }
}

