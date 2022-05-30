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

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "Utils/color.h"

#define COL_ERROR ANSI_BRED
#define COL_SKIP  ANSI_BCYAN
#define COL_OK    ANSI_GREEN
#define COL_DEBUG ANSI_BMAGENTA

// NOTE: end must point AFTER last valid char (could be 0-terminator)

class TestUtils {
public:
    static void skipInitialWhitespace(const char** start, const char* end);
    static void skipTrailingWhitespace(const char* start, const char** end);
};

namespace C2 { class StringBuilder; }

void color_print(const char* color, const char* format, ...);
void color_print2(C2::StringBuilder& output ,const char* color, const char* format, ...);

#endif

