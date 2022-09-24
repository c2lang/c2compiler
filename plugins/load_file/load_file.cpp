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

#include "load_file.h"
#include "common/file_utils.h"

#include <Builder/C2Builder.h>
#include <Builder/Recipe.h>
#include <AST/Module.h>
#include <AST/AST.h>
#include <AST/Decl.h>
#include <AST/Expr.h>
#include <Utils/StringBuilder.h>
#include <Utils/Log.h>

#include <FileUtils/FileUtils.h>

using namespace C2;
using namespace std;

static const char* files_name = "file.files";
static const char* plugin_name = "LoadFile";

LoadFile::LoadFile()
    : Plugin(std::string("LoadFile v1.0"))
    , module_name("load_file")
{}

LoadFile::~LoadFile() {
    free_files();
}

bool LoadFile::setGlobalCfg(bool verbose, const std::string& config) {
    Log::error(plugin_name, "this plugin is only allowed at target scope");
    return false;
}

bool LoadFile::setTargetCfg(bool verbose, const std::string& config) {
    free_files();

    const char* config_file = config.c_str();
    if (config_file[0] == 0) config_file = "load_file.yaml";
    if (verbose) Log::info(plugin_name, "target cfg [%s]", config_file);

    return parse_yaml(config_file);
}

void LoadFile::escape(StringBuilder& out, const File& file) {
    out.indent(3);
    for (unsigned i=0; i<file.len; i++) {
        out.print("0x%02X,", file.data[i]);
        if (i%20 == 19) {
            out << '\n';
            out.indent(3);
        }
    }
    out << '\n';
}

void LoadFile::build(C2Builder& builder) {
#if 1
    // NOTE: generate a .C2 file and add it to the recipe.

    StringBuilder out(1024*1024);   // TEMP hardcoded 1Mb output limit (input file ~200 Kb)

    out << "module " << module_name << ";\n\n";
    out << "public type File struct {\n";
    out << "   const u8* data;\n";
    out << "   u32 size;\n",
        out << "}\n\n";

    for (unsigned i=0; i<results.size(); i++) {
        const File& file = results[i];
        out << "public const u8[] Data_" << file.variable << " = {\n";
        escape(out, file);
        out << "}\n\n";

        out << "public const File " << file.variable << " = {\n";
        out << "   .data = Data_" << file.variable << ",\n";
        out << "   .size = " << file.len << ",\n";
        out << "}\n\n";
    }

    const std::string& path = builder.getOutputDir();
    std::string fullname = path + "loadfile.c2";
    FileUtils::writeFile(path.c_str(), fullname, out);

    Recipe& recipe = builder.getRecipe();
    recipe.addFile(fullname);
#else
    // NOTE: generate the AST directly
    Module* m = builder.addPluginModule(module_name);
    if (!m) exit(-1);

    AST* ast = new AST("<generated>", false, false);
    c2lang::SourceLocation loc;
    ast->setName(module_name, loc);
    m->addAST(ast);
    ASTContext& Context = ast->getContext();

    for (unsigned i=0; i<results.size(); i++) {
        const File& file = results[i];

        // TODO create const uint8[] data, uint32_t size -> also needs Files type!!
        QualType inner = Type::UInt8();
        inner.addConst();
        QualType QT = new (Context) ArrayType(inner, NULL, false);
        QT->setCanonicalType(QT);
        // NOTE: addIdentifier always adds 0-terminator
        const char* value2 = Context.addIdentifier((char*)file.data, file.len);
        Expr* init = new (Context) StringLiteral(loc, value2, file.len);
        init->setCTC(CTC_NONE); // Don't check range, only type
        init->setConstant();
        init->setType(QT);

        const char* name2 = Context.addIdentifier(file.variable.c_str(), strlen(file.variable.c_str()));
        VarDecl* var = new (Context) VarDecl(VARDECL_GLOBAL, name2, loc, QT, init, true);
        var->setType(QT);
        ast->addVar(var);
        m->addSymbol(var);
    }

    ast->setChecked();
#endif

    free_files();
}

bool LoadFile::parse_yaml(const char* config_file)
{
    FileReader reader;
    bool ok = file_reader_open(&reader, config_file);
    if (!ok) {
        Log::error(plugin_name, "error opening %s: %s", config_file, strerror(errno));
        return false;
    }

    YamlParser* parser = yaml_create((const char*)reader.map, reader.size);
    ok = yaml_parse(parser);
    if (!ok) {
        Log::error(plugin_name, "invalid config: %s", yaml_get_error(parser));
    } else {
        ok = parse_config(parser, config_file);
    }

    yaml_destroy(parser);
    file_reader_close(&reader);

    return ok;
}

bool LoadFile::parse_config(const YamlParser* parser, const char* config_file) {
    const char* mod_name = yaml_get_scalar_value(parser, "file.module");
    if (mod_name) module_name = mod_name;

    const YamlNode* files = yaml_find_node(parser, files_name);
    if (!files) {
        Log::error(plugin_name, "no '%s' found in %s", files_name, config_file);
        return false;
    }

    YamlIter iter = yaml_node_get_child_iter(parser, files);
    while (!yaml_iter_done(&iter)) {

        const char* variable = yaml_iter_get_child_scalar_value(&iter, "variable");
        if (!variable) {
            Log::error(plugin_name, "no 'variable' found for file in %s", config_file);
            return false;
        }
        if (variable_exists(variable)) {
            Log::error(plugin_name, "variable '%s' already exists in %s", variable, config_file);
            return false;
        }
        // TODO check valid variable name (utils)
        if (!isupper(variable[0])) {
            Log::error(plugin_name, "variable '%s' must start with an Upper case in %s", variable, config_file);
            return false;
        }

        const char* filename = yaml_iter_get_child_scalar_value(&iter, "file");
        if (!filename) {
            Log::error(plugin_name, "no 'variable' found for file in %s", config_file);
            return false;
        }

        const char* terminate_str = yaml_iter_get_child_scalar_value(&iter, "null-terminate");
        bool terminate = false;
        if (terminate_str && strcmp(terminate_str, "true") == 0) terminate = true;

        if (!load_file(variable, filename, terminate, config_file)) return false;

        yaml_iter_next(&iter);
    }
    return true;
}

bool LoadFile::load_file(const char* variable, const char* filename, bool terminate, const char* config_file) {
    FileReader reader;
    bool ok = file_reader_open(&reader, filename);
    if (!ok) {
        Log::error(plugin_name, "error opening %s: %s", filename, strerror(errno));
        return false;
    }

    if (reader.size == 0) {
        file_reader_close(&reader);
        Log::error(plugin_name, "file %s is empty", filename);
        return false;
    }

    uint32_t len = reader.size;
    if (terminate) len++;
    uint8_t* data = (uint8_t*)malloc(len);
    memcpy(data, reader.map, reader.size);
    if (terminate) data[reader.size] = 0;

    results.push_back(File(variable, data, len));
    file_reader_close(&reader);
    return true;
}

bool LoadFile::variable_exists(const char* name) const {
    for (unsigned i=0; i<results.size(); i++) {
        const File& file = results[i];
        if (file.variable == name) return true;
    }
    return false;
}

void LoadFile::free_files() {
    for (unsigned i=0; i<results.size(); i++) {
        File& file = results[i];
        free(file.data);
    }
    results.clear();
}
