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
#include <stdio.h>

#include <llvm/ADT/APInt.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/FileAnalyser.h"
#include "Analyser/Scope.h"
#include "Analyser/TypeResolver.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/AST.h"
#include "Utils/color.h"

using namespace C2;
using namespace clang;
using namespace llvm;

//#define ANALYSER_DEBUG

#ifdef ANALYSER_DEBUG
#include <iostream>
#define LOG_FUNC std::cerr << ANSI_BLUE << __func__ << "()" << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#endif

FileAnalyser::FileAnalyser(const Modules& modules, clang::DiagnosticsEngine& Diags_,
                    AST& ast_, TypeContext& typeContext_, bool verbose_)
    : ast(ast_)
    , globals(new Scope(ast_.getModuleName(), modules, Diags_))
    , TR(new TypeResolver(*globals, Diags_, typeContext_))
    , Diags(Diags_)
    , functionAnalyser(*globals, *TR, typeContext_, Diags_)
    , typeContext(typeContext_)
    , verbose(verbose_)
{}

unsigned  FileAnalyser::checkImports() {
    LOG_FUNC
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numImports(); i++) {
        if (!globals->addImportDecl(ast.getImport(i))) errors++;
    }
    return errors;
}

unsigned FileAnalyser::resolveTypes() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numTypes(); i++) {
        errors += checkTypeDecl(ast.getType(i));
    }
    return errors;
}

unsigned FileAnalyser::resolveTypeCanonicals() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numTypes(); i++) {
        const TypeDecl* D = ast.getType(i);
        // check generic type
        TR->resolveCanonicals(D, D->getType(), true);

        // NOTE dont check any subclass specific things yet

        // check extra stuff depending on subclass
        switch (D->getKind()) {
        case DECL_FUNC:
        case DECL_VAR:
        case DECL_ENUMVALUE:
            assert(0);
            break;
        case DECL_ALIASTYPE:
            // nothing to do
            break;
        case DECL_STRUCTTYPE:
            //dont check members yet
            break;
        case DECL_ENUMTYPE:
            // dont check constants / implType yet
            break;
        case DECL_FUNCTIONTYPE:
            {
                // return + argument types
                const FunctionTypeDecl* FTD = cast<FunctionTypeDecl>(D);
                errors += resolveFunctionDecl(FTD->getDecl());
                break;
            }
        case DECL_ARRAYVALUE:
        case DECL_IMPORT:
            assert(0);
            break;
        }
    }
    return errors;
}

unsigned FileAnalyser::resolveStructMembers() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numTypes(); i++) {
        TypeDecl* D = ast.getType(i);
        if (isa<StructTypeDecl>(D)) {
            errors += checkStructTypeDecl(cast<StructTypeDecl>(D));
        }
    }
    return errors;
}

unsigned FileAnalyser::resolveVars() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numVars(); i++) {
        errors += resolveVarDecl(ast.getVar(i));
    }
    return errors;
}

unsigned FileAnalyser::checkVarInits() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numVars(); i++) {
        VarDecl* V = ast.getVar(i);
        if (V->getInitValue()) {
            errors += functionAnalyser.checkVarInit(V);
        } else {
            QualType T = V->getType();
            if (T.isConstQualified()) {
                Diags.Report(V->getLocation(), diag::err_uninitialized_const_var) << V->getName();
                errors++;
            } else if (T->isArrayType()) {
                const ArrayType* AT = cast<ArrayType>(T.getCanonicalType());
                if (!AT->getSizeExpr()) {
                    // Move to checking of array type (same as in FunctionAnalyser::analyseDeclExpr())
                    Diags.Report(V->getLocation(), diag::err_typecheck_incomplete_array_needs_initializer);
                    errors++;
                }
            }
        }
    }
    for (unsigned i=0; i<ast.numArrayValues(); i++) {
        errors += checkArrayValue(ast.getArrayValue(i));
    }
    return errors;
}

