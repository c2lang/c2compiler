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

#ifndef BUILDER_PLUGIN_H
#define BUILDER_PLUGIN_H

#include <string>

namespace c2lang {
class SourceManager;
}

namespace C2 {

class C2Builder;

class Plugin {
public:
    Plugin(const std::string& name_);
    virtual ~Plugin() {}

    virtual bool init(bool verbose, const std::string& config)  = 0;
    virtual void build(C2Builder&) = 0;
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) = 0;
private:
    std::string name;
};

}

#endif

