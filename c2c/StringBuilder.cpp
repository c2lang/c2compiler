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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "StringBuilder.h"

#define SIZE_DEBUG

#ifdef SIZE_DEBUG
#include <assert.h>
#endif

using namespace C2;
using namespace std;

StringBuilder::StringBuilder(unsigned int cap, char* buf)
    : capacity(cap)
    , buffer(buf)
    , ownBuf(buf == 0)
{
    if (!buf) buffer = (char*)malloc(capacity);
    clear();
}

StringBuilder::~StringBuilder() {
    if (ownBuf) free(buffer);
}

StringBuilder& StringBuilder::operator<<(const char* input) {
    int len = strlen(input);
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(len < cap && "buffer overflow");
#endif
    strcpy(ptr, input);
    ptr += len;
    return *this;
}

StringBuilder& StringBuilder::operator<<(void* input) {
    int len = 10;   // 0x12345678
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(len < cap && "buffer overflow");
#endif
    ptr += sprintf(ptr, "0x%p", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(const string& input) {
    int len = input.size();
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(len < cap && "buffer overflow");
#endif
    strcpy(ptr, input.c_str());
    ptr += len;
    return *this;
}

StringBuilder& StringBuilder::operator<<(char input) {
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(1 < cap && "buffer overflow");
#endif
    *ptr = input;
    ++ptr;
    *ptr = 0;
    return *this;
}

StringBuilder& StringBuilder::operator<<(int input) {
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(10 < cap && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%d", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(unsigned int input) {
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(10 < cap && "buffer overflow");
#endif
    ptr += sprintf(ptr, "%u", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(const StringBuilder& input) {
    int len = input.size();
#ifdef SIZE_DEBUG
    int cap = capacity - (ptr-buffer);
    assert(len < cap && "buffer overflow");
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

unsigned int StringBuilder::size() const { return ptr - buffer; }

StringBuilder::operator const char*() const { return buffer; }

bool StringBuilder::isEmpty() const { return (size() == 0); }

void StringBuilder::strip(char c) {
    if (size() > 0 && *(ptr-1) == c) {
        --ptr;
        *ptr = 0;
    }
}

void StringBuilder::indent(int num) {
    for (int i=0; i<num; i++) {
        *ptr = ' ';
        ++ptr;
    }
    *ptr = 0;
}

