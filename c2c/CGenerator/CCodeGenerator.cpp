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

#include <vector>
#include <set>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
// for tool_output_file
//#include <llvm/Support/ToolOutputFile.h>

#include "CGenerator/CCodeGenerator.h"
#include "CGenerator/HeaderNamer.h"
#include "AST/AST.h"
#include "AST/Attr.h"
#include "AST/Stmt.h"
#include "AST/Type.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "FileUtils/FileUtils.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"
#include "Utils/GenUtils.h"
#include "Utils/TargetInfo.h"
#include "Utils/Utils.h"

//#define CCODE_DEBUG

#ifdef CCODE_DEBUG
#include "Utils/color.h"
#include <iostream>
#define LOG_FUNC std::cerr << ANSI_BLUE << __func__ << "()" << ANSI_NORMAL << "\n";
#define LOG_DECL(_d) std::cerr << ANSI_BLUE << __func__ << "() " << ANSI_YELLOW  << _d->getName() << ANSI_NORMAL << "\n";
#define LOG_MSG(_msg) std::cerr << ANSI_BLUE << __func__ << "() " << ANSI_YELLOW  << _msg << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#define LOG_DECL(_d)
#define LOG_MSG(_msg)
#endif


using namespace C2;
using namespace llvm;
using namespace c2lang;

CCodeGenerator::CCodeGenerator(const std::string& filename_,
                               Mode mode_,
                               const Modules& modules_,
                               const ModuleList& mods_,
                               const HeaderNamer& namer_,
                               const TargetInfo& targetInfo_,
                               bool genChecks_,
                               bool genAsserts_)
    : filename(filename_)
    , mode(mode_)
    , inInterface(false)
    , generateChecks(genChecks_)
    , generateAsserts(genAsserts_)
    , modules(modules_)
    , mods(mods_)
    , headerNamer(namer_)
    , targetInfo(targetInfo_)
{
    hfilename = filename + ".h";
    cfilename = filename + ".c";
}

CCodeGenerator::~CCodeGenerator() {
}

void CCodeGenerator::generate(bool printCode, const std::string& outputDir) {
    inInterface = false;
    EmitAll();

    if (printCode) {
        if (mode != SINGLE_FILE) {
            printf("---- code for %s%s ----\n%s\n", outputDir.c_str(), hfilename.c_str(), (const char*)hbuf);
        }
        printf("---- code for %s%s ----\n%s\n", outputDir.c_str(), cfilename.c_str(), (const char*)cbuf);
    }
    switch (mode) {
    case MULTI_FILE:
        FileUtils::writeFile(outputDir.c_str(), outputDir + hfilename, hbuf);
        break;
    case SINGLE_FILE:
        break;
    }
    FileUtils::writeFile(outputDir.c_str(), outputDir + cfilename, cbuf);

    StringBuilder out;
    out << "#ifndef C2_TYPES_H\n";
    out << "#define C2_TYPES_H\n";
    out << '\n';
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";
    out << "#define NULL ((void*)0)\n";
    // NOTE: might change for 32 bit?
    out << "#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)\n";
    out << "#define to_container(type, member, ptr) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))\n";


    out << '\n';
    // NOTE: 64-bit only for now
    out << "typedef signed char int8_t;\n";
    out << "typedef unsigned char uint8_t;\n";
    out << "typedef signed short int int16_t;\n";
    out << "typedef unsigned short int uint16_t;\n";
    out << "typedef signed int int32_t;\n";
    out << "typedef unsigned int uint32_t;\n";
    out << "typedef signed long int64_t;\n";
    out << "typedef unsigned long uint64_t;\n";
    out << '\n';
    out << "#ifdef __cplusplus\n}\n#endif\n\n";
    out << "#endif\n";
    FileUtils::writeFile(outputDir.c_str(), outputDir + "c2types.h", out);
}

void CCodeGenerator::EmitAll() {
    LOG_FUNC
    EmitIncludeGuard();
    EmitIncludes();

    LOG_MSG("constants");
    hbuf << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";
    for (unsigned m=0; m<mods.size(); m++) {
        const AstList& files = mods[m]->getFiles();
        for (unsigned a=0; a<files.size(); a++) {
            const AST* ast = files[a];
            for (unsigned i=0; i<ast->numVars(); i++) {
                EmitConstant(ast->getVar(i));
            }
        }
    }

    // generate types, reorder and do forward decls if needed
    LOG_MSG("types");
    TypeSorter sorter;
    for (unsigned m=0; m<mods.size(); m++) {
        const AstList& files = mods[m]->getFiles();
        for (unsigned a=0; a<files.size(); a++) {
            const AST* ast = files[a];
            for (unsigned i=0; i<ast->numTypes(); i++) {
                sorter.add(ast->getType(i));
            }
        }
    }
    sorter.write(*this);

    LOG_MSG("function protos");
    // generate function prototypes
    for (unsigned m=0; m<mods.size(); m++) {
        const AstList& files = mods[m]->getFiles();
        for (unsigned a=0; a<files.size(); a++) {
            const AST* ast = files[a];
            for (unsigned i=0; i<ast->numFunctions(); i++) {
                EmitFunctionForward(ast->getFunction(i));
            }
        }
    }
    cbuf << '\n';
    hbuf << '\n';

    // generate variables
    LOG_MSG("variables");
    for (unsigned m=0; m<mods.size(); m++) {
        const Module* M = mods[m];
        for (unsigned i=0; i<M->numSortedVar(); i++) {
            EmitGlobalVariable(M->getSortedVar(i));
        }
    }
    // TODO Arrayvalues

    if (!inInterface) {
        // generate functions
        for (unsigned m=0; m<mods.size(); m++) {
            const AstList& files = mods[m]->getFiles();
            for (unsigned a=0; a<files.size(); a++) {
                const AST* ast = files[a];
                for (unsigned i=0; i<ast->numFunctions(); i++) {
                    EmitFunction(ast->getFunction(i));
                }
            }
        }
    }

    hbuf << "#ifdef __cplusplus\n}\n#endif\n\n";

    // emit end of include guard
    hbuf << "#endif\n";
}

void CCodeGenerator::createLibHeader(bool printCode, const std::string& outputDir) {
    inInterface = true;
    // workaround for builtin va_list, va_start, va_end for varargs
    if (filename == "stdarg") {
        EmitIncludeGuard();
        hbuf << "#include \"c2.h\"\n";
        hbuf << '\n';
        hbuf << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";

        hbuf << "#define va_list __builtin_va_list\n";
        hbuf << "#define va_start __builtin_va_start\n";
        hbuf << "#define va_end __builtin_va_end\n";
        hbuf << '\n';

        hbuf << "int32_t vdprintf(int32_t __fd, const char* __fmt, va_list __arg);\n";
        hbuf << "int32_t vsprintf(char* str, const char* format, va_list __ap);\n";
        hbuf << "int32_t vsnprintf(char* str, uint64_t size, const char* format, va_list __ap);\n";
        hbuf << '\n';

        hbuf << "#ifdef __cplusplus\n}\n#endif\n\n";
        hbuf << "#endif\n";
    } else {
        EmitAll();
    }

    if (printCode) {
        printf("---- code for %s%s ----\n%s\n", outputDir.c_str(), hfilename.c_str(), (const char*)hbuf);
    }

    FileUtils::writeFile(outputDir.c_str(), outputDir + hfilename, hbuf);
}

