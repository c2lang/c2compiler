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
#include "C2Sema.h"
#include "Decl.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"

//#define DEBUG_CODEGEN

using namespace C2;
using namespace llvm;

CCodeGenerator::CCodeGenerator(const Package* pkg_)
    : pkg(pkg_)
{
    hfilename = pkg->getName() + ".h";
    cfilename = pkg->getName() + ".c";
}

CCodeGenerator::~CCodeGenerator() {
}

void CCodeGenerator::addEntry(const std::string& filename, C2Sema& sema) {
    entries.push_back(Entry(filename, sema));
}

void CCodeGenerator::generate() {
    hbuf << "#ifndef ";
    Utils::toCapital(pkg->getName(), hbuf);
    hbuf << "_H\n";
    hbuf << "#define ";
    Utils::toCapital(pkg->getName(), hbuf);
    hbuf << "_H\n";
    hbuf << '\n';

    cbuf << "#include \"" << hfilename << "\"\n";
    cbuf << '\n';

    for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
        C2Sema* sema = iter->sema;
        for (unsigned int i=0; i<sema->getNumDecls(); i++) {
            Decl* D = sema->getDecl(i);
            switch (D->dtype()) {
            case DECL_FUNC:
                EmitFunction(D);
                break;
            case DECL_VAR:
                EmitVariable(D);
                break;
            case DECL_TYPE:
                EmitType(D);
                break;
            case DECL_ARRAYVALUE:
                // TODO
                break;
            case DECL_USE:
                EmitUse(D);
                break;
            }
        }
    }
}

