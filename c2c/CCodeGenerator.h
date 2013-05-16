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

#ifndef CCODE_GENERATOR_H
#define CCODE_GENERATOR_H

#include <string>
#include <vector>
#include "StringBuilder.h"

namespace C2 {

class C2Sema;
class Decl;
class Expr;
class Type;
class Package;

// generates LLVM Module from (multiple) ASTs
class CCodeGenerator {
public:
    CCodeGenerator(const Package* pkg_);
    ~CCodeGenerator();
    void addEntry(const std::string& filename, C2Sema& sema);

    void generate();
    void write(const std::string& target, const std::string& name);
    void dump();

private:
    const char* ConvertType(C2::Type* type);

    void EmitFunction(Decl* D);
    void EmitVariable(Decl* D);
    void EmitType(Decl* D);
    void EmitUse(Decl* D);

    void EmitExpr(Expr* E);

    const Package* pkg;

    struct Entry {
        Entry(const std::string& f, C2Sema& s)
            : filename(&f), sema(&s) {}
        const std::string* filename;
        C2Sema* sema;
    };
    typedef std::vector<Entry> Entries;
    typedef Entries::iterator EntriesIter;
    Entries entries;

    StringBuilder cbuf;
    StringBuilder hbuf;

    CCodeGenerator(const CCodeGenerator&);
    CCodeGenerator& operator= (const CCodeGenerator&);
};

}

#endif

