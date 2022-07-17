/* Copyright 2013-2022 Bas van den Berg
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
#include "Clang/ParseDiagnostic.h"
#include "Clang/SemaDiagnostic.h"
#include "Clang/TokenKinds.h"

#include "Analyser/FileAnalyser.h"
#include "Analyser/AnalyserUtils.h"
#include "Analyser/AnalyserConstants.h"
#include "Analyser/FunctionAnalyser.h"
#include "Analyser/CTVAnalyser.h"
#include "AST/Decl.h"
#include "AST/AST.h"
#include "Utils/color.h"
#include "Utils/StringBuilder.h"
#include <ctype.h>

using namespace C2;
using namespace c2lang;
using namespace llvm;

//#define ANALYSER_DEBUG

#ifdef ANALYSER_DEBUG
#include <iostream>
#define LOG_FUNC std::cerr << ANSI_BLUE << __func__ << "()" << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#endif

FileAnalyser::FileAnalyser(Module& module_,
                           const Modules& allModules,
                           DiagnosticsEngine& Diags_,
                           const TargetInfo& target_,
                           AST& ast_,
                           bool verbose_)
    : ast(ast_)
    , module(module_)
    , scope(new Scope(ast.getModuleName(), allModules, Diags_))
    , TR(new TypeResolver(*scope, Diags_))
    , Context(ast.getContext())
    , Diags(Diags_)
    , target(target_)
    , EA(Diags_, target_, Context)
    , checkIndex(0)
    , verbose(verbose_)
{
    // add Imports
    for (unsigned i=0; i<ast.numImports(); i++) {
        scope->addImportDecl(ast.getImport(i));
    }
}

bool FileAnalyser::collectIncrementals(IncrementalArrayVals& values, IncrementalEnums& enums) {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());
    for (unsigned i=0; i<ast.numArrayValues(); i++) {
        if (!collectIncremental(ast.getArrayValue(i), values, enums)) return false;
    }
    return true;
}

bool FileAnalyser::collectIncremental(ArrayValueDecl* D, IncrementalArrayVals& values, IncrementalEnums& enums) {
    LOG_FUNC
    Expr* incr = D->getExpr();
    // find decl
    Decl* found = scope->findSymbolInModule(D->getName(), D->getLocation(), D->getModule());
    if (!found) return false;

    EnumTypeDecl* ETD = dyncast<EnumTypeDecl>(found);
    if (ETD) {
        IdentifierExpr* IE = dyncast<IdentifierExpr>(incr);
        if (!IE) {
            Diags.Report(incr->getLocation(), diag::err_expected_after) << "incremental enum" << tok::identifier;
            return false;
        }
        EnumConstantDecl* ECD = new (Context) EnumConstantDecl(IE->getName(), IE->getLocation(), ETD->getType(), 0, ETD->isPublic(), &module);
        if (ETD->isPublic() && module.isExported()) ECD->setExported();
        enums[ETD].push_back(ECD);
        return true;
    }

    VarDecl* VD = dyncast<VarDecl>(found);
    if (VD) {
        QualType QT = VD->getType();
        if (!isa<ArrayType>(QT)) {
            Diags.Report(D->getLocation(), diag::err_not_incremental_array) << D->getName();
            return false;
        }
        const ArrayType* AT = cast<ArrayType>(QT);
        if (!AT->isIncremental()) {
            Diags.Report(D->getLocation(), diag::err_not_incremental_array) << D->getName();
            return false;
        }

        if (VD->getCheckState() == CHECK_UNCHECKED) {
            Expr* init = VD->getInitValue();
            if (init) {
                Diags.Report(VD->getInitValue()->getLocation(),  diag::err_incremental_array_initlist);
                return false;
            }
            InitListExpr* ILE = new (Context) InitListExpr(VD->getLocation(), VD->getLocation());
            VD->setInitValue(ILE);
            VD->setCheckState(CHECK_IN_PROGRESS);
        }

        values[VD].push_back(incr);
        return true;
    }

    Diags.Report(D->getLocation(), diag::err_not_incremental_array) << D->getName();
    return false;
}

bool FileAnalyser::collectStructFunctions(StructFunctionList& structFuncs) {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    bool ok = true;
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);
        if (F->isStructFunction()) {
            ok |= collectStructFunction(F, structFuncs);
        }
    }
    return ok;
}

bool FileAnalyser::analyseTypes() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    for (unsigned i=0; i<ast.numTypes(); i++) {
        TypeDecl* T = ast.getType(i);
        if (!analyseDecl(T)) return false;
    }
    return true;
}

bool FileAnalyser::analyseVars() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    for (unsigned i=0; i<ast.numVars(); i++) {
        VarDecl* V = ast.getVar(i);
        if (!analyseDecl(V)) return false;
    }
    return true;
}

bool FileAnalyser::analyseStaticAsserts() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    for (unsigned i=0; i<ast.numStaticAsserts(); i++) {
        StaticAssertDecl* D = ast.getStaticAssert(i);
        if (!analyseStaticAssert(D)) return false;
    }
    return true;
}

bool FileAnalyser::analyseFunctionProtos() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);

        if (!analyseDecl(F)) return false;

        bool ok = true;
        // TODO move somewhere else
        if (strcmp(F->getName(), "main") == 0) {
            if (!F->isPublic()) {
                Diags.Report(F->getLocation(), diag::err_main_non_public);
                ok = false;
            }
            if (F->getReturnType() != Type::Int32()) {
                Diags.Report(F->getLocation(), diag::err_main_returns_nonint32);
                ok = false;
            }
            //if (!F->getReturnType().isBuiltinType() || cast<BuiltinType>(F->getReturnType()).getKind() == BuiltinType::Int32) {
            // }
        }
        if (!ok) return false;
    }
    return true;
}

void FileAnalyser::analyseFunctionBodies() {
    LOG_FUNC
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    FunctionAnalyser functionAnalyser(*scope, *TR, Context, Diags, target, ast.isInterface());
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        functionAnalyser.check(ast.getFunction(i));
    }
}

bool FileAnalyser::isTop(Decl* d) {
    if (checkIndex == 0) return false;
    if (checkStack[checkIndex-1] == d) return true;
    return false;
}

bool FileAnalyser::pushCheck(Decl* d) {
    //printf("push [%d] %s\n", checkIndex, d->getName());
    assert(checkIndex != sizeof(checkStack) / sizeof(checkStack[0]));
    for (unsigned i=0; i<checkIndex; i++) {
        if (checkStack[i] == d) {
            for (unsigned j=i; j<checkIndex; j++) {
                Diag(checkStack[j]->getLocation(), diag::err_circular_decl);
            }
            return false;
        }
    }
    checkStack[checkIndex] = d;
    checkIndex++;
    d->setCheckState(CHECK_IN_PROGRESS);
    return true;
}

void FileAnalyser::popCheck() {
    assert(checkIndex != 0);
    checkIndex--;
    //printf("pop  [%d] %s\n", checkIndex, checkStack[checkIndex]->getName());
}

bool FileAnalyser::analyseStaticAssertInteger(const Expr* lhs, const Expr* rhs) {
    LOG_FUNC

    CTVAnalyser LA(Diags);
    llvm::APSInt L = LA.checkLiterals(lhs);
    llvm::APSInt R = LA.checkLiterals(rhs);
    if (L != R) {
        Diag(rhs->getLocation(), diag::err_static_assert_not_same) << L.toString(10) << R.toString(10);
        return false;
    }
    return true;
}

#if 0
bool FileAnalyser::analyseStaticAssertString(const Expr* lhs, const Expr* rhs) {
    LOG_FUNC

    // must be stringLiteral
    const StringLiteral* l = dyncast<StringLiteral>(lhs);
    const StringLiteral* r = dyncast<StringLiteral>(rhs);
    if (!l || !r) {
        // TODO BB
        fprintf(stderr, "Both arguments must be string literals\n");
        return false;
    }

    if (strcmp(l->getValue(), r->getValue()) != 0) {
        Diag(rhs->getLocation(), diag::err_static_assert_not_same_str) << l->getValue() << r->getValue();
        return false;
    }
    return true;
}
#endif

bool FileAnalyser::analyseStaticAssert(StaticAssertDecl* D) {
    LOG_FUNC

    // TODO or use side=0 (not marked with used then)
    QualType Q = analyseExpr(D->getLHS2(), false, false);
    if (!Q.isValid()) return false;

    Expr* lhs = D->getLHS();
    if (!lhs->isCTV()) {
        Diag(lhs->getLocation(), diag::err_expr_not_ice) << 0 << lhs->getSourceRange();
        return false;
    }

    Q = analyseExpr(D->getRHS2(), false, false);
    if (!Q.isValid()) return false;
    Expr* rhs = D->getRHS();
    if (!rhs->isCTV()) {
        Diag(rhs->getLocation(), diag::err_expr_not_ice) << 0 << rhs->getSourceRange();
        return false;
    }

    QualType lhsType = lhs->getType();
    switch (lhsType.getTypePtr()->getTypeClass()) {
    case TC_BUILTIN:
        return analyseStaticAssertInteger(lhs, rhs);
    case TC_POINTER:
        //return analyseStaticAssertString(lhs, rhs);
    case TC_ARRAY:
    case TC_REF:
    case TC_ALIAS:
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
    case TC_MODULE:
        TODO;
        break;
    }
    return false;
}

bool FileAnalyser::analyseEnumConstants(EnumTypeDecl* ETD) {
    LOG_FUNC
    APSInt value(64, false);
    APInt I(64, 0, false);
    value = I;
    // check duplicate values
    // TODO dont use map, just vector and walk through
    typedef std::map<int64_t, EnumConstantDecl*> Values;
    typedef Values::iterator ValuesIter;
    Values values;
    CTVAnalyser LA(Diags);
    for (unsigned c=0; c<ETD->numConstants(); c++) {
        EnumConstantDecl* ECD = ETD->getConstant(c);
        Expr* init = ECD->getInitValue();
        if (init) {
            ECD->setCheckState(CHECK_IN_PROGRESS);
            QualType Q = analyseExpr(ECD->getInitValue2(), ETD->isPublic(), true);
            if (!Q.isValid()) return false;
            init = ECD->getInitValue(); // re-read in case of ImplicitCast

            if (!init->isCTV()) {
                Diag(init->getLocation(), diag::err_expr_not_ice) << 0 << init->getSourceRange();
                return false;
            }
            value = LA.checkLiterals(init);
        }
        //printf("ENUM [%s] -> %ld\n", ECD->getName(), value.getSExtValue());
        ECD->setValue(value);
        ++value;
        if (!LA.checkRange(ECD->getType(), init, ECD->getLocation(), ECD->getValue())) return false;
        ECD->setCheckState(CHECK_DONE);

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

    return true;
}

bool FileAnalyser::analyseDecl(Decl* D) {
    LOG_FUNC
#ifdef ANALYSER_DEBUG
    printf("DECL %s\n", D->getName());
#endif
    if (D->isChecked()) return true;
    if (isTop(D) && (isa<EnumTypeDecl>(D) || isa<StructTypeDecl>(D))) {
        // Only allow for enums? or also push Enum Constants? (for self-ref check?)
        // ALSO PUSH ENUM CONSTANTS + set checked
        // happens if enum constants references other constant in same enum: B = State.A + 1,
        //printf("IS TOP!\n");
        return true;
    }
    if (!pushCheck(D)) return false;

    bool ok = false;
    switch (D->getKind()) {
    case DECL_FUNC:
        ok = analyseFunctionDecl(cast<FunctionDecl>(D));
        break;
    case DECL_VAR: {
        VarDecl* VD = cast<VarDecl>(D);
        ok = analyseVarDecl(VD);
        if (VD->isGlobal()) {
            assert(D->getModule());
            D->getModule()->addSortedVar(cast<VarDecl>(D));
        }
        break;
    }
    case DECL_ENUMVALUE:
        FATAL_ERROR("should not come here?");
        break;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        ok = analyseTypeDecl(cast<TypeDecl>(D));
        break;
    case DECL_ARRAYVALUE:
        FATAL_ERROR("should not come here");
        break;
    case DECL_IMPORT:
        ok = true;
        break;
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        D->dump();
        FATAL_ERROR("should not come here");
        break;
    }
    if (ok) D->setCheckState(CHECK_DONE);
    popCheck();
    return ok;
}

bool FileAnalyser::analyseVarDecl(VarDecl* V) {
    LOG_FUNC

    // TYPE
    QualType Q = analyseType(V->getType(), V->getLocation(), V->isPublic(), true);
    if (!Q.isValid()) return false;

    if (Q.isIncompleteType()) {
        StringBuilder name;
        Q.DiagName(name);
        uint32_t msg = diag::err_typecheck_decl_incomplete_type;
        if (V->isField()) msg = diag::err_field_incomplete;
        if (V->isParameter()) msg = diag::err_argument_incomplete;
        //if (Q.isVoidType()) msg = diag::err_bla;
        Diags.Report(V->getLocation(), msg) << name;
        return false;
    }

    V->setType(Q);
    // TODO should be inside analyseType (needs inner/outer function)
    TR->checkOpaqueType(V->getLocation(), V->isPublic(), Q);

    // ATTR
    if (!checkVarDeclAttributes(V)) return false;

    // INIT
    Expr** init = V->getInitValue2();
    if (init) {
#if 0
        // TODO FIX on collecting entries, a ILE is created! so this gives an error
        const ArrayType* AT = dyncast<ArrayType>(Q);
        if (AT && AT->isIncremental()) {
            Diags.Report(init->getLocation(),  diag::err_incremental_array_initlist);
            return false;
        }
#endif

        // if VarDecl is not an array and not const, the init is not public
        bool usedPublicly = (V->isPublic() && (!V->isGlobal() || V->getType().isConstQualified()));
        if (!analyseInitExpr(init, Q, usedPublicly)) return false;
/*
        // TODO move this inside analyseInitExpr
        QualType RT = analyseExpr(init);
        if (!RT.isValid()) return 1;
        CTVAnalyser LA(Diags);
        APSInt result = LA.checkLiterals(init);

        printf("INIT VALUE of %s is %ld\n", V->getName(), result.getSExtValue());
*/
    } else {
        QualType T = V->getType();
        if (T.isConstQualified() && V->isGlobal()) {
            Diags.Report(V->getLocation(), diag::err_uninitialized_const_var) << V->getName();
            return false;
        }

        if (T->isArrayType()) {
            const ArrayType* AT = cast<ArrayType>(T.getCanonicalType());

            // NOTE: only struct members may have array size 0
            const Expr* sizeExpr = AT->getSizeExpr();
            if (sizeExpr && AT->getSize() == 0 && !V->isField()) {
                // TODO: range not displayed correctly
                Diag(sizeExpr->getLocation(), diag::err_array_size_zero); // << sizeExpr->getSourceRange();
                return false;
            }

            switch (V->getVarKind()) {
            case VARDECL_GLOBAL:
                if (!AT->isIncremental() && !sizeExpr) {
                    Diags.Report(V->getLocation(), diag::err_typecheck_incomplete_array_needs_initializer);
                    return false;
                }
                break;
            case VARDECL_LOCAL:
                FATAL_ERROR("should not come here");
                break;
            case VARDECL_PARAM:
                Diags.Report(V->getLocation(), diag::err_param_array);
                return false;
            case VARDECL_MEMBER:
                if (!AT->getSizeExpr()) {
                    Diags.Report(V->getLocation(), diag::err_array_struct_member_needs_size);
                    return false;
                }
                break;
            }
        }
    }

    if (!ast.isInterface() && !V->hasEmptyName()) {
        char first = V->getName()[0];
        switch (V->getVarKind()) {
        case VARDECL_GLOBAL:
            if (Q.isConstant()) {
                if (!isupper(first)) {
                    Diags.Report(V->getLocation(), diag::err_lower_casing) << 0;
                    return false;
                }
            } else {
                if (!islower(first)) {
                    Diags.Report(V->getLocation(), diag::err_upper_casing) << 1;
                    return false;
                }
            }
            break;
        case VARDECL_LOCAL:
            FATAL_ERROR("should not come here");
            break;
        case VARDECL_PARAM:
            if (!islower(first)) Diags.Report(V->getLocation(), diag::err_upper_casing) << 0;
            break;
        case VARDECL_MEMBER:
            if (!islower(first)) Diags.Report(V->getLocation(), diag::err_upper_casing) << 4;
            break;
        }
    }
    return true;
}

