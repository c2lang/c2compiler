/* Copyright 2013-2020 Bas van den Berg
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

#ifndef FILE_UTILS_TOML_READER_H
#define FILE_UTILS_TOML_READER_H

#include <string>
#include <vector>

#include "Utils/StringList.h"

namespace C2 {

struct Blocks;
struct Node;
class StringBuilder;

class TomlReader {
public:
    TomlReader();
    ~TomlReader();

    bool parse(const char* filename);
    bool parse(const char* input, int);

    const char* getErrorMsg() const { return errorMsg; }
    void test(StringBuilder& out) const;

    const char* getValue(const char* key) const;

    class NodeIter {
    public:
        NodeIter(const Blocks* b_, const Node* n_) : blocks(b_), node(n_) {}
        bool done() const;
        void next();
        const char* getValue(const char* key) const;
    private:
        const Blocks* blocks;
        const Node* node;
    };
    NodeIter getNodeIter(const char* key) const;

    class ValueIter {
    public:
        ValueIter(const char* values_, bool isArray_) : values(values_), isArray(isArray_) {}
        bool done() const;
        void next();
        const char* getValue() const;
    private:
        const char* values;
        bool isArray;
    };
    ValueIter getValueIter(const char* key) const;
private:
    const Node* findNode(const char* key) const;

    char errorMsg[256];

    Blocks* blocks;
};

}

#endif

