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

#include "DepsPlugin.h"
#include <Builder/C2Builder.h>
#include "DepGenerator.h"
#include "common/plugin_utils.h"
#include "Utils/Log.h"

#include <string.h>

using namespace C2;
using namespace std;

static const char* plugin_name = "DepsPlugin";

DepsPlugin::DepsPlugin()
    : Plugin(std::string("DepsPlugin v1.0"))
    , cur(&globalConfig)
{}

DepsPlugin::~DepsPlugin() {}

// config options: "all-targets files private externals"

bool DepsPlugin::setGlobalCfg(bool verbose, const std::string& config) {
    return parseConfig(config, true);
}

bool DepsPlugin::setTargetCfg(bool verbose, const std::string& config) {
    return parseConfig(config, false);
}

void DepsPlugin::beginTarget(C2Builder& builder) {
    cur = &globalConfig;
}

bool DepsPlugin::generate(C2Builder& builder, const c2lang::SourceManager& sm_) {
    assert(cur);
    DepGenerator generator(cur->showFiles, cur->showPrivate, cur->showExternals);
    generator.write(builder.getComponents(), builder.getRecipeName(), builder.getOutputDir());
    return true;
}

bool DepsPlugin::parseConfig(const std::string& config, bool global) {
    cur = global ? &globalConfig : &localConfig;
    cur->init();

    WordTokenizer tok;
    word_tokenizer_run(&tok, config.c_str());
    while (word_tokenizer_next(&tok)) {
        const char* word = word_tokenizer_get(&tok);

        if (strcmp(word, "all-targets") == 0) {
            if (!global) {
                Log::error(plugin_name, "cannot specify 'all-targets' at target level");
                return false;
            }
            setAllTargets();
        } else if (strcmp(word, "externals") == 0) {
            cur->showExternals = true;
        } else if (strcmp(word, "files") == 0) {
            cur->showFiles = true;
        } else if (strcmp(word, "private") == 0) {
            cur->showPrivate = true;
        } else {
            Log::error(plugin_name, "invalid config item: %s", word);
            return false;
        }
    }
    return true;
}