Decl* FileAnalyser::analyseIdentifier(Expr** expr_ptr, bool usedPublic) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    IdentifierExpr* id = cast<IdentifierExpr>(expr);
    Decl* D = scope->findSymbol(id->getName(), id->getLocation(), false, usedPublic);
    if (D) {
        id->setDecl(D, AnalyserUtils::globalDecl2RefKind(D));
        id->setType(D->getType());
        if (!D->isChecked()) {
            if (!isTop(D)) D->setUsed();
            if (!analyseDecl(D)) return 0;
        }
        AnalyserUtils::SetConstantFlags(D, id);

        if (usedPublic && !D->isPublic()) {
            Diag(id->getLocation(), diag::err_non_public_constant) << D->DiagName();
        }

        if (D->getKind() == DECL_FUNC) {
            expr->setCTC();
            expr->setIsRValue();
            insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, D->getType());
        }

        if (id->isCTV()) id->setIsRValue();
    }
    return D;
}

QualType FileAnalyser::analyseExpr(Expr** expr_ptr, bool usedPublic, bool need_rvalue) {
    LOG_FUNC
    QualType Result = analyseExprInner(expr_ptr, usedPublic);
    if (!Result.isValid()) return Result;

    Expr* expr = *expr_ptr;
    expr->setType(Result);

    if (need_rvalue && expr->isLValue()) {
        // TODO just Result?
        QualType Q = expr->getType();
        if (Q.isArrayType()) {
            QualType Qptr = AnalyserUtils::getPointerFromArray(Context, Q);
            insertImplicitCast(CK_ArrayToPointerDecay, expr_ptr, Qptr);
            return Qptr;
        } else {
            insertImplicitCast(CK_LValueToRValue, expr_ptr, Q);
        }
    }

    return Result;
}

QualType FileAnalyser::analyseExprInner(Expr** expr_ptr, bool usedPublic) {
    LOG_FUNC
    Expr* expr = *expr_ptr;

    QualType Q;

    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
        return EA.analyseIntegerLiteral(expr);
    case EXPR_FLOAT_LITERAL:
        // For now always return type float
        return Type::Float32();
    case EXPR_BOOL_LITERAL:
        return Type::Bool();
    case EXPR_CHAR_LITERAL:
        return Type::Int8();
    case EXPR_STRING_LITERAL: {
        StringLiteral* s = cast<StringLiteral>(expr);
        int len = s->getByteLength();
        Q = Context.getArrayType(Type::Int8(), s->getByteLength());
        Q.addConst();
        QualType Ptr = Context.getPointerType(Type::Int8());
        Q->setCanonicalType(Ptr);
        return Q;
    }
    case EXPR_NIL:
        Q = Context.getPointerType(Type::Void());
        if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
        return Q;
    case EXPR_CALL:
        Diag(expr->getLocation(), diag::err_init_element_not_constant) << expr->getSourceRange();
        return QualType();
    case EXPR_IDENTIFIER:
    {
        IdentifierExpr* id = cast<IdentifierExpr>(expr);
        Decl* D = analyseIdentifier(expr_ptr, usedPublic);
        if (!D) return QualType();

        // use checkStack()
/*
        if (D == CurrentVarDecl) {
            Diag(id->getLocation(), diag::err_var_self_init) << D->getName();
            return QualType();
        }
*/
        switch (D->getKind()) {
        case DECL_FUNC:
            break;
        case DECL_VAR:
            break;
        case DECL_ENUMVALUE:
            break;
        case DECL_STRUCTTYPE:
            break;
        case DECL_ALIASTYPE:
        case DECL_FUNCTIONTYPE:
            Diag(id->getLocation(), diag::err_unexpected_typedef) << id->getName();
            return QualType();
        case DECL_ENUMTYPE:
            break;
        case DECL_ARRAYVALUE:
            break;
        case DECL_IMPORT:
            break;
        case DECL_LABEL:
        case DECL_STATIC_ASSERT:
            TODO;
            break;
        }

        D->setUsed();
        if (usedPublic) D->setUsedPublic();
        return D->getType();
    }
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
    case EXPR_TYPE:
        FATAL_ERROR("Should not come here");
        break;
    case EXPR_BINOP:
        return analyseBinaryOperator(expr, usedPublic);
    case EXPR_CONDOP:
        //return analyseConditionalOperator(expr);
        break;
    case EXPR_UNARYOP:
        return analyseUnaryOperator(expr_ptr, usedPublic);
    case EXPR_BUILTIN:
        return analyseBuiltinExpr(expr, usedPublic);
    case EXPR_ARRAYSUBSCRIPT:
        return analyseArraySubscript(expr, usedPublic);
    case EXPR_MEMBER:
        return analyseMemberExpr(expr_ptr, usedPublic);
    case EXPR_PAREN:
        return analyseParenExpr(expr, usedPublic);
    case EXPR_BITOFFSET:
        FATAL_ERROR("Should not come here");
        break;
    case EXPR_EXPLICIT_CAST:
        //return analyseExplicitCastExpr(expr);
        break;
    case EXPR_IMPLICIT_CAST:
        TODO;
        break;
    }
    expr->dump();
    TODO;
    return QualType();
}

