/* Copyright 2022 Bas van den Berg
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

#include "git_version.h"
#include "common/process_utils.h"
#include <Builder/C2Builder.h>
#include <AST/Module.h>
#include <AST/AST.h>
#include <AST/Decl.h>
#include <AST/Expr.h>
#include <Utils/StringBuilder.h>
#include <Utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace C2;
using namespace std;

static const char* plugin_name = "GitVersion";

GitVersion::GitVersion()
    : Plugin(std::string("GitVersion v1.0"))
    , verbose(false)
{
    strcpy(version, "unknown");
}


GitVersion::~GitVersion() {}

bool GitVersion::setGlobalCfg(bool verbose_, const std::string& config) {
    verbose = verbose_;
    if (verbose) Log::info(plugin_name, "global cfg [%s]", config.c_str());
    return read_git_info();
}

bool GitVersion::setTargetCfg(bool verbose_, const std::string& config) {
    verbose = verbose_;
    if (verbose) Log::info(plugin_name, "target cfg [%s]", config.c_str());
    return read_git_info();
}

void GitVersion::build(C2Builder& builder) {
    Module* m = builder.addPluginModule("git_version");
    if (!m) exit(-1);

    AST* ast = new AST("<generated>", false, false);
    c2lang::SourceLocation loc;
    ast->setName("git_version", loc);
    m->addAST(ast);
    ASTContext& Context = ast->getContext();
    {
        QualType inner = Type::Int8();
        inner.addConst();
        //QualType QT = new (Context) PointerType(inner);
        QualType QT = new (Context) ArrayType(inner, NULL, false);
        // TODO need to set size in canonical type?
        QT->setCanonicalType(QT);
        unsigned len = strlen(version);
        const char* value = Context.addIdentifier(version, len);
        Expr* init = new (Context) StringLiteral(loc, value, len);
        init->setCTC();
        init->setType(QT);
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, "describe", loc, QT, init, true, m);
        var->setType(QT);
        ast->addVar(var);
        m->addSymbol(var);
    }

    ast->setChecked();
}

bool GitVersion::read_git_info()
{
    int ret = process_run(".", "git", "describe --tags --always --dirty", version);
    if (ret != 0) {
        Log::info(plugin_name, "no git archive?");
        return false;
    }
    if (verbose) Log::info(plugin_name, "git-version: %s", version);
    return true;
}
