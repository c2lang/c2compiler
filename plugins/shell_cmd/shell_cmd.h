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

#ifndef SHELL_CMD_H
#define SHELL_CMD_H

#include <Builder/Plugin.h>
#include <FileUtils/YamlParser.h>
#include <vector>

namespace C2 {

class ShellCmd : public Plugin {
public:
    ShellCmd();
    virtual ~ShellCmd();

    virtual bool setGlobalCfg(bool verbose, const std::string& config);
    virtual bool setTargetCfg(bool verbose, const std::string& config);

    virtual void beginTarget(C2Builder& builder);
    virtual void build(C2Builder& builder);
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) {
        return true;
    }

private:
    struct Cmd {
        std::string name;
        std::string variable;
        std::string path;
        std::string cmd;
        unsigned timeout_sec;
    };

    bool parse_yaml(const std::string& config_file);
    bool parse_config(const YamlParser* parser);
    bool run_cmd(const Cmd& cmd);

    typedef std::pair<std::string, std::string>Result;
    typedef std::vector<Result> Results;

    struct Cfg {
        Cfg(const std::string& name_) : module_name(name_) {}
        Results results;
        std::string module_name;
    };

    Cfg globalCfg;
    Cfg targetCfg;
    Cfg* cur;
};

}

extern "C" C2::Plugin *createPlugin() {
    return new C2::ShellCmd();
}
extern "C" void deletePlugin(C2::Plugin* p) {
    delete p;
}

#endif

