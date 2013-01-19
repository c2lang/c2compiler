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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "C2Sema.h"
#include "Decl.h"
#include "Type.h"
#include "StringBuilder.h"
#include "Type.h"
#include "color.h"
#include "ASTVisitor.h"

#define SEMA_DEBUG

#define COL_SEMA ANSI_RED

using namespace C2;
using namespace clang;

C2Sema::C2Sema(SourceManager& sm_, DiagnosticsEngine& Diags_)
    : SourceMgr(sm_)
    , Diags(Diags_)
{
}

C2Sema::~C2Sema() {
    // TODO delete all decls
}

void C2Sema::ActOnPackage(const char* name, SourceLocation loc) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: package " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    pkgName = name;
    pkgLoc = loc;
}

void C2Sema::ActOnUse(const char* name, SourceLocation loc) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: use " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // check if use-ing own package
    if (pkgName == name) {
        Diag(loc, diag::err_use_own_package) << name;
        return;
    }
    // check for duplicate use
    const Decl* old = findUse(name);
    if (old) {
        Diag(loc, diag::err_duplicate_use) << name;
        Diag(old->getLocation(), diag::note_previous_use);
        return;
    }

    decls.push_back(new UseDecl(name, loc));
}

void C2Sema::ActOnTypeDef(const char* name, SourceLocation loc, Expr* type, bool is_public) {
    assert(name);
    assert(type);
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: type def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    TypeDecl* decl = new TypeDecl(name, loc, typeExpr->takeType(), is_public);
    decls.push_back(decl);
    delete type;
}

void C2Sema::ActOnVarDef(const char* name, SourceLocation loc,
                        bool is_public, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: var def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    DeclExpr* declExpr = new DeclExpr(name, loc, typeExpr->takeType(), InitValue);
    VarDecl* decl = new VarDecl(declExpr, is_public, false);
    decls.push_back(decl);
    delete type;
}

C2::FunctionDecl* C2Sema::ActOnFuncDef(const char* name, SourceLocation loc, bool is_public, Expr* rtype) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: func def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    assert(rtype);
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(rtype);
    assert(typeExpr);
    FunctionDecl* decl = new FunctionDecl(name, loc, is_public, typeExpr->takeType());
    decls.push_back(decl); 
    delete rtype;
    return decl;
}

void C2Sema::ActOnFinishFunctionBody(Decl* decl, Stmt* body) {
    FunctionDecl* func = DeclCaster<FunctionDecl>::getType(decl);
    assert(func);
    func->setBody(body);
}

C2::StmtResult C2Sema::ActOnReturnStmt(SourceLocation loc, Expr* value) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: return at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new ReturnStmt(value));
}

C2::StmtResult C2Sema::ActOnIfStmt(SourceLocation ifLoc,
                                   ExprResult condition, StmtResult thenStmt,
                                   SourceLocation elseLoc, StmtResult elseStmt) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: if statement at ";
    ifLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new IfStmt(ifLoc, condition.release(), thenStmt.release(), elseLoc, elseStmt.release()));
}

C2::StmtResult C2Sema::ActOnCompoundStmt(SourceLocation L, SourceLocation R, StmtList& stmts) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: compound statement at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new CompoundStmt(L, R, stmts));
}

C2::StmtResult C2Sema::ActOnDeclaration(const char* name, SourceLocation loc, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: decl at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    DeclExpr* declExpr = new DeclExpr(name, loc, typeExpr->takeType(), InitValue);
    declExpr->setStatementFlag();
    delete type;
    return StmtResult(declExpr);
}

C2::ExprResult C2Sema::ActOnCallExpr(Expr* id, Expr** args, unsigned numArgs, SourceLocation RParenLoc) {
    IdentifierExpr* Fn = ExprCaster<IdentifierExpr>::getType(id);
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: call to " << Fn->getName() << " at ";
    Fn->getLocation().dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    CallExpr* call = new CallExpr(Fn);
    assert(call);
    for (int i=0; i<numArgs; i++) call->addArg(args[i]);
    return ExprResult(call);
}

C2::ExprResult C2Sema::ActOnIdExpression(IdentifierInfo* pkgII, SourceLocation pkgLoc,
                                         IdentifierInfo& symII, SourceLocation symLoc) {
    std::string id(symII.getNameStart(), symII.getLength());
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: identifier " << id << " at ";
    symLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    std::string pkg;
    if (pkgII) pkg = std::string(pkgII->getNameStart(), pkgII->getLength());
    return ExprResult(new IdentifierExpr(pkgLoc, pkg, symLoc, id));
}

C2::ExprResult C2Sema::ActOnInitList(SourceLocation left_, SourceLocation right_, ExprList& vals) {
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: initlist at ";
    left_.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new InitListExpr(left_, right_, vals));
}

