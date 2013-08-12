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

#include <vector>
// for SmallString
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
// for tool_output_file
#include <llvm/Support/ToolOutputFile.h>
// TODO REMOVE
#include <stdio.h>


#include "CCodeGenerator.h"
#include "CodeGenFunction.h"
#include "Package.h"
#include "AST.h"
#include "Decl.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"

//#define DEBUG_CODEGEN

using namespace C2;
using namespace llvm;
using namespace clang;

CCodeGenerator::CCodeGenerator(const std::string& filename_, Mode mode_, const Pkgs& pkgs_, bool prefix)
    : filename(filename_)
    , curpkg(0)
    , mode(mode_)
    , no_local_prefix(prefix)
    , pkgs(pkgs_)
{
    hfilename = filename + ".h";
    cfilename = filename + ".c";
}

CCodeGenerator::~CCodeGenerator() {
}

void CCodeGenerator::addEntry(const std::string& filename, AST& ast) {
    entries.push_back(Entry(filename, ast));
}

void CCodeGenerator::generate() {
    hbuf << "#ifndef ";
    Utils::toCapital(filename, hbuf);
    hbuf << "_H\n";
    hbuf << "#define ";
    Utils::toCapital(filename, hbuf);
    hbuf << "_H\n";
    hbuf << '\n';

    // generate all includes
    for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
        AST* ast = iter->ast;
        curpkg = &ast->getPkgName();
        for (unsigned int i=0; i<ast->getNumDecls(); i++) {
            Decl* D = ast->getDecl(i);
            switch (D->getKind()) {
            case DECL_USE:
                EmitUse(D);
                break;
            default:
                break;
            }
        }
        curpkg = 0;
    }
    cbuf << "#include \"" << hfilename << "\"\n";
    cbuf << '\n';

    for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
        AST* ast = iter->ast;
        curpkg = &ast->getPkgName();
        for (unsigned int i=0; i<ast->getNumDecls(); i++) {
            Decl* D = ast->getDecl(i);
            switch (D->getKind()) {
            case DECL_FUNC:
                EmitFunction(D);
                break;
            case DECL_VAR:
                EmitVariable(D);
                break;
            case DECL_ENUMVALUE:
                assert(0 && "TODO");
                break;
            case DECL_TYPE:
                EmitType(D);
                break;
            case DECL_ARRAYVALUE:
                // TODO
                break;
            case DECL_USE:
                break;
            }
        }
        curpkg = 0;
    }
    hbuf << "#endif\n";
}

const char* CCodeGenerator::ConvertType(const C2::Type* type) {
    switch (type->getKind()) {
    case Type::BUILTIN:
        {
            // TODO make u8/16/32 unsigned
            switch (type->getBuiltinType()) {
            case TYPE_U8:       return "unsigned char";
            case TYPE_U16:      return "unsigned short";
            case TYPE_U32:      return "unsigned int";
            case TYPE_U64:      return "unsigned long long";
            case TYPE_I8:       return "char";
            case TYPE_I16:      return "short";
            case TYPE_I32:      return "int";
            case TYPE_I64:      return "long long";
            case TYPE_INT:      return "const char*";
            case TYPE_STRING:
                return 0;  // TODO remove this type?
            case TYPE_F32:      return "float";
            case TYPE_F64:      return "double";
            case TYPE_BOOL:     return "int";
            case TYPE_VOID:     return "void";
            }
        }
        break;
    case Type::USER:
    case Type::STRUCT:
    case Type::UNION:
    case Type::ENUM:
    case Type::FUNC:
        assert(0 && "TODO");
        break;
    case Type::POINTER:
        {
            //llvm::Type* tt = ConvertType(type->getRefType());
            //return tt->getPointerTo();
            break;
        }
    case Type::ARRAY:
        {
            //llvm::Type* tt = ConvertType(type->getRefType());
            //return tt->getPointerTo();
            break;
        }
    }

    return 0;
}