unsigned FileAnalyser::resolveEnumConstants() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numTypes(); i++) {
        TypeDecl* TD = ast.getType(i);
        EnumTypeDecl* ETD = dyncast<EnumTypeDecl>(TD);
        if (ETD) {
            APSInt value(64, false);
            APInt I(64, 0, false);
            value = I;
            // check duplicate values
            typedef std::map<int64_t, EnumConstantDecl*> Values;
            typedef Values::iterator ValuesIter;
            Values values;
            for (unsigned i=0; i<ETD->numConstants(); i++) {
                EnumConstantDecl* ECD = ETD->getConstant(i);
                int errs = functionAnalyser.checkEnumValue(ECD, value);
                errors += errs;
                if (!errors) {
                    // NOTE: once there are errors, checking for duplicate values is pointless
                    APSInt newVal = ECD->getValue();
                    // check for duplicates
                    int64_t v = newVal.getSExtValue();
                    ValuesIter iter = values.find(v);
                    if (iter == values.end()) {
                        values[v] = ECD;
                    } else {
                        Diags.Report(ECD->getLocation(), diag::err_duplicate_enum_value);
                        EnumConstantDecl* Old = iter->second;
                        Diags.Report(Old->getLocation(), diag::note_duplicate_element) << Old->getName() << newVal.toString(10);
                    }
                }
            }
        }
    }
    return errors;
}

unsigned FileAnalyser::checkFunctionProtos() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);
        errors += resolveFunctionDecl(F);
        if (F->getName() == "main" && !F->isPublic()) {
            Diags.Report(F->getLocation(), diag::err_main_non_public);
            errors++;
        }
    }
    return errors;
}

unsigned FileAnalyser::checkFunctionBodies() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        errors += functionAnalyser.check(ast.getFunction(i));
    }
    return errors;
}

void FileAnalyser::checkDeclsForUsed() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());

    // checkfor unused uses
    for (unsigned i=0; i<ast.numImports(); i++) {
        ImportDecl* U = ast.getImport(i);
        if (!U->isUsed()) {
            Diags.Report(U->getLocation(), diag::warn_unused_module) << U->getModuleName();
        }
    }

    // check for unused variables
    for (unsigned i=0; i<ast.numVars(); i++) {
        VarDecl* V = ast.getVar(i);
        if (!V->isUsed()) {
            Diags.Report(V->getLocation(), diag::warn_unused_variable) << V->getName();
        } else {
            if (V->isPublic() && !V->isUsedPublic()) {
                Diags.Report(V->getLocation(), diag::warn_unused_public) << 2 << V->getName();
            }
        }
    }

    // check for unused functions
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);
        if (F->getName() == "main") continue;
        if (!F->isUsed()) {
            Diags.Report(F->getLocation(), diag::warn_unused_function) << F->getName();
        } else {
            if (F->isPublic() && !F->isUsedPublic()) {
                Diags.Report(F->getLocation(), diag::warn_unused_public) << 1 << F->getName();
            }
        }
    }

    // check for unused types
    for (unsigned i=0; i<ast.numTypes(); i++) {
        TypeDecl* T = ast.getType(i);

        // mark Enum Types as used(public) if its constants are used(public)
        if (EnumTypeDecl* ETD = dyncast<EnumTypeDecl>(T)) {
            for (unsigned i=0; i<ETD->numConstants(); i++) {
                EnumConstantDecl* C = ETD->getConstant(i);
                if (C->isUsed()) ETD->setUsed();
                if (C->isUsedPublic()) ETD->setUsedPublic();
                if (C->isUsed() && C->isUsedPublic()) break;
            }
        }

        if (!T->isUsed()) {
            Diags.Report(T->getLocation(), diag::warn_unused_type) << T->getName();
        } else {
            if (T->isPublic() && !T->isUsedPublic()) {
                Diags.Report(T->getLocation(), diag::warn_unused_public) << 0 << T->getName();
            }
            // check if members are used
            if (isa<StructTypeDecl>(T)) {
                StructTypeDecl* S = cast<StructTypeDecl>(T);
                for (unsigned j=0; j<S->numMembers(); j++) {
                    Decl* M = S->getMember(j);
                    if (!M->isUsed()) {
                        Diags.Report(M->getLocation(), diag::warn_unused_struct_member) << M->getName();
                    }
                }
            }
        }
    }
}

