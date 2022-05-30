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

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ExpectFile.h"
#include "TestUtils.h"
#include "FileUtils/FileMap.h"
#include "Utils/StringBuilder.h"

//#define DEBUG

using namespace C2;

/*
    Design:
    - compares file content with expectations
    - start/end whitespace is always ingored
    - empty lines test file are ignored
    - can be in mode complete -> every line must match
    - can be in mode atleast -> at least given lines must be present
    - if lines are sequential, they must also be sequential in test file

    Q: must there be space between 2 blocks in test file? -> no
*/

ExpectFile::ExpectFile(const std::string& name, Mode m)
    : filename(name)
    , mode(m)
    , output(0)
    , lastLineNr(0)
    , expIndex(0)
    , expectedLine(0)
    , consecutive(false)
{}

void ExpectFile::addLine(unsigned line_nr, const char* start, const char* end) {
    bool consecutive_ = (lastLineNr + 1 == line_nr); // ignore for first one (always ok)

    TestUtils::skipInitialWhitespace(&start, end);
    TestUtils::skipTrailingWhitespace(start, &end);

    if (strncmp(start, "//", 2) == 0) return;   // ignore comments

    lastLineNr = line_nr;
    std::string line_str(start, end);
    lines.push_back(Line(line_str, consecutive_));
}

bool ExpectFile::checkLine(unsigned line_nr, const char* start, const char* end) {

    TestUtils::skipInitialWhitespace(&start, end);
    TestUtils::skipTrailingWhitespace(start, &end);

    char line[EXPECT_MAX_LINE];
    int len = (int)(end - start);
    if (len == 0) return true;  // ignore empty lines
    if (start[0] == '/' && start[1] == '/') return true;    // ignore comment lines

    if (len >= EXPECT_MAX_LINE) {
        color_print2(*output, COL_ERROR, "  in file %s: line %u: line too long", filename.c_str(), line_nr);
        return false;
    }
    if (len != 0) memcpy(line, start, len);
    line[len] = 0;
#ifdef DEBUG
    printf("got %u [%s]\n", line_nr, line);
#endif

    if (!expectedLine) {
        if (mode == COMPLETE) {
            color_print2(*output, COL_ERROR, "  in file %s: line %u\n     unexpected line '%s'", filename.c_str(), line_nr, line);
            return false;
        } else {
            return true;
        }
    }
    if (strcmp(expectedLine, line) == 0) {
#ifdef DEBUG
        printf("-> match\n");
#endif
        expIndex++;
        setExpected();
    } else {
        if (mode == COMPLETE || consecutive) {
            color_print2(*output, COL_ERROR, "  in file %s: line %u\n  expected '%s'\n       got '%s'", filename.c_str(), line_nr, expectedLine, line);
            return false;
        }
    }
    return true;
}

void ExpectFile::setExpected() {
    if (expIndex == lines.size()) {
        expectedLine = 0;
        consecutive = false;
    } else {
        expectedLine = lines[expIndex].first.c_str();
        consecutive = lines[expIndex].second;
    }
#ifdef DEBUG
    printf(ANSI_BCYAN "exp [%s]  %d" ANSI_NORMAL "\n", expectedLine, consecutive);
#endif
}

bool ExpectFile::check(StringBuilder& output_, const std::string& basedir) {
    std::string fullname = basedir + filename;
    output = &output_;

    // check if file exists
    struct stat statbuf;
    int err = stat(fullname.c_str(), &statbuf);
    if (err) {
        color_print2(*output, COL_ERROR, "  missing expected file '%s' (%s)", filename.c_str(), fullname.c_str());
        return false;
    }
    FileMap file(fullname.c_str());
    file.open();

    if (lines.size() == 0) return true;

    const char* lineStart = (const char*)file.region;
    setExpected();

    unsigned line_nr = 1;
    while (1) {
        // cut it up into lines (even if empty)
        const char* cp = lineStart;
        while (*cp) {
            if (*cp == '\n') break;
            cp++;
        }

        if (*cp == 0) break;
        if (!checkLine(line_nr, lineStart, cp)) return false;
        line_nr++;
        cp++;   // skip newline
        lineStart = cp;
    }

    if (expectedLine != 0) {
        color_print2(*output, COL_ERROR, "  in file %s: expected '%s'", filename.c_str(), expectedLine);
        return false;
    }
    return true;
}