void CCodeGenerator::EmitExpr(Expr* E, StringBuilder& output) {
    switch (E->getKind()) {
    case EXPR_INTEGER_LITERAL:
        {
            IntegerLiteral* N = cast<IntegerLiteral>(E);
            output << (int) N->Value.getSExtValue();
            return;
        }
    case EXPR_STRING:
        {
            StringExpr* S = cast<StringExpr>(E);
            EmitStringLiteral(S->value, output);
            return;
        }
    case EXPR_BOOL:
        {
            BoolLiteralExpr* B = cast<BoolLiteralExpr>(E);
            cbuf << (int)B->value;
            return;
        }
    case EXPR_CHARLITERAL:
        {
            CharLiteralExpr* C = cast<CharLiteralExpr>(E);
            output << '\'' << (char)C->value << '\'';
            return;
        }
    case EXPR_FLOAT_LITERAL:
        {
            FloatingLiteral* F = cast<FloatingLiteral>(E);
            char temp[20];
            sprintf(temp, "%f", F->Value.convertToFloat());
            output << temp;
            return;
        }
    case EXPR_CALL:
        EmitCallExpr(E, output);
        return;
    case EXPR_IDENTIFIER:
        EmitIdentifierExpr(E, output);
        return;
    case EXPR_INITLIST:
        {
            InitListExpr* I = cast<InitListExpr>(E);
            output << "{ ";
            ExprList& values = I->getValues();
            for (unsigned int i=0; i<values.size(); i++) {
                if (i == 0 && values[0]->getKind() == EXPR_INITLIST) output << '\n';
                EmitExpr(values[i], output);
                if (i != values.size() -1) output << ',';
                if (values[i]->getKind() == EXPR_INITLIST) output << '\n';
            }
            output << " }";
            return;
        }
    case EXPR_TYPE:
        {
            TypeExpr* T = cast<TypeExpr>(E);
            EmitTypePreName(T->getType(), output);
            EmitTypePostName(T->getType(), output);
            return;
        }
    case EXPR_DECL:
        EmitDeclExpr(cast<DeclExpr>(E), output, 0);
        return;
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
        {
            BuiltinExpr* S = cast<BuiltinExpr>(E);
            if (S->isSizeFunc()) {
                output << "sizeof(";
                EmitExpr(S->getExpr(), output);
                output << ')';
            } else {
                assert(0 && "TODO");
                // TODO  sizeof(array) / sizeof(array[0]);
                // NOTE cannot be converted to C if used with enums
            }
            return;
        }
    case EXPR_ARRAYSUBSCRIPT:
        {
            ArraySubscriptExpr* A = cast<ArraySubscriptExpr>(E);
            EmitExpr(A->getBase(), output);
            output << '[';
            EmitExpr(A->getIndex(), output);
            output << ']';
            return;
        }
    case EXPR_MEMBER:
        EmitMemberExpr(E, output);
        return;
    case EXPR_PAREN:
        {
            ParenExpr* P = cast<ParenExpr>(E);
            cbuf << '(';
            EmitExpr(P->getExpr(), cbuf);
            cbuf << ')';
            return;
        }
    }
}

void CCodeGenerator::EmitBinaryOperator(Expr* E, StringBuilder& output) {
    BinaryOperator* B = cast<BinaryOperator>(E);
    EmitExpr(B->getLHS(), output);
    output << ' ' << BinaryOperator::OpCode2str(B->getOpcode()) << ' ';
    EmitExpr(B->getRHS(), output);
}

void CCodeGenerator::EmitConditionalOperator(Expr* E, StringBuilder& output) {
    ConditionalOperator* C = cast<ConditionalOperator>(E);
    EmitExpr(C->getCond(), output);
    output << " ? ";
    EmitExpr(C->getLHS(), output);
    output << " : ";
    EmitExpr(C->getRHS(), output);

}

void CCodeGenerator::EmitUnaryOperator(Expr* E, StringBuilder& output) {
    UnaryOperator* U = cast<UnaryOperator>(E);

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
    case UO_Plus:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        //output.indent(indent);
        output << UnaryOperator::OpCode2str(U->getOpcode());
        EmitExpr(U->getExpr(), output);
        break;
    default:
        assert(0);
    }
}

void CCodeGenerator::EmitMemberExpr(Expr* E, StringBuilder& output) {
    MemberExpr* M = cast<MemberExpr>(E);
    IdentifierExpr* RHS = M->getMember();
    if (RHS->getPackage()) {
        // A.B where A is a package
        EmitIdentifierExpr(RHS, output);
    } else {
        // A.B where A is decl of struct/union type
        EmitExpr(M->getBase(), cbuf);
        if (M->isArrowOp()) cbuf << "->";
        else cbuf << '.';
        cbuf << M->getMember()->getName();
    }
}

void CCodeGenerator::EmitDeclExpr(DeclExpr* E, StringBuilder& output, unsigned indent) {
    output.indent(indent);
    if (E->hasLocalQualifier()) output << "static ";
    EmitTypePreName(E->getType(), output);
    output << ' ';
    output << E->getName();
    EmitTypePostName(E->getType(), output);
    if (E->getInitValue()) {
        output << " = ";
        EmitExpr(E->getInitValue(), output);
    }
}

