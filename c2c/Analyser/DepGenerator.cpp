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

#include <assert.h>

#include "Analyser/DepGenerator.h"
#include "AST/Decl.h"
#include "AST/Package.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace clang;

DepGenerator::DepGenerator(StringBuilder& output_)
    : output(output_)
    , package(0)
{
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    output << "<dependencies>\n";
}

void DepGenerator::startFile(const Package* p, unsigned file_id_) {
    package = p;
    file_id = file_id_;
}

void DepGenerator::doneFile() {
    // TODO find better way to map id -> pkg
    unsigned strengths[100] = { 0 };     // TEMP hardcoded size
    const Package* id2pkg[100] = {0};   // TODO make size dynamic
    for (unsigned i=0; i<decls.size(); i++) {
        const Decl* D = decls[i];
        id2pkg[D->getFileID()] = D->getPackage();
        strengths[D->getFileID()]++;
    }
    output.indent(2);
    output << "<file name=\"" << package->getName() << '.' << file_id <<"\">\n";

    for (unsigned i=0; i<100; i++) {
        if (strengths[i] == 0) continue;
        output.indent(4);
        const Package* P = id2pkg[i];
        assert(P);
        output << "<uses file=\"" << P->getName() << '.' << i << "\" strength=\"" << strengths[i] << "\" />\n";
    }
    output.indent(2);
    output << "</file>\n";
    decls.clear();

}
void DepGenerator::add(unsigned file_id, const Decl* D) {
    // check if already in list
    for (unsigned i=0; i<decls.size(); i++) {
        if (decls[i] == D) return;
    }
    decls.push_back(D);
}

void DepGenerator::close() {
    output << "</dependencies>\n";
}

