/* Copyright 2013 Bas van den Berg
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

#ifndef DEP_GENERATOR_H
#define DEP_GENERATOR_H

#include <vector>
#include <string>

#include "Analyser/DepAnalyser.h"

namespace C2 {

class Decl;
class StringBuilder;
class Package;

class DepGenerator : public DepAnalyser {
public:
    DepGenerator(StringBuilder& output_);

    virtual void startFile(const Package* p, unsigned file_id);
    virtual void doneFile();
    virtual void add(unsigned file_id, const Decl* decl);

    void close();
private:
    StringBuilder& output;
    typedef std::vector<const Decl*> Decls;
    Decls decls;

    const Package* package;
    unsigned file_id;
};

}

#endif

