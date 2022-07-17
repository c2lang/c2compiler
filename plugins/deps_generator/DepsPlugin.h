/* Copyright 2013-2022 Bas van den Berg
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

#ifndef DEPS_PLUGIN_H
#define DEPS_PLUGIN_H

#include <Builder/Plugin.h>

namespace C2 {

class DepsPlugin : public Plugin {
public:
    DepsPlugin();
    virtual ~DepsPlugin();

    virtual bool setGlobalCfg(bool verbose, const std::string& config);
    virtual bool setTargetCfg(bool verbose, const std::string& config);

    virtual void beginTarget(C2Builder& builder);
    virtual void build(C2Builder& builder) {}
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& sm_);
private:
    bool parseConfig(const std::string& config, bool global);
    struct Config {
        void init() {
            showFiles = false;
            showPrivate = false;
            showExternals = false;
        }
        bool showFiles;
        bool showPrivate;
        bool showExternals;
    };
    Config globalConfig;
    Config localConfig;
    Config* cur;
};

}

extern "C" C2::Plugin *createPlugin() {
    return new C2::DepsPlugin();
}
extern "C" void deletePlugin(C2::Plugin* p) {
    delete p;
}

#endif

