/* Copyright 2013-2017 Bas van den Berg
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Utils/StringBuilder.h"

#define SIZE_DEBUG

#ifdef SIZE_DEBUG
#include <assert.h>
#endif

using namespace C2;
using namespace std;

StringBuilder::StringBuilder(unsigned cap, char* buf)
    : capacity(cap)
    , buffer(buf)
    , ownBuf(buf == 0)
    , colors(false)
{
    if (!buf) buffer = (char*)malloc(capacity);
    clear();
}

StringBuilder::~StringBuilder() {
    if (ownBuf) free(buffer);
}

StringBuilder& StringBuilder::operator<<(const char* input) {
    unsigned len = strlen(input);
#ifdef SIZE_DEBUG
    assert(len < space_left() && "buffer overflow");
#endif
    strcpy(ptr, input);
    ptr += len;
    return *this;
}

StringBuilder& StringBuilder::operator<<(void* input) {
#ifdef SIZE_DEBUG
    assert(10 < space_left() && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%p", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(const string& input) {
    unsigned len = input.size();
#ifdef SIZE_DEBUG
    assert(len < space_left() && "buffer overflow");
#endif
    strcpy(ptr, input.c_str());
    ptr += len;
    return *this;
}

StringBuilder& StringBuilder::operator<<(char input) {
#ifdef SIZE_DEBUG
    assert(1 < space_left() && "buffer overflow");
#endif
    *ptr = input;
    ++ptr;
    *ptr = 0;
    return *this;
}

StringBuilder& StringBuilder::operator<<(int32_t input) {
#ifdef SIZE_DEBUG
    assert(10 < space_left() && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%" PRId32"", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(uint32_t input) {
#ifdef SIZE_DEBUG
    assert(10 < space_left() && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%" PRIu32"", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(int64_t input) {
#ifdef SIZE_DEBUG
    assert(10 < space_left() && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%" PRId64"", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(uint64_t input) {
#ifdef SIZE_DEBUG
    assert(10 < space_left() && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%" PRIu64"", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(const StringBuilder& input) {
    unsigned len = input.size();
#ifdef SIZE_DEBUG
    assert(len < space_left() && "buffer overflow");
#endif
    memcpy(ptr, input.buffer, len);
    ptr += len;
    *ptr = 0;
    return *this;
}

void StringBuilder::clear() {
    ptr = buffer;
    buffer[0] = 0;
}

void StringBuilder::print(const char* format, ...) {
    va_list(Args);
    va_start(Args, format);
    unsigned len = vsprintf(ptr, format, Args);
#ifdef SIZE_DEBUG
    assert(len < space_left() && "buffer overflow");
#endif
    va_end(Args);
    ptr += len;
    *ptr = 0;
}

void StringBuilder::number(unsigned radix_, int64_t value) {
    char temp[80];
    switch (radix_) {
    case 2:
    {
        char* cp = &temp[2];
        temp[0] = '0';
        temp[1] = 'b';
        bool show = false;
        for (int i=63; i>0; i--) {
            if (value & (1lu<<i)) show = true;
            if (show) {
                *cp++ = ((value & (1lu<<i)) ? '1' : '0');
            }
        }
        *cp++ = ((value & (1lu<<0)) ? '1' : '0');
        *cp = 0;
    }
    break;
    case 8:
        sprintf(temp, "0%" PRIo64, value);
        break;
    case 10:
        sprintf(temp, "%" PRId64, value);
        break;
    case 16:
        sprintf(temp, "0x%" PRIX64, value);
        break;
    }
    (*this) << temp;
}

void StringBuilder::strip(char c) {
    if (size() > 0 && *(ptr-1) == c) {
        --ptr;
        *ptr = 0;
    }
}

void StringBuilder::indent(unsigned num) {
    for (unsigned i=0; i<num; i++) {
        *ptr = ' ';
        ++ptr;
    }
    *ptr = 0;
}

void StringBuilder::setColor(const char* color) {
    if (colors) {
        unsigned len = strlen(color);
#ifdef SIZE_DEBUG
        assert(len < space_left() && "buffer overflow");
#endif
        strcpy(ptr, color);
        ptr += len;
    }
}

