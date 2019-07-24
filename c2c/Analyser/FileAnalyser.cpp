/* Copyright 2013-2019 Bas van den Berg
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
#include "Analyser/LiteralAnalyser.h"
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

FileAnalyser::FileAnalyser(const Module& module_,
                           const Modules& allModules,
                           DiagnosticsEngine& Diags_,
                           const TargetInfo& target_,
                           AST& ast_,
                           bool verbose_)
    : ast(ast_)
    , module(module_)
    , scope(new Scope(ast_.getModuleName(), allModules, Diags_))
    , TR(new TypeResolver(*scope, Diags_, ast.getContext()))
    , Diags(Diags_)
    , EA(Diags_, target_)
    , functionAnalyser(*scope, *TR, ast.getContext(), Diags_, target_, ast.isInterface())
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
        EnumConstantDecl* ECD = new (ast.getContext()) EnumConstantDecl(IE->getName(), IE->getLocation(), ETD->getType(), 0, ETD->isPublic());
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
            InitListExpr* ILE = new (ast.getContext()) InitListExpr(VD->getLocation(), VD->getLocation());
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
    LiteralAnalyser LA(Diags);
    for (unsigned c=0; c<ETD->numConstants(); c++) {
        EnumConstantDecl* ECD = ETD->getConstant(c);
        Expr* init = ECD->getInitValue();
        if (init) {
            ECD->setCheckState(CHECK_IN_PROGRESS);
            if (!analyseExpr(init, ETD->isPublic())) return false;

            if (init->getCTC() != CTC_FULL) {
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

// TODO refactor to remove duplicates
static IdentifierExpr::RefKind globalDecl2RefKind(const Decl* D) {
    switch (D->getKind()) {
    case DECL_FUNC:         return IdentifierExpr::REF_FUNC;
    case DECL_VAR:          return IdentifierExpr::REF_VAR;
    case DECL_ENUMVALUE:    return IdentifierExpr::REF_ENUM_CONSTANT;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
                            return IdentifierExpr::REF_TYPE;
    case DECL_ARRAYVALUE:
                            return IdentifierExpr::REF_VAR;
    case DECL_IMPORT:
                            return IdentifierExpr::REF_MODULE;
    case DECL_LABEL:        return IdentifierExpr::REF_LABEL;
    }
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
    case DECL_VAR:
        ok = analyseVarDecl(cast<VarDecl>(D));
        break;
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
        Diags.Report(V->getLocation(), msg) << name;
        return QualType();
    }

    V->setType(Q);
    // TODO should be inside analyseType (needs inner/outer function)
    TR->checkOpaqueType(V->getLocation(), V->isPublic(), Q);

    // ATTR
    if (!checkVarDeclAttributes(V)) return false;

    // INIT
    Expr* init = V->getInitValue();
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
        LiteralAnalyser LA(Diags);
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
                break;
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
        if (Q.isConstant()) {
            if (!isupper(V->getName()[0])) {
                Diags.Report(V->getLocation(), diag::err_const_casing);
                return false;
            }
        } else {
            if (!islower(V->getName()[0])) {
                Diags.Report(V->getLocation(), diag::err_var_casing);
                return false;
            }
        }
    }
    return true;
}

Decl* FileAnalyser::analyseIdentifier(IdentifierExpr* id, bool usedPublic) {
    LOG_FUNC
    Decl* D = scope->findSymbol(id->getName(), id->getLocation(), false, usedPublic);
    if (D) {
        id->setDecl(D, globalDecl2RefKind(D));
        // TODO after analysing D?
        id->setType(D->getType());
        if (!D->isChecked()) {
            if (!isTop(D)) D->setUsed();
            //printf("RECURSE INTO %s\n", D->getName());
            if (!analyseDecl(D)) return 0;
        }
        AnalyserUtils::SetConstantFlags(D, id);

        if (usedPublic && !D->isPublic()) {
            Diag(id->getLocation(), diag::err_non_public_constant) << D->DiagName();
        }
    }
    return D;
}

bool FileAnalyser::analyseIntegerLiteral(Expr* expr) {
    LOG_FUNC
    IntegerLiteral* I = cast<IntegerLiteral>(expr);
    // Fit smallest Type: int32 > uint32 > int64 > uint64
    // TODO unsigned types

    // TEMP for now assume signed
    // Q: we can determine size, but don't know if we need signed/unsigned
    //unsigned numbits = I->Value.getMinSignedBits();  // signed
    unsigned numbits = I->Value.getActiveBits();   // unsigned
    //if (numbits <= 8) return Type::Int8();
    //if (numbits <= 16) return Type::Int16();

    if (numbits <= 32) expr->setType(Type::Int32());
    else expr->setType(Type::Int64());
    return true;
}

// Q: maybe pass r/lvalue usage?
bool FileAnalyser::analyseExpr(Expr* expr, bool usedPublic) {
    LOG_FUNC

    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
        return analyseIntegerLiteral(expr);
    case EXPR_FLOAT_LITERAL:
        // For now always return type float
        expr->setType(Type::Float32());
        return true;
    case EXPR_BOOL_LITERAL:
        expr->setType(Type::Bool());
        return true;
    case EXPR_CHAR_LITERAL:
        expr->setType(Type::Int8());
        return true;
    case EXPR_STRING_LITERAL:
    {
        // return type: 'const i8*'
        QualType Q = ast.getContext().getPointerType(Type::Int8());
        Q.addConst();
        if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
        expr->setType(Q);
        return true;
    }
    case EXPR_NIL:
    {
        QualType Q = ast.getContext().getPointerType(Type::Void());
        if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
        expr->setType(Q);
        return true;
    }
    case EXPR_CALL:
        Diag(expr->getLocation(), diag::err_init_element_not_constant) << expr->getSourceRange();
        return false;
    case EXPR_IDENTIFIER:
    {
        IdentifierExpr* id = cast<IdentifierExpr>(expr);
        Decl* D = analyseIdentifier(id, usedPublic);
        if (!D) return false;
#if 1
        // use checkStack()
/*
        if (D == CurrentVarDecl) {
            Diag(id->getLocation(), diag::err_var_self_init) << D->getName();
            return QualType();
        }
*/
        switch (D->getKind()) {
        case DECL_FUNC:
            //expr->setConstant();
            break;
        case DECL_VAR:
            break;
        case DECL_ENUMVALUE:
            //expr->setCTC(CTC_FULL);
            //expr->setConstant();
            break;
        case DECL_STRUCTTYPE:
            // Type.func is allowed as init
            // TODO handle, mark that Type was specified, not just return StructType
            break;
        case DECL_ALIASTYPE:
        case DECL_FUNCTIONTYPE:
            Diag(id->getLocation(), diag::err_unexpected_typedef) << id->getName();
            //expr->setConstant();
            break;
        case DECL_ENUMTYPE:
            //expr->setConstant();
            break;
        case DECL_ARRAYVALUE:
            break;
        case DECL_IMPORT:
            //expr->setConstant();
            break;
        case DECL_LABEL:
            TODO;
            break;
        }
