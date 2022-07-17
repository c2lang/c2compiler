/* Copyright 2013-2021 Bas van den Berg
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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <vector>
#include "Builder/C2Builder.h"

namespace C2 {

class Plugin;

class PluginManager : public PluginHandler {
public:
    PluginManager(bool verbose_);
    ~PluginManager();

    void print() const;
    void show() const;

    void addPath(const std::string& path);

    // global config
    bool loadGlobal(const std::string& plugin_name, const std::string& plugin_config, bool from_recipe);
    // target-specific config
    bool loadLocal(const std::string& plugin_name, const std::string& plugin_config);

    void endTarget();

    virtual void beginTarget(C2Builder& builder);
    virtual bool build(C2Builder& builder);
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& src_mgr);
private:
    struct PluginWrapper {
        Plugin* plugin;
        void* handle;
        std::string name;
        bool is_global;;
        bool active;

        PluginWrapper(Plugin* p, void* h, const std::string& name_)
            : plugin(p), handle(h), name(name_)
            , is_global(false), active(false)
            {}
    };

    PluginWrapper* loadPlugin(const std::string& plugin_name, bool from_recipe);
    void removePlugin(PluginWrapper& plugin);
    bool find_file(const std::string& filename, std::string& fullname);

    typedef std::vector<PluginWrapper> Plugins;
    Plugins plugins;
    typedef std::vector<std::string> Paths;
    Paths paths;
    bool verbose;
};

}

#endif