QualType FileAnalyser::analyseArraySubscript(Expr* expr, bool usedPublic) {
    LOG_FUNC
    ArraySubscriptExpr* sub = cast<ArraySubscriptExpr>(expr);
    QualType Q = analyseExpr(sub->getBase2(), usedPublic, true);
    if (!Q.isValid()) return Q;
    QualType LType = sub->getBase()->getType();

#if 0
    if (BitOffsetExpr* BO = dyncast<BitOffsetExpr>(sub->getIndex())) {
        QualType T = analyseBitOffsetExpr(sub->getIndex(), LType, sub->getLocation());
        expr->setType(T);
        AnalyserUtils::combineCTV(expr, sub->getBase(), sub->getIndex());
        // dont analyse partial BitOffset expressions per part
        return T;
    }
#endif

    // Deference alias types
    if (isa<AliasType>(LType)) {
        LType = cast<AliasType>(LType)->getRefType();
    }

    if (!LType.isPointerType()) {
        Diag(expr->getLocation(), diag::err_typecheck_subscript);
        return QualType();
    }

    Q = analyseExpr(sub->getIndex2(), usedPublic, true);
    if (!Q.isValid()) return Q;

    // TODO BBB still needed?
    QualType Result;
    if (isa<PointerType>(LType)) {
        Result = cast<PointerType>(LType)->getPointeeType();
    } else if (isa<ArrayType>(LType)) {
        Result = cast<ArrayType>(LType)->getElementType();
    }
    assert(Result.isValid());
    return Result;
}

QualType FileAnalyser::analyseParenExpr(Expr* expr, bool usedPublic) {
    LOG_FUNC
    ParenExpr* P = cast<ParenExpr>(expr);
    Expr** inner_ptr = P->getExpr2();
    Expr* inner = *inner_ptr;
    QualType Q = analyseExpr(inner_ptr, usedPublic, false);

    expr->setCTV(inner->isCTV());
    if (inner->isCTC()) expr->setCTC();
    return Q;
}

QualType FileAnalyser::analyseBuiltinExpr(Expr* expr, bool usedPublic) {
    LOG_FUNC
    BuiltinExpr* B = cast<BuiltinExpr>(expr);
    switch (B->getBuiltinKind()) {
    case BuiltinExpr::BUILTIN_SIZEOF:
        return analyseSizeOfExpr(B, usedPublic);
    case BuiltinExpr::BUILTIN_ELEMSOF:
        return analyseElemsOfExpr(B, usedPublic);
    case BuiltinExpr::BUILTIN_ENUM_MIN:
        return analyseEnumMinMaxExpr(B, true, usedPublic);
    case BuiltinExpr::BUILTIN_ENUM_MAX:
        return analyseEnumMinMaxExpr(B, false, usedPublic);
    case BuiltinExpr::BUILTIN_OFFSETOF:
        return analyseOffsetOf(B, usedPublic);
    case BuiltinExpr::BUILTIN_TO_CONTAINER:
        return analyseToContainer(B, usedPublic);
    }
    FATAL_ERROR("should not come here");
    return QualType();
}

QualType FileAnalyser::analyseOffsetOf(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    B->setType(Type::UInt32());
    StructTypeDecl* std = builtinExprToStructTypeDecl(B, usedPublic);
    if (!std) return QualType();


    uint64_t off = 0;
    if (EA.analyseOffsetOf(B, std, B->getMember(), &off) != 0) return Type::UInt32();
    return QualType();
}

StructTypeDecl* FileAnalyser::builtinExprToStructTypeDecl(BuiltinExpr* B, bool usedPublic) {
    Expr* structExpr = B->getExpr();
    Decl* structDecl = 0;
    // TODO scope.checkAccess(std, structExpr->getLocation());

    IdentifierExpr* I = dyncast<IdentifierExpr>(structExpr);
    if (I) {
        structDecl = analyseIdentifier(B->getExpr2(), usedPublic);
        if (!structDecl) return 0;
    } else {    // MemberExpr: module.StructType
        assert(isa<MemberExpr>(structExpr));
        MemberExpr* M = cast<MemberExpr>(structExpr);
        Expr* base = M->getBase();
        IdentifierExpr* B = dyncast<IdentifierExpr>(base);
        assert(isa<IdentifierExpr>(base));  // Don't allow substructs as type
        Decl* D = analyseIdentifier(M->getBase2(), usedPublic);
        if (!D) return 0;

        ImportDecl* ID = dyncast<ImportDecl>(D);
        if (!ID) {
            Diag(B->getLocation(), diag::err_unknown_module) << B->getName() << B->getSourceRange();
            return 0;
        }
        M->setModulePrefix();
        I = M->getMember();
        structDecl = scope->findSymbolInModule(I->getName(), I->getLocation(), ID->getModule());
        if (!structDecl) return 0;

        M->setDecl(structDecl);
        M->setType(D->getType());
        AnalyserUtils::SetConstantFlags(structDecl, M);
        QualType Q = structDecl->getType();
        I->setType(Q);
        I->setDecl(structDecl, AnalyserUtils::globalDecl2RefKind(structDecl));
    }
    structDecl->setUsed();

    StructTypeDecl* std = dyncast<StructTypeDecl>(structDecl);
    if (!std) {
        int idx = (B->getBuiltinKind() == BuiltinExpr::BUILTIN_OFFSETOF) ? 0 : 1;
        Diag(I->getLocation(), diag::err_builtin_non_struct_union) << idx << I->getName();
        return 0;
    }

    if (&module != std->getModule() && std->hasAttribute(ATTR_OPAQUE)) {
        Expr* structExpr = B->getExpr();
        Diag(structExpr->getLocation(), diag::err_deref_opaque) << std->isStruct() << std->DiagName() << structExpr->getSourceRange();
        return 0;
    }
    return std;
}

void FileAnalyser::error(SourceLocation loc, QualType left, QualType right) const {
    StringBuilder buf1(MAX_LEN_TYPENAME);
    StringBuilder buf2(MAX_LEN_TYPENAME);
    right.DiagName(buf1);
    left.DiagName(buf2);

    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(loc, diag::err_illegal_type_conversion)
            << buf1 << buf2;
}

QualType FileAnalyser::analyseToContainer(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    // check struct type
    StructTypeDecl* std = builtinExprToStructTypeDecl(B, usedPublic);
    if (!std) return QualType();
    QualType ST = std->getType();

    // check member
    // TODO try re-using code with analyseMemberExpr() or analyseStructMember()
    Expr* memberExpr = B->getMember();
    IdentifierExpr* member = dyncast<IdentifierExpr>(memberExpr);
    if (!member) { TODO; } // TODO support sub-member (memberExpr)

    Decl* match = std->findMember(member->getName());
    if (!match) {
        return EA.outputStructDiagnostics(ST, member, diag::err_no_member);
    }
    member->setDecl(match, IdentifierExpr::REF_STRUCT_MEMBER);
    QualType MT = match->getType();
    member->setType(MT);

    // check ptr
    Expr** ptrExpr_ptr = B->getPointer2();
    Expr* ptrExpr = *ptrExpr_ptr;
    QualType ptrType = analyseExpr(ptrExpr_ptr, usedPublic, false);
    if (!ptrType.isPointerType()) {
        EA.error(ptrExpr, Context.getPointerType(member->getType()), ptrType);
        return QualType();
    }
    QualType PT = cast<PointerType>(ptrType)->getPointeeType();
    // TODO BB allow conversion from void*
    // TODO BB use ExprAnalyser to do and insert casts etc
    if (PT != MT) {
        error(ptrExpr->getLocation(), Context.getPointerType(member->getType()), ptrType);
        return QualType();
    }

    // if ptr is const qualified, tehn so should resulting Type (const Type*)
    if (PT.isConstQualified()) ST.addConst();
    QualType Q = Context.getPointerType(ST);
    return Q;
}

QualType FileAnalyser::analyseSizeOfExpr(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    static constexpr unsigned FLOAT_SIZE_DEFAULT = 8;

    uint64_t width;
    //allowStaticMember = true;

    Expr** expr_ptr = B->getExpr2();
    Expr* expr = *expr_ptr;

    QualType Q;

    switch (expr->getKind()) {
    case EXPR_FLOAT_LITERAL:
        width = FLOAT_SIZE_DEFAULT;
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        TODO; // Good error
        //allowStaticMember = false;
        return QualType();
    case EXPR_TYPE:
    {
        // TODO use stack?
        Q = analyseType(expr->getType(), expr->getLocation(), usedPublic, true);
        if (!Q.isValid()) return Q;
        TR->checkOpaqueType(B->getLocation(), usedPublic, Q);
        expr->setType(Q);
        unsigned align;
        width = AnalyserUtils::sizeOfType(Q, &align);
        break;
    }
    case EXPR_INTEGER_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
    case EXPR_CALL:
    case EXPR_BINOP:
    case EXPR_CONDOP:
    case EXPR_UNARYOP:
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
    case EXPR_IDENTIFIER:
    case EXPR_EXPLICIT_CAST: {
        Q = analyseExpr(expr_ptr, usedPublic, false);
        if (!Q.isValid()) return Q;
        QualType type = expr->getType();
        TR->checkOpaqueType(expr->getLocation(), usedPublic, type);
        unsigned align;
        width = AnalyserUtils::sizeOfType(type, &align);
        break;
    }
    case EXPR_IMPLICIT_CAST:
        TODO;
        break;
    case EXPR_BUILTIN:
        FATAL_ERROR("Unreachable");
        //allowStaticMember = false;
        break;
    case EXPR_PAREN:
    case EXPR_BITOFFSET:
        FATAL_ERROR("Unreachable");
        //allowStaticMember = false;
        return QualType();
    }
    B->setValue(llvm::APSInt::getUnsigned(width));
    //allowStaticMember = false;
    return Type::UInt32();
}

