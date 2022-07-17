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

#include "TagsPlugin.h"
#include <Builder/C2Builder.h>
#include "TagWriter.h"
#include "Utils/Log.h"

using namespace C2;
using namespace std;

static const char* plugin_name = "RefsPlugin";

TagsPlugin::TagsPlugin()
    : Plugin(std::string("RefsPlugin v1.0"))
{}

TagsPlugin::~TagsPlugin() {}

bool TagsPlugin::setGlobalCfg(bool verbose, const std::string& config) {
    if (verbose) Log::info(plugin_name, "global cfg [%s]", config.c_str());

    // TEMP set to allTarget, should get from config
    setAllTargets();

    return true;
}

bool TagsPlugin::setTargetCfg(bool verbose, const std::string& config) {
    if (verbose) Log::info(plugin_name, "target cfg [%s]", config.c_str());

    return true;
}

bool TagsPlugin::generate(C2Builder& builder, const c2lang::SourceManager& src_mgr) {
    TagWriter generator(src_mgr, builder.getComponents());
    generator.write(builder.getRecipeName(), builder.getOutputDir());
    return true;
}

