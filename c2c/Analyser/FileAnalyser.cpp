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
#include <stdio.h>

#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/FileAnalyser.h"
#include "Analyser/Scope.h"
#include "Analyser/TypeChecker.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/AST.h"
#include "Utils/color.h"

using namespace C2;
using namespace clang;

//#define ANALYSER_DEBUG

#ifdef ANALYSER_DEBUG
#include <iostream>
#define LOG_FUNC std::cerr << ANSI_BLUE << __func__ << "()" << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#endif

FileAnalyser::FileAnalyser(const Pkgs& pkgs, clang::DiagnosticsEngine& Diags_,
                    AST& ast_, TypeContext& typeContext_, bool verbose_)
    : ast(ast_)
    , typeContext(typeContext_)
    , globals(new Scope(ast_.getPkgName(), pkgs, Diags_))
    , typeResolver(new TypeChecker(*globals, Diags_, typeContext_))
    , Diags(Diags_)
    , functionAnalyser(*globals, *typeResolver, typeContext_, Diags_)
    , verbose(verbose_)
{}

FileAnalyser::~FileAnalyser() {
    delete typeResolver;
    delete globals;
}

unsigned  FileAnalyser::checkUses() {
    LOG_FUNC
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numUses(); i++) {
        UseDecl* useDecl = ast.getUse(i);
        std::string pkgName = useDecl->getName();

        // check if package exists
        const Package* pkg = globals->findAnyPackage(pkgName);
        if (pkg == 0) {
            Diags.Report(useDecl->getLocation(), diag::err_unknown_package) << pkgName;
            errors++;
            continue;
        }

        // check if aliasname is not a package
        const std::string& aliasName = useDecl->getAlias();
        if (aliasName != "") {
            const Package* pkg2 = globals->findAnyPackage(aliasName);
            if (pkg2) {
                Diags.Report(useDecl->getAliasLocation(), diag::err_alias_is_package) << aliasName;
                errors++;
                continue;
            }
            pkgName = aliasName;
        }

        // add to Scope
        globals->addPackage(useDecl->isLocal(), pkgName, pkg);
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
        typeResolver->resolveCanonicals(D, D->getType(), true);

        // NOTE dont check any subclass specific things yet

        // check extra stuff depending on subclass
        switch (D->getKind()) {
        case DECL_FUNC:
        case DECL_VAR:
        case DECL_ENUMVALUE:
            assert(0);
            break;
        case DECL_ALIASTYPE:
            // nothing extra to do
            break;
        case DECL_STRUCTTYPE:
            //resolveStructType
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
        case DECL_USE:
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
        Expr* initVal = V->getInitValue();
        QualType T = V->getType();
        if (initVal) {
            errors += checkInitValue(V, initVal, T);
        } else {
            if (T.isConstQualified()) {
                Diags.Report(V->getLocation(), diag::err_uninitialized_const_var) << V->getName();
                errors++;
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
    return 0;
}

unsigned FileAnalyser::checkFunctionProtos() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE"%s %s"ANSI_NORMAL"\n", __func__, ast.getFileName().c_str());
    unsigned errors = 0;
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        errors += resolveFunctionDecl(ast.getFunction(i));
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

    // NOTE: only check VarDecls for now (not funcs/types)

    for (unsigned i=0; i<ast.numVars(); i++) {
        VarDecl* V = ast.getVar(i);
        if (!V->isUsed()) {
            Diags.Report(V->getLocation(), diag::warn_unused_variable) << V->getName();
        }
    }
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);
        if (F->getName() == "main") continue;
        if (!F->isUsed()) {
            Diags.Report(F->getLocation(), diag::warn_unused_function) << F->getName();
        }
    }
    for (unsigned i=0; i<ast.numTypes(); i++) {
        TypeDecl* T = ast.getType(i);
        if (!T->isUsed()) {
            Diags.Report(T->getLocation(), diag::warn_unused_type) << T->getName();
        } else {
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
    unsigned errors = 0;
    errors += typeResolver->checkType(D->getType(), D->isPublic());

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
        // dont check struct members yet
        break;
    case DECL_ENUMTYPE:
        // nothing to do
        break;
    case DECL_FUNCTIONTYPE:
        // dont check return/argument types yet
        break;
    case DECL_ARRAYVALUE:
    case DECL_USE:
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
    QualType Q = D->getType();
    if (Q->hasCanonicalType()) return 0;

    unsigned errors = typeResolver->checkType(Q, D->isPublic());
    if (!errors) {
        typeResolver->resolveCanonicals(D, Q, true);
        // TODO same as FunctionAnalyser code!
        ArrayType* AT = dyncast<ArrayType>(Q.getTypePtr());
        if (AT && AT->getSize()) {
            // TODO need analyseExpr(AT->getSize(), RHS);
            // TODO check type of size expr
        }

        // NOTE: dont check initValue here (doesn't have canonical type yet)
    }
    return errors;
}

unsigned FileAnalyser::resolveFunctionDecl(FunctionDecl* D) {
    LOG_FUNC
    unsigned errors = 0;
    // return type
    QualType RT = D->getReturnType();
    if (!RT->hasCanonicalType()) {
        unsigned errs = typeResolver->checkType(RT, D->isPublic());
        errors += errs;
        if (!errs) typeResolver->resolveCanonicals(D, RT, true);
    }

    // args
    for (unsigned i=0; i<D->numArgs(); i++) {
        VarDecl* Arg = D->getArg(i);
        unsigned errs = resolveVarDecl(Arg);
        errors += errs;
        if (!errs && Arg->getInitValue()) {
            errors += checkInitValue(Arg, Arg->getInitValue(), Arg->getType());
        }
    }
    return errors;
}

unsigned FileAnalyser::checkArrayValue(ArrayValueDecl* D) {
    LOG_FUNC
#if 0
    ScopeResult Result = globals->checkSymbol(D->getName(), D->getLocation(), IDENTIFIER);
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

unsigned FileAnalyser::checkInitValue(VarDecl* decl, Expr* expr, QualType expected) {
    LOG_FUNC
    // NOTE: expr must be compile-time constant
    // check return type from expressions? (pass expected along is not handy)
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        // TODO
        break;
    case EXPR_CALL:
        assert(0);
        break;
    case EXPR_IDENTIFIER:
        {
            IdentifierExpr* I = cast<IdentifierExpr>(expr);
            ScopeResult Res = globals->findSymbol(I->getName(), I->getLocation());
            if (!Res.ok) return 1;
            if (!Res.decl) {
                Diags.Report(I->getLocation(), diag::err_undeclared_var_use) << I->getName();
                return 1;
            }
            if (Res.decl == decl) {
                Diags.Report(I->getLocation(), diag::err_var_self_init) << Res.decl->getName();
                return 1;
            }
            I->setDecl(Res.decl);
            // TODO check types (need code from FunctionAnalyser)
            break;
        }
    case EXPR_INITLIST:
        return checkInitList(decl, cast<InitListExpr>(expr), expected);
    case EXPR_TYPE:
    case EXPR_DECL:
        assert(0);
        break;
    case EXPR_BINOP:
    case EXPR_CONDOP:
    case EXPR_UNARYOP:
    case EXPR_BUILTIN:
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
    case EXPR_PAREN:
        // TODO
        break;
    }
    return 0;
}


unsigned FileAnalyser::checkInitList(VarDecl* decl, InitListExpr* initVal, QualType expected) {
    QualType Q = decl->getType();
    ExprList& values = initVal->getValues();
    unsigned errors = 0;
    if (Q.isArrayType()) {
        // TODO use helper function
        ArrayType* AT = cast<ArrayType>(Q->getCanonicalType().getTypePtr());
        QualType ET = AT->getElementType();
        // TODO check if size is specifier in type
        for (unsigned i=0; i<values.size(); i++) {
            errors += checkInitValue(decl, values[i], ET);
        }
    } else if (Q.isStructType()) {
        // TODO use helper function
        StructType* TT = cast<StructType>(Q->getCanonicalType().getTypePtr());
        StructTypeDecl* STD = TT->getDecl();
        assert(STD->isStruct() && "TEMP only support structs for now");
        for (unsigned i=0; i<values.size(); i++) {
            if (i >= STD->numMembers()) {
                // note: 0 for array, 3 for union, 4 for structs
                Diags.Report(values[STD->numMembers()]->getLocation(), diag::err_excess_initializers)
                    << 4;
                errors++;
                return errors;
            }
            // NOTE: doesn't fit for sub-struct members! (need Decl in interface)
            VarDecl* VD = dyncast<VarDecl>(STD->getMember(i));
            assert(VD && "TEMP don't support sub-struct member inits");
            errors += checkInitValue(VD, values[i], VD->getType());
        }
    } else {
        // TODO error
    }
    return errors;
}