QualType FileAnalyser::analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin, bool usedPublic) {
    LOG_FUNC
    Expr* E = B->getExpr();
    // TODO support memberExpr (module.Type)
    assert(isa<IdentifierExpr>(E));
    Decl* decl = analyseIdentifier(B->getExpr2(), usedPublic);
    if (!decl) return QualType();

    EnumTypeDecl* Enum = 0;
    decl->setUsed();

    // = Type::UInt32();
    switch (decl->getKind()) {
    case DECL_VAR:
        if (const EnumType* ET = dyncast<EnumType>(decl->getType())) {
            Enum = ET->getDecl();
        }
        break;
    case DECL_ENUMTYPE:
        Enum = cast<EnumTypeDecl>(decl);
        break;
    default:
        break;
    }

    if (!Enum) {
        Diag(E->getLocation(), diag::err_enum_minmax_no_enum) << (isMin ? 0 : 1) << E->getSourceRange();
        return QualType();
    }

    if (isMin) B->setValue(Enum->getMinValue());
    else B->setValue(Enum->getMaxValue());

    return E->getType();
}

QualType FileAnalyser::analyseElemsOfExpr(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    Expr** E_ptr = B->getExpr2();
    QualType Q = analyseExpr(E_ptr, usedPublic, false);
    if (!Q.isValid()) return Q;

    llvm::APSInt i(32, 1);
    const ArrayType* AT = dyncast<ArrayType>(Q.getCanonicalType());
    if (AT) {
        i = AT->getSize();
        B->setValue(i);
        return Type::UInt32();
    }
    const EnumType* ET = dyncast<EnumType>(Q);  // NOTE: dont use canonicalType!
    if (ET) {
        EnumTypeDecl* ETD = ET->getDecl();
        i = ETD->numConstants();
        B->setValue(i);
        return Type::UInt32();
    }
    Expr* E = *E_ptr;
    Diag(E->getLocation(), diag::err_elemsof_no_array) << E->getSourceRange();
    return QualType();
}


QualType FileAnalyser::analyseBinaryOperator(Expr* expr, bool usedPublic) {
    LOG_FUNC
    BinaryOperator* binop = cast<BinaryOperator>(expr);

    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
    case BINOP_Shl:
    case BINOP_Shr:
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
    case BINOP_LAnd:
    case BINOP_LOr:
        break;
    case BINOP_Assign:
    case BINOP_MulAssign:
    case BINOP_DivAssign:
    case BINOP_RemAssign:
    case BINOP_AddAssign:
    case BINOP_SubAssign:
    case BINOP_ShlAssign:
    case BINOP_ShrAssign:
    case BINOP_AndAssign:
    case BINOP_XorAssign:
    case BINOP_OrAssign:
        FATAL_ERROR("should not come here");
        break;
    case BINOP_Comma:
        FATAL_ERROR("unhandled binary operator type");
        break;
    }

    QualType TLeft = analyseExpr(binop->getLHS2(), usedPublic, true);
    if (!TLeft.isValid()) return TLeft;
    QualType TRight = analyseExpr(binop->getRHS2(), usedPublic, true);
    if (!TRight.isValid()) return TRight;

    Expr* Left = binop->getLHS();
    Expr* Right = binop->getRHS();

    QualType Result;
    // TODO BBB remove the switch?
    // determine Result type
    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
        Result = AnalyserUtils::UsualUnaryConversions(Left);
        AnalyserUtils::UsualUnaryConversions(Right);
        break;
    case BINOP_Shl:
    case BINOP_Shr:
        Result = TLeft;
        break;
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
        Result = Type::Bool();
        break;
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
        Result = TLeft;
        break;
    case BINOP_LAnd:
    case BINOP_LOr:
        Result = Type::Bool();
        break;
    case BINOP_Assign:
    case BINOP_MulAssign:
    case BINOP_DivAssign:
    case BINOP_RemAssign:
    case BINOP_AddAssign:
    case BINOP_SubAssign:
    case BINOP_ShlAssign:
    case BINOP_ShrAssign:
    case BINOP_AndAssign:
    case BINOP_XorAssign:
    case BINOP_OrAssign:
        FATAL_ERROR("should not come here");
        break;
    case BINOP_Comma:
        FATAL_ERROR("unhandled binary operator type");
        break;
    }

    Result = EA.getBinOpType(binop);

    expr->combineFlags(Left, Right);
    return Result;
}

QualType FileAnalyser::analyseUnaryOperator(Expr** expr_ptr, bool usedPublic) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    UnaryOperator* unaryop = cast<UnaryOperator>(expr);
    Expr** SubExpr_ptr = unaryop->getExpr2();
    bool need_rvalue = true;

    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        Diag(unaryop->getOpLoc(), diag::err_init_element_not_constant) << expr->getSourceRange();
        return QualType();
    case UO_AddrOf:
        need_rvalue = false;
        break;
    case UO_Deref:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        break;
    }

    QualType LType = analyseExpr(SubExpr_ptr, usedPublic, need_rvalue);
    if (!LType.isValid()) return LType;
    SubExpr_ptr = unaryop->getExpr2();  // re-read in case casts were inserted
    Expr* SubExpr = *SubExpr_ptr;

    if (LType.isVoidType()) {
        Diag(unaryop->getOpLoc(), diag::err_typecheck_unary_expr) << "'void'" << SubExpr->getSourceRange();
        return QualType();
    }

    // TODO cleanup, always break and do common stuff at end
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        FATAL_ERROR("cannot come here");
        break;
    case UO_AddrOf:
        if (!checkAddressOfOperand(SubExpr)) return QualType();
        LType = Context.getPointerType(LType.getCanonicalType());
        expr->setCTC();
        break;
    case UO_Deref:
        if (!LType.isPointerType()) {
            char typeName[MAX_LEN_TYPENAME];
            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
            LType.DiagName(buf);
            Diag(unaryop->getOpLoc(), diag::err_typecheck_indirection_requires_pointer) << buf;
            return QualType();
        } else {
            // TEMP use CanonicalType to avoid Unresolved types etc
            LType = LType.getCanonicalType();
            const PointerType* P = cast<PointerType>(LType);
            return P->getPointeeType();
        }
        break;
    case UO_Minus:
    case UO_Not:
        unaryop->setCTV(SubExpr->isCTV());
        if (SubExpr->isCTC()) unaryop->setCTC();
        LType = AnalyserUtils::UsualUnaryConversions(SubExpr);
        break;
    case UO_LNot:
        // TODO first cast expr to bool, then invert here, return type bool
        // TODO extract to function
        // TODO check conversion to bool here!!
        unaryop->setCTV(SubExpr->isCTV());
        // Also set type?
        //AnalyserUtils::UsualUnaryConversions(SubExpr);
        return Type::Bool();
    }
    return LType;
}

bool FileAnalyser::checkAddressOfOperand(Expr* expr) {
    expr = AnalyserUtils::getInnerExprAddressOf(expr);
    IdentifierExpr* I = dyncast<IdentifierExpr>(expr);
    if (!I) {
        StringBuilder buf(MAX_LEN_TYPENAME);
        expr->getType().DiagName(buf);
        Diag(expr->getLocation(), diag::err_typecheck_invalid_lvalue_addrof) << buf;
        return false;
    }
    IdentifierExpr::RefKind ref = I->getRefType();
    switch (ref) {
        case IdentifierExpr::REF_UNRESOLVED:
            FATAL_ERROR("should not come here");
            return false;
        case IdentifierExpr::REF_MODULE:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a module";
            return false;
        case IdentifierExpr::REF_FUNC:
            // NOTE: C2 does not allow address of function like C
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a function";
            return false;
        case IdentifierExpr::REF_TYPE:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a type";
            return false;
        case IdentifierExpr::REF_VAR:
            return true;
        case IdentifierExpr::REF_ENUM_CONSTANT:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "an enum constant";
            return false;
        case IdentifierExpr::REF_STRUCT_MEMBER:
            // TODO not if on type! (only on instance)
            // ok
            return true;
        case IdentifierExpr::REF_STRUCT_FUNC:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a function";
            return false;
        case IdentifierExpr::REF_LABEL:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a label";
            return false;
    }

    // NOTE: C also allow funcions, C2 not? (no just use function itself)
    // actually it should be an IdentifierExpr pointing to a VarDecl?
    // Must be IdentifierExpr that points to VarDecl?
    return true;
}