void CCodeGenerator::EmitExpr(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    switch (E->getKind()) {
    case EXPR_INTEGER_LITERAL:
    {
        const IntegerLiteral* N = cast<IntegerLiteral>(E);
        // TODO FIX handle uint64_t values correctly (use getZExtValue)
        output.number(N->getRadix(), N->Value.getSExtValue());
        return;
    }
    case EXPR_FLOAT_LITERAL:
    {
        const FloatingLiteral* F = cast<FloatingLiteral>(E);
        char temp[20];
        sprintf(temp, "%f", F->Value.convertToFloat());
        output << temp;
        return;
    }
    case EXPR_BOOL_LITERAL:
    {
        const BooleanLiteral* B = cast<BooleanLiteral>(E);
        output << (int)B->getValue();
        return;
    }
    case EXPR_CHAR_LITERAL:
    {
        const CharacterLiteral* C = cast<CharacterLiteral>(E);
        C->printLiteral(output);
        return;
    }
    case EXPR_STRING_LITERAL:
    {
        const StringLiteral* S = cast<StringLiteral>(E);
        S->printLiteral(output);
        return;
    }
    case EXPR_NIL:
        output << "NULL";
        return;
    case EXPR_CALL:
        EmitCallExpr(E, output);
        return;
    case EXPR_IDENTIFIER:
        EmitIdentifierExpr(E, output);
        return;
    case EXPR_INITLIST:
    {
        const InitListExpr* I = cast<InitListExpr>(E);
        output << "{ ";
        Expr** values = I->getValues();
        const unsigned numValues = I->numValues();
        for (unsigned i=0; i<numValues; i++) {
            if (i == 0 && values[0]->getKind() == EXPR_INITLIST) output << '\n';
            EmitExpr(values[i], output);
            if (i != numValues -1) output << ", ";
            if (values[i]->getKind() == EXPR_INITLIST) output << '\n';
        }
        output << " }";
        return;
    }
    case EXPR_DESIGNATOR_INIT:
    {
        const DesignatedInitExpr* D = cast<DesignatedInitExpr>(E);
        if (D->getDesignatorKind() == DesignatedInitExpr::ARRAY_DESIGNATOR) {
            output << '[';
            EmitExpr(D->getDesignator(), output);
            output << "] = ";
        } else {
            const IdentifierExpr* field = D->getField();
            output << '.' << field->getName() << " = ";
        }
        EmitExpr(D->getInitValue(), output);
        return;
    }
    case EXPR_TYPE:
    {
        const TypeExpr* T = cast<TypeExpr>(E);
        EmitTypePreName(T->getType(), output);
        EmitTypePostName(T->getType(), output);
        return;
    }
    case EXPR_BINOP:
        EmitBinaryOperator(E, output);
        return;
    case EXPR_CONDOP:
        EmitConditionalOperator(E, output);
        return;
    case EXPR_UNARYOP:
        EmitUnaryOperator(E, output);
        return;
    case EXPR_BUILTIN:
        EmitBuiltinExpr(E, output);
        return;
    case EXPR_ARRAYSUBSCRIPT:
    {
        const ArraySubscriptExpr* A = cast<ArraySubscriptExpr>(E);
        if (isa<BitOffsetExpr>(A->getIndex())) {
            EmitBitOffsetExpr(A->getBase(), A->getIndex(), output);
        } else {
            EmitExpr(A->getBase(), output);
            output << '[';
            EmitExpr(A->getIndex(), output);
            output << ']';
        }
        return;
    }
    case EXPR_MEMBER:
        EmitMemberExpr(E, output);
        return;
    case EXPR_PAREN:
    {
        const ParenExpr* P = cast<ParenExpr>(E);
        output << '(';
        EmitExpr(P->getExpr(), output);
        output << ')';
        return;
    }
    case EXPR_BITOFFSET:
        FATAL_ERROR("should not happen");
        break;
    case EXPR_EXPLICIT_CAST:
    {
        const ExplicitCastExpr* ECE = cast<ExplicitCastExpr>(E);
        output << '(';
        EmitTypePreName(ECE->getType(), output);
        EmitTypePostName(ECE->getType(), output);
        output << ")(";
        EmitExpr(ECE->getInner(), output);
        output << ')';
        return;
    }
    case EXPR_IMPLICIT_CAST: {
        const ImplicitCastExpr* IC = cast<ImplicitCastExpr>(E);
        EmitExpr(IC->getInner(), output);
        break;
    }
    }
}

void CCodeGenerator::EmitBuiltinExpr(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const BuiltinExpr* B = cast<BuiltinExpr>(E);
    switch (B->getBuiltinKind()) {
    case BuiltinExpr::BUILTIN_SIZEOF:
        // We need to know the ARCH (32/64 bit to generate the sizeof correctly)
        if (B->getValue().getZExtValue() > 0) {
            output << B->getValue().toString(10);
        } else {
            output << "sizeof(";
            EmitExpr(B->getExpr(), output);
            output << ')';
        }
        break;
    case BuiltinExpr::BUILTIN_ELEMSOF:
    case BuiltinExpr::BUILTIN_ENUM_MIN:
    case BuiltinExpr::BUILTIN_ENUM_MAX:
        output.number(10, B->getValue().getSExtValue());
        break;
    case BuiltinExpr::BUILTIN_OFFSETOF:
        output << B->getValue().toString(10);
        break;
    case BuiltinExpr::BUILTIN_TO_CONTAINER:
        output << "to_container(";
        EmitExpr(B->getExpr(), output);
        output << ", ";
        EmitExpr(B->getMember(), output);
        output << ", ";
        EmitExpr(B->getPointer(), output);
        output << ')';
        break;
    }
}

void CCodeGenerator::EmitBinaryOperator(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const BinaryOperator* B = cast<BinaryOperator>(E);
    bool requiresParens = B->requiresParensForC();
    if (requiresParens) output << '(';
    EmitExpr(B->getLHS(), output);
    output << ' ' << BinaryOperator::OpCode2str(B->getOpcode()) << ' ';
    EmitExpr(B->getRHS(), output);
    if (requiresParens) output << ')';
}

void CCodeGenerator::EmitConditionalOperator(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const ConditionalOperator* C = cast<ConditionalOperator>(E);
    EmitExpr(C->getCond(), output);
    output << " ? ";
    EmitExpr(C->getLHS(), output);
    output << " : ";
    EmitExpr(C->getRHS(), output);

}