#endif
        D->setUsed();
        if (usedPublic) D->setUsedPublic();
        return true;
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
        return analyseUnaryOperator(expr, usedPublic);
    case EXPR_BUILTIN:
        return analyseBuiltinExpr(expr, usedPublic);
        break;
    case EXPR_ARRAYSUBSCRIPT:
        //return analyseArraySubscript(expr, side);
        break;
    case EXPR_MEMBER:
        return analyseMemberExpr(expr, usedPublic);
    case EXPR_PAREN:
        return analyseParenExpr(expr, usedPublic);
    case EXPR_BITOFFSET:
        FATAL_ERROR("Should not come here");
        break;
    case EXPR_CAST:
        //return analyseExplicitCastExpr(expr);
        break;
    }
    expr->dump();
    TODO;
    return QualType();
}

bool FileAnalyser::analyseParenExpr(Expr* expr, bool usedPublic) {
    LOG_FUNC
    ParenExpr* P = cast<ParenExpr>(expr);
    Expr* inner = P->getExpr();
    if (!analyseExpr(inner, usedPublic)) return false;
    QualType Q = inner->getType();

    expr->setCTC(inner->getCTC());
    if (inner->isConstant()) expr->setConstant();
    expr->setType(Q);
    return true;
}

bool FileAnalyser::analyseBuiltinExpr(Expr* expr, bool usedPublic) {
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
        return analyseOffsetof(B, usedPublic);
    case BuiltinExpr::BUILTIN_TO_CONTAINER:
        return analyseToContainer(B, usedPublic);
    }
}

