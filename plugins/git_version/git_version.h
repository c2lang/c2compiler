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

#ifndef GIT_VERSION_H
#define GIT_VERSION_H

#include <Builder/Plugin.h>

namespace C2 {

class GitVersion : public Plugin {
public:
    GitVersion();
    virtual ~GitVersion();

    virtual bool setGlobalCfg(bool verbose_, const std::string& config);
    virtual bool setTargetCfg(bool verbose_, const std::string& config);
    virtual void build(C2Builder& builder);
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) {
        return true;
    }
private:
    bool read_git_info();
    bool verbose;

    char version[256];
};

}

extern "C" C2::Plugin *createPlugin() {
    return new C2::GitVersion();
}
extern "C" void deletePlugin(C2::Plugin* p) {
    delete p;
}

#endif