QualType FileAnalyser::analyseMemberExpr(Expr** expr_ptr, bool usedPublic) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    MemberExpr* M = cast<MemberExpr>(expr);
    IdentifierExpr* member = M->getMember();

    // we dont know what we're looking at here, it could be:
    // mod.Type
    // mod.var
    // mod.func
    // var<Type=struct>.member
    // var<Type=struct>.struct_function
    // var[index].member
    // var->member (= error)
    // Type.struct_function
    // Enum.Constant (eg. Color.Red)

    // NOTE: since we dont know which one, we can't insert LValueToRValue casts in analyseExpr(RHS)
    // Also optionally insert CK_ArrayToPointerDecay cast here

    QualType LType = analyseExpr(M->getBase2(), usedPublic, false);
    if (!LType.isValid()) return LType;

    const ModuleType* MT = dyncast<ModuleType>(LType);
    // module.X
    if (MT) {
        ImportDecl* ID = MT->getDecl();
        ID->setUsed();
        M->setModulePrefix();
        MT = cast<ModuleType>(LType.getTypePtr());
        Decl* D = scope->findSymbolInModule(member->getName(), member->getLocation(), MT->getModule());
        if (!D) return QualType();

        D->setUsed();
        M->setDecl(D);
        AnalyserUtils::SetConstantFlags(D, M);
        QualType Q = D->getType();
        M->setType(Q);
        member->setType(Q);
        member->setDecl(D, AnalyserUtils::globalDecl2RefKind(D));

        if (Q.isArrayType()) {
            QualType Qptr = AnalyserUtils::getPointerFromArray(Context, Q);
            insertImplicitCast(CK_ArrayToPointerDecay, expr_ptr, Qptr);
            return Qptr;
        }
        if (Q.isFunctionType()) {
            M->setIsRValue();
            member->setIsRValue();
            insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, Q);
        }

        if (M->isCTV()) M->setIsRValue(); // points to const number

        return Q;
    // Enum.Constant
    } else if (isa<EnumType>(LType)) {
        EnumType* ET = cast<EnumType>(LType.getTypePtr());
        EnumTypeDecl* ETD = ET->getDecl();
        ETD->setUsed();
        EnumConstantDecl* ECD = ETD->findConstant(member->getName());
        if (!ECD) {
            StringBuilder buf(MAX_LEN_TYPENAME);
            ETD->fullName(buf);
            Diag(member->getLocation(), diag::err_unknown_enum_constant)
                << buf << member->getName();
            return QualType();
        }
        if (isTop(ETD) && !ECD->isChecked()) {
            if (ECD->getCheckState() == CHECK_IN_PROGRESS) {
                Diag(member->getLocation(), diag::err_var_self_init) << ECD->getName();
                return QualType();
            } else { // CHECK_UNCHECKED
                printf("ERROR: using un-initialized enum constant\n");
                // TEMP TODO correct error
                Diag(member->getLocation(), diag::err_var_self_init) << ECD->getName();
                return QualType();
            }
        }

        QualType Q = ETD->getType();
        M->setDecl(ECD);
        M->setIsEnumConstant();
        M->setIsRValue();
        AnalyserUtils::SetConstantFlags(ECD, M);
        member->setCTV(true);
        member->setCTC();
        member->setType(Q);
        member->setDecl(ECD, AnalyserUtils::globalDecl2RefKind(ECD));
        return Q;
    } else {
        // dereference pointer
        if (LType.isPointerType()) {
            // TODO use function to get elementPtr (could be alias type)
            const PointerType* PT = cast<PointerType>(LType.getTypePtr());
            LType = PT->getPointeeType();
        }
        QualType S = AnalyserUtils::getStructType(LType);
        if (!S.isValid()) {
            StringBuilder buf(MAX_LEN_TYPENAME);
            LType.DiagName(buf);
            Diag(M->getLocation(), diag::err_typecheck_member_reference_struct_union)
                    << buf << M->getSourceRange() << member->getLocation();
            return QualType();
        }
        return analyseStructMember(S, expr_ptr, AnalyserUtils::exprIsType(M->getBase()));
    }
    return QualType();
}

QualType FileAnalyser::analyseStaticStructMember(QualType T, Expr** expr_ptr, const StructTypeDecl* S)
{
    LOG_FUNC
    Expr* expr = *expr_ptr;
    MemberExpr* M = cast<MemberExpr>(expr);
    IdentifierExpr* member = M->getMember();

    Decl *field = S->findMember(member->getName());
    FunctionDecl* sfunc = S->findFunction(member->getName());

    Decl* d = field ? field : sfunc;
    if (!d) return EA.outputStructDiagnostics(T, member, diag::err_no_member_struct_func);

    scope->checkAccess(d, member->getLocation());
    d->setUsed();
    QualType Q = d->getType();
    M->setDecl(d);
    M->setType(Q);
    if (field) {
        assert(field->isChecked());
        member->setDecl(d, IdentifierExpr::REF_STRUCT_MEMBER);
    }
    else {
        if (!analyseDecl(sfunc)) return QualType();
        member->setDecl(d, IdentifierExpr::REF_STRUCT_FUNC);
        M->setIsStructFunction();
        if (sfunc->isStaticStructFunc()) M->setIsStaticStructFunction();
    }
    member->setType(Q);
    AnalyserUtils::SetConstantFlags(d, M);

    // TEMP just declare here
    bool allowStaticMember = false;
    if (!allowStaticMember) {
        if (sfunc) {
            // TBD: allow non-static use of struct functions?
            //if (!sfunc->isStaticStructFunc()) return EA.outputStructDiagnostics(T, member, diag::err_invalid_use_nonstatic_struct_func);
        } else {
            return EA.outputStructDiagnostics(T, member, diag::err_static_use_nonstatic_member);
        }
    }

    if (Q.isFunctionType()) {
        M->setIsRValue();
        M->setCTC();
        member->setIsRValue();
        insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, Q);
    }
    return Q;
}

// T is the type of the struct
// M the whole expression
// isStatic is true for Foo.myFunction(...)
QualType FileAnalyser::analyseStructMember(QualType T, Expr** expr_ptr, bool isStatic) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    MemberExpr* M = cast<MemberExpr>(expr);
    assert(M && "Expression missing");
    const StructType* ST = cast<StructType>(T);
    const StructTypeDecl* S = ST->getDecl();

    if (isStatic) return analyseStaticStructMember(T, expr_ptr, S);

    IdentifierExpr* member = M->getMember();
    Decl* match = S->find(member->getName());

    if (!match) {
        return EA.outputStructDiagnostics(T, member, diag::err_no_member_struct_func);
    }

    IdentifierExpr::RefKind ref = IdentifierExpr::REF_STRUCT_MEMBER;
    FunctionDecl* func = dyncast<FunctionDecl>(match);

    if (func) {
        // cannot happen at global scope?
        // TODO recurse into function as well
        scope->checkAccess(func, member->getLocation());

        if (func->isStaticStructFunc()) {
            Diag(member->getLocation(), diag::err_static_struct_func_notype);// << func->DiagName();
            return QualType();
        }
        M->setIsStructFunction();
        ref = IdentifierExpr::REF_STRUCT_FUNC;
        //M->setIsRValue();
        QualType Q = func->getType();
        M->setType(Q);
        M->setCTC();
        insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, Q);

#if 0
        // Is this a simple member access? If so
        // disallow this (we don't have closures!)
        if (callStack.callDepth == 0) {
            Diag(member->getLocation(), diag::err_non_static_struct_func_usage)
                << M->getBase()->getSourceRange();
            return QualType();
        }
        callStack.setStructFunction(M->getBase());
#endif
    }
    else {
        // NOTE: access of struct-function is not a dereference
        if (&module != S->getModule() && S->hasAttribute(ATTR_OPAQUE)) {
            Diag(M->getLocation(), diag::err_deref_opaque) << S->isStruct() << S->DiagName();
            return QualType();
        }
    }

    match->setUsed();
    M->setDecl(match);
    QualType Q = match->getType();
    member->setDecl(match, ref);
    member->setType(Q);
    return Q;
}

QualType FileAnalyser::analyseRefType(QualType Q, bool usedPublic, bool full) {
    LOG_FUNC

    const Type* T = Q.getTypePtr();
    const RefType* RT = cast<RefType>(T);
    IdentifierExpr* moduleName = RT->getModuleName();
    IdentifierExpr* typeName = RT->getTypeName();
    SourceLocation tLoc = typeName->getLocation();
    const std::string& tName = typeName->getName();

    Decl* D = 0;
    if (moduleName) { // mod.type
        const std::string& mName = moduleName->getName();
        const Module* mod = scope->findUsedModule(mName, moduleName->getLocation(), usedPublic);
        if (!mod) return QualType();
        Decl* modDecl = scope->findSymbol(mName, moduleName->getLocation(), true, usedPublic);
        assert(modDecl);
        moduleName->setDecl(modDecl, IdentifierExpr::REF_MODULE);

        D =  scope->findSymbolInModule(tName, tLoc, mod);
    } else { // type
        D = scope->findSymbol(tName, tLoc, true, usedPublic);
    }
    if (!D) return QualType();
    TypeDecl* TD = dyncast<TypeDecl>(D);
    if (!TD) {
        StringBuilder name;
        RT->printLiteral(name);
        Diags.Report(tLoc, diag::err_not_a_typename) << name.c_str();
        return QualType();
    }
    bool external = scope->isExternal(D->getModule());
    if (usedPublic &&!external && !TD->isPublic()) {
        StringBuilder name;
        RT->printLiteral(name);
        Diags.Report(tLoc, diag::err_non_public_type) << AnalyserUtils::fullName(TD->getModule()->getName(), TD->getName());
        //Diags.Report(tLoc, diag::err_non_public_type) << name;
        return QualType();
    }

    if (full && D->getCheckState() == CHECK_IN_PROGRESS) {
        Diag(tLoc, diag::err_circular_decl);
        return QualType();
    }

    if (full && !analyseDecl(D)) return QualType();

    D->setUsed();
    if (usedPublic || external) D->setUsedPublic();
    typeName->setDecl(TD, IdentifierExpr::REF_TYPE);

    QualType result = TD->getType();
    if (Q.isConstQualified()) result.addConst();
    if (Q.isVolatileQualified()) result.addVolatile();
    return result;
}

bool FileAnalyser::analyseTypeDecl(TypeDecl* D) {
    LOG_FUNC

    QualType Q = analyseType(D->getType(), D->getLocation(), D->isPublic(), true);
    if (!Q.isValid()) return false;
    //D->setType(Q);    // NOTE: does not seem needed

    // check extra stuff depending on subclass
    switch (D->getKind()) {
    case DECL_FUNC:
    case DECL_VAR:
    case DECL_ENUMVALUE:
        FATAL_ERROR("Cannot have type");
        break;
    case DECL_ALIASTYPE:
        break;
    case DECL_STRUCTTYPE:
    {
        Names names;
        StructTypeDecl* S = cast<StructTypeDecl>(D);
        if (!analyseStructNames(S, names, S->isStruct())) return false;
        if (!analyseStructTypeDecl(cast<StructTypeDecl>(D))) return false;
        break;
    }
    case DECL_ENUMTYPE:
    {
        EnumTypeDecl* E = cast<EnumTypeDecl>(D);
        if (E->numConstants() == 0) {
            Diags.Report(D->getLocation(), diag::err_empty_enum) << D->getName();
            return 1;
        } else {
            if (!analyseEnumConstants(E)) return false;
        }
        break;
    }
    case DECL_FUNCTIONTYPE:
    {
        const FunctionTypeDecl* FTD = cast<FunctionTypeDecl>(D);
        FunctionDecl* FD = FTD->getDecl();
        if (!analyseDecl(FD)) return false;
        break;
    }
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        FATAL_ERROR("Cannot have type decl");
        break;
    }
    if (!checkAttributes(D)) return false;
    return true;
}