void CCodeGenerator::EmitCallExpr(Expr* E, StringBuilder& output) {
    CallExpr* C = cast<CallExpr>(E);
    EmitExpr(C->getFn(), output);
    output << '(';
    for (unsigned int i=0; i<C->numArgs(); i++) {
        if (i != 0) output << ", ";
        EmitExpr(C->getArg(i), output);
    }
    output << ')';
}

void CCodeGenerator::EmitIdentifierExpr(Expr* E, StringBuilder& output) {
    IdentifierExpr* I = cast<IdentifierExpr>(E);
    if (I->getPackage()) {
        addPrefix(I->getPackage()->getCName(), I->getName(), output);
    } else {
        output << I->getName();
    }
}

void CCodeGenerator::dump() {
    printf("---- code for %s ----\n%s\n", hfilename.c_str(), (const char*)hbuf);
    printf("---- code for %s ----\n%s\n", cfilename.c_str(), (const char*)cbuf);
}

void CCodeGenerator::write(const std::string& target, const std::string& name) {
    // TODO
}

void CCodeGenerator::EmitFunction(Decl* D) {
    FunctionDecl* F = cast<FunctionDecl>(D);
    if (mode == SINGLE_FILE) {
        // emit all function protos as forward declarations in header
        EmitFunctionProto(F, hbuf);
        hbuf << ";\n\n";
    } else {
        if (F->isPublic()) {
            EmitFunctionProto(F, hbuf);
            hbuf << ";\n\n";
        } else {
            cbuf << "static ";
        }
    }

    EmitFunctionProto(F, cbuf);
    cbuf << ' ';
    EmitCompoundStmt(F->getBody(), 0, false);
    cbuf << '\n';
}

void CCodeGenerator::EmitVariable(Decl* D) {
    VarDecl* V = cast<VarDecl>(D);
    if (V->isPublic() && mode != SINGLE_FILE) {
        // TODO type
        hbuf << "extern ";
        EmitTypePreName(V->getType(), hbuf);
        hbuf << ' ';
        addPrefix(*curpkg, V->getName(), hbuf);
        EmitTypePostName(V->getType(), hbuf);
        // TODO add space if needed (on StringBuilder)
        hbuf << ";\n";
        hbuf << '\n';
    } else {
        cbuf << "static ";
    }
    EmitTypePreName(V->getType(), cbuf);
    cbuf << ' ';
    addPrefix(*curpkg, V->getName(), cbuf);
    EmitTypePostName(V->getType(), cbuf);
    if (V->getInitValue()) {
        cbuf << " = ";
        EmitExpr(V->getInitValue(), cbuf);
    }
    const VarDecl::InitValues& inits = V->getIncrValues();
    if (inits.size()) {
        cbuf << " = {\n";
        VarDecl::InitValuesConstIter iter=inits.begin();
        while (iter != inits.end()) {
            const ArrayValueDecl* E = (*iter);
            cbuf.indent(INDENT);
            EmitExpr(E->getExpr(), cbuf);
            cbuf << ",\n";
            ++iter;
        }
        cbuf << '}';
    }
    cbuf << ";\n";
    cbuf << '\n';
}

void CCodeGenerator::EmitType(Decl* D) {
    TypeDecl* T = cast<TypeDecl>(D);
    StringBuilder* out = &cbuf;
    if (D->isPublic()) out = &hbuf;
    *out << "typedef ";
    EmitTypePreName(T->getType(), *out);
    EmitTypePostName(T->getType(), *out);
    *out << ' ';
    addPrefix(*curpkg, T->getName(), *out);
    *out << ";\n";
    *out << '\n';
}

void CCodeGenerator::EmitUse(Decl* D) {
    typedef Pkgs::const_iterator PkgsConstIter;
    PkgsConstIter iter = pkgs.find(D->getName());
    assert(iter != pkgs.end());
    const Package* P = iter->second;

    if (mode == MULTI_FILE || P->isPlainC()) {
        cbuf << "#include ";
        if (P->isPlainC()) cbuf << '<';
        else cbuf << '"';
        cbuf << D->getName() << ".h";
        if (P->isPlainC()) cbuf << '>';
        else cbuf << '\'';
        cbuf << '\n';
    }
}

