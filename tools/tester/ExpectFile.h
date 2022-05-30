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

#include <string>
#include <vector>

#define EXPECT_MAX_LINE 512

namespace C2 {

class StringBuilder;

class ExpectFile {
public:
    enum Mode {
        ATLEAST,
        COMPLETE,
    };
    ExpectFile(const std::string& name, Mode m);

    void addLine(unsigned line_nr, const char* start, const char* end);
    bool check(StringBuilder& output_, const std::string& basedir);
private:
    bool checkLine(unsigned line_nr, const char* start, const char* end);
    void setExpected();

    std::string filename;
    Mode mode;
    StringBuilder* output;

    // during analysis
    unsigned lastLineNr;
    unsigned expIndex;
    // cache for Lines[expIndex]
    const char* expectedLine;
    bool consecutive;

    typedef std::pair<std::string, bool> Line;
    typedef std::vector<Line> Lines;
    Lines lines;
};

}