void CCodeGenerator::EmitUnaryOperator(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const UnaryOperator* U = cast<UnaryOperator>(E);

    switch (U->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
        EmitExpr(U->getExpr(), output);
        output << UnaryOperator::OpCode2str(U->getOpcode());
        break;
    case UO_PreInc:
    case UO_PreDec:
    case UO_AddrOf:
    case UO_Deref:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        //output.indent(indent);
        output << UnaryOperator::OpCode2str(U->getOpcode());
        EmitExpr(U->getExpr(), output);
        break;
    }
}

void CCodeGenerator::EmitMemberExpr(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const MemberExpr* M = cast<MemberExpr>(E);
    //TODO rewrite to switch?
    const IdentifierExpr* rhs = M->getMember();
    if (M->isModulePrefix() || M->isStructFunction()) {
        // A.B where A is a module
        // A.B where B is struct-function
        EmitDecl(M->getDecl(), output);
    } else if (M->isEnumConstant()) {
        const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(M->getDecl());
        assert(ECD);
        const EnumTypeDecl* ETD = ECD->getTypeDecl();
        assert(ETD);
        EmitEnumConstant(cast<EnumConstantDecl>(M->getDecl()), ETD->getName(), output);
    } else {
        // A.B where A is decl of struct/union type
        EmitExpr(M->getBase(), cbuf);
        QualType LType = M->getBase()->getType();
        if (LType.isPointerType()) cbuf << "->";
        else cbuf << '.';
        cbuf << rhs->getName();
    }
}

void CCodeGenerator::EmitCallExpr(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const CallExpr* C = cast<CallExpr>(E);
    const Expr* F = stripImplicitCast(C->getFn());
    const bool isSF = C->isStructFunction();

    bool hasArg = false;
    if (isSF) {
        assert(isa<MemberExpr>(F));
        const MemberExpr* M = cast<MemberExpr>(F);
        assert(M->getDecl());
        EmitDecl(M->getDecl(), output);
        output << '(';

        if (!M->isStaticStructFunction()) {
            QualType arg1Type = M->getBase()->getType();
            if (!arg1Type.isPointerType()) output << '&';
            EmitExpr(M->getBase(), output);
            hasArg = true;
        }
    } else {
        EmitExpr(F, output);
        output << '(';
    }
    for (unsigned i=0; i<C->numArgs(); i++) {
        if (i != 0 || hasArg) output << ", ";
        EmitExpr(C->getArg(i), output);
    }
    const FunctionType* FT = cast<FunctionType>(F->getType());
    const FunctionDecl* func = FT->getDecl();
    unsigned callArgs = C->numArgs() + (hasArg ? 1 : 0);
    // generate default arguments in call
    if (callArgs < func->numArgs()) {
        for (unsigned i=callArgs; i<func->numArgs(); i++) {
            if (i != 0 || hasArg) output << ", ";
            VarDecl* arg = func->getArg(i);
            assert(arg->getInitValue());
            EmitExpr(arg->getInitValue(), output);
        }
    }
    output << ')';
}

void CCodeGenerator::EmitIdentifierExpr(const Expr* E, StringBuilder& output) {
    LOG_FUNC
    const IdentifierExpr* I = cast<IdentifierExpr>(E);
    EmitDecl(I->getDecl(), output);
}

static void bitmask(unsigned width, StringBuilder& output) {
    char tmp[20];
    sprintf(tmp, "0x%" PRIX64"", Utils::bitmask(width));
    output << tmp;
}

void CCodeGenerator::EmitBitOffsetExpr(const Expr* Base, Expr* E, StringBuilder& output) {
    LOG_FUNC
    // NOTE: only support RHS for now!
    // a[7:4] -> ((a >> 4) & 0xF);
    const BitOffsetExpr* B = cast<BitOffsetExpr>(E);
    assert(B->getLHS()->isCTV() && "only support constant bitoffset for now");
    assert(B->getRHS()->isCTV() && "only support constant bitoffset for now");
    assert(B->getWidth() != 0);
    output << "((";
    EmitExpr(Base, output);
    output << " >> ";
    EmitExpr(B->getRHS(), output);
    output << ") & ";
    bitmask(B->getWidth(), output);
    output << ')';
}

static const char* getCName(const Decl* D) {
    if (!D->hasCName()) return D->getName();

    const AttrList& AL = D->getAttributes();
    for (AttrListConstIter iter = AL.begin(); iter != AL.end(); ++iter) {
        const Attr* A = *iter;
        const Expr* Arg = A->getArg();
        if (A->getKind() == ATTR_CNAME) {
            const C2::StringLiteral* S = cast<C2::StringLiteral>(Arg);
            return S->getValue();
        }
    }
    // should not come here
    return D->getName();
}

void CCodeGenerator::EmitDecl(const Decl* D, StringBuilder& output) {
    assert(D);

    if (D->getModule()) {
        const FunctionDecl* F = dyncast<FunctionDecl>(D);
        if (F && F->isStructFunction() && !D->hasCName()) {
            StringBuilder cname(128);
            const IdentifierExpr* structName = F->getStructName();
            cname << structName->getName() << '_' << F->getMemberName();
            GenUtils::addName(D->getModule()->getCName(), cname.c_str(), output);
        } else if (isa<EnumConstantDecl>(D)) {
            const EnumType* et = cast<EnumType>(D->getType());
            const EnumTypeDecl* etd = et->getDecl();
            StringBuilder cname(128);
            cname << etd->getName() << '_' << D->getName();
            GenUtils::addName(D->getModule()->getCName(), cname.c_str(), output);
        } else {
            const char* cname = getCName(D);
            GenUtils::addName(D->getModule()->getCName(), cname, output);
        }
    } else {
        output << D->getName();
    }
}

void CCodeGenerator::EmitStructDecl(const StructTypeDecl* D, StringBuilder& output) {
    assert(D->getModule());
    if (!D->hasTypedef()) {
        output << (D->isStruct() ? "struct " : "union ");
    }
    const char* cname = getCName(D);
    GenUtils::addName(D->getModule()->getCName(), cname, output);
}

void CCodeGenerator::EmitEnumConstant(const EnumConstantDecl* D, const char* typeName, StringBuilder& output) {
    output << D->getModule()->getCName() << '_' << typeName << '_' << D->getName();
}

void CCodeGenerator::forwardDecl(const Decl* D) {
    EmitForwardTypeDecl(cast<TypeDecl>(D));
}

void CCodeGenerator::fullDecl(const Decl* D) {
    EmitTypeDecl(cast<TypeDecl>(D));
}