bool FileAnalyser::analyseStructNames(const StructTypeDecl* S, Names& names, bool isStruct) {
    LOG_FUNC
    typedef Names::iterator NamesIter;
    for (unsigned i=0; i<S->numMembers(); i++) {
        const Decl* member = S->getMember(i);
        const std::string& name = member->getName();
        if (name == "") {
            assert(isa<StructTypeDecl>(member));
            if (!analyseStructNames(cast<StructTypeDecl>(member), names, isStruct)) return false;
        } else {
            NamesIter iter = names.find(name);
            if (iter != names.end()) {
                const Decl* existing = iter->second;
                Diags.Report(member->getLocation(), diag::err_duplicate_struct_member) << isStruct << member->DiagName();
                Diags.Report(existing->getLocation(), diag::note_previous_declaration);
                return false;
            } else {
                names[name] = member;
            }
            const StructTypeDecl* sub = dyncast<StructTypeDecl>(member);
            if (sub) {
                Names subNames;
                if (!analyseStructNames(sub, subNames, sub->isStruct())) return false;
            }
        }
    }
    return true;
}

bool FileAnalyser::analyseStructTypeDecl(StructTypeDecl* D) {
    LOG_FUNC

    if (!D->isGlobal() && !ast.isInterface() && !D->hasEmptyName() && !islower(D->getName()[0])) {
        Diags.Report(D->getLocation(), diag::err_upper_casing) << 4;
        return false;
    }

    bool isPacked = D->isPacked();
    if (isPacked) D->setIsPacked();
    for (unsigned i=0; i<D->numMembers(); i++) {
        Decl* M = D->getMember(i);
        if (isa<VarDecl>(M)) {
            VarDecl* V = cast<VarDecl>(M);
            assert(V->getInitValue() == 0);
            // NOTE: dont push to stack, because can self-ref to StructType
            V->setCheckState(CHECK_IN_PROGRESS);   // manually set
            if (!analyseVarDecl(V)) return false;
            V->setCheckState(CHECK_DONE);   // manually set
        } else if (isa<StructTypeDecl>(M)) {
            StructTypeDecl* sub = cast<StructTypeDecl>(M);
            if (isPacked) sub->setIsPacked();
            if (!analyseStructTypeDecl(sub)) return false;
        }
    }

    unsigned alignment = 0;
    uint64_t size = AnalyserUtils::sizeOfStruct(D, &alignment);
    D->setInfo(size, alignment);
    return true;
}

bool FileAnalyser::checkArrayDesignators(InitListExpr* expr, int64_t* size) {
    LOG_FUNC
    typedef std::vector<Expr*> Indexes;
    Indexes indexes;
    Expr** values = expr->getValues();
    unsigned numValues = expr->numValues();
    indexes.resize(numValues);
    int maxIndex = 0;
    int currentIndex = -1;
    bool ok = true;
    for (unsigned i=0; i<numValues; i++) {
        Expr* E = values[i];
        if (DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(E)) {
            currentIndex = D->getIndex().getSExtValue();
            if (currentIndex == -1) return false; // some designators are invalid
            if (*size != -1 && currentIndex >= *size) {
                Diag(E->getLocation(), diag::err_array_designator_too_large) << D->getIndex().toString(10) << (int)*size;
                return false;
            }
        } else {
            currentIndex++;
        }
        if (*size != -1 && currentIndex >= *size) {
            Diag(E->getLocation(), diag::err_excess_initializers) << 0;
            return false;
        }
        if (currentIndex >= (int)indexes.size()) {
            indexes.resize(currentIndex + 1);
        }
        Expr* existing = indexes[currentIndex];
        if (existing) {
            Diag(E->getLocation(), diag::err_duplicate_array_index_init) << E->getSourceRange();
            Diag(existing->getLocation(), diag::note_previous_initializer) << 0 << 0 << E->getSourceRange();
            ok = false;
        } else {
            indexes[currentIndex] = E;
        }
        if (currentIndex > maxIndex) maxIndex = currentIndex;
    }
    if (*size == -1) *size = maxIndex + 1;
    //for (unsigned i=0; i<indexes.size(); i++) printf("[%d] = %p\n", i, indexes[i]);
    return ok;
}


bool FileAnalyser::analyseInitListArray(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values, bool usedPublic) {
    LOG_FUNC
    bool haveDesignators = false;
    bool ok = true;
    // TODO use helper function
    ArrayType* AT = cast<ArrayType>(Q.getCanonicalType().getTypePtr());
    QualType ET = AT->getElementType();
    bool constant = true;
    for (unsigned i=0; i<numValues; i++) {
        ok |= analyseInitExpr(&values[i], ET, usedPublic);
        if (DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(values[i])) {
            haveDesignators = true;
            if (D->getDesignatorKind() != DesignatedInitExpr::ARRAY_DESIGNATOR) {
                StringBuilder buf;
                Q.DiagName(buf);
                Diag(D->getLocation(), diag::err_field_designator_non_aggr) << 0 << buf;
                ok = false;
                continue;
            }
        }
    }
    if (!ok) return false;

    if (constant) expr->setCTC();
    if (haveDesignators) expr->setDesignators();
    // determine real array size
    llvm::APInt initSize(64, 0, false);
    if (haveDesignators && ok) {
        int64_t arraySize = -1;
        if (AT->getSizeExpr()) {    // size determined by expr
            arraySize = AT->getSize().getZExtValue();
        } //else, size determined by designators
        ok |= checkArrayDesignators(expr, &arraySize);
        initSize = arraySize;
    } else {
        if (AT->getSizeExpr()) {    // size determined by expr
            initSize = AT->getSize();
            uint64_t arraySize = AT->getSize().getZExtValue();
            if (numValues > arraySize) {
                int firstExceed = AT->getSize().getZExtValue();
                Diag(values[firstExceed]->getLocation(), diag::err_excess_initializers) << 0;
                return false;
            }
        } else {    // size determined from #elems in initializer list
            initSize = numValues;
        }
    }
    AT->setSize(initSize);
    expr->setType(Q);
    return ok;
}

bool FileAnalyser::analyseInitListStruct(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values, bool usedPublic) {
    LOG_FUNC
    bool haveDesignators = false;
    expr->setType(Q);
    // TODO use helper function
    StructType* TT = cast<StructType>(Q.getCanonicalType().getTypePtr());
    StructTypeDecl* STD = TT->getDecl();
    bool constant = true;
    // ether init whole struct with field designators, or don't use any (no mixing allowed)
    // NOTE: using different type for anonymous sub-union is allowed
    if (numValues != 0 && isa<DesignatedInitExpr>(values[0])) {
        haveDesignators = true;
    }

    // Check if this is a union, if so then we need designators when initializing.
    if (numValues > 0 && !haveDesignators && !STD->isStruct()) {
        Diag(values[0]->getLocation(), diag::err_field_designator_required_union);
        return false;
    }

    // TODO cleanup this code (after unit-tests) Split into field-designator / non-designator init
    Fields fields;
    fields.resize(STD->numMembers());
    for (unsigned i = 0; i < numValues; i++) {
        if (i >= STD->numMembers()) {
            // note: 0 for array, 2 for scalar, 3 for union, 4 for structs
            Diag(values[STD->numMembers()]->getLocation(), diag::err_excess_initializers) << 4;
            return false;
        }
        DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(values[i]);
        if (!D) {
            VarDecl* VD = dyncast<VarDecl>(STD->getMember(i));
            if (!VD) {
                TODO;
                return false;
            }
            analyseInitExpr(&values[i], VD->getType(), usedPublic);
            if (!values[i]->isCTC()) constant = false;
            if (isa<DesignatedInitExpr>(values[i]) != haveDesignators) {
                Diag(values[i]->getLocation(), diag::err_mixed_field_designator);
                return false;
            }
            continue;
        }
        switch (D->getDesignatorKind()) {
            case DesignatedInitExpr::ARRAY_DESIGNATOR:
            {
                StringBuilder buf;
                Q.DiagName(buf);
                Diag(D->getLocation(), diag::err_array_designator_non_array) << buf;
                return false;
            }
            case DesignatedInitExpr::FIELD_DESIGNATOR:
                if (!analyseFieldInDesignatedInitExpr(D, STD, Q, fields, values[i], haveDesignators, usedPublic)) return false;
                break;
        }
    }
    // TODO BB should always be true?
    if (constant) expr->setCTC();
    return true;
}

bool FileAnalyser::analyseFieldInDesignatedInitExpr(DesignatedInitExpr* D,
                                                        StructTypeDecl* STD,
                                                        QualType Q,
                                                        Fields &fields,
                                                        Expr* value,
                                                        bool &haveDesignators,
                                                        bool usedPublic) {
    LOG_FUNC
    IdentifierExpr* field = cast<IdentifierExpr>(D->getField());
    // TODO can by member of anonymous sub-struct/union
    int memberIndex = STD->findIndex(field->getName());
    int anonUnionIndex = -1;
    StructTypeDecl *anonUnionDecl;
    // Check for matching a sub field of an anonymous union.
    if (memberIndex == -1) {
        // Let's get the anon field if available.
        memberIndex = STD->findIndex("");
        if (memberIndex > -1) {
            // Goto analysis of sub field.
            anonUnionDecl = dyncast<StructTypeDecl>(STD->getMember((unsigned)memberIndex));
            if (anonUnionDecl) {
                anonUnionIndex = anonUnionDecl->findIndex(field->getName());
            } else {
                memberIndex = -1;
            }
        }
    }

    // No match of direct value or anon union.
    if (memberIndex == -1) {
        StringBuilder fname(MAX_LEN_VARNAME);
        AnalyserUtils::quotedField(fname, field);
        StringBuilder tname(MAX_LEN_TYPENAME);
        Q.DiagName(tname);
        Diag(D->getLocation(), diag::err_field_designator_unknown) << fname << tname;
        return true;
    }


    Expr* existing = fields[memberIndex];
    if (existing) {
        StringBuilder fname(MAX_LEN_VARNAME);
        AnalyserUtils::quotedField(fname, field);
        Diag(D->getLocation(), diag::err_duplicate_field_init) << fname;
        Diag(existing->getLocation(), diag::note_previous_initializer) << 0 << 0;
        return true;
    }
    fields[memberIndex] = value;
    VarDecl* VD;
    if (anonUnionIndex > -1) {
        field->setDecl(anonUnionDecl, AnalyserUtils::globalDecl2RefKind(anonUnionDecl));
        VD = dyncast<VarDecl>(anonUnionDecl->getMember((unsigned)anonUnionIndex));
    } else {
        VD = dyncast<VarDecl>(STD->getMember((unsigned)memberIndex));
    }
    if (!VD) {
        TODO;
        assert(VD && "TEMP don't support sub-struct member inits");
    }
    field->setDecl(VD, AnalyserUtils::globalDecl2RefKind(VD));
    analyseInitExpr(D->getInitValue2(), VD->getType(), usedPublic);
    if (anonUnionIndex < 0 && isa<DesignatedInitExpr>(value) != haveDesignators) {
        Diag(value->getLocation(), diag::err_mixed_field_designator);
        return false;
    }
    return true;
}

