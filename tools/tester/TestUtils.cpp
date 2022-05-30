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

#include "TestUtils.h"
#include "Utils/StringBuilder.h"
#include <ctype.h>

extern int color_output;

using namespace C2;

void TestUtils::skipInitialWhitespace(const char** start, const char* end) {
    const char* cp = *start;
    while (cp != end && isblank(*cp)) cp++;
    *start = cp;
}

void TestUtils::skipTrailingWhitespace(const char* start, const char** end) {
    const char* cp = *end;
    while (cp > start && isblank(*(cp-1))) cp--;
    *end = cp;
}

void color_print(const char* color, const char* format, ...) {
    char buffer[1024];
    va_list(Args);
    va_start(Args, format);
    vsnprintf(buffer, sizeof(buffer), format, Args);
    va_end(Args);

    if (color_output) printf("%s%s" ANSI_NORMAL"\n", color, buffer);
    else printf("%s\n", buffer);
}

void color_print2(StringBuilder& output ,const char* color, const char* format, ...) {
    char buffer[1024];
    va_list(Args);
    va_start(Args, format);
    vsnprintf(buffer, sizeof(buffer), format, Args);
    va_end(Args);

    if (color_output) output.print("%s%s" ANSI_NORMAL"\n", color, buffer);
    else output.print("%s\n", buffer);
}

