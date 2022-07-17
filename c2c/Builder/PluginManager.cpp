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
#include <sys/types.h>
#include <dirent.h>

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

static bool is_plugin(const struct dirent* entry) {
    if (entry->d_type != DT_REG) return false;
    if (entry->d_name[0] == '.') return false;
    int len = strlen(entry->d_name);
    if (len < 5) return false;
    if (strcmp(&entry->d_name[len-3], ".so") == 0) return true;
    return false;
}

void PluginManager::show() const {
    printf("Plugins:\n");
    for (unsigned i=0; i<paths.size(); i++) {
        const char* dirname = paths[i].c_str();
        DIR* dir = opendir(dirname);
        if (dir == 0) {
            fprintf(stderr, "error reading %s: %s\n", dirname, strerror(errno));
            continue;
        }
        struct dirent* cur = readdir(dir);
        while (cur != 0) {
            if (is_plugin(cur)) {
                printf("  %s/%s\n", dirname, cur->d_name);
            }
            // TODO try to load and run plugin->showInfo();
            cur = readdir(dir);
        }
        closedir(dir);
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

bool PluginManager::loadGlobal(const std::string& plugin_name, const std::string& plugin_config, bool from_recipe) {
    PluginWrapper* wrapper = loadPlugin(plugin_name, from_recipe);
    if (!wrapper) return false;

    wrapper->is_global = true;
    if (!wrapper->plugin->setGlobalCfg(verbose, plugin_config)) {
        fprintf(stderr, "Error initializing plugin %s\n", plugin_name.c_str());
        // TODO free Plugin?
        return false;
    }
    return true;
}

bool PluginManager::loadLocal(const std::string& plugin_name, const std::string& plugin_config) {
    PluginWrapper* wrapper = loadPlugin(plugin_name, true);
    if (!wrapper) return false;

    wrapper->active = true;
    if (!wrapper->plugin->setTargetCfg(verbose, plugin_config)) {
        fprintf(stderr, "Error initializing plugin %s\n", plugin_name.c_str());
        // TODO free Plugin?
        return false;
    }
    return true;
}

PluginManager::PluginWrapper* PluginManager::loadPlugin(const std::string& plugin_name, bool from_recipe) {
    for (unsigned i=0; i<plugins.size(); i++) {
        PluginWrapper& wrapper = plugins[i];
        if (wrapper.name == plugin_name) {
            return &wrapper; // ignore duplicates between build + recipe file, keep options from build-file
        }
    }
    const char* bare_name = Utils::getFileName(plugin_name);
    if (verbose) Log::log(COL_VERBOSE, "loading plugin %s from %s", bare_name, from_recipe ? "recipe" : "build-file");

    std::string filename = plugin_name + ".so";

    std::string fullname;
    if (!find_file(filename, fullname)) {
        fprintf(stderr, "Error: cannot find plugin %s\n", filename.c_str());
        return 0;
    }
    if (verbose) Log::log(COL_VERBOSE, "found plugin %s: %s", plugin_name.c_str(), fullname.c_str());

    Handle handle(dlopen(fullname.c_str(), RTLD_NOW | RTLD_GLOBAL));
    if(!handle.val()) {
        fprintf(stderr, "Error loading plugin %s: %s\n", fullname.c_str(), dlerror());
        return 0;
    }

    LoadFunc loadfn = (LoadFunc)dlsym(handle.val(), "createPlugin");
    char* error = dlerror();
    if (error) {
        fprintf(stderr, "Error loading plugin %s: %s\n", fullname.c_str(), error);
        return 0;
    }

    Plugin* p = loadfn();
    if (p == 0) {
        fprintf(stderr, "Error loading plugin %s: plugin error\n", fullname.c_str());
        return 0;
    }

    plugins.push_back(PluginWrapper(p, handle.get(), plugin_name));
    return &plugins[plugins.size()-1];
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

void PluginManager::beginTarget(C2Builder& builder) {
    for (unsigned i=0; i<plugins.size(); i++) {
        PluginWrapper& wrapper = plugins[i];
        if (wrapper.is_global && wrapper.plugin->allTargets()) {
            wrapper.active = true;
            wrapper.plugin->beginTarget(builder);
        } else {
            wrapper.active = false;
        }
    }
}

void PluginManager::endTarget() {
    // TODO needed?
    for (unsigned i=0; i<plugins.size(); i++) {
        PluginWrapper& wrapper = plugins[i];
        if (wrapper.active) {
            //wrapper.plugin->endTarget();
        }
    }
}

bool PluginManager::build(C2Builder& builder) {
    for (unsigned i=0; i<plugins.size(); i++) {
        PluginWrapper& wrapper = plugins[i];
        if (wrapper.active) {
            if (verbose) Log::log(COL_VERBOSE, "plugin %s build()", wrapper.name.c_str());
            wrapper.plugin->build(builder);
        }
    }
    return true;
}

bool PluginManager::generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) {
    for (unsigned i=0; i<plugins.size(); i++) {
        PluginWrapper& wrapper = plugins[i];
        if (wrapper.active) {
            if (verbose) Log::log(COL_VERBOSE, "plugin %s generate()", wrapper.name.c_str());
            if (!wrapper.plugin->generate(builder, src_mgr)) return false;
        }
    }
    return true;
}

