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

#ifndef UTILS_TARGET_INFO_H
#define UTILS_TARGET_INFO_H

namespace C2 {

class TargetInfo {
public:
    enum System { SYS_LINUX, SYS_DARWIN, SYS_UNKNOWN };
    enum Machine { MACH_I686, MACH_X86_64, MACH_UNKNOWN };
    enum Vendor { VENDOR_UNKNOWN, VENDOR_APPLE };
    enum Abi { ABI_GNU, ABI_MACHO };

    System sys;
    Machine mach;
    Vendor vendor;
    Abi abi;

    static void getNative(TargetInfo& info);
};

const char* Str(const TargetInfo& info);
const char* Str(TargetInfo::System sys);
const char* Str(TargetInfo::Machine mach);
const char* Str(TargetInfo::Vendor vendor);
const char* Str(TargetInfo::Abi abi);

}

#endif

