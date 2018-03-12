/* Copyright 2013-2018 Bas van den Berg
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

#ifndef UTILS_TARGET_INFO_H
#define UTILS_TARGET_INFO_H

#include <string>

namespace C2 {

class TargetInfo {
public:
    enum Arch { ARCH_UNKNOWN, ARCH_I686, ARCH_X86_64, ARCH_ARM };
    enum System { SYS_UNKNOWN, SYS_LINUX, SYS_DARWIN, SYS_CYGWIN };
    enum Vendor { VENDOR_UNKNOWN, VENDOR_APPLE };
    enum Abi { ABI_UNKNOWN, ABI_GNU, ABI_GNUEABI, ABI_MACHO, ABI_WIN32 };

    Arch arch;
    System sys;
    Vendor vendor;
    Abi abi;

    static void getNative(TargetInfo& info);
    static bool fromString(TargetInfo& info, const std::string& triple);
};

const char* Str(const TargetInfo& info);
const char* Str(TargetInfo::Arch mach);
const char* Str(TargetInfo::Vendor vendor);
const char* Str(TargetInfo::System sys);
const char* Str(TargetInfo::Abi abi);

}

#endif

