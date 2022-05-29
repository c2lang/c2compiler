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

#include "Builder/PluginManager.h"
#include "Builder/Plugin.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"
#include "Utils/color.h"

#include <dlfcn.h>
#include <string>

using namespace C2;
using namespace std;

typedef Plugin* (*LoadFunc)();
typedef void (*UnloadFunc)(Plugin*);

namespace C2 {

class Handle {
public:
    Handle(void* handle_) : handle(handle_) {}
    ~Handle() { if (handle) dlclose(handle); }
    void* val() const { return handle; }
    void* get() {
        void *tmp = handle;
        handle = 0;
        return tmp;
    }
private:
    void* handle;
};

}


PluginManager::PluginManager(bool verbose_)
    : verbose(verbose_)
{}

PluginManager::~PluginManager() {
    while (!plugins.empty()) {
        removePlugin(plugins.back());
        plugins.pop_back();
    }
}

void PluginManager::print() const {
    for (unsigned i=0; i<plugins.size(); i++) {
        const PluginWrapper& w = plugins[i];
        printf("%s\n", w.name.c_str());
    }
}

void PluginManager::addPath(const std::string& path) {
    paths.push_back(path);
}

bool PluginManager::find_file(const std::string& filename, std::string& fullname) {
    for (unsigned i=0; i<paths.size(); i++) {
        std::string tmp = paths[i] + '/' + filename;
        if (Utils::file_exists(tmp)) {
            fullname = tmp;
            return true;
        }
    }
    return false;
}

bool PluginManager::load(const std::string& plugin_name, const std::string& plugin_config, bool from_recipe) {
    const char* bare_name = Utils::getFileName(plugin_name);
    for (unsigned i=0; i<plugins.size(); i++) {
        if (plugins[i].name == bare_name) return true; // ignore duplicates between build + recipe file
    }
    if (verbose) Log::log(COL_VERBOSE, "loading plugin %s [%s] from %s", bare_name, plugin_config.c_str(), from_recipe ? "recipe" : "build-file");

    std::string filename = plugin_name + ".so";

    std::string fullname;
    if (!find_file(filename, fullname)) {
        fprintf(stderr, "Error: cannot find %s\n", filename.c_str());
        return false;
    }
    if (verbose) Log::log(COL_VERBOSE, "found plugin %s: %s", plugin_name.c_str(), fullname.c_str());

    Handle handle(dlopen(fullname.c_str(), RTLD_NOW | RTLD_GLOBAL));
    if(!handle.val()) {
        fprintf(stderr, "Error loading %s: %s\n", fullname.c_str(), dlerror());
        return false;
    }

    LoadFunc loadfn = (LoadFunc)dlsym(handle.val(), "createPlugin");
    char* error = dlerror();
    if (error) {
        fprintf(stderr, "Error loading %s: %s\n", fullname.c_str(), error);
        return false;
    }

    Plugin* p = loadfn();
    if (p == 0) {
        fprintf(stderr, "Error loading %s: plugin error\n", fullname.c_str());
        return false;
    }

    if (!p->init(verbose, plugin_config)) {
        fprintf(stderr, "Error initializing %s\n", fullname.c_str());
        return false;
    }
    plugins.push_back(PluginWrapper(p, handle.get(), plugin_name));
    return true;
}

void PluginManager::removePlugin(PluginWrapper& wrapper) {
    Handle handle(wrapper.handle);
    UnloadFunc unloadPlugin = (UnloadFunc)dlsym(wrapper.handle, "deletePlugin");
    char* error = dlerror();
    if (error) {
        fprintf(stderr, "Error loading <plugin>: %s\n", error);
        return;
    }
    unloadPlugin(wrapper.plugin);
}

bool PluginManager::build(C2Builder& builder) {
    for (unsigned i=0; i<plugins.size(); i++) {
        plugins[i].plugin->build(builder);
    }
    return true;
}

bool PluginManager::generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) {
    for (unsigned i=0; i<plugins.size(); i++) {
        if (!plugins[i].plugin->generate(builder, src_mgr)) return false;
    }
    return true;
}