Decl* FileAnalyser::analyseStructMemberOffset(BuiltinExpr* expr, StructTypeDecl* S, Expr* member) {
    LOG_FUNC

    IdentifierExpr* I = dyncast<IdentifierExpr>(member);
    if (I) {
        Decl *field = S->findMember(I->getName());
        if (!field) {
            outputStructDiagnostics(S->getType(), I, diag::err_no_member);
            return 0;
        }
        field->setUsed();
        I->setType(field->getType());
        I->setDecl(field, IdentifierExpr::REF_STRUCT_MEMBER);
        return field;
    }

    assert(isa<MemberExpr>(member));
    MemberExpr* M = cast<MemberExpr>(member);

    Decl* subStruct = analyseStructMemberOffset(expr, S, M->getBase());
    if (!subStruct) return 0;
    StructTypeDecl* sub = dyncast<StructTypeDecl>(subStruct);
    if (!sub) {
        // Can also be variable of another struct type
        VarDecl* var = dyncast<VarDecl>(subStruct);
        if (var) {
            QualType T = var->getType();
            if (T.isStructType()) {
                const StructType* ST = cast<StructType>(T);
                sub = ST->getDecl();
            }
        }

        if (!sub) {
            StringBuilder buf(MAX_LEN_TYPENAME);
            QualType LType = subStruct->getType();
            LType.DiagName(buf);
            Diag(M->getLocation(), diag::err_typecheck_member_reference_struct_union)
                    << buf << M->getSourceRange() << M->getMember()->getLocation();
            return 0;
        }
    }
    Decl* field = analyseStructMemberOffset(expr, sub, M->getMember());
    if (field) {
        M->setDecl(field);
        M->setType(field->getType());
    }
    return field;
}

bool FileAnalyser::analyseOffsetof(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    B->setType(Type::UInt32());
    StructTypeDecl* std = builtinExprToStructTypeDecl(B, usedPublic);
    if (!std) return false;

    return analyseStructMemberOffset(B, std, B->getMember()) != 0;
}

