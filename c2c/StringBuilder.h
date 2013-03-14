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

#ifndef STRINGBUILDER_H 
#define STRINGBUILDER_H

#include <string>

namespace C2 {

class StringBuilder {
public:
    StringBuilder();
    ~StringBuilder();

    StringBuilder& operator<<(const char* input);
    StringBuilder& operator<<(const std::string& input);
    StringBuilder& operator<<(char input);
    StringBuilder& operator<<(int input);
    StringBuilder& operator<<(unsigned int input);
    StringBuilder& operator<<(const StringBuilder& input);

    operator const char*() const;
    void clear();
    unsigned int size() const;
    bool isEmpty() const;
    void strip(char c);
    void indent(int num);
private:
    static const unsigned int BUFSIZE = 1024*1024;
    char* buffer;
    char* ptr;
    StringBuilder(const StringBuilder&);
    StringBuilder& operator= (const StringBuilder&);
};

}

#endif

