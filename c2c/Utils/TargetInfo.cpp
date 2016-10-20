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

#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "Utils/TargetInfo.h"

using namespace C2;

void TargetInfo::getNative(TargetInfo& info) {
    struct utsname un;
    if (uname(&un) != 0) {
        fprintf(stderr, "error getting system info: %s\n", strerror(errno));
        exit(-1);
    }

    if (strcmp(un.sysname, "Darwin") == 0) {
        info.sys = SYS_DARWIN;
    } else if (strcmp(un.sysname, "Linux") == 0) {
        info.sys = SYS_LINUX;
    } else {
        fprintf(stderr, "unsupported system: %s\n", un.sysname);
        exit(-1);
    }

    if (strcmp(un.machine, "i686") == 0) {
        info.mach = MACH_I686;
    } else if (strcmp(un.machine, "x86_64") == 0) {
        info.mach = MACH_X86_64;
    } else {
        fprintf(stderr, "unsupported machine: %s\n", un.machine);
        exit(-1);
    }
}

const char* C2::Str(TargetInfo::System sys) {
    switch (sys) {
    case TargetInfo::SYS_LINUX:     return "linux";
    case TargetInfo::SYS_DARWIN:    return "darwin";
    case TargetInfo::SYS_UNKNOWN:   return "unknown";
    }
    return "";
}

const char* C2::Str(TargetInfo::Machine mach) {
    switch (mach) {
    case TargetInfo::MACH_I686:     return "i686";
    case TargetInfo::MACH_X86_64:   return "x86_64";
    case TargetInfo::MACH_UNKNOWN:  return "unknown";
    }
    return "";
}