StructTypeDecl* FileAnalyser::builtinExprToStructTypeDecl(BuiltinExpr* B, bool usedPublic) {
    Expr* structExpr = B->getExpr();
    Decl* structDecl = 0;
    // TODO scope.checkAccess(std, structExpr->getLocation());

    IdentifierExpr* I = dyncast<IdentifierExpr>(structExpr);
    if (I) {
        structDecl = analyseIdentifier(I, usedPublic);
        if (!structDecl) return 0;
    } else {    // MemberExpr: module.StructType
        assert(isa<memberExpr>(structExpr));
        MemberExpr* M = cast<MemberExpr>(structExpr);
        Expr* base = M->getBase();
        IdentifierExpr* B = dyncast<IdentifierExpr>(base);
        assert(B);  // Don't allow substructs as type
        Decl* D = analyseIdentifier(B, usedPublic);
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
        I->setDecl(structDecl, globalDecl2RefKind(structDecl));
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

bool FileAnalyser::analyseToContainer(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    // check struct type
    StructTypeDecl* std = builtinExprToStructTypeDecl(B, usedPublic);
    if (!std) return false;
    QualType ST = std->getType();

    // check member
    // TODO try re-using code with analyseMemberExpr() or analyseStructMember()
    Expr* memberExpr = B->getMember();
    IdentifierExpr* member = dyncast<IdentifierExpr>(memberExpr);
    if (!member) { TODO; } // TODO support sub-member (memberExpr)

    Decl* match = std->findMember(member->getName());
    if (!match) {
        outputStructDiagnostics(ST, member, diag::err_no_member);
        return false;
    }
    member->setDecl(match, IdentifierExpr::REF_STRUCT_MEMBER);
    QualType MT = match->getType();
    member->setType(MT);

    // check ptr
    Expr* ptrExpr = B->getPointer();
    if (!analyseExpr(ptrExpr, usedPublic)) return false;
    QualType ptrType = ptrExpr->getType();
    if (!ptrType.isPointerType()) {
        error(ptrExpr->getLocation(), ast.getContext().getPointerType(member->getType()), ptrType);
        return false;
    }
    QualType PT = cast<PointerType>(ptrType)->getPointeeType();
    // TODO BB allow conversion from void*
    // TODO BB use ExprTypeAnalyser to do and insert casts etc
    if (PT != MT) {
        error(ptrExpr->getLocation(), ast.getContext().getPointerType(member->getType()), ptrType);
        return false;
    }

    // if ptr is const qualified, tehn so should resulting Type (const Type*)
    if (PT.isConstQualified()) ST.addConst();
    QualType Q = ast.getContext().getPointerType(ST);
    B->setType(Q);
    return true;
}

bool FileAnalyser::analyseSizeOfExpr(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    static constexpr unsigned FLOAT_SIZE_DEFAULT = 8;
    B->setType(Type::UInt32());

    uint64_t width;
    //allowStaticMember = true;

    Expr* expr = B->getExpr();

    switch (expr->getKind()) {
    case EXPR_FLOAT_LITERAL:
        width = FLOAT_SIZE_DEFAULT;
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        TODO; // Good error
        //allowStaticMember = false;
        return false;
    case EXPR_TYPE:
    {
        // TODO use stack?
        QualType Q = analyseType(expr->getType(), expr->getLocation(), usedPublic, true);
        if (!Q.isValid()) return false;
        TR->checkOpaqueType(B->getLocation(), usedPublic, Q);
        expr->setType(Q);
        width = AnalyserUtils::sizeOfType(Q);
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
    case EXPR_CAST: {
        if (!analyseExpr(expr, usedPublic)) return false;
        QualType type = expr->getType();
        TR->checkOpaqueType(expr->getLocation(), usedPublic, type);
        width = AnalyserUtils::sizeOfType(type);
        break;
    }
    case EXPR_BUILTIN:
        FATAL_ERROR("Unreachable");
        //allowStaticMember = false;
        return false;
    case EXPR_PAREN:
    case EXPR_BITOFFSET:
        FATAL_ERROR("Unreachable");
        //allowStaticMember = false;
        return false;
    }
    B->setValue(llvm::APSInt::getUnsigned(width));
    //allowStaticMember = false;
    return true;
}

bool FileAnalyser::analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin, bool usedPublic) {
    LOG_FUNC
    B->setType(Type::UInt32());
    Expr* E = B->getExpr();
    // TODO support memberExpr (module.Type)
    assert(isa<IdentifierExpr>(E));
    IdentifierExpr* I = cast<IdentifierExpr>(E);
    Decl* decl = analyseIdentifier(I, usedPublic);
    if (!decl) return false;

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
        return false;
    }

    if (isMin) B->setValue(Enum->getMinValue());
    else B->setValue(Enum->getMaxValue());

    return true;
}

bool FileAnalyser::analyseElemsOfExpr(BuiltinExpr* B, bool usedPublic) {
    LOG_FUNC

    B->setType(Type::UInt32());
    Expr* E = B->getExpr();
    if (!analyseExpr(E, usedPublic)) return false;
    QualType T = E->getType();
    const ArrayType* AT = dyncast<ArrayType>(T.getCanonicalType());
    if (!AT) {
        Diag(E->getLocation(), diag::err_elemsof_no_array) << E->getSourceRange();
        return false;
    }
    B->setValue(llvm::APSInt(AT->getSize()));
    return true;
}


bool FileAnalyser::analyseBinaryOperator(Expr* expr, bool usedPublic) {
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

    Expr* Left = binop->getLHS();
    Expr* Right = binop->getRHS();
    if (!analyseExpr(Left, usedPublic)) return false;
    if (!analyseExpr(Right, usedPublic)) return false;

    QualType Result;
    // determine Result type
    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
        // TODO return largest witdth of left/right (long*short -> long)
        // TODO apply UsualArithmeticConversions() to L + R
        // TEMP for now just return Right side
        // TEMP use UnaryConversion
        Result = AnalyserUtils::UsualUnaryConversions(Left);
        AnalyserUtils::UsualUnaryConversions(Right);
        break;
    case BINOP_Shl:
    case BINOP_Shr:
        Result = Left->getType();
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
        Result = Left->getType();
        break;
    case BINOP_LAnd:
    case BINOP_LOr:
        Result = Type::Bool();
        break;
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

    expr->setConstant();
    AnalyserUtils::combineCtc(expr, Left, Right);
    expr->setType(Result);
    return true;
}

bool FileAnalyser::analyseUnaryOperator(Expr* expr, bool usedPublic) {
    LOG_FUNC
    UnaryOperator* unaryop = cast<UnaryOperator>(expr);
    Expr* SubExpr = unaryop->getExpr();

    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        Diag(unaryop->getOpLoc(), diag::err_init_element_not_constant) << expr->getSourceRange();
        return false;
    case UO_AddrOf:
    case UO_Deref:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        break;
    }

    if (!analyseExpr(SubExpr, usedPublic)) return false;
    QualType LType = SubExpr->getType();
    //if (LType.isNull()) return false;
    if (LType.isVoidType()) {
        Diag(unaryop->getOpLoc(), diag::err_typecheck_unary_expr) << "'void'" << SubExpr->getSourceRange();
        return false;
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
    {
        QualType Q = ast.getContext().getPointerType(LType.getCanonicalType());
        expr->setType(Q);
        expr->setConstant();
        break;
    }
    case UO_Deref:
        if (!LType.isPointerType()) {
            char typeName[MAX_LEN_TYPENAME];
            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
            LType.DiagName(buf);
            Diag(unaryop->getOpLoc(), diag::err_typecheck_indirection_requires_pointer)
                    << buf;
            return false;
        } else {
            // TEMP use CanonicalType to avoid Unresolved types etc
            QualType Q = LType.getCanonicalType();
            const PointerType* P = cast<PointerType>(Q);
            expr->setType(P->getPointeeType());
        }
        break;
    case UO_Minus:
    case UO_Not:
        unaryop->setCTC(SubExpr->getCTC());
        if (SubExpr->isConstant()) unaryop->setConstant();
        expr->setType(AnalyserUtils::UsualUnaryConversions(SubExpr));
        break;
    case UO_LNot:
        // TODO first cast expr to bool, then invert here, return type bool
        // TODO extract to function
        // TODO check conversion to bool here!!
        unaryop->setCTC(SubExpr->getCTC());
        // Also set type?
        //AnalyserUtils::UsualUnaryConversions(SubExpr);
        LType = Type::Bool();
        expr->setType(LType);
        break;
    }
    return true;
}

