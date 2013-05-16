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

//#define DEBUG_CODEGEN

using namespace C2;
using namespace llvm;

CCodeGenerator::CCodeGenerator(const Package* pkg_)
    : pkg(pkg_)
{
}

CCodeGenerator::~CCodeGenerator() {
}

void CCodeGenerator::addEntry(const std::string& filename, C2Sema& sema) {
    entries.push_back(Entry(filename, sema));
}

void CCodeGenerator::generate() {
    hbuf << "#ifndef TODO_H\n";
    hbuf << "#define TODO_H\n";
    hbuf << '\n';

    cbuf << "#include \"todo.h\"\n";
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

void CCodeGenerator::EmitExpr(Expr* E) {

}

void CCodeGenerator::dump() {
    printf("C-code for TODO.h\n%s\n", (const char*)hbuf);
    printf("C-code for TODO.c\n%s\n", (const char*)cbuf);
}

void CCodeGenerator::write(const std::string& target, const std::string& name) {
    // TODO
}

void CCodeGenerator::EmitFunction(Decl* D) {
    FunctionDecl* F = DeclCaster<FunctionDecl>::getType(D);
    if (F->isPublic()) {
        // TODO add proto to header
    } else {
        cbuf << "static ";
    }
#if 0
    rtype->generateC_PreName(buffer);
    rtype->generateC_PostName(buffer);
    buffer << ' ';
    Utils::addName(pkgName, name, buffer);
    buffer << '(';
    int count = args.size();
    for (unsigned int i=0; i<args.size(); i++) {
        args[i]->generateC(0, buffer);
        if (count != 1) buffer << ", ";
        count--;
    }
    if (m_isVariadic) buffer << "...";
    buffer << ")\n";
    assert(body);
    body->generateC(0, buffer);
#endif
    cbuf << '\n';
}

void CCodeGenerator::EmitVariable(Decl* D) {
    VarDecl* V = DeclCaster<VarDecl>::getType(D);
#if 0
    if (isPublic()) buffer << "static ";
    decl->generateC(buffer, pkgName);
    // TODO semicolon not needed when ending initlist with '}'
    // Q: add bool return value to Expr.generateC()?
    buffer << ";\n";
#endif
}

void CCodeGenerator::EmitType(Decl* D) {
#if 0
    buffer << "typedef ";
    type->generateC_PreName(buffer);
    buffer << ' ';
    Utils::addName(pkgName, name, buffer);
    type->generateC_PostName(buffer);
    buffer << ";\n";
#endif
}

void CCodeGenerator::EmitUse(Decl* D) {
#if 0
    // Temp hardcoded for stdio
    if (name == "stdio") {
        buffer << "#include <stdio.h>\n";
    }
#endif
}


#if 0
void NumberExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << (int)value;
}

void StringExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '"' << value << '"';
}

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

void CallExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    Fn->generateC(0, buffer);
    buffer << '(';
    for (unsigned int i=0; i<args.size(); i++) {
        if (i != 0) buffer << ", ";
        args[i]->generateC(0, buffer);
    }
    buffer << ')';
    if (isStmt()) buffer << ";\n";
}

void IdentifierExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    if (pkg) {
        Utils::addName(pkg->getCName(), name, buffer);
    } else {
        buffer << name;
    }
}

void InitListExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "{ ";
    for (unsigned int i=0; i<values.size(); i++) {
        if (i != 0) buffer << ", ";
        values[i]->generateC(0, buffer);
    }
    buffer << " }";
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

void BinOpExpr::generateC(int indent, StringBuilder& buffer) {
    lhs->generateC(indent, buffer);
    buffer << ' ' << BinOpCode2str(opc) << ' ';
    rhs->generateC(0, buffer);
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

void MemberExpr::generateC(int indent, StringBuilder& buffer) {
    if (Member->getPackage()) {
        // A.B where A is a package
        Member->generateC(indent, buffer);
    } else {
        // A.B where A is decl of struct/union type
        Base->generateC(indent, buffer);
        if (isArrow) buffer << "->";
        else buffer << '.';
        Member->generateC(0, buffer);
    }
}

void ParenExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '(';
    Val->generateC(0, buffer);
    buffer << ')';
}
#endif


#if 0

void ReturnStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "return";
    if (value) {
        buffer << ' ';
        value->generateC(0, buffer);
    }
    buffer << ";\n";
}

void IfStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "if (";
    SubExprs[COND]->generateC(0, buffer);
    buffer << ")\n";
    SubExprs[THEN]->generateC(indent, buffer);
    if (SubExprs[ELSE]) {
        buffer.indent(indent);
        buffer << "else\n";
        SubExprs[ELSE]->generateC(indent, buffer);
    }
}

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

void CompoundStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "{\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->generateC(indent + INDENT, buffer);
    }
    buffer.indent(indent);
    buffer << "}\n";
}

#endif

#if 0
void Type::generateC_PreName(StringBuilder& buffer) const {
    switch (kind) {
    case BUILTIN:
        assert(cname);
        buffer << cname;
        break;
    case STRUCT:
        buffer << "struct {\n";
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
        break;
    case UNION:
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
        break;
    case ENUM:
    case FUNC:
        assert(0 && "TODO");
        break;
    case USER:
        userType->generateC(0, buffer);
        break;
    case POINTER:
        refType->generateC_PreName(buffer);
        buffer << '*';
        break;
    case ARRAY:
        refType->generateC_PreName(buffer);
        break;
    case QUALIFIER:
        printQualifier(buffer, qualifiers);
        refType->generateC_PreName(buffer);
        break;
    }
}

void Type::generateC_PostName(StringBuilder& buffer) const {
    if (kind == ARRAY) {
        assert(refType);
        refType->generateC_PostName(buffer);
        buffer << '[';
        if (arrayExpr) arrayExpr->generateC(0, buffer);
        buffer << ']';
    }
}

#endif