unsigned FileAnalyser::checkTypeDecl(TypeDecl* D) {
    LOG_FUNC
    // check generic type
    unsigned errors = TR->checkType(D->getType(), D->isPublic());

    // check extra stuff depending on subclass
    switch (D->getKind()) {
    case DECL_FUNC:
    case DECL_VAR:
    case DECL_ENUMVALUE:
        assert(0);
        break;
    case DECL_ALIASTYPE:
        // Any UnresolvedType should point to decl that has type set
        if (errors == 0) {
            AliasType* A = cast<AliasType>(D->getType().getTypePtr());
            QualType Q = TR->resolveUnresolved(A->getRefType());
            A->updateRefType(Q);
        }
        break;
    case DECL_STRUCTTYPE:
        // dont check struct members yet
        break;
    case DECL_ENUMTYPE:
    {
        EnumTypeDecl* E = cast<EnumTypeDecl>(D);
        if (E->numConstants() == 0) {
            Diags.Report(D->getLocation(), diag::error_empty_enum) << D->getName();
        }
        break;
    }
    case DECL_FUNCTIONTYPE:
    {
        const FunctionTypeDecl* FTD = cast<FunctionTypeDecl>(D);
        // set module on inner FunctionDecl
        FTD->getDecl()->setModule(FTD->getModule());
        // dont check return/argument types yet
        break;
    }
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
        assert(0);
        break;
    }
    return errors;
}

unsigned FileAnalyser::checkStructTypeDecl(StructTypeDecl* D) {
    LOG_FUNC
    unsigned errors = 0;
    for (unsigned i=0; i<D->numMembers(); i++) {
        Decl* M = D->getMember(i);
        if (isa<VarDecl>(M)) {
            VarDecl* V = cast<VarDecl>(M);
            assert(V->getInitValue() == 0);
            errors += resolveVarDecl(V);
        }
        if (isa<StructTypeDecl>(M)) {
            errors += checkStructTypeDecl(cast<StructTypeDecl>(M));
        }
    }
    return errors;
}

unsigned FileAnalyser::resolveVarDecl(VarDecl* D) {
    LOG_FUNC
    // TODO duplicate code with FileAnalyser::analyseDeclExpr()
    QualType Q = TR->resolveType(D->getType(), D->isPublic());
    if (Q.isValid()) {
        D->setType(Q);

        // TODO move to after checkVarInits() (to allow constants in array size)
        if (Q.isArrayType()) {
            functionAnalyser.checkArraySizeExpr(D);
        }

        // NOTE: dont check initValue here (doesn't have canonical type yet)
        return 0;
    }
    return 1;
}

unsigned FileAnalyser::resolveFunctionDecl(FunctionDecl* D) {
    LOG_FUNC
    unsigned errors = 0;
    // return type
    QualType Q = TR->resolveType(D->getReturnType(), D->isPublic());
    if (Q.isValid()) D->updateReturnType(Q);
    else errors++;

    // args
    for (unsigned i=0; i<D->numArgs(); i++) {
        VarDecl* Arg = D->getArg(i);
        unsigned errs = resolveVarDecl(Arg);
        errors += errs;
        if (!errs && Arg->getInitValue()) {
            errors += functionAnalyser.checkVarInit(Arg);
        }
    }
    return errors;
}

unsigned FileAnalyser::checkArrayValue(ArrayValueDecl* D) {
    LOG_FUNC
#if 0
    ScopeResult Result = globals->checkScopedSymbol(D->getName(), D->getLocation(), IDENTIFIER);
    if (!Result.ok) return 1;
    assert(Result.decl);
    VarDecl* V = dyncast<VarDecl>(Result.decl);
    if (!V) {
        fprintf(stderr, "TODO Error: 'x' is not a variable\n");
        return 1;
    }

    QualType Q = V->getType();
    if (!Q->isArrayType()) {
        fprintf(stderr, "TODO Error: 'x' is not an array type\n");
        return 1;
    }

    return checkInitValue(D->getExpr(), Q);
#endif
    return 0;
}