bool FileAnalyser::analyseMemberExpr(Expr* expr, bool usedPublic) {
    LOG_FUNC
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

    if (!analyseExpr(M->getBase(), usedPublic)) return false;
    QualType LType = M->getBase()->getType();

    if (isa<ModuleType>(LType)) {
        M->setModulePrefix();
        ModuleType* MT = cast<ModuleType>(LType.getTypePtr());
        Decl* D = scope->findSymbolInModule(member->getName(), member->getLocation(), MT->getModule());
        if (D) {
            D->setUsed();
            M->setDecl(D);
            AnalyserUtils::SetConstantFlags(D, M);
            QualType Q = D->getType();
            expr->setType(Q);
            member->setType(Q);
            member->setDecl(D, globalDecl2RefKind(D));
            return true;
        }
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
            return false;
        }
        if (isTop(ETD) && !ECD->isChecked()) {
            if (ECD->getCheckState() == CHECK_IN_PROGRESS) {
                Diag(member->getLocation(), diag::err_var_self_init) << ECD->getName();
                return QualType();
            } else { // CHECK_UNCHECKED
                printf("ERROR: using un-initialized enum constant\n");
                // TEMP TODO correct error
                Diag(member->getLocation(), diag::err_var_self_init) << ECD->getName();
                return false;
            }
        }

        QualType Q = ETD->getType();
        M->setDecl(ECD);
        M->setType(Q);
        M->setIsEnumConstant();
        AnalyserUtils::SetConstantFlags(ECD, M);
        member->setCTC(CTC_FULL);
        member->setConstant();
        member->setType(Q);
        member->setDecl(ECD, globalDecl2RefKind(ECD));
        return true;
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
            return false;
        }
        return analyseStructMember(S, M, AnalyserUtils::exprIsType(M->getBase()));
    }
    return false;
}

