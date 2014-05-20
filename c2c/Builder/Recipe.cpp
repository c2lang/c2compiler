/* Copyright 2013,2014 Bas van den Berg
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

#include "Builder/Recipe.h"

using namespace C2;

void Recipe::addFile(const std::string& name_) {
    files.push_back(name_);
}

void Recipe::addConfig(const std::string& config_) {
    configs.push_back(config_);
}

void Recipe::addAnsiCConfig(const std::string& config_) {
    cConfigs.push_back(config_);
}

void Recipe::addCodeGenConfig(const std::string& config_) {
    genConfigs.push_back(config_);
}

void Recipe::addDepsConfig(const std::string& config_) {
    depConfigs.push_back(config_);
}

void Recipe::silenceWarning(const std::string& warn_) {
    silentWarnings.push_back(warn_);
}

const std::string& Recipe::get(int i) const {
    return files[i];
}