void CCodeGenerator::EmitStmt(Stmt* S, unsigned indent) {
    switch (S->getKind()) {
    case STMT_RETURN:
        {
            ReturnStmt* R = cast<ReturnStmt>(S);
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
            Expr* E = cast<Expr>(S);
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
    case STMT_CASE:
    case STMT_DEFAULT:
        assert(0 && "Should already be generated");
        break;
    case STMT_BREAK:
        cbuf.indent(indent);
        cbuf << "break;\n";
        return;
    case STMT_CONTINUE:
        cbuf.indent(indent);
        cbuf << "continue;\n";
        return;
    case STMT_LABEL:
        {
            LabelStmt* L = cast<LabelStmt>(S);
            cbuf << L->getName();
            cbuf << ":\n";
            EmitStmt(L->getSubStmt(), indent);
            return;
        }
    case STMT_GOTO:
        {
            GotoStmt* G = cast<GotoStmt>(S);
            cbuf.indent(indent);
            cbuf << "goto " << G->getName() << ";\n";
            return;
        }
    case STMT_COMPOUND:
        {
            CompoundStmt* C = cast<CompoundStmt>(S);
            EmitCompoundStmt(C, indent, true);
            return;
        }
    }
}

void CCodeGenerator::EmitCompoundStmt(CompoundStmt* C, unsigned indent, bool startOnNewLine) {
    if (startOnNewLine) cbuf.indent(indent);
    cbuf << "{\n";
    const StmtList& Stmts = C->getStmts();
    for (unsigned int i=0; i<Stmts.size(); i++) {
        EmitStmt(Stmts[i], indent + INDENT);
    }
    cbuf.indent(indent);
    cbuf << "}\n";
}

void CCodeGenerator::EmitIfStmt(Stmt* S, unsigned indent) {
    IfStmt* I = cast<IfStmt>(S);
    cbuf.indent(indent);
    cbuf << "if (";
    EmitExpr(I->getCond(), cbuf);
    cbuf << ")\n";
    EmitStmt(I->getThen(), indent);
    if (I->getElse()) {
        cbuf.indent(indent);
        cbuf << "else\n";
        EmitStmt(I->getElse(), indent);
    }
}

void CCodeGenerator::EmitWhileStmt(Stmt* S, unsigned indent) {
    WhileStmt* W = cast<WhileStmt>(S);
    cbuf.indent(indent);
    cbuf << "while (";
    // TEMP, assume Expr
    Expr* E = cast<Expr>(W->getCond());
    assert(E);
    EmitExpr(E, cbuf);
    cbuf << ") ";
    Stmt* Body = W->getBody();
    if (Body->getKind() == STMT_COMPOUND) {
        CompoundStmt* C = cast<CompoundStmt>(Body);
        EmitCompoundStmt(C, indent, false);
    } else {
        EmitStmt(Body, 0);
    }
}

void CCodeGenerator::EmitDoStmt(Stmt* S, unsigned indent) {
    DoStmt* D = cast<DoStmt>(S);
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
    assert(E);
    EmitExpr(E, cbuf);
    cbuf << ");\n";
}

void CCodeGenerator::EmitForStmt(Stmt* S, unsigned indent) {
    ForStmt* F = cast<ForStmt>(S);
    cbuf.indent(indent);
    cbuf << "for (";
    Stmt* Init = F->getInit();
    if (Init) {
        // assume Expr
        Expr* E = cast<Expr>(Init);
        assert(E);
        EmitExpr(E, cbuf);
    }
    cbuf << ';';

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
        CompoundStmt* C = cast<CompoundStmt>(Body);
        EmitCompoundStmt(C, indent, false);
    } else {
        EmitStmt(Body, 0);
    }
}

void CCodeGenerator::EmitSwitchStmt(Stmt* S, unsigned indent) {
    SwitchStmt* SW = cast<SwitchStmt>(S);
    cbuf.indent(indent);
    cbuf << "switch (";
    EmitExpr(SW->getCond(), cbuf);
    cbuf << ") {\n";

    const StmtList& Cases = SW->getCases();
    for (unsigned int i=0; i<Cases.size(); i++) {
        Stmt* Case = Cases[i];
        switch (Case->getKind()) {
        case STMT_CASE:
            {
                CaseStmt* C = cast<CaseStmt>(Case);
                cbuf.indent(indent + INDENT);
                cbuf << "case ";
                EmitExpr(C->getCond(), cbuf);
                cbuf << ":\n";
                const StmtList& Stmts = C->getStmts();
                for (unsigned int i=0; i<Stmts.size(); i++) {
                    EmitStmt(Stmts[i], indent + INDENT + INDENT);
                }
                break;
            }
        case STMT_DEFAULT:
            {
                DefaultStmt* D = cast<DefaultStmt>(Case);
                cbuf.indent(indent + INDENT);
                cbuf << "default:\n";
                const StmtList& Stmts = D->getStmts();
                for (unsigned int i=0; i<Stmts.size(); i++) {
                    EmitStmt(Stmts[i], indent + INDENT + INDENT);
                }
                break;
            }
        default:
            assert(0);
        }
    }

    cbuf.indent(indent);
    cbuf << "}\n";
}