void CCodeGenerator::EmitIncludeGuard() {
    LOG_FUNC
    static const char* warning = "// WARNING: this file is auto-generated by the C2 compiler.\n// Any changes you make might be lost!\n\n";
    hbuf << warning;
    cbuf << warning;
    hbuf << "#ifndef ";
    GenUtils::toCapital(filename, hbuf);
    hbuf << "_H\n";
    hbuf << "#define ";
    GenUtils::toCapital(filename, hbuf);
    hbuf << "_H\n";
    hbuf << '\n';
}

struct IncludeEntry {
    std::string name;
    bool isSystem;
    bool usedPublic;
};
typedef std::vector<IncludeEntry> Includes;

static void addInclude(Includes& includes, IncludeEntry& ie) {
    IncludeEntry* existing = 0;
    for (unsigned i=0; i<includes.size(); i++) {
        IncludeEntry* cur = &includes[i];
        if (cur->name == ie.name) {
            existing = cur;
            break;
        }
    }
    if (existing) {
        existing->usedPublic |= ie.usedPublic;   // possibly upgrade non-public to public
    } else {
        includes.push_back(ie);
    }
}

void CCodeGenerator::EmitIncludes() {
    LOG_FUNC
    {
        StringBuilder* out = &hbuf;
        if (mode != MULTI_FILE) out = &cbuf;
        (*out) << "#include \"c2types.h\"\n";
    }
    if (generateAsserts) cbuf << "#include <assert.h>\n";

    Includes includes;

    // filter out unique entries, split into system and local includes and .c/.h
    for (unsigned m=0; m<mods.size(); m++) {
        Module* cur = mods[m];
        const AstList& files = cur->getFiles();
        for (unsigned a=0; a<files.size(); a++) {
            const AST* ast = files[a];
            for (unsigned i=0; i<ast->numImports(); i++) {
                const ImportDecl* D = ast->getImport(i);
                ModulesConstIter iter = modules.find(D->getModuleName());
                assert(iter != modules.end());
                const Module* M = iter->second;
                if (M == cur) continue; // never include self
                IncludeEntry ie;
                ie.isSystem = false;
                ie.usedPublic = D->isUsedPublic();
                if (M->isPlainC()) {
                    ie.name = headerNamer.getIncludeName(M->getName());
                    ie.isSystem = true;
                    addInclude(includes, ie);
                } else if (mode == MULTI_FILE) {
                    ie.name = M->getName();
                    addInclude(includes, ie);
                }
            }
        }
        // always add include to own header
        if (mode == MULTI_FILE) {
            IncludeEntry ie = { cur->getName() };
            ie.name = cur->getName();
            ie.isSystem = false;
            ie.usedPublic = false;
            addInclude(includes, ie);
        }
    }

    for (unsigned i=0; i<includes.size(); ++i) {
        const IncludeEntry& entry = includes[i];
        StringBuilder* out = &cbuf;
        // dont put in header if no header is generated

        if (mode != SINGLE_FILE && entry.usedPublic) out = &hbuf;
        (*out) << "#include \"" << entry.name;
        if (!entry.isSystem) (*out) << ".h";
        (*out) << "\"\n";

    }
    hbuf << '\n';
    cbuf << '\n';
}

void CCodeGenerator::EmitFunctionForward(const FunctionDecl* F) {
    LOG_DECL(F)

    if (strcmp(F->getName(), "main") == 0) return;

    StringBuilder* out = &cbuf;
    if (mode != SINGLE_FILE && F->isPublic()) out = &hbuf;
    bool hasAttr = EmitAttributes(F, *out, false);
    if (hasAttr) (*out) << ' ';
    EmitFunctionProto(F, *out);
    (*out) << ";\n";
}

void CCodeGenerator::EmitFunction(const FunctionDecl* F) {
    LOG_DECL(F)

    bool hasAttr = EmitAttributes(F, cbuf, false);
    if (hasAttr) cbuf << ' ';
    EmitFunctionProto(F, cbuf);
    cbuf << ' ';
    EmitCompoundStmt(F->getBody(), 0, false);
    cbuf << '\n';
}

void CCodeGenerator::EmitFunctionArgs(const FunctionDecl* F, StringBuilder& output) {
    LOG_DECL(F)
    output << '(';
    int count = F->numArgs();
    if (F->isVariadic()) count++;
    for (unsigned i=0; i<F->numArgs(); i++) {
        VarDecl* A = F->getArg(i);
        EmitArgVarDecl(A, output, i);
        if (count != 1) output << ", ";
        count--;
    }
    if (F->isVariadic()) output << "...";
    if (F->numArgs() == 0 && !F->isVariadic()) output << "void";
    output << ')';
}

void CCodeGenerator::EmitConstant(const VarDecl* V) {
    LOG_DECL(V)
    if (!EmitAsDefine(V)) return;

    // convert const integer literals to define
    // convert const char[] x = ".." and const char[x] y = .."" to define
    StringBuilder* out = &cbuf;
    if (mode != SINGLE_FILE && V->isPublic()) out = &hbuf;
    *out << "#define ";
    EmitDecl(V, *out);
    assert(V->getInitValue());
    *out << ' ';
    EmitExpr(V->getInitValue(), *out);
    *out << "\n";
}

void CCodeGenerator::EmitGlobalVariable(const VarDecl* V) {
    LOG_DECL(V)
    if (EmitAsDefine(V)) return;    // already done in EmitConstant
    const Expr* init = V->getInitValue();

    if (targetInfo.sys == TargetInfo::SYS_DARWIN && filename == "stdio") {
        // Darwing workaround, since it defines stdin/out/err as #define stdin __stdinp, etc
        if (strcmp(V->getName(), "stdin") == 0) {
            hbuf << "#define stdin __stdinp\n";
            hbuf << "extern FILE* __stdinp;\n";
            hbuf << '\n';
            return;
        }
        if (strcmp(V->getName(), "stdout") == 0) {
            hbuf << "#define stdout __stdoutp\n";
            hbuf << "extern FILE* __stdoutp;\n";
            hbuf << '\n';
            return;
        }
        if (strcmp(V->getName(), "stderr") == 0) {
            hbuf << "#define stderr __stderrp\n";
            hbuf << "extern FILE* __stderrp;\n";
            hbuf << '\n';
            return;
        }
    }
    if (mode != SINGLE_FILE && V->isPublic()) {
        // TODO type
        hbuf << "extern ";
        EmitTypePreName(V->getType(), hbuf);
        hbuf << ' ';
        EmitDecl(V, hbuf);
        EmitTypePostName(V->getType(), hbuf);
        // TODO add space if needed (on StringBuilder)
        hbuf << ";\n";
        hbuf << '\n';
    } else {
        if (EmitAsStatic(V)) cbuf << "static ";
    }
    EmitTypePreName(V->getType(), cbuf);
    cbuf << ' ';
    EmitDecl(V, cbuf);
    EmitTypePostName(V->getType(), cbuf);
    EmitAttributes(V, cbuf, true);

    cbuf << " = ";
    if (init) {
        EmitExpr(init, cbuf);
    } else {
        if (V->getType().isPointerType()) {
            cbuf << "NULL";
        } else if (V->getType().isStructType() || V->getType().isArrayType()) {
            cbuf << "{ }";
        } else {
            cbuf << "0";
        }
    }

    cbuf << ";\n";
    cbuf << '\n';
}