const char* CCodeGenerator::ConvertType(C2::Type* type) {
    switch (type->getKind()) {
    case Type::BUILTIN:
        {
            // TODO make u8/16/32 unsigned
            switch (type->getBuiltinType()) {
            case TYPE_U8:       return "unsigned char";
            case TYPE_U16:      return "unsigned short";
            case TYPE_U32:      return "unsigned int";
            case TYPE_S8:       return "char";
            case TYPE_S16:      return "short";
            case TYPE_S32:      return "int";
            case TYPE_INT:      return "const char*";
            case TYPE_STRING:
                return 0;  // TODO remove this type?
            case TYPE_FLOAT:    return "float";
            case TYPE_F32:      return "float";
            case TYPE_F64:      return "double";
            case TYPE_CHAR:     return "char";
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
    case Type::QUALIFIER:
        assert(0 && "TODO");
        break;
    }

    return 0;
}

void CCodeGenerator::EmitExpr(Expr* E, StringBuilder& output) {
    switch (E->etype()) {
    case EXPR_NUMBER:
        {
            NumberExpr* N = ExprCaster<NumberExpr>::getType(E);
            output << (int) N->value;
            return;
        }
    case EXPR_STRING:
        {
            StringExpr* S = ExprCaster<StringExpr>::getType(E);
            output << '"' << S->value << '"';
            return;
        }
    case EXPR_BOOL:
        assert(0 && "TODO");
        return;
    case EXPR_CHARLITERAL:
        {
            CharLiteralExpr* C = ExprCaster<CharLiteralExpr>::getType(E);
            output << '\'' << (char)C->value << '\'';
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
            InitListExpr* I = ExprCaster<InitListExpr>::getType(E);
            output << "{ ";
            ExprList& values = I->getValues();
            for (unsigned int i=0; i<values.size(); i++) {
                if (i != 0) output << ", ";
                EmitExpr(values[i], output);
            }
            output << " }";
            return;
        }
    case EXPR_TYPE:
        break;
    case EXPR_DECL:
        {
            DeclExpr* D = ExprCaster<DeclExpr>::getType(E);
            EmitDeclExpr(D, output, 0);
            return;
        }
    case EXPR_BINOP:
        EmitBinaryOperator(E, output);
        return;
    case EXPR_UNARYOP:
    case EXPR_SIZEOF:
    case EXPR_ARRAYSUBSCRIPT:
        assert(0 && "TODO");
        break;
    case EXPR_MEMBER:
        EmitMemberExpr(E, output);
        return;
    case EXPR_PAREN:
        assert(0 && "TODO");
        break;
    }
}

void CCodeGenerator::EmitBinaryOperator(Expr* E, StringBuilder& output) {
    BinaryOperator* B = ExprCaster<BinaryOperator>::getType(E);
    EmitExpr(B->getLHS(), output);
    output << ' ' << BinaryOperator::OpCode2str(B->getOpcode()) << ' ';
    EmitExpr(B->getRHS(), output);
}

void CCodeGenerator::EmitMemberExpr(Expr* E, StringBuilder& output) {
    MemberExpr* M = ExprCaster<MemberExpr>::getType(E);
    IdentifierExpr* RHS = M->getMember();
    if (RHS->getPackage()) {
        // A.B where A is a package
        EmitIdentifierExpr(RHS, output);
    } else {
        // A.B where A is decl of struct/union type
#if 0
        Base->generateC(indent, buffer);
        if (isArrow) buffer << "->";
        else buffer << '.';
        Member->generateC(0, buffer);
#endif
    }
}

void CCodeGenerator::EmitDeclExpr(DeclExpr* E, StringBuilder& output, unsigned indent) {
    output.indent(indent);
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
    CallExpr* C = ExprCaster<CallExpr>::getType(E);
    EmitExpr(C->getFn(), output);
    output << '(';
    for (unsigned int i=0; i<C->numArgs(); i++) {
        if (i != 0) output << ", ";
        EmitExpr(C->getArg(i), output);
    }
    output << ')';
}

void CCodeGenerator::EmitIdentifierExpr(Expr* E, StringBuilder& output) {
    IdentifierExpr* I = ExprCaster<IdentifierExpr>::getType(E);
    if (I->getPackage()) {
        Utils::addName(I->getPackage()->getCName(), I->getName(), output);
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
    FunctionDecl* F = DeclCaster<FunctionDecl>::getType(D);
    if (F->isPublic()) {
        EmitFunctionProto(F, hbuf);
        hbuf << ";\n";
    } else {
        cbuf << "static ";
    }
    EmitFunctionProto(F, cbuf);
    cbuf << ' ';
    EmitCompoundStmt(StmtCaster<CompoundStmt>::getType(F->getBody()), 0, false);
    cbuf << '\n';
}

void CCodeGenerator::EmitVariable(Decl* D) {
    VarDecl* V = DeclCaster<VarDecl>::getType(D);
    if (V->isPublic()) {
        // TODO type
        hbuf << "extern ";
        EmitTypePreName(V->getType(), hbuf);
        hbuf << ' ';
        Utils::addName(pkg->getName(), V->getName(), hbuf);
        EmitTypePostName(V->getType(), hbuf);
        // TODO add space if needed (on StringBuilder)
        hbuf << ";\n";
        hbuf << '\n';
    } else {
        cbuf << "static ";
    }
    EmitTypePreName(V->getType(), cbuf);
    cbuf << ' ';
    Utils::addName(pkg->getName(), V->getName(), cbuf);
    EmitTypePostName(V->getType(), cbuf);
    if (V->getInitValue()) {
        cbuf << " = ";
        EmitExpr(V->getInitValue(), cbuf);
    }
    cbuf << ";\n";
    cbuf << '\n';
}

void CCodeGenerator::EmitType(Decl* D) {
    TypeDecl* T = DeclCaster<TypeDecl>::getType(D);
    StringBuilder* out = &cbuf;
    if (D->isPublic()) out = &hbuf;
    *out << "typedef ";
    EmitTypePreName(T->getType(), *out);
    EmitTypePostName(T->getType(), *out);
    *out << ' ';
    Utils::addName(pkg->getName(), T->getName(), *out);
    *out << ";\n";
    *out << '\n';
}

void CCodeGenerator::EmitUse(Decl* D) {
#if 0
    // Temp hardcoded for stdio
    if (name == "stdio") {
        buffer << "#include <stdio.h>\n";
    }
#endif
}

void CCodeGenerator::EmitStmt(Stmt* S, unsigned indent) {
    switch (S->stype()) {
    case STMT_RETURN:
        {
            ReturnStmt* R = StmtCaster<ReturnStmt>::getType(S);
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
            Expr* E = StmtCaster<Expr>::getType(S);
            cbuf.indent(indent);
            EmitExpr(E, cbuf);
            cbuf << ";\n";
            return;
        }
    case STMT_IF:
        EmitIfStmt(S, indent);
        return;
    case STMT_WHILE:
    case STMT_DO:
    case STMT_FOR:
    case STMT_SWITCH:
    case STMT_CASE:
    case STMT_DEFAULT:
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_LABEL:
    case STMT_GOTO:
        break;
    case STMT_COMPOUND:
        {
            CompoundStmt* C = StmtCaster<CompoundStmt>::getType(S);
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
    IfStmt* I = StmtCaster<IfStmt>::getType(S);
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

void CCodeGenerator::EmitFunctionProto(FunctionDecl* F, StringBuilder& output) {
    EmitTypePreName(F->getReturnType(), output);
    EmitTypePostName(F->getReturnType(), output);
    output << ' ';
    Utils::addName(pkg->getName(), F->getName(), output);
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

void CCodeGenerator::EmitTypePreName(Type* T, StringBuilder& output) {
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

        output << "}";
        return;
    case Type::UNION:
#if 0
        buffer << "union {\n";
        if (members) {
            for (unsigned i=0; i<members->size(); i++) {
                DeclExpr* mem = (*members)[i];
                buffer.indent(INDENT);
                mem->getType()->generateC_PreName(buffer);
                buffer << ' ' << mem->getName();
                mem->getType()->generateC_PostName(buffer);
                buffer << ";\n";
            }
        }
        buffer << "}";
#endif
        break;
    case Type::ENUM:
    case Type::FUNC:
        assert(0 && "TODO");
        break;
    case Type::USER:
        assert(0 && "TODO");
        //userType->generateC(0, buffer);
        break;
    case Type::POINTER:
        EmitTypePreName(T->getRefType(), output);
        output << '*';
        break;
    case Type::ARRAY:
        EmitTypePreName(T->getRefType(), output);
        break;
    case Type::QUALIFIER:
        Type::printQualifier(output, T->getQualifier());
        EmitTypePreName(T->getRefType(), output);
        break;
    }
}

void CCodeGenerator::EmitTypePostName(Type* T, StringBuilder& output) {
    if (T->isArrayType()) {
        EmitTypePostName(T->getRefType(), output);
        output << '[';
        if (T->getArrayExpr()) {
            EmitExpr(T->getArrayExpr(), output);
        }
        output << ']';
    }
}

#if 0
void BoolLiteralExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << (int)value;
}

void CharLiteralExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '\'';
    buffer << (char)value;
    buffer << '\'';
}

void IdentifierExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    if (pkg) {
        Utils::addName(pkg->getCName(), name, buffer);
    } else {
        buffer << name;
    }
}

void DeclExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    type->generateC_PreName(buffer);
    buffer << ' ' << name;
    type->generateC_PostName(buffer);
    if (initValue) {
        buffer << " = ";
        initValue->generateC(0, buffer);
    }
    if (isStmt()) buffer << ";\n";
}

void DeclExpr::generateC(StringBuilder& buffer, const std::string& pkgName) {
    type->generateC_PreName(buffer);
    buffer << ' ';
    Utils::addName(pkgName, name, buffer);
    type->generateC_PostName(buffer);
    if (initValue) {
        buffer << " = ";
        initValue->generateC(0, buffer);
    }
    if (isStmt()) buffer << ";\n";
}

void UnaryOpExpr::generateC(int indent, StringBuilder& buffer) {
    switch (opc) {
    case UO_PostInc:
    case UO_PostDec:
        val->generateC(indent, buffer);
        buffer << UnaryOpCode2str(opc);
        break;
    case UO_PreInc:
    case UO_PreDec:
    case UO_AddrOf:
    case UO_Deref:
    case UO_Plus:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        buffer.indent(indent);
        buffer << UnaryOpCode2str(opc);
        val->generateC(0, buffer);
        break;
    default:
        assert(0);
    }
}

void SizeofExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "sizeof(";
    expr->generateC(0, buffer);
    buffer << ")";
    if (isStmt()) buffer << ";\n";
}

void ArraySubscriptExpr::generateC(int indent, StringBuilder& buffer) {
    base->generateC(indent, buffer);
    buffer << '[';
    idx->generateC(0, buffer);
    buffer << ']';
}

void ParenExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '(';
    Val->generateC(0, buffer);
    buffer << ')';
}
#endif