void CCodeGenerator::EmitFunctionProto(FunctionDecl* F, StringBuilder& output) {
    if (mode == SINGLE_FILE && F->getName() != "main") output << "static ";
    EmitTypePreName(F->getReturnType(), output);
    EmitTypePostName(F->getReturnType(), output);
    output << ' ';
    addPrefix(*curpkg, F->getName(), output);
    output << '(';
    int count = F->numArgs();
    if (F->isVariadic()) count++;
    for (unsigned int i=0; i<F->numArgs(); i++) {
        DeclExpr* A = F->getArg(i);
        EmitDeclExpr(A, output, 0);
        if (count != 1) output << ", ";
        count--;
    }
    if (F->isVariadic()) output << "...";
    output << ')';
}

void CCodeGenerator::EmitTypePreName(QualType type, StringBuilder& output) {
    const Type* T = type.getTypePtr();
    switch (T->getKind()) {
    case Type::BUILTIN:
        output << T->getCName();
        break;
    case Type::STRUCT:
        output << "struct {\n";
        if (T->getMembers()) {
            MemberList* members = T->getMembers();
            for (unsigned i=0; i<members->size(); i++) {
                DeclExpr* mem = (*members)[i];
                EmitDeclExpr(mem, output, INDENT);
                output << ";\n";
            }
        }
        output << '}';
        return;
    case Type::UNION:
        output << "union {\n";
        if (T->getMembers()) {
            MemberList* members = T->getMembers();
            for (unsigned i=0; i<members->size(); i++) {
                DeclExpr* mem = (*members)[i];
                EmitDeclExpr(mem, output, INDENT);
                output << ";\n";
            }
        }
        output << '}';
        return;
    case Type::ENUM:
        output << "enum {\n";
        if (T->getMembers()) {
            MemberList* members = T->getMembers();
            for (unsigned i=0; i<members->size(); i++) {
                DeclExpr* mem = (*members)[i];
                output.indent(INDENT);
                addPrefix(*curpkg, mem->getName(), output);
                if (mem->getInitValue()) {
                    output << " = ";
                    EmitExpr(mem->getInitValue(), output);
                }
                output << ",\n";
            }
        }
        output << '}';
        return;
    case Type::FUNC:
        assert(0 && "TODO");
        break;
    case Type::USER:
        EmitExpr(T->getUserType(), output);
        return;
        //assert(0 && "TODO");
        //userType->generateC(0, buffer);
        break;
    case Type::POINTER:
        // TODO handle Qualifiers
        EmitTypePreName(T->getRefType(), output);
        output << '*';
        break;
    case Type::ARRAY:
        // TODO handle Qualifiers
        EmitTypePreName(T->getRefType(), output);
        break;
    }
}

void CCodeGenerator::EmitTypePostName(QualType type, StringBuilder& output) {
    if (type.isArrayType()) {
        const Type* T = type.getTypePtr();
        EmitTypePostName(T->getRefType(), output);
        output << '[';
        if (T->getArrayExpr()) {
            EmitExpr(T->getArrayExpr(), output);
        }
        output << ']';
    }
}

void CCodeGenerator::EmitStringLiteral(const std::string& input, StringBuilder& output) {
    output << '"';
    const char* cp = input.c_str();
    for (unsigned i=0; i<input.size(); i++) {
        switch (*cp) {
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        case '\033':
            output << "\\033";
            break;
        // TODO other escaped chars
        default:
            output << *cp;
            break;
        }
        cp++;
    }
    output << '"';
}

void CCodeGenerator::addPrefix(const std::string& pkgName, const std::string& name, StringBuilder& buffer) const {
    if (pkgName.empty()) {
        buffer << name;
        return;
    }
    if (no_local_prefix && pkgName == *curpkg) {
        buffer << name;
        return;
    }
    Utils::addName(pkgName, name, buffer);
}

