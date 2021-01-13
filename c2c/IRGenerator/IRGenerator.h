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

#ifndef IRGENERATOR_IRGENERATOR_H
#define IRGENERATOR_IRGENERATOR_H

#include <string>

#include "AST/Module.h"

namespace C2 {

class IRGenerator {
public:
    IRGenerator(const std::string& name_,
                const std::string& dir_,
                const ModuleList& modules_,
                bool single_module_,
                bool single_threaded_,
                bool keep_intermediates_,
                bool verbose_,
                bool printTiming_,
                bool printIR_,
                bool useColors_);
    void build();
private:
    const std::string& name;
    const std::string& dir;
    const ModuleList& modules;
    bool single_module;
    bool single_threaded;
    bool keep_intermediates;
    bool verbose;
    bool printTiming;
    bool printIR;
    bool useColors;
};

}

#endif