void CCodeGenerator::EmitTypeDecl(const TypeDecl* T) {
    LOG_DECL(T)

    StringBuilder* out = &cbuf;
    if (mode != SINGLE_FILE && T->isPublic()) out = &hbuf;
    switch (T->getKind()) {
    case DECL_FUNC:
    case DECL_VAR:
    case DECL_ENUMVALUE:
        FATAL_ERROR("Unreachable");
        break;
    case DECL_ALIASTYPE:
        *out << "typedef ";
        EmitTypePreName(T->getType(), *out);
        *out << ' ';
        EmitDecl(T, *out);
        EmitTypePostName(T->getType(), *out);
        *out << ";\n\n";
        break;
    case DECL_STRUCTTYPE:
        if (T->hasAttribute(ATTR_OPAQUE)) out = &cbuf;
        EmitStructType(cast<StructTypeDecl>(T), *out, 0);
        return;
    case DECL_ENUMTYPE:
        EmitEnumType(cast<EnumTypeDecl>(T), *out);
        return;
    case DECL_FUNCTIONTYPE:
        EmitFunctionType(cast<FunctionTypeDecl>(T), *out);
        return;
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        FATAL_ERROR("Unreachable");
        break;
    }
}

void CCodeGenerator::EmitForwardTypeDecl(const TypeDecl* D) {
    LOG_DECL(D)
    assert(isa<StructTypeDecl>(D));

    const StructTypeDecl* S = cast<StructTypeDecl>(D);

    if (!S->hasTypedef()) return;

    StringBuilder* out = &cbuf;
    if (mode != SINGLE_FILE && D->isPublic()) out = &hbuf;

    // Syntax: typedef struct/union mod_name_ mod_name;
    *out << "typedef ";
    *out << (S->isStruct() ? "struct " : "union ");
    EmitDecl(D, *out);
    *out << "_ ";
    EmitDecl(D, *out);
    *out << ";\n\n";
}

void CCodeGenerator::EmitStructType(const StructTypeDecl* S, StringBuilder& out, unsigned indent) {
    if (S->hasAttribute(ATTR_OPAQUE) && inInterface) return;
    LOG_DECL(S)

    out.indent(indent);
    out << (S->isStruct() ? "struct " : "union ");
    if (!S->hasEmptyName() && S->isGlobal()) {
        EmitDecl(S, out);
        if (S->hasTypedef()) out << '_';  // either generate 'struct stat_' or 'struct stat'
        out << ' ';
    }
    out << "{\n";
    for (unsigned i=0; i<S->numMembers(); i++) {
        Decl* member = S->getMember(i);
        if (isa<VarDecl>(member)) {
            EmitVarDecl(cast<VarDecl>(member), out, indent + INDENT);
            out << ";\n";
        } else if (isa<StructTypeDecl>(member)) {
            EmitStructType(cast<StructTypeDecl>(member), out, indent+INDENT);
        } else {
            FATAL_ERROR("Member is neither struct nor var");
        }
    }
    out.indent(indent);
    out << '}';
    if (!S->hasEmptyName() && !S->isGlobal()) {
        out << ' ';
        EmitDecl(S, out);
    }
    EmitAttributes(S, out, true);
    out << ";\n";

    if (generateChecks && S->isGlobal()) {
        uint64_t size = S->getSize();
        out << "_Static_assert(sizeof(";
        out << (S->isStruct() ? "struct " : "union ");
        EmitDecl(S, out);
        if (S->hasTypedef()) out << '_';
        out << ") == " << size << ", \"sizeof(" << S->getName() << ")\");\n";
    }

    if (S->isGlobal()) out << '\n';
}

void CCodeGenerator::EmitEnumType(const EnumTypeDecl* E, StringBuilder& out) {
    LOG_DECL(E)

    const BuiltinType* BI = cast<BuiltinType>(E->getImplType().getTypePtr());
    unsigned size = BI->getAlignment();

    out << "typedef enum {\n";
    for (unsigned i=0; i<E->numConstants(); i++) {
        EnumConstantDecl* C = E->getConstant(i);
        out.indent(INDENT);
        EmitEnumConstant(C, E->getName(), out);
        if (C->getInitValue()) {
            out << " = ";
            EmitExpr(C->getInitValue(), out);
        }
        out << ",\n";
    }
    // emit max value to force size
    {
        // NOTE: only valid for unsigned implemenation types
        out.indent(INDENT);
        out << '_' << E->getModule()->getCName() << '_' << E->getName() << "_max = ";
        switch (size) {
        case 1:
            out << 255;
            break;
        case 2:
            out << (uint16_t)-1;
            break;
        case 4:
            out << (uint32_t)-1;
            break;
        case 8:
            out << ((uint64_t)-1)/4;    // avoid unsigned 64-bit warnings in GCC
            break;
        }
        out << '\n';
    }
    out << "} __attribute__((packed)) ";
    EmitDecl(E, out);
    EmitAttributes(E, out, true);
    out << ";\n";

    if (generateChecks) {
        out << "_Static_assert(sizeof(";
        EmitDecl(E, out);
        out << ") == " << size << ", \"sizeof(" << E->getName() << ")\");\n";
    }
    out << '\n';
}

// output: typedef void (*name)(args);
void CCodeGenerator::EmitFunctionType(const FunctionTypeDecl* FTD, StringBuilder& output) {
    LOG_DECL(FTD)
    FunctionDecl* F = FTD->getDecl();
    output << "typedef ";
    EmitTypePreName(F->getReturnType(), output);
    EmitTypePostName(F->getReturnType(), output);
    output << " (*";
    EmitDecl(F, output);
    output << ')';
    EmitFunctionArgs(F, output);
    EmitAttributes(F, output, true);
    output << ";\n\n";
}

void CCodeGenerator::EmitArgVarDecl(const VarDecl* D, StringBuilder& output, unsigned index) {
    LOG_DECL(D)
    EmitTypePreName(D->getType(), output);
    output << ' ';
    if (D->hasEmptyName()) {
        output << "_arg" << index;
    } else {
        output << D->getName();
    }
    EmitTypePostName(D->getType(), output);
    // NOTE dont generate init values since C doesn't support default args
}

