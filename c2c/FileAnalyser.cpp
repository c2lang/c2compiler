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

#include "FileAnalyser.h"
#include "Decl.h"
#include "Expr.h"
#include "Scope.h"
#include "color.h"
#include "AST.h"

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
    , globals(new FileScope(ast_.getPkgName(), pkgs, Diags_))
    , Diags(Diags_)
    , functionAnalyser(*globals, typeContext_, Diags_)
    , verbose(verbose_)
{}

FileAnalyser::~FileAnalyser() {
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
        resolveCanonical(D->getType(), true);

        // NOTE dont check any subclass specific things yet

        // check extra stuff depending on subclass
        switch (D->getKind()) {
        case DECL_FUNC:
        case DECL_VAR:
        case DECL_ENUMVALUE:
            assert(0);
            break;
        case DECL_TYPE:
            // nothing to do
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
        if (V->getInitValue()) {
            errors += checkInitValue(V->getInitValue(), V->getType());
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

unsigned FileAnalyser::checkTypeDecl(TypeDecl* D) {
    LOG_FUNC
    // check generic type
    unsigned errors = 0;
    errors += checkType(D->getType(), D->isPublic());

    // check extra stuff depending on subclass
    switch (D->getKind()) {
    case DECL_FUNC:
    case DECL_VAR:
    case DECL_ENUMVALUE:
        assert(0);
        break;
    case DECL_TYPE:
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

unsigned FileAnalyser::checkType(QualType Q, bool used_public) {
    LOG_FUNC
    const Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        // ok
        return 0;
    case TC_POINTER:
        return checkType(cast<PointerType>(T)->getPointeeType(), used_public);
    case TC_ARRAY:
        return checkType(cast<ArrayType>(T)->getElementType(), used_public);
    case TC_UNRESOLVED:
        return globals->resolveType(cast<UnresolvedType>(T), used_public);
    case TC_ALIAS:
        // will be removed
        return 0;
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        // ok (TypeDecl will be checked)
        return 0;
    }
}

QualType FileAnalyser::resolveCanonical(QualType Q, bool set) {
    LOG_FUNC
    const Type* T = Q.getTypePtr();
    if (T->hasCanonicalType()) return T->getCanonicalType();

    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return T->getCanonicalType();
    case TC_POINTER:
        {
            const PointerType* P = cast<PointerType>(T);
            QualType t1 = P->getPointeeType();
            // Pointee will always be in same TypeContext (file), since it's either built-in or UnresolvedType
            QualType t2 = resolveCanonical(t1, true);
            assert(t2.isValid());
            QualType canonical;
            // create new PointerType if PointeeType has different canonical than itself
            if (t1 == t2) canonical = t2;
            else canonical = typeContext.getPointerType(t2);

            if (set) P->setCanonicalType(canonical);
            return canonical;
        }
    case TC_ARRAY:
        {
            const ArrayType* A = cast<ArrayType>(T);
            QualType t1 = A->getElementType();
            QualType t2 = resolveCanonical(t1, true);
            assert(t2.isValid());
            QualType canonical;
            if (t1 == t2) canonical = t2;
            // NOTE need size Expr, but set ownership to none?
            else canonical = typeContext.getArrayType(t2, A->getSize(), false);
            if (set) A->setCanonicalType(canonical);
            return canonical;
        }
    case TC_UNRESOLVED:
        {
            const UnresolvedType* U = cast<UnresolvedType>(T);
            const TypeDecl* TD = U->getMatch();
            assert(TD);
            QualType canonical = resolveCanonical(TD->getType(), false);
            if (set) U->setCanonicalType(canonical);
            return canonical;
        }
    case TC_ALIAS:
        // will be removed
        return 0;
    case TC_STRUCT:
        return T->getCanonicalType();
    case TC_ENUM:
        {
            assert(0 && "TODO");
            return 0;
        }
    case TC_FUNCTION:
        return T->getCanonicalType();
    }
}

unsigned FileAnalyser::resolveVarDecl(VarDecl* D) {
    LOG_FUNC
    QualType Q = D->getType();
    if (Q->hasCanonicalType()) return 0;

    unsigned errors = checkType(Q, D->isPublic());
    if (!errors) {
        resolveCanonical(Q, true);
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
        unsigned errs = checkType(RT, D->isPublic());
        errors += errs;
        if (!errs) resolveCanonical(RT, true);
    }

    // args
    for (unsigned i=0; i<D->numArgs(); i++) {
        VarDecl* Arg = D->getArg(i);
        unsigned errs = resolveVarDecl(Arg);
        errors += errs;
        if (!errs && Arg->getInitValue()) {
            errors += checkInitValue(Arg->getInitValue(), Arg->getType());
        }
    }
    return errors;
}

unsigned FileAnalyser::checkArrayValue(ArrayValueDecl* D) {
    LOG_FUNC
#warning "TODO"
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

unsigned FileAnalyser::checkInitValue(Expr* expr, QualType expected) {
    LOG_FUNC
    // NOTE: expr must be compile-time constant
    // check return type from expressions? (pass expected along is not handy)
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_FLOAT_LITERAL:
        // TODO
        break;
    case EXPR_CALL:
        assert(0);
        break;
    case EXPR_IDENTIFIER:
        // TODO
        break;
    case EXPR_INITLIST:
        // TODO
        break;
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


#if 0
(FROM GlobalVarAnalyser)
    if (qt->isEnumType()) {
        const EnumType* ET = cast<EnumType>(qt);
        // TEMP use unsigned only
        unsigned lastValue = 0;
        for (unsigned i=0; i<ET->numConstants(); i++) {
            EnumConstantDecl* C = ET->getConstant(i);
            if (C->getInitValue()) {
                //C->setValue(lastValue);
                //lastValue = val;
                // TEMP just ignore
                C->setValue(lastValue);
                lastValue++;
            } else {
                C->setValue(lastValue);
                lastValue++;
            }
            // TODO check for duplicates
        }
    }
#endif

#if 0
void FileAnalyser::handle(Decl* decl) {
    bool is_public = decl->isPublic();
    switch (decl->getKind()) {
    case DECL_FUNC:
        {
            FunctionDecl* func = cast<FunctionDecl>(decl);
            // check return type
            checkType(func->getReturnType(), is_public);
            // check argument types
            for (unsigned i=0; i<func->numArgs(); i++) {
                VarDecl* V = func->getArg(i);
                checkType(V->getType(), is_public);
            }
            // TEMP (do elsewhere?)
            if (!is_public && func->getName() == "main") {
                Diags.Report(decl->getLocation(), diag::err_main_non_public);
            }
        }
        break;
    case DECL_ARRAYVALUE:
        {
            ArrayValueDecl* A = cast<ArrayValueDecl>(decl);
            ScopeResult SR = globals.findSymbol(A->getName());
            if (!SR.ok) break;
            if (!SR.decl) {
                Diags.Report(A->getLocation(), diag::err_undeclared_var_use)
                << A->getName();
                break;
            }
            if (SR.external) {
                // TODO proper error
                fprintf(stderr, "Incremental Array Value for %s cannot be for external symbol\n", A->getName().c_str());
                break;
            }
            if (!isa<VarDecl>(SR.decl)) {
                fprintf(stderr, "TODO symbol '%s' is not a variable\n", A->getName().c_str());
                break;
            }
            VarDecl* VD = cast<VarDecl>(SR.decl);
            QualType T = VD->getType();
            if (!T.isArrayType()) {
                Diags.Report(A->getLocation(), diag::err_typecheck_subscript);
                break;
            }
            // TODO add to VarDecl
            VD->addInitValue(A);
        }
        break;
    }
}

#endif
