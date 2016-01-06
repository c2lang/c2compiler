/* Copyright 2013-2016 Bas van den Berg
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

#ifndef TAG_READER_H
#define TAG_READER_H

#include <vector>
#include <string>
#include <inttypes.h>
#include <sys/types.h>

struct TagFile;

class TagReader {
public:
    TagReader(const char* target_);
    ~TagReader() {}

    struct Result {
        std::string filename;
        uint32_t line;
        uint32_t column;

        bool operator==(const Result& other) const {
            return filename == other.filename &&
                   line == other.line &&
                   column == other.column;
        }
    };

    unsigned find(const char* filename, uint32_t line, uint32_t col);
    unsigned findReverse(const char* filename, uint32_t line, uint32_t col);

    const Result& getResult(unsigned i) const { return results[i]; }
private:
    void findRefs();
    bool isDuplicate(const Result& res) const;
    void addResult(const TagFile* file, uint32_t line, uint32_t column);

    const char* target;     // NULL allowed

    typedef std::vector<std::string> Strings;
    Strings refFiles;

    typedef std::vector<Result> Results;
    Results results;
};

#endif