C2::ExprResult C2Sema::ActOnArrayType(Expr* base, Expr* size) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Array Type"ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(base);
    assert(typeExpr);
    typeExpr->addArray(size);
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnPointerType(Expr* base) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Pointer Type"ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(base);
    assert(typeExpr);
    typeExpr->addPointer();
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnUserType(Expr* expr) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: User Type"ANSI_NORMAL"\n";
#endif
    assert(expr);
    IdentifierExpr* idExpr = ExprCaster<IdentifierExpr>::getType(expr);
    assert(idExpr && "invalid expr type");
    
    Type* type = new Type(Type::USER, 0);
    type->setUserType(idExpr);
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnBuiltinType(C2Type t) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Builtin Type"ANSI_NORMAL"\n";
#endif
    Type* type = getBuiltinType(t);
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnStructType(SourceLocation leftBrace, SourceLocation rightBrace, ExprList& members, bool isStruct) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Struct/Union Type"ANSI_NORMAL"\n";
#endif
    Type* type = new Type(isStruct ? Type::STRUCT : Type::UNION);
    // TODO use left/rightBrace
    for (int i=0; i<members.size(); i++) {
        fprintf(stderr, "FIX ADDSTRUCTMEMBER()\n");
        DeclExpr* member = ExprCaster<DeclExpr>::getType(members[i]);
        assert(member);
        // NOTE: memleak on members (but we get type from it.. HACK HACK!)
        // HACK: type ownership transfers..
        // TODO Type should use Expr
        type->addStructMember(member->getName().c_str(), member->getType());
    }
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnTypeQualifier(ExprResult R, unsigned int qualifier) {
    if (qualifier) {
#ifdef SEMA_DEBUG
        std::cerr << COL_SEMA"SEMA: Qualifier Type"ANSI_NORMAL"\n";
#endif
        assert(R.get());
        TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(R.get());
        assert(typeExpr);
        typeExpr->addQualifier(qualifier);
    }
    return R;
}

C2::ExprResult C2Sema::ActOnVarExpr(const char* name, SourceLocation loc, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG    
    std::cerr << COL_SEMA"SEMA: var expr " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    DeclExpr* declExpr = new DeclExpr(name, loc, typeExpr->takeType(), InitValue);
    delete type;
    return ExprResult(declExpr);
}

void C2Sema::visitAST(ASTVisitor& visitor) {
    for (unsigned int i=0; i<decls.size(); i++) {
        bool stop = visitor.handle(decls[i]);
        if (stop) break;
    }
}

C2::Type* C2Sema::getBuiltinType(C2Type t) const {
    static Type type_u8(Type::BUILTIN);
    type_u8.setBuiltinName("u8", "unsigned char");
    static Type type_u16(Type::BUILTIN);
    type_u16.setBuiltinName("u16", "unsigned short");
    static Type type_u32(Type::BUILTIN);
    type_u32.setBuiltinName("u32", "unsigned int");
    static Type type_s8(Type::BUILTIN);
    type_s8.setBuiltinName("s8", "char");
    static Type type_s16(Type::BUILTIN);
    type_s16.setBuiltinName("s16", "short");
    static Type type_s32(Type::BUILTIN);
    type_s32.setBuiltinName("s32", "int");
    static Type type_int(Type::BUILTIN);
    type_int.setBuiltinName("int", "int");
    static Type type_char(Type::BUILTIN);
    type_char.setBuiltinName("char", "char");
    static Type type_string(Type::BUILTIN);
    type_string.setBuiltinName("string", "const char*");
    static Type type_float(Type::BUILTIN);
    type_float.setBuiltinName("float", "float");
    static Type type_void(Type::BUILTIN);
    type_void.setBuiltinName("void", "void");

    switch (t) {
    case TYPE_U8:
        return &type_u8;
    case TYPE_U16:
        return &type_u16;
    case TYPE_U32:
        return &type_u32;
    case TYPE_S8:
        return &type_s8;
    case TYPE_S16:
        return &type_s16;
    case TYPE_S32:
        return &type_s32;
    case TYPE_INT:
        return &type_int;
    case TYPE_STRING:
        return &type_string;
    case TYPE_FLOAT:
        return &type_float;
    case TYPE_CHAR:
        return &type_char;
    case TYPE_VOID:
        return &type_void;
    }
}

const C2::Decl* C2Sema::findUse(const char* name) const {
    for (unsigned int i=0; i<decls.size(); i++) {
        Decl* d = decls[i];
        if (d->dtype() != DECL_USE) break;
        if (d->getName() == name) return d;
    }
    return 0;
}


C2::ExprResult C2Sema::ActOnNumericConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: numeric constant"ANSI_NORMAL"\n";
#endif
    // TEMP, only support integers
    const char* D = Tok.getLiteralData();
    char buffer[30];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, Tok.getLiteralData(), Tok.getLength());

    unsigned int size = 4;  // size of int
    //return Owned(IntegerLiteral::Create(Context, :e 
    // (see ActOnIntegerConstant)
    return ExprResult(new NumberExpr(Tok.getLocation(), atoi(buffer)));
}


C2::ExprResult C2Sema::ActOnStringLiteral(const Token* StringToks, unsigned int NumStringToks) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: string literal"ANSI_NORMAL"\n";
#endif
    // TEMP
    assert(NumStringToks == 1 && "only 1 string supported for now");

    // Strip off double-quotes here
    std::string text(StringToks[0].getLiteralData()+1, StringToks[0].getLength()-2);

    return ExprResult(new StringExpr(StringToks[0].getLocation(), text));
}

void C2Sema::printAST() const {
    StringBuilder buffer;
    buffer << "---- AST " << "TODO FILE.c2" << " ----\n";
    for (DeclListConstIter iter = decls.begin(); iter != decls.end(); ++iter) {
        (*iter)->print(buffer);
        buffer << '\n';
    }
    printf("%s", (const char*)buffer);
}

void C2Sema::generateC() const {
    printf("---- C-code %s.c ----\n", pkgName.c_str());

    StringBuilder buffer;

    // top levels
    for (DeclListConstIter iter = decls.begin(); iter != decls.end(); ++iter) {
        (*iter)->generateC(buffer);
        buffer << '\n';
    }
    printf("%s", (const char*)buffer);
}

DiagnosticBuilder C2Sema::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