bool FileAnalyser::analyseInitList(InitListExpr* expr, QualType Q, bool usedPublic) {
    LOG_FUNC
    Expr** values = expr->getValues();
    unsigned numValues = expr->numValues();
    if (Q.isArrayType()) {
        return analyseInitListArray(expr, Q, numValues, values, usedPublic);
    }
    if (Q.isStructType()) {
        // TODO pass usedPublic?
        return analyseInitListStruct(expr, Q, numValues, values, usedPublic);
    }
    // TODO always give error like case 1?
    // only allow 1
    switch (numValues) {
        case 0:
            fprintf(stderr, "TODO ERROR: scalar initializer cannot be empty\n");
            TODO;
            break;
        case 1:
        {
            StringBuilder buf;
            Q.DiagName(buf);
            Diag(expr->getLocation(), diag::err_invalid_initlist_init) << buf << expr->getSourceRange();
            break;
        }
        default:
            Diag(values[1]->getLocation(), diag::err_excess_initializers) << 2;
            break;
    }
    return false;
}

bool FileAnalyser::analyseDesignatorInitExpr(Expr* expr, QualType expectedType, bool usedPublic) {
    LOG_FUNC
    DesignatedInitExpr* D = cast<DesignatedInitExpr>(expr);
    if (D->getDesignatorKind() == DesignatedInitExpr::ARRAY_DESIGNATOR) {
        Expr** Desig_ptr = D->getDesignator2();
        QualType DT = analyseExpr(Desig_ptr, usedPublic, true);
        if (!DT.isValid()) return false;
        Expr* Desig = *Desig_ptr;
        // TODO sync with FunctionAnalyser (it's different!)

        if (!Desig->isCTC()) {
            Diag(Desig->getLocation(), diag::err_init_element_not_constant) << Desig->getSourceRange();
            return false;
        }

        if (!Desig->getType().isIntegerType()) {
            Diag(Desig->getLocation(), diag::err_typecheck_subscript_not_integer) << Desig->getSourceRange();
            return false;
        }

        CTVAnalyser LA(Diags);
        llvm::APSInt V = LA.checkLiterals(Desig);
        if (V.isSigned() && V.isNegative()) {
            Diag(Desig->getLocation(), diag::err_array_designator_negative) << V.toString(10) << Desig->getSourceRange();
            return false;
        }
        D->setIndex(V);
    } else {
        // cannot check here, need structtype
    }

    D->setType(expectedType);
    return analyseInitExpr(D->getInitValue2(), expectedType, usedPublic);
    //if (D->getInitValue()->isCTC()) D->setCTC();
}

bool FileAnalyser::analyseInitExpr(Expr** expr_ptr, QualType expectedType, bool usedPublic) {
    LOG_FUNC

    Expr* expr = *expr_ptr;
    InitListExpr* ILE = dyncast<InitListExpr>(expr);
    const ArrayType* AT = dyncast<ArrayType>(expectedType.getCanonicalType());
    if (AT) {
        const QualType ET = AT->getElementType().getCanonicalType();
        bool isCharArray = (ET == Type::Int8() || ET == Type::UInt8());

        if (!ILE) {
            if (isCharArray) {
                if (!isa<StringLiteral>(expr)) {
                    Diag(expr->getLocation(), diag::err_array_init_not_init_list) << 1;
                    return false;
                }
            } else {
                Diag(expr->getLocation(), diag::err_array_init_not_init_list) << 0;
                return false;
            }
        }
    }
    if (ILE) return analyseInitList(ILE, expectedType, usedPublic);

    if (isa<DesignatedInitExpr>(expr)) return analyseDesignatorInitExpr(expr, expectedType, usedPublic);

    QualType Q = analyseExpr(expr_ptr, usedPublic, true);
    if (!Q.isValid()) return false;

    if (AT) {
         // if right is array and left is pointer, insert ArrayToPointerDecay

        if (AT->getSizeExpr()) {
            // it should be char array type already and expr is string literal
            assert(isa<StringLiteral>(expr));
            const StringLiteral* S = cast<StringLiteral>(expr);
            if (S->getByteLength() > (int)AT->getSize().getZExtValue()) {
                Diag(S->getLocation(), diag::err_initializer_string_for_char_array_too_long) << S->getSourceRange();
                return false;
            }
        }
    } else {
        // TODO BB: add Implicit casts here?
        EA.check(expectedType, *expr_ptr);
        if (EA.hasError()) return false;
    }

    expr = *expr_ptr;   // Need to re-read, since ImplicitCasts could have been inserted
    if (!expr->isCTC()) {
        Diag(expr->getLocation(), diag::err_init_element_not_constant) << expr->getSourceRange();
        return false;
    }

    return true;
}

QualType FileAnalyser::analyseType(QualType Q, SourceLocation loc, bool usedPublic, bool full) {
    LOG_FUNC

    if (Q->hasCanonicalType()) return Q;    // should be ok already

    QualType resolved;
    Type* T = Q.getTypePtr();

    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        FATAL_ERROR("should not come here");
        break;
    case TC_POINTER:
    {
        // Dont return new type if not needed
        const PointerType* P = cast<PointerType>(T);
        QualType t1 = P->getPointeeType();
        QualType Result = analyseType(t1, loc, usedPublic, false);
        if (!Result.isValid()) return QualType();
        if (t1 == Result) {
            resolved = Q;
            Q->setCanonicalType(Q);
        } else {
            resolved = Context.getPointerType(Result);
        }
        break;
    }
    case TC_ARRAY:
    {
        ArrayType* AT = cast<ArrayType>(T);
        QualType ET = analyseType(AT->getElementType(), loc, usedPublic, full);
        if (!ET.isValid()) return QualType();

        Expr* sizeExpr = AT->getSizeExpr();
        if (sizeExpr) {
            if (!analyseArraySizeExpr(AT, usedPublic)) return QualType();
        }

        if (ET == AT->getElementType()) {
            resolved = Q;
            Q->setCanonicalType(Q);
        } else {
            resolved = Context.getArrayType(ET, AT->getSizeExpr(), AT->isIncremental());
            resolved->setCanonicalType(resolved);
            if (sizeExpr) {
                ArrayType* RA = cast<ArrayType>(resolved.getTypePtr());
                RA->setSize(AT->getSize());
            }
        }

        // TODO qualifiers
        break;
    }
    case TC_REF:
        resolved = analyseRefType(Q, usedPublic, full);
        break;
    case TC_ALIAS:
        resolved = analyseType(cast<AliasType>(T)->getRefType(), loc, usedPublic, true);
        break;
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        TODO;
        break;
    case TC_MODULE:
        TODO;
        break;
    }

    // NOTE: CanonicalType is always resolved (ie will not point to another AliasType, but to its Canonicaltype)
    if (!Q->hasCanonicalType()) {
        if (!resolved.isValid()) return QualType();

        if (resolved.getCanonicalType().getTypePtrOrNull() == NULL) {
            resolved.dump();
            FATAL_ERROR("missing canonical on resolved");
        }

        Q->setCanonicalType(resolved.getCanonicalType());
    }

    return resolved;
}

bool FileAnalyser::analyseArraySizeExpr(ArrayType* AT, bool usedPublic) {
    LOG_FUNC

    QualType T = analyseExpr(AT->getSizeExpr2(), usedPublic, false);
    if (!T.isValid()) return false;

    Expr* sizeExpr = AT->getSizeExpr();

    if (!sizeExpr->isCTV()) {
        Diag(sizeExpr->getLocation(), diag::err_array_size_non_const);
        return false;
    }

    // check if negative
    CTVAnalyser LA(Diags);
    llvm::APSInt value = LA.checkLiterals(sizeExpr);

    if (value.isSigned() && value.isNegative()) {
        Diag(sizeExpr->getLocation(), diag::err_typecheck_negative_array_size) << value.toString(10) << sizeExpr->getSourceRange();
        return false;
    }

    AT->setSize(value);
    return true;
}

bool FileAnalyser::analyseFunctionDecl(FunctionDecl* F) {
    LOG_FUNC

    // return type
    QualType Q = analyseType(F->getReturnType(), F->getLocation(), F->isPublic(), true);
    if (!Q.isValid()) return false;
    F->setReturnType(Q);

    // TODO BB should be inside analyseType
    TR->checkOpaqueType(F->getLocation(), F->isPublic(), Q);

    // args
    bool ok = true;
    for (unsigned i=0; i<F->numArgs(); i++) {
        VarDecl* Arg = F->getArg(i);
        ok |= analyseDecl(Arg);
    }
    if (!ok) return false;

    if (F->isStructFunction() && checkIfStaticStructFunction(F)) {
        F->setIsStaticStructFunc(true);
    }

    if (!checkAttributes(F)) return false;
    return true;
}

static Decl* getStructDecl(QualType T) {
    // try to convert QualType(Struct*) -> StructDecl
    const PointerType* pt = dyncast<PointerType>(T);
    if (!pt) return 0;

    const StructType* st = dyncast<StructType>(pt->getPointeeType());
    if (!st) return 0;

    return st->getDecl();
}