void CCodeGenerator::EmitVarDecl(const VarDecl* D, StringBuilder& output, unsigned indent) {
    LOG_DECL(D)
    output.indent(indent);
    EmitTypePreName(D->getType(), output);
    output << ' ';
    if (D->isGlobal()) EmitDecl(D, output);
    else output << D->getName();
    EmitTypePostName(D->getType(), output);
    const Expr* bitfield = D->getBitfield();
    if (bitfield) {
        output << " : ";
        EmitExpr(bitfield, output);
    }
    if (D->getInitValue()) {
        output << " = ";
        EmitExpr(D->getInitValue(), output);
    }
}

void CCodeGenerator::EmitStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    switch (S->getKind()) {
    case STMT_RETURN:
    {
        const ReturnStmt* R = cast<ReturnStmt>(S);
        cbuf.indent(indent);
        cbuf << "return";
        if (R->getExpr()) {
            cbuf << ' ';
            EmitExpr(R->getExpr(), cbuf);
        }
        cbuf << ";\n";
        return;
    }
    case STMT_EXPR:
    {
        const Expr* E = cast<Expr>(S);
        cbuf.indent(indent);
        EmitExpr(E, cbuf);
        cbuf << ";\n";
        return;
    }
    case STMT_IF:
        EmitIfStmt(S, indent);
        return;
    case STMT_WHILE:
        EmitWhileStmt(S, indent);
        return;
    case STMT_DO:
        EmitDoStmt(S, indent);
        return;
    case STMT_FOR:
        EmitForStmt(S, indent);
        return;
    case STMT_SWITCH:
        EmitSwitchStmt(S, indent);
        return;
    case STMT_SSWITCH:
        EmitSSwitchStmt(S, indent);
        return;
    case STMT_CASE:
    case STMT_DEFAULT:
        FATAL_ERROR("Should already be generated");
        break;
    case STMT_BREAK:
        cbuf.indent(indent);
        cbuf << "break;\n";
        return;
    case STMT_CONTINUE:
        cbuf.indent(indent);
        cbuf << "continue;\n";
        return;
    case STMT_FALLTHROUGH:
        cbuf.indent(indent);
        cbuf << "__attribute__((fallthrough));\n";
        break;
    case STMT_LABEL:
    {
        const LabelStmt* L = cast<LabelStmt>(S);
        cbuf << L->getName();
        cbuf << ":\n";
        EmitStmt(L->getSubStmt(), indent);
        return;
    }
    case STMT_GOTO:
    {
        const GotoStmt* G = cast<GotoStmt>(S);
        cbuf.indent(indent);
        cbuf << "goto " << G->getLabel()->getName() << ";\n";
        return;
    }
    case STMT_COMPOUND:
        EmitCompoundStmt(cast<CompoundStmt>(S), indent, true);
        return;
    case STMT_DECL:
        EmitDeclStmt(cast<DeclStmt>(S), indent);
        return;
    case STMT_ASM:
        EmitAsmStmt(cast<AsmStmt>(S), indent);
        return;
    case STMT_ASSERT:
        EmitAssertStmt(cast<AssertStmt>(S), indent);
        return;
    }
}

void CCodeGenerator::EmitCompoundStmt(const CompoundStmt* C, unsigned indent, bool startOnNewLine) {
    LOG_FUNC
    if (startOnNewLine) cbuf.indent(indent);
    cbuf << "{\n";
    Stmt** stmts = C->getStmts();
    for (unsigned i=0; i<C->numStmts(); i++) {
        EmitStmt(stmts[i], indent + INDENT);
    }
    cbuf.indent(indent);
    cbuf << "}\n";
}

void CCodeGenerator::EmitIfStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const IfStmt* I = cast<IfStmt>(S);
    EmitConditionPre(I->getCond(), indent);
    cbuf.indent(indent);
    cbuf << "if (";
    EmitConditionPost(I->getCond());
    cbuf << ')';

    if (isa<CompoundStmt>(I->getThen())) {
        cbuf << '\n';
        EmitStmt(I->getThen(), indent);
    } else if (isa<IfStmt>(I->getThen())) {
        cbuf << " {\n";
        EmitStmt(I->getThen(), indent+INDENT);
        cbuf.indent(indent);
        cbuf << "}\n";
    } else {
        cbuf << ' ';
        EmitStmt(I->getThen(), 0);
    }

    if (I->getElse()) {
        cbuf.indent(indent);
        cbuf << "else";
        if (isa<CompoundStmt>(I->getElse())) {
            cbuf << '\n';
            EmitStmt(I->getElse(), indent);
        } else {
            cbuf << ' ';
            EmitStmt(I->getElse(), 0);
        }
    }
}

void CCodeGenerator::EmitWhileStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const WhileStmt* W = cast<WhileStmt>(S);
    EmitConditionPre(W->getCond(), indent);
    cbuf.indent(indent);
    cbuf << "while (";
    EmitConditionPost(W->getCond());
    cbuf << ") ";
    Stmt* Body = W->getBody();
    if (Body->getKind() == STMT_COMPOUND) {
        CompoundStmt* C = cast<CompoundStmt>(Body);
        EmitCompoundStmt(C, indent, false);
    } else {
        EmitStmt(Body, 0);
    }
}

void CCodeGenerator::EmitDoStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const DoStmt* D = cast<DoStmt>(S);
    cbuf.indent(indent);
    cbuf << "do ";
    Stmt* Body = D->getBody();
    if (Body->getKind() == STMT_COMPOUND) {
        CompoundStmt* C = cast<CompoundStmt>(Body);
        EmitCompoundStmt(C, indent, false);
    } else {
        EmitStmt(Body, 0);
    }
    cbuf.indent(indent);
    // TODO add after '}'
    cbuf << "while (";
    // TEMP, assume Expr
    Expr* E = cast<Expr>(D->getCond());
    EmitExpr(E, cbuf);
    cbuf << ");\n";
}

void CCodeGenerator::EmitForStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const ForStmt* F = cast<ForStmt>(S);
    cbuf.indent(indent);
    cbuf << "for (";
    Stmt* Init = F->getInit();
    if (Init) {
        EmitStmt(Init, 0);
        cbuf.strip('\n');
    } else {
        cbuf << ';';
    }

    Expr* Cond = F->getCond();
    if (Cond) {
        cbuf << ' ';
        EmitExpr(Cond, cbuf);
    }
    cbuf << ';';

    Expr* Incr = F->getIncr();
    if (Incr) {
        cbuf << ' ';
        EmitExpr(Incr, cbuf);
    }

    cbuf << ") ";
    Stmt* Body = F->getBody();
    if (Body->getKind() == STMT_COMPOUND) {
        EmitCompoundStmt(cast<CompoundStmt>(Body), indent, false);
    } else {
        EmitStmt(Body, 0);
    }
}

