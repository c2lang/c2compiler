/* Copyright 2013-2017 Bas van den Berg
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

#include <clang/Basic/SourceLocation.h>

#include "Builder/C2ModuleLoader.h"
#include "AST/Module.h"
#include "AST/Decl.h"
#include "AST/Type.h"
#include "AST/Expr.h"
#include "AST/ASTContext.h"

using namespace C2;

struct CTypes {
    const char* name;
    QualType type;
};

// NOTE: hardcoded for x86_64 architecture
static CTypes ctypes[] = {
    { "c_char",      Type::Int8() },
    { "c_int",       Type::Int32() },
    { "c_uint",      Type::UInt32() },
    { "c_long",      Type::Int64() },
    { "c_ulong",     Type::UInt64() },
    { "c_size",      Type::UInt64() },
    { "c_ssize",     Type::Int64() },
    { "c_longlong",  Type::Int64() },
    { "c_ulonglong", Type::UInt64() },
    { "c_float",     Type::Float32() },
    { "c_double",    Type::Float64() },
};

void C2ModuleLoader::load(C2::Module* c2Mod) {
    clang::SourceLocation loc;

    AST* ast = new AST("<generated>", false);
    ASTContext& Context = ast->getASTContext();
    ast->setName("c2", loc);
    c2Mod->addAST(ast);

    // uint64 buildtime
    {
        // make constant, CTC_NONE
        QualType QT = Type::UInt64();
        QT.addConst();
        uint64_t value = time(0);
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, value, false));
        // TODO get error without value if CTC_NONE, CTC_FULL gives out-of-bounds for value 123?!!
        init->setCTC(CTC_NONE); // Don't check range, only type
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "buildtime", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int8 min_int8
    {
        QualType QT = Type::Int8();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, -128, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_int8", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int8 max_int8
    {
        QualType QT = Type::Int8();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 127, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_int8", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint8 min_uint8
    {
        QualType QT = Type::UInt8();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 0, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_uint8", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint8 max_uint8
    {
        QualType QT = Type::UInt8();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 255, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_uint8", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int16 min_int16
    {
        QualType QT = Type::Int16();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, -32768, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_int16", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int16 max_int16
    {
        QualType QT = Type::Int16();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 32767, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_int16", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint16 min_uint16
    {
        QualType QT = Type::UInt16();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 0, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_uint16", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint16 max_uint16
    {
        QualType QT = Type::UInt16();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 65535, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_uint16", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int32 min_int32
    {
        QualType QT = Type::Int32();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, -2147483648, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_int32", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int32 max_int32
    {
        QualType QT = Type::Int32();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 2147483647, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_int32", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint32 min_uint32
    {
        QualType QT = Type::UInt32();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 0, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_uint32", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint32 max_uint32
    {
        QualType QT = Type::UInt32();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 4294967295, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_uint32", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int64 min_int64
    {
        QualType QT = Type::Int64();
        QT.addConst();
        // NOTE: minimum should be -..808, but clang complains about it..
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, -9223372036854775807ll, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_int64", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // int64 max_int64
    {
        QualType QT = Type::Int64();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 9223372036854775807ll, true));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_int64", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint64 min_uint64
    {
        QualType QT = Type::UInt64();
        QT.addConst();
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 0, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "min_uint64", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }
    // uint64 max_uint64
    {
        QualType QT = Type::UInt64();
        QT.addConst();
        // NOTE: capped to -1 since APInt is always signed?
        Expr* init = new (Context) IntegerLiteral(loc, llvm::APInt(64, 18446744073709551615llu, false));
        init->setCTC(CTC_FULL);
        init->setConstant();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "max_uint64", loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        c2Mod->addSymbol(var);
    }

#if 0
    // type c_char int8
    {
        QualType QT = Type::Int8();
        static const char* name = "c_char";
        AliasTypeDecl* T = new (Context) AliasTypeDecl(name, loc, QT, true);
        QualType A = Context.getAliasType(T, QT);
        A->setCanonicalType(QT);
        T->setType(A);
        c2Mod->addSymbol(T);
    }
#endif
    // create C types (depend on target)
    // NOTE: all types are lower-cased!
    for (unsigned i=0; i<sizeof(ctypes)/sizeof(ctypes[0]); i++) {
        QualType QT = ctypes[i].type;
        AliasTypeDecl* T = new (Context) AliasTypeDecl(ctypes[i].name, loc, QT, true);
        QualType A = Context.getAliasType(T, QT);
        A->setCanonicalType(QT);
        T->setType(A);
        ast->addType(T);
        c2Mod->addSymbol(T);
    }
}