bool FileAnalyser::checkIfStaticStructFunction(FunctionDecl* F) const {
    // first argument for static struct functions will be (const)Type*  where Type is RefType
    LOG_FUNC
    if (F->numArgs() == 0) return true;

    VarDecl* arg1 = F->getArg(0);
    Decl* argDecl = getStructDecl(arg1->getType());

    IdentifierExpr* i = F->getStructName();
    Decl* structDecl = i->getDecl();

    return (argDecl != structDecl);
}

bool FileAnalyser::collectStructFunction(FunctionDecl* F, StructFunctionList& structFuncs) {
    LOG_FUNC
    // NOTE: dont use analyseIdentifier() here, since it well analyse recursively
    IdentifierExpr* id = F->getStructName();
    Decl* D = scope->findSymbolInModule(id->getName(), id->getLocation(), F->getModule());
    if (!D) return false;

    id->setDecl(D, IdentifierExpr::REF_STRUCT_FUNC);
    id->setType(D->getType());

    StructTypeDecl* S = dyncast<StructTypeDecl>(D);
    if (!S) {
        Diags.Report(id->getLocation(), diag::err_typecheck_member_reference_struct_union)
                << id->getName() << id->getLocation();
        return false;
    }

    const char* memberName = F->getMemberName();
    Decl* match = S->find(memberName);
    if (match) {
        Diags.Report(match->getLocation(), diag::err_struct_function_conflict) << match->DiagName() << F->DiagName();
        Diags.Report(F->getLocation(), diag::note_previous_declaration);
        return false;
    }

    structFuncs[S].push_back(F);
    return true;
}

bool FileAnalyser::checkVarDeclAttributes(VarDecl* D) {
    LOG_FUNC
    if (!D->hasAttributes()) return true;
    if (!checkAttributes(D)) return false;

    // constants cannot have section|aligned|weak attributes, because they're similar to #define MAX 10
    QualType T = D->getType();
    if (T.isConstQualified() && isa<BuiltinType>(T.getCanonicalType())) {
        const AttrList& AL = D->getAttributes();
        for (AttrListConstIter iter = AL.begin(); iter != AL.end(); ++iter) {
            const Attr* A = *iter;
            switch (A->getKind()) {
            case ATTR_UNKNOWN:
            case ATTR_EXPORT:
            case ATTR_PACKED:
            case ATTR_UNUSED:
            case ATTR_NORETURN:
            case ATTR_INLINE:
                // should not happen?
                break;
            case ATTR_UNUSED_PARAMS:
            case ATTR_SECTION:
            case ATTR_ALIGNED:
            case ATTR_WEAK:
            case ATTR_OPAQUE:
                Diags.Report(A->getLocation(), diag::err_attribute_invalid_constants) << A->kind2str() << A->getRange();
                return false;
            case ATTR_CNAME:
            case ATTR_NO_TYPEDEF:
                // should not happen?
                break;
            }
        }
    }
    return true;
}

bool FileAnalyser::checkAttributes(Decl* D) {
    LOG_FUNC

    // For FunctionTypeDecl, use FunctionDecl itself
    FunctionTypeDecl* FTD = dyncast<FunctionTypeDecl>(D);
    if (FTD) D = FTD->getDecl();

    if (!D->hasAttributes()) return true;

    const AttrList& AL = D->getAttributes();
    for (AttrListConstIter iter = AL.begin(); iter != AL.end(); ++iter) {
        const Attr* A = *iter;
        const Expr* arg = A->getArg();
        switch (A->getKind()) {
        case ATTR_UNKNOWN:
            break;
        case ATTR_EXPORT:
            if (!D->isPublic()) {
                Diags.Report(A->getLocation(), diag::err_attribute_export_non_public) << A->getRange();
                return false;
            }
            D->setExported();
            break;
        case ATTR_PACKED:
            if (!isa<StructTypeDecl>(D)) {
                Diags.Report(A->getLocation(), diag::err_attribute_non_struct) << A->kind2str() <<A->getRange();
                return false;
            }
            break;
        case ATTR_UNUSED:
            break;
        case ATTR_UNUSED_PARAMS:
            break;
        case ATTR_SECTION:
            if (const StringLiteral* S = dyncast<StringLiteral>(arg)) {
                if (S->getValue()[0] == 0) {
                    Diags.Report(arg->getLocation(), diag::err_attribute_argument_empty_string) << arg->getSourceRange();
                    return false;
                }
            } else {
                Diags.Report(arg->getLocation(), diag::err_attribute_argument_type) << A->kind2str() << 2 << arg->getSourceRange();
                return false;
            }
            break;
        case ATTR_NORETURN:
        case ATTR_INLINE:
            break;
        case ATTR_ALIGNED:
        {
            assert(arg);
            const IntegerLiteral* I = dyncast<IntegerLiteral>(arg);
            if (!I) {
                Diags.Report(arg->getLocation(), diag::err_aligned_attribute_argument_not_int) << arg->getSourceRange();
                return false;
            }
            if (!I->Value.isPowerOf2()) {
                Diags.Report(arg->getLocation(), diag::err_alignment_not_power_of_two) << arg->getSourceRange();
                return false;
            }
            // TODO check if alignment is too small (smaller then size of type)
            break;
        }
        case ATTR_WEAK:
            if (!D->isPublic()) {
                Diags.Report(A->getLocation(), diag::err_attribute_weak_non_public) << A->getRange();
                return false;
            }
            if (!D->isExported()) {
                Diags.Report(A->getLocation(), diag::err_attribute_weak_non_exported) << A->getRange();
                return false;
            }
            break;
        case ATTR_OPAQUE:
            if (!isa<StructTypeDecl>(D)) {
                Diags.Report(A->getLocation(), diag::err_attribute_non_struct) << A->kind2str() << A->getRange();
                return false;
            }
            if (!D->isPublic()) {
                Diags.Report(A->getLocation(), diag::err_attr_opaque_non_public) << A->getRange();
                return false;
            }
            break;
        case ATTR_CNAME:
            if (!ast.isInterface()) {
                Diags.Report(A->getLocation(), diag::err_attribute_non_interface) << A->kind2str() << A->getRange();
                return false;
            }
            D->setHasCName();
            break;
        case ATTR_NO_TYPEDEF:
        {
            if (!ast.isInterface()) {
                Diags.Report(A->getLocation(), diag::err_attribute_non_interface) << A->kind2str() << A->getRange();
                return false;
            }
            StructTypeDecl* S = dyncast<StructTypeDecl>(D);
            if (!S) {
                Diags.Report(A->getLocation(), diag::err_attribute_non_struct) << A->kind2str() << A->getRange();
                return false;
            }
            S->setNoTypedef();
            break;
        }
        }
    }
    return true;
}

void FileAnalyser::checkUnusedDecls() {
    LOG_FUNC
    if (ast.isExternal()) return;
    if (verbose) printf(COL_VERBOSE "%s %s" ANSI_NORMAL "\n", __func__, ast.getFileName().c_str());

    // checkfor unused uses
    for (unsigned i=0; i<ast.numImports(); i++) {
        ImportDecl* U = ast.getImport(i);
        if (!U->isUsed()) {
            Diags.Report(U->getLocation(), diag::warn_unused_import) << U->getModuleName();
        }
    }

    // check for unused variables
    for (unsigned i=0; i<ast.numVars(); i++) {
        VarDecl* V = ast.getVar(i);
        if (V->hasAttribute(ATTR_UNUSED)) continue;
        if (V->isExported()) continue;

        if (!V->isUsed()) {
            Diags.Report(V->getLocation(), diag::warn_unused_variable) << V->DiagName();
        } else {
            if (V->isPublic() && !V->isUsedPublic()) {
                Diags.Report(V->getLocation(), diag::warn_unused_public) << 2 << V->DiagName();
            }
        }
    }

    // check for unused functions
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);
        if (F->hasAttribute(ATTR_UNUSED)) continue;
        if (F->isExported()) continue;

        if (!F->isUsed()) {
            Diags.Report(F->getLocation(), diag::warn_unused_function) << F->DiagName();
        } else {
            if (F->isPublic() && !F->isUsedPublic()) {
                Diags.Report(F->getLocation(), diag::warn_unused_public) << 1 << F->DiagName();
            }
        }
    }

    // check for unused types
    for (unsigned i=0; i<ast.numTypes(); i++) {
        TypeDecl* T = ast.getType(i);
        if (T->hasAttribute(ATTR_UNUSED)) continue;
        if (T->isExported()) continue;

        // mark Enum Types as used(public) if its constants are used(public)
        if (EnumTypeDecl* ETD = dyncast<EnumTypeDecl>(T)) {
            for (unsigned c=0; c<ETD->numConstants(); c++) {
                EnumConstantDecl* C = ETD->getConstant(c);
                if (C->isUsed()) ETD->setUsed();
                if (C->isUsedPublic()) ETD->setUsedPublic();
                if (C->isUsed() && C->isUsedPublic()) break;
            }
        }

        if (!T->isUsed()) {
            Diags.Report(T->getLocation(), diag::warn_unused_type) << T->DiagName();
        } else {
            if (T->isPublic() && !T->isUsedPublic()) {
                Diags.Report(T->getLocation(), diag::warn_unused_public) << 0 << T->DiagName();
            }
            // check if members are used
            if (isa<StructTypeDecl>(T)) {
                checkStructMembersForUsed(cast<StructTypeDecl>(T));
            }
        }
    }
}

void FileAnalyser::checkStructMembersForUsed(const StructTypeDecl* S) {
    for (unsigned j=0; j<S->numMembers(); j++) {
        Decl* M = S->getMember(j);
        if (!M->isUsed() && !M->hasEmptyName()) {   // dont warn for anonymous structs/unions
            Diags.Report(M->getLocation(), diag::warn_unused_struct_member) << S->isStruct() << M->DiagName();
        }
        if (isa<StructTypeDecl>(M)) {
            checkStructMembersForUsed(cast<StructTypeDecl>(M));
        }
    }

}

DiagnosticBuilder FileAnalyser::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

Expr* FileAnalyser::insertImplicitCast(CastKind ck, Expr** inner_ptr, QualType Q) {
    Expr* inner = *inner_ptr;
    ImplicitCastExpr* ic = new (Context) ImplicitCastExpr(inner->getLocation(), ck, inner);
    ic->setType(Q);
    *inner_ptr = ic;
    return ic;
}

