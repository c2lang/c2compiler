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

using namespace C2;
using namespace std;

StringBuilder::StringBuilder()
    : buffer((char*)malloc(8192))
{
    clear();
}

StringBuilder::~StringBuilder() {
    free(buffer);
}

StringBuilder& StringBuilder::operator<<(const char* input) {
    strcpy(ptr, input);
    ptr += strlen(input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(const string& input) {
    strcpy(ptr, input.c_str());
    ptr += input.size();
    return *this;
}

StringBuilder& StringBuilder::operator<<(char input) {
    *ptr = input;
    ++ptr;
    *ptr = 0;
    return *this;
}

StringBuilder& StringBuilder::operator<<(int input) {
    ptr += sprintf(ptr, "%d", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(unsigned int input) {
    ptr += sprintf(ptr, "%u", input);
    return *this;
}

StringBuilder& StringBuilder::operator<<(const StringBuilder& input) {
    memcpy(ptr, input.buffer, input.size());
    ptr += input.size(); 
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

void StringBuilder::stripNewline() {
    if (size() > 0 && *(ptr-1) == '\n') {
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

