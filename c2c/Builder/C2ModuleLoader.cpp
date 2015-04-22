/* Copyright 2013,2014,2015 Bas van den Berg
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

using namespace C2;

C2::Module* C2ModuleLoader::load(const std::string& name) {

    // NOTE: MEMLEAK on Types

    clang::SourceLocation loc;

    if (name == "stdio") {
        Module* stdioMod = new C2::Module("stdio", true, true);
        // int puts(const char* s);
        {
            FunctionDecl* func = new FunctionDecl("puts", loc, true, Type::Int32());
            // TODO correct arg
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "s", loc, QT, 0, true);
            Arg1->setType(QT);
            func->addArg(Arg1);
            stdioMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        //int printf(const char *format, ...);
        {
            FunctionDecl* func = new FunctionDecl("printf", loc, true, Type::Int32());
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "format", loc, QT, 0, true);
            Arg1->setType(QT);
            func->addArg(Arg1);
            func->setVariadic();
            stdioMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        //int sprintf(char *str, const char *format, ...);
        {
            FunctionDecl* func = new FunctionDecl("sprintf", loc, true, Type::Int32());
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "str", loc, QT, 0, true);
            Arg1->setType(QT);
            func->addArg(Arg1);
            VarDecl* Arg2 = new VarDecl(VARDECL_PARAM, "format", loc, QT, 0, true);
            Arg2->setType(QT);
            func->addArg(Arg2);
            func->setVariadic();
            stdioMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        return stdioMod;
    }

    if (name == "string") {
        Module* stringMod = new C2::Module("string", true, true);
        // void *memset(void *s, int c, size_t n);
        {
            QualType VP(new PointerType(Type::Void()));
            VP->setCanonicalType(VP);
            FunctionDecl* func = new FunctionDecl("memset", loc, true, VP);
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "s", loc, VP, 0, true);
            Arg1->setType(VP);
            func->addArg(Arg1);
            VarDecl* Arg2 = new VarDecl(VARDECL_PARAM, "c", loc, Type::Int32(), 0, true);
            Arg2->setType(Type::Int32());
            func->addArg(Arg2);
            // TEMP size_t -> uint32
            VarDecl* Arg3 = new VarDecl(VARDECL_PARAM, "n", loc, Type::UInt32(), 0, true);
            Arg3->setType(Type::UInt32());
            func->addArg(Arg3);
            stringMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        return stringMod;

    }

    if (name == "stdlib") {
        Module* stdlibMod = new C2::Module("stdlib", true, true);
        //void exit(int status);
        {
            FunctionDecl* func = new FunctionDecl("exit", loc, true, Type::Void());
            // TODO correct arg
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "status", loc, Type::Int32(), 0, true);
            Arg1->setType(Type::Int32());
            func->addArg(Arg1);
            stdlibMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        return stdlibMod;
    }

    if (name == "c2") {
        Module* c2Mod = new C2::Module("c2", true, false);
        // uint64 buildtime
        {
            // make constant, CTC_NONE
            QualType QT = Type::UInt64();
            QT.addConst();
            uint64_t value = time(0);
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, value, false));
            // TODO get error without value if CTC_NONE, CTC_FULL gives out-of-bounds for value 123?!!
            init->setCTC(CTC_NONE); // Don't check range, only type
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "buildtime", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int8 min_int8
        {
            QualType QT = Type::Int8();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, -128, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_int8", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int8 max_int8
        {
            QualType QT = Type::Int8();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 127, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_int8", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint8 min_uint8
        {
            QualType QT = Type::UInt8();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 0, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_uint8", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint8 max_uint8
        {
            QualType QT = Type::UInt8();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 255, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_uint8", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int16 min_int16
        {
            QualType QT = Type::Int16();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, -32768, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_int16", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int16 max_int16
        {
            QualType QT = Type::Int16();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 32767, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_int16", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint16 min_uint16
        {
            QualType QT = Type::UInt16();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 0, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_uint16", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint16 max_uint16
        {
            QualType QT = Type::UInt16();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 65535, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_uint16", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int32 min_int32
        {
            QualType QT = Type::Int32();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, -2147483648, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_int32", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int32 max_int32
        {
            QualType QT = Type::Int32();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 2147483647, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_int32", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint32 min_uint32
        {
            QualType QT = Type::UInt32();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 0, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_uint32", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint32 max_uint32
        {
            QualType QT = Type::UInt32();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 4294967295, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_uint32", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int64 min_int64
        {
            QualType QT = Type::Int64();
            QT.addConst();
            // NOTE: minimum should be -..808, but clang complains about it..
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, -9223372036854775807ll, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_int64", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // int64 max_int64
        {
            QualType QT = Type::Int64();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 9223372036854775807ll, true));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_int64", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint64 min_uint64
        {
            QualType QT = Type::UInt64();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 0, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "min_uint64", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        // uint64 max_uint64
        {
            QualType QT = Type::UInt64();
            QT.addConst();
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, 18446744073709551615llu, false));
            init->setCTC(CTC_FULL);
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "max_uint64", loc, QT, init, true);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        return c2Mod;
    }

    return 0;
}

