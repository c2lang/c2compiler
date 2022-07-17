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

#include "shell_cmd.h"
#include <Builder/C2Builder.h>
#include <AST/Module.h>
#include <AST/AST.h>
#include <AST/Decl.h>
#include <AST/Expr.h>
#include <Utils/StringBuilder.h>
#include <Utils/Log.h>
#include "common/file_utils.h"
#include "common/process_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace C2;
using namespace std;

static const char* plugin_name = "ShellCmd";

ShellCmd::ShellCmd()
    : Plugin(std::string("ShellCmd v1.0"))
    , globalCfg("shell_cmd")
    , targetCfg("shell_cmd")
    , cur(&globalCfg)
{}

ShellCmd::~ShellCmd() {}

bool ShellCmd::setGlobalCfg(bool verbose, const std::string& config) {
    if (verbose) Log::info(plugin_name, "global cfg [%s]", config.c_str());

    return parse_yaml(config);
}

bool ShellCmd::setTargetCfg(bool verbose, const std::string& config) {
    if (verbose) Log::info(plugin_name, "target cfg [%s]", config.c_str());

    cur = &targetCfg;
    return parse_yaml(config);
}

void ShellCmd::beginTarget(C2Builder& builder) {
    cur = &globalCfg;
}

void ShellCmd::build(C2Builder& builder) {
    // TODO determine global/target
    Module* m = builder.addPluginModule(cur->module_name);
    if (!m) exit(-1);

    AST* ast = new AST("<generated>", false, false);
    c2lang::SourceLocation loc;
    ast->setName(cur->module_name, loc);
    m->addAST(ast);
    // TODO read from file
    ASTContext& Context = ast->getContext();

    for (unsigned i=0; i<cur->results.size(); i++) {
        const std::string& variable = cur->results[i].first;
        const std::string& value = cur->results[i].second;

        QualType inner = Type::Int8();
        inner.addConst();
        //QualType QT = new (Context) PointerType(inner);
        QualType QT = new (Context) ArrayType(inner, NULL, false);
        // TODO need to set size in canonical type?
        QT->setCanonicalType(QT);
        unsigned len = strlen(value.c_str()); // TODO use std::string func
        const char* value2 = Context.addIdentifier(value.c_str(), len);
        Expr* init = new (Context) StringLiteral(loc, value2, len);
        init->setCTC();
        init->setType(QT);
        const char* name = Context.addIdentifier(variable.c_str(), strlen(variable.c_str()));
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, name, loc, QT, init, true, m);
        var->setType(QT);
        ast->addVar(var);
        m->addSymbol(var);
    }

    ast->setChecked();
}

bool ShellCmd::parse_yaml(const std::string& config_file)
{
    FileReader reader;
    bool ok = file_reader_open(&reader, config_file.c_str());
    if (!ok) {
        Log::error(plugin_name, "error opening %s: %s", config_file.c_str(), strerror(errno));
        return false;
    }

    YamlParser* parser = yaml_create((const char*)reader.map, reader.size);
    ok = yaml_parse(parser);
    if (!ok) {
        Log::error(plugin_name, "invalid config: %s", yaml_get_error(parser));
    } else {
        ok = parse_config(parser);
    }

    yaml_destroy(parser);
    file_reader_close(&reader);

    return ok;
}

bool ShellCmd::parse_config(const YamlParser* parser) {
    const char* mod_name = yaml_get_scalar_value(parser, "shell.module");
    if (mod_name) cur->module_name = mod_name;

    const YamlNode* yaml_cmds = yaml_find_node(parser, "shell.commands");
    if (!yaml_cmds) {
        Log::error(plugin_name, "no 'shell.commands' found");
        return false;
    }
    YamlIter iter = yaml_node_get_child_iter(parser, yaml_cmds);
    while (!yaml_iter_done(&iter)) {
        Cmd cmd;
        const char* cmd_name = yaml_iter_get_name(&iter);
        cmd.name = cmd_name;
        const char* cmd_path = yaml_iter_get_child_scalar_value(&iter, "path");
        if (cmd_path) cmd.path = cmd_path;
        else cmd.path = ".";

        const char* cmd_cmd = yaml_iter_get_child_scalar_value(&iter, "cmd");
        if (!cmd_cmd) {
            Log::error(plugin_name, "no %s.cmd found", cmd_name);
            return false;
        }
        cmd.cmd = cmd_cmd;

        // if no variable, take same as command
        const char* cmd_var = yaml_iter_get_child_scalar_value(&iter, "variable");
        if (!cmd_var) cmd_var = cmd_name;
        // TODO check cmd_var for valid variable name
        cmd.variable = cmd_var;

        const char* cmd_timeout = yaml_iter_get_child_scalar_value(&iter, "timeout");
        // TODO check if number
        if (!cmd_timeout) cmd_timeout = "1";
        cmd.timeout_sec = atoi(cmd_timeout);

        if (!run_cmd(cmd)) return false;
        yaml_iter_next(&iter);
    }
    return true;
}

bool ShellCmd::run_cmd(const Cmd& cmd) {
    char cmd_str[512];
    char args[512];
    process_split_cmd(cmd.cmd.c_str(), cmd_str, args);

    // if verbose, print running .. + result
    char out[512];
    out[0] = 0;
    int res = process_run(cmd.path.c_str(), cmd_str, args, out);
    if (res != 0) {
        Log::error(plugin_name, "error running cmd %s", cmd.cmd.c_str());
        return false;
    }
    cur->results.push_back(Result(cmd.variable, out));
    return true;
}