bool FileAnalyser::outputStructDiagnostics(QualType T, IdentifierExpr* member, unsigned msg)
{
    char temp1[MAX_LEN_TYPENAME];
    StringBuilder buf1(MAX_LEN_TYPENAME, temp1);
    T.DiagName(buf1);
    char temp2[MAX_LEN_VARNAME];
    StringBuilder buf2(MAX_LEN_VARNAME, temp2);
    buf2 << '\'' << member->getName() << '\'';
    Diag(member->getLocation(), msg) << temp2 << temp1;
    return false;
}

bool FileAnalyser::analyseStaticStructMember(QualType T, MemberExpr* M, const StructTypeDecl* S)
{
    LOG_FUNC
    IdentifierExpr* member = M->getMember();

    Decl *field = S->findMember(member->getName());
    FunctionDecl* sfunc = S->findFunction(member->getName());

    Decl* d = field ? field : sfunc;
    if (!d) return outputStructDiagnostics(T, member, diag::err_no_member_struct_func);

    scope->checkAccess(d, member->getLocation());
    d->setUsed();
    M->setDecl(d);
    M->setType(d->getType());
    if (field) {
        assert(field->isChecked());
        member->setDecl(d, IdentifierExpr::REF_STRUCT_MEMBER);
    }
    else {
        if (!analyseDecl(sfunc)) return false;
        member->setDecl(d, IdentifierExpr::REF_STRUCT_FUNC);
        M->setIsStructFunction();
        if (sfunc->isStaticStructFunc()) M->setIsStaticStructFunction();
    }
    member->setType(d->getType());
    AnalyserUtils::SetConstantFlags(d, M);

    // TEMP just declare here
    bool allowStaticMember = false;
    if (!allowStaticMember) {
        if (sfunc) {
            // TBD: allow non-static use of struct functions?
            //if (!sfunc->isStaticStructFunc()) return outputStructDiagnostics(T, member, diag::err_invalid_use_nonstatic_struct_func);
        } else {
            return outputStructDiagnostics(T, member, diag::err_static_use_nonstatic_member);
        }
    }
    return true;
}

