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

#ifndef TAG_WRITER_H
#define TAG_WRITER_H

#include <string>

#include "AST/Component.h"
#include "common/Refs.h"

namespace c2lang {
class SourceManager;
}

namespace C2 {

class AST;
class Decl;
class StringBuilder;

class TagWriter {
public:
    TagWriter(const c2lang::SourceManager& SM_, const Components& components);
    ~TagWriter();

    void write(const std::string& title, const std::string& path) const;
private:
    friend class TagVisitor;

    void analyse(const AST& ast);
    void addRef(unsigned src_line, unsigned src_col, const std::string&  symbol,
                const std::string& dest_file, unsigned dst_line, unsigned dst_col);

    const c2lang::SourceManager& SM;
    Refs* refs;

    TagWriter(const TagWriter&);
    TagWriter& operator= (const TagWriter&);
};

}

#endif