void CCodeGenerator::EmitSwitchStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const SwitchStmt* SW = cast<SwitchStmt>(S);
    EmitConditionPre(SW->getCond(), indent);
    cbuf.indent(indent);
    cbuf << "switch (";
    EmitConditionPost(SW->getCond());
    cbuf << ") {\n";

    Stmt** cases = SW->getCases();
    for (unsigned i=0; i<SW->numCases(); i++) {
        Stmt* Case = cases[i];
        switch (Case->getKind()) {
        case STMT_CASE:
        {
            CaseStmt* C = cast<CaseStmt>(Case);
            cbuf.indent(indent + INDENT);
            cbuf << "case ";
            EmitExpr(C->getCond(), cbuf);
            cbuf << ':';
            if (C->hasDecls()) cbuf << " {";
            cbuf << '\n';
            Stmt** stmts = C->getStmts();
            for (unsigned s=0; s<C->numStmts(); s++) {
                EmitStmt(stmts[s], indent + INDENT + INDENT);
            }
            if (C->hasDecls()) {
                cbuf.indent(indent + INDENT);
                cbuf << "}\n";
            }
            break;
        }
        case STMT_DEFAULT:
        {
            DefaultStmt* D = cast<DefaultStmt>(Case);
            cbuf.indent(indent + INDENT);
            cbuf << "default:";
            if (D->hasDecls()) cbuf << " {";
            cbuf << '\n';
            Stmt** stmts = D->getStmts();
            for (unsigned s=0; s<D->numStmts(); s++) {
                EmitStmt(stmts[s], indent + INDENT + INDENT);
            }
            if (D->hasDecls()) {
                cbuf.indent(indent + INDENT);
                cbuf << "}\n";
            }
            break;
        }
        default:
            FATAL_ERROR("Unreachable");
        }
    }

    cbuf.indent(indent);
    cbuf << "}\n";
}

void CCodeGenerator::EmitSSwitchStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const SSwitchStmt* M = cast<SSwitchStmt>(S);

    cbuf.indent(indent);
    cbuf << "do {\n";
    indent += INDENT;

    // TODO create proper tmp var name (per function)
    cbuf.indent(indent);
    cbuf << "const char* _tmp = ";
    EmitExpr(M->getCond(), cbuf);
    cbuf << ";\n";

    Stmt** cases = M->getCases();
    bool need_end = false;
    for (unsigned i=0; i<M->numCases(); i++) {
        Stmt* Case = cases[i];
        switch (Case->getKind()) {
        case STMT_CASE:
        {
            CaseStmt* C = cast<CaseStmt>(Case);
            cbuf.indent(indent);
            if (i == 0) {
                cbuf << "if (";
            } else {
                cbuf << "} else if (";
            }
            // TODO if argument = NIL, generate (tmp == nil)
            const Expr* cond = C->getCond();
            const StringLiteral* S = dyncast<StringLiteral>(cond);
            if (S) { // string literal
                cbuf << "strcmp(_tmp, ";
                S->printLiteral(cbuf);
                cbuf << ") == 0) {\n";
            } else { // nil
                cbuf << "_tmp == NULL) {\n";
            }
            Stmt** stmts = C->getStmts();
            for (unsigned s=0; s<C->numStmts(); s++) {
                EmitStmt(stmts[s], indent + INDENT);
            }
            need_end = true;
            break;
        }
        case STMT_DEFAULT:
        {
            if (i != 0) {
                cbuf.indent(indent);
                cbuf << "} else {\n";
                need_end = true;
            }
            DefaultStmt* D = cast<DefaultStmt>(Case);
            Stmt** stmts = D->getStmts();
            for (unsigned s=0; s<D->numStmts(); s++) {
                EmitStmt(stmts[s], indent + INDENT + INDENT);
            }
            break;
        }
        default:
            FATAL_ERROR("Unreachable");
        }
    }
    if (need_end) {
        cbuf.indent(indent);
        cbuf << "}\n";
    }
    indent -= INDENT;
    cbuf.indent(indent);
    cbuf << "} while (0);\n";
}

void CCodeGenerator::EmitDeclStmt(const Stmt* S, unsigned indent) {
    LOG_FUNC
    const DeclStmt* DS = cast<DeclStmt>(S);
    const VarDecl* VD = DS->getDecl();
    cbuf.indent(indent);
    if (VD->hasLocalQualifier()) cbuf << "static ";
    EmitTypePreName(VD->getType(), cbuf);
    cbuf << ' ';
    cbuf << VD->getName();
    EmitTypePostName(VD->getType(), cbuf);
    if (VD->getInitValue()) {
        cbuf << " = ";
        EmitExpr(VD->getInitValue(), cbuf);
    }
    cbuf << ";\n";
}

void CCodeGenerator::EmitAsmPart(bool multiline, unsigned indent)
{
    if (multiline) {
        cbuf << '\n';
        cbuf.indent(indent + INDENT*3);
    } else {
        cbuf << ' ';
    }
    cbuf << ": ";
}

void CCodeGenerator::EmitAsmOperand(const char* name, const StringLiteral* c, const Expr* e) {
    assert(c);
    assert(e);
    if (name) {
        cbuf << '[' << name << "] ";
    }
    c->printLiteral(cbuf);
    cbuf << ' ';
    cbuf << '(';
    EmitExpr(e, cbuf);
    cbuf << ')';
}

void CCodeGenerator::EmitAsmStmt(const AsmStmt* S, unsigned indent) {
    LOG_FUNC
    cbuf.indent(indent);
    cbuf << "__asm__ ";
    if (S->isVolatile()) cbuf << "volatile ";

    cbuf << '(';
    const StringLiteral* asmString = S->getString();
    asmString->printLiteral(cbuf);

    unsigned numOutputs = S->getNumOutputs();
    unsigned numInputs = S->getNumInputs();
    unsigned numClobbers = S->getNumClobbers();
    bool multiline = false;
    if (numOutputs + numInputs + numClobbers > 4 || numClobbers != 0) multiline = true;
    if (numOutputs || numInputs || numClobbers) {
        EmitAsmPart(multiline, indent);
        for (unsigned i=0; i<numOutputs; i++) {
            if (i != 0) cbuf << ", ";
            EmitAsmOperand(S->getOutputName(i), S->getOutputConstraint(i), S->getOutputExpr(i));
        }
        // inputs
        if (numInputs || numClobbers) {
            EmitAsmPart(multiline, indent);
            for (unsigned i=0; i<numInputs; i++) {
                if (i != 0) cbuf << ", ";
                EmitAsmOperand(S->getInputName(i), S->getInputConstraint(i), S->getInputExpr(i));
            }
        }
        // clobbers
        if (numClobbers) {
            EmitAsmPart(multiline, indent);
            for (unsigned i=0; i<numClobbers; i++) {
                if (i != 0) cbuf << ", ";
                const StringLiteral* c = S->getClobber(i);
                c->printLiteral(cbuf);
            }
        }
    }
    cbuf << ");\n";
}