#if 0
void WhileStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "while (";
    Cond->generateC(0, buffer);
    buffer << ")\n";
    Then->generateC(indent, buffer);
}

void DoStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "do\n";
    Then->generateC(indent, buffer);
    buffer.indent(indent);
    buffer << "while (";
    Cond->generateC(0, buffer);
    buffer << ");\n";
}

void ForStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "for (";
    if (Init) {
        Init->generateC(0, buffer);
        buffer.strip('\n');
        buffer.strip(';');
    }
    buffer << ';';
    if (Cond) {
        buffer << ' ';
        Cond->generateC(0, buffer);
    }
    buffer << ';';
    if (Incr) {
        buffer << ' ';
        Incr->generateC(0, buffer);
    }
    buffer << ")\n";
    // TODO fix indentation
    Body->generateC(indent, buffer);
}

void SwitchStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "switch(";
    Cond->generateC(0, buffer);
    buffer << ") {\n";
    for (unsigned int i=0; i<Cases.size(); i++) {
        Cases[i]->generateC(indent + INDENT, buffer);
    }
    buffer.indent(indent);
    buffer << "}\n";
}

void CaseStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "case ";
    Cond->generateC(0, buffer);
    buffer << ":\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->generateC(indent + INDENT, buffer);
    }
}

void DefaultStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "default:\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->generateC(indent + INDENT, buffer);
    }
}

void BreakStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "break;\n";
}

void ContinueStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "continue;\n";
}

void LabelStmt::generateC(int indent, StringBuilder& buffer) {
    buffer << name << ":\n";
    subStmt->generateC(indent, buffer);
}

void GotoStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "goto " << name << ";\n";
}
#endif

