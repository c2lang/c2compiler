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

#ifndef LOAD_FILE_H
#define LOAD_FILE_H

#include <Builder/Plugin.h>
#include "common/yaml_parser.h"
#include <vector>
#include <stdint.h>

namespace C2 {

class StringBuilder;

class LoadFile : public Plugin {
public:
    LoadFile();
    virtual ~LoadFile();

    virtual bool setGlobalCfg(bool verbose, const std::string& config);
    virtual bool setTargetCfg(bool verbose, const std::string& config);
    virtual void build(C2Builder& builder);
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) {
        return true;
    }

private:
    struct File {
        std::string variable;
        uint8_t* data;    // malloc-ed
        uint32_t len;

        File(const char* var, uint8_t* data_, uint32_t len_)
            : variable(var)
            , data(data_)
            , len(len_)     // including 0-termination
        {}
    };

    bool parse_yaml(const char* filename);
    bool parse_config(const YamlParser* parser, const char* filename);
    bool load_file(const char* variable, const char* filename, bool terminate, const char* config_file);
    bool variable_exists(const char* name) const;
    void escape(StringBuilder& out, const File& file);
    void free_files();

    typedef std::vector<File> Results;
    Results results;

    std::string module_name;
};

}

extern "C" C2::Plugin *createPlugin() {
    return new C2::LoadFile();
}
extern "C" void deletePlugin(C2::Plugin* p) {
    delete p;
}

#endif

