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

#include <time.h>
#include <stdio.h>
#include "Utils.h"
#include "StringBuilder.h"

using namespace C2;

u_int64_t Utils::getCurrentTime()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    u_int64_t now64 = now.tv_sec;
    now64 *= 1000000;
    now64 += (now.tv_nsec/1000);
    return now64;
}


void Utils::addName(const std::string& pkgName, const std::string& name, StringBuilder& buffer) {
    if (name == "main") {
        buffer << name;
    } else {
        buffer << "__" << pkgName << '_' << name;
    }
}


const char* Utils::fullName(const std::string& pkgname, const std::string& symname) {
    static char buffer[128];
    sprintf(buffer, "%s.%s", pkgname.c_str(), symname.c_str());
    return buffer;
}