// T is the type of the struct
// M the whole expression
// isStatic is true for Foo.myFunction(...)
bool FileAnalyser::analyseStructMember(QualType T, MemberExpr* M, bool isStatic) {
    LOG_FUNC
    assert(M && "Expression missing");
    const StructType* ST = cast<StructType>(T);
    const StructTypeDecl* S = ST->getDecl();

    if (isStatic) return analyseStaticStructMember(T, M, S);

    IdentifierExpr* member = M->getMember();
    Decl* match = S->find(member->getName());

    if (!match) {
        return outputStructDiagnostics(T, member, diag::err_no_member_struct_func);
    }


    IdentifierExpr::RefKind ref = IdentifierExpr::REF_STRUCT_MEMBER;
    FunctionDecl* func = dyncast<FunctionDecl>(match);

    if (func) {
        // cannot happen at global scope?
        // TODO recurse into function as well
        scope->checkAccess(func, member->getLocation());

        if (func->isStaticStructFunc()) {
            Diag(member->getLocation(), diag::err_static_struct_func_notype);// << func->DiagName();
            return false;
        }
        M->setIsStructFunction();
        ref = IdentifierExpr::REF_STRUCT_FUNC;

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
            return false;
        }
    }

    match->setUsed();
    M->setDecl(match);
    M->setType(match->getType());
    member->setDecl(match, ref);
    member->setType(match->getType());
    return true;
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

    //printf("BEFORE %s\n", D->getName());
    //D->getType().dump();
    QualType Q = analyseType(D->getType(), D->getLocation(), D->isPublic(), true);
    //printf("AFTER %s\n", D->getName());
    //D->getType().dump();

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
        FD->setModule(FTD->getModule());
        if (!analyseDecl(FD)) return false;
        break;
    }
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
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
        Diags.Report(D->getLocation(), diag::err_var_casing);
        return false;
    }

    for (unsigned i=0; i<D->numMembers(); i++) {
        Decl* M = D->getMember(i);
        if (isa<VarDecl>(M)) {
            VarDecl* V = cast<VarDecl>(M);
            assert(V->getInitValue() == 0);
            // NOTE: dont push to stack, because can self-ref to StructType
            V->setCheckState(CHECK_IN_PROGRESS);   // manually set
            if (!analyseVarDecl(V)) return false;
            V->setCheckState(CHECK_DONE);   // manually set
        }
        if (isa<StructTypeDecl>(M)) {
            if (!analyseStructTypeDecl(cast<StructTypeDecl>(M))) return false;
        }
    }
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
        ok |= analyseInitExpr(values[i], ET, usedPublic);
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
        // TODO BB remove, just give error if not constant (already given?)
        if (!values[i]->isConstant()) constant = false;
    }
    if (!ok) return false;

    if (constant) expr->setConstant();
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
    assert(STD->isStruct() && "TEMP only support structs for now");
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
            Diag(values[STD->numMembers()]->getLocation(), diag::err_excess_initializers)
                << 4;
            return false;
        }
        DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(values[i]);
        if (!D) {
            VarDecl* VD = dyncast<VarDecl>(STD->getMember(i));
            if (!VD) {
                TODO;
                return false;
            }
            analyseInitExpr(values[i], VD->getType(), usedPublic);
            if (!values[i]->isConstant()) constant = false;
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
    if (constant) expr->setConstant();
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
        field->setDecl(anonUnionDecl, globalDecl2RefKind(anonUnionDecl));
        VD = dyncast<VarDecl>(anonUnionDecl->getMember((unsigned)anonUnionIndex));
    } else {
        VD = dyncast<VarDecl>(STD->getMember((unsigned)memberIndex));
    }
    if (!VD) {
        TODO;
        assert(VD && "TEMP don't support sub-struct member inits");
    }
    field->setDecl(VD, globalDecl2RefKind(VD));
    analyseInitExpr(D->getInitValue(), VD->getType(), usedPublic);
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
        Expr* Desig = D->getDesignator();
        if (!analyseExpr(Desig, usedPublic)) return false;;

        if (!Desig->isConstant()) {
            Diag(Desig->getLocation(), diag::err_init_element_not_constant) << Desig->getSourceRange();
            return false;
        }
        if (!Desig->getType().isIntegerType()) {
            Diag(Desig->getLocation(), diag::err_typecheck_subscript_not_integer) << Desig->getSourceRange();
            return false;
        }
        LiteralAnalyser LA(Diags);
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
    return analyseInitExpr(D->getInitValue(), expectedType, usedPublic);
    //if (D->getInitValue()->isConstant()) D->setConstant();
}

bool FileAnalyser::analyseInitExpr(Expr* expr, QualType expectedType, bool usedPublic) {
    LOG_FUNC
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

    if (!analyseExpr(expr, usedPublic)) return false;

    if (AT) {
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
        EA.check(expectedType, expr);
        if (EA.hasError()) return false;
    }

    if (!expr->isConstant()) {
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
            resolved = ast.getContext().getPointerType(Result);
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
            if (!analyseExpr(sizeExpr, usedPublic)) return QualType();
            LiteralAnalyser LA(Diags);
            APSInt value = LA.checkLiterals(sizeExpr);
            if (value.isSigned() && value.isNegative()) {
                Diag(sizeExpr->getLocation(), diag::err_typecheck_negative_array_size) << value.toString(10) << sizeExpr->getSourceRange();
                return QualType();
            }
            // TODO still needed?
            if (sizeExpr->getCTC() != CTC_FULL) {
                Diag(sizeExpr->getLocation(), diag::err_array_size_non_const);
                return QualType();
            }
            AT->setSize(value);
        }
        if (ET == AT->getElementType()) {
            resolved = Q;
            Q->setCanonicalType(Q);
        } else {
            resolved = ast.getContext().getArrayType(ET, AT->getSizeExpr(), AT->isIncremental());
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
    assert(structDecl);

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
    if (ast.isInterface()) return;
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