void CCodeGenerator::EmitAssertStmt(const AssertStmt* S, unsigned indent) {
    LOG_FUNC
    if (!generateAsserts) return;
    cbuf.indent(indent);
    cbuf << "assert(";
    EmitExpr(S->getExpr(), cbuf);
    cbuf << ");\n";
}

void CCodeGenerator::EmitFunctionProto(const FunctionDecl* F, StringBuilder& output) {
    LOG_FUNC

    if (EmitAsStatic(F)) output << "static ";

    EmitTypePreName(F->getReturnType(), output);
    EmitTypePostName(F->getReturnType(), output);
    output << ' ';
    EmitDecl(F, output);
    EmitFunctionArgs(F, output);
}

static const char* builtin2cname(BuiltinType::Kind kind) {
    switch (kind) {
    case BuiltinType::Int8:     return "char";
    case BuiltinType::Int16:    return "int16_t";
    case BuiltinType::Int32:    return "int32_t";
    case BuiltinType::Int64:    return "int64_t";
    case BuiltinType::UInt8:    return "unsigned char";
    case BuiltinType::UInt16:   return "uint16_t";
    case BuiltinType::UInt32:   return "uint32_t";
    case BuiltinType::UInt64:   return "uint64_t";
    case BuiltinType::Float32:  return "float";
    case BuiltinType::Float64:  return "double";
    case BuiltinType::Bool:     return "char";
    case BuiltinType::Void:     return "void";
    }
    FATAL_ERROR("Unreachable");
    return 0;
}

void CCodeGenerator::EmitTypePreName(QualType type, StringBuilder& output) {
    LOG_FUNC
    if (type.isConstQualified()) output << "const ";
    if (type.isVolatileQualified()) output << "volatile ";
    const Type* T = type.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
    {
        const BuiltinType* BI = cast<BuiltinType>(T);
        output << builtin2cname(BI->getKind());
        break;
    }
    case TC_POINTER:
        EmitTypePreName(cast<PointerType>(T)->getPointeeType(), output);
        output << '*';
        break;
    case TC_ARRAY:
        EmitTypePreName(cast<ArrayType>(T)->getElementType(), output);
        break;
    case TC_REF:
    {
        const RefType* rt = cast<RefType>(T);
        EmitDecl(rt->getDecl(), output);
        break;
    }
    case TC_ALIAS:
    {
        EmitTypePreName(cast<AliasType>(T)->getRefType(), output);
        break;
    }
    case TC_STRUCT:
        EmitStructDecl(cast<StructType>(T)->getDecl(), output);
        break;
    case TC_ENUM:
        EmitDecl(cast<EnumType>(T)->getDecl(), output);
        break;
    case TC_FUNCTION:
        EmitDecl(cast<FunctionType>(T)->getDecl(), output);
        break;
    case TC_MODULE:
        TODO;
        break;
    }
}

void CCodeGenerator::EmitTypePostName(QualType type, StringBuilder& output) {
    LOG_FUNC
    if (type.isArrayType()) {
        // TEMP, use canonical type, since type can be AliasType
        type = type.getCanonicalType();
        const ArrayType* A = cast<ArrayType>(type);
        output << '[';
        if (A->getSizeExpr()) {
            EmitExpr(A->getSizeExpr(), output);
        }
        output << ']';
        EmitTypePostName(A->getElementType(), output);
    }
}

void CCodeGenerator::EmitConditionPre(const Stmt* S, unsigned indent) {
    LOG_FUNC
    if (isa<DeclStmt>(S)) {
        EmitDeclStmt(S, indent);
    }
}

void CCodeGenerator::EmitConditionPost(const Stmt* S) {
    LOG_FUNC
    if (isa<DeclStmt>(S)) {
        // only emit name, declaration has already been done in Pre part
        const DeclStmt* DS = cast<DeclStmt>(S);
        cbuf << DS->getDecl()->getName();
    } else {
        assert(isa<Expr>(S));
        EmitExpr(cast<Expr>(S), cbuf);
    }

}

bool CCodeGenerator::EmitAttributes(const Decl* D, StringBuilder& output, bool addStartSpace) {
    if (!D->hasAttributes()) return false;

    bool hasOutput = false;
    bool first = true;
    const AttrList& AL = D->getAttributes();
    for (AttrListConstIter iter = AL.begin(); iter != AL.end(); ++iter) {
        const Attr* A = *iter;
        const Expr* Arg = A->getArg();
        switch (A->getKind()) {
        case ATTR_UNKNOWN:
        case ATTR_EXPORT:
        case ATTR_NORETURN:
            // ignore for now
            break;
        case ATTR_INLINE:
            if (!hasOutput && addStartSpace) output << ' ';
            hasOutput = true;
            output << "__inline__";
            break;
        case ATTR_UNUSED_PARAMS:
            // ignore for now
            break;
        case ATTR_WEAK:
        case ATTR_PACKED:
        case ATTR_UNUSED:
        case ATTR_SECTION:
        case ATTR_ALIGNED:
            if (!hasOutput && addStartSpace) output << ' ';
            hasOutput = true;
            if (first) output << "__attribute__((";
            else output << ", ";
            output << A->kind2str();
            if (Arg) {
                output << '(';
                Arg->printLiteral(output);
                output << ')';
            }
            first = false;
            break;
        case ATTR_OPAQUE:
        case ATTR_CNAME:
        case ATTR_NO_TYPEDEF:
            // dont emit
            break;
        }
    }
    if (!first) output << "))";
    return hasOutput;
}

bool CCodeGenerator::EmitAsStatic(const Decl* D) const {
    if (!D->isPublic()) return true;
    if (D->isExported()) return false;
    if (mode == SINGLE_FILE) return true;
    return false;
}

bool CCodeGenerator::EmitAsDefine(const VarDecl* V) const {
    QualType type = V->getType();
    if (type.isArrayType()) {
        // convert const char[] and const char[x] to define.
        const ArrayType* AT = cast<ArrayType>(type.getTypePtr());
        QualType ET = AT->getElementType();
        if (!ET.isBuiltinType()) return false;
        if (ET.isVolatileQualified()) return false;
        if (!ET.isConstQualified()) return false;
        const Expr* init = V->getInitValue();
        if (!init) return false;
        if (isa<StringLiteral>(init)) return true;

        return false;

    }

    if (type.isVolatileQualified()) return false;
    // convert const integer literals to define
    if (type.isBuiltinType() && type.isConstQualified()) return true;
#if 0
    if (type.isArrayType()) {
        const ArrayType* AT = cast<ArrayType>(type.getTypePtr());
        QualType ET = AT->getElementType();
        if (!AT->isIncremental() && ET.isBuiltinType() && ET.isConstQualified()) return true;
    }
#endif
    return false;
}

const Expr* CCodeGenerator::stripImplicitCast(const Expr* E) {
    const ImplicitCastExpr* ic = dyncast<ImplicitCastExpr>(E);
    if (ic) return ic->getInner();
    return E;
}

