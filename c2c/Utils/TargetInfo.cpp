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

#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "Utils/TargetInfo.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

using namespace C2;

struct ArchList {
    TargetInfo::Arch arch;
    const char* name;
};

static ArchList archList[] = {
    { TargetInfo::ARCH_UNKNOWN,   "unknown" },
    { TargetInfo::ARCH_I686,      "i686" },
    { TargetInfo::ARCH_X86_64,    "x86_64" },
    { TargetInfo::ARCH_ARM,       "arm" },
};

//TargetInfo::TargetInfo(const llvm::Triple& T)

TargetInfo::~TargetInfo() {}

const char* C2::Str(TargetInfo::Arch arch) {
    for (unsigned i=0; i<ARRAY_SIZE(archList); i++) {
        const ArchList* a = &archList[i];
        if (a->arch == arch) return a->name;
    }
    return "";
}

static TargetInfo::Arch str2arch(const char* name) {
    for (unsigned i=0; i<ARRAY_SIZE(archList); i++) {
        const ArchList* a = &archList[i];
        if (strcmp(a->name, name) == 0) return a->arch;
    }
    return TargetInfo::ARCH_UNKNOWN;
}

struct VendorList {
    TargetInfo::Vendor vendor;
    const char* name;
};

static VendorList vendorList[] = {
    { TargetInfo::VENDOR_UNKNOWN,    "unknown" },
    { TargetInfo::VENDOR_APPLE,      "apple" },
};

const char* C2::Str(TargetInfo::Vendor vendor) {
    for (unsigned i=0; i<ARRAY_SIZE(vendorList); i++) {
        const VendorList* v = &vendorList[i];
        if (v->vendor == vendor) return v->name;
    }
    return "";
}

static TargetInfo::Vendor str2vendor(const char* name) {
    for (unsigned i=0; i<ARRAY_SIZE(vendorList); i++) {
        const VendorList* v = &vendorList[i];
        if (strcmp(v->name, name) == 0) return v->vendor;
    }
    return TargetInfo::VENDOR_UNKNOWN;
}

struct SystemList {
    TargetInfo::System sys;
    const char* name;
};

static SystemList sysList[] = {
    { TargetInfo::SYS_UNKNOWN, "unknown" },
    { TargetInfo::SYS_LINUX,   "linux" },
    { TargetInfo::SYS_DARWIN,  "darwin" },
    { TargetInfo::SYS_CYGWIN,  "cygwin" },
};

const char* C2::Str(TargetInfo::System sys) {
    for (unsigned i=0; i<ARRAY_SIZE(sysList); i++) {
        const SystemList* s = &sysList[i];
        if (s->sys == sys) return s->name;
    }
    return "";
}

TargetInfo::System str2sys(const char* name) {
    for (unsigned i=0; i<ARRAY_SIZE(sysList); i++) {
        const SystemList* s = &sysList[i];
        if (strcasecmp(s->name, name) == 0) return s->sys;
    }
    return TargetInfo::SYS_UNKNOWN;
}

struct AbiList {
    TargetInfo::Abi abi;
    const char* name;
};

static AbiList abiList[] = {
    { TargetInfo::ABI_UNKNOWN,   "unknown" },
    { TargetInfo::ABI_GNU,        "gnu" },
    { TargetInfo::ABI_GNUEABI,    "gnueabi" },
    { TargetInfo::ABI_MACHO,      "macho" },
    { TargetInfo::ABI_WIN32,      "win32" },
};

const char* C2::Str(TargetInfo::Abi abi) {
    for (unsigned i=0; i<ARRAY_SIZE(abiList); i++) {
        const AbiList* a = &abiList[i];
        if (a->abi == abi) return a->name;
    }
    return "";
}

static TargetInfo::Abi str2abi(const char* name) {
    for (unsigned i=0; i<ARRAY_SIZE(abiList); i++) {
        const AbiList* a = &abiList[i];
        if (strcmp(a->name, name) == 0) return a->abi;
    }
    return TargetInfo::ABI_UNKNOWN;
}

void TargetInfo::getNative(TargetInfo& info) {
    struct utsname un;
    if (uname(&un) != 0) {
        fprintf(stderr, "error getting system info: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    info.sys = str2sys(un.sysname);
    switch (info.sys) {
    case SYS_UNKNOWN:
        fprintf(stderr, "unsupported system: %s\n", un.sysname);
        exit(EXIT_FAILURE);
    case SYS_LINUX:
        info.vendor = VENDOR_UNKNOWN;   // hardcoded
        info.abi = ABI_GNU;             // hardcoded
        break;
    case SYS_DARWIN:
        info.vendor = VENDOR_APPLE;     // hardcoded
        info.abi = ABI_MACHO;           // hardcoded
        break;
    case SYS_CYGWIN:
        info.vendor = VENDOR_UNKNOWN;
        info.abi = ABI_WIN32;
        break;
    }

    info.arch = str2arch(un.machine);
    if (info.arch == ARCH_UNKNOWN) {
        fprintf(stderr, "unsupported arch: %s\n", un.machine);
        exit(EXIT_FAILURE);
    }
}

bool TargetInfo::fromString(TargetInfo& info, const std::string& triple) {
    // expect format: <arch><sub>-<vendor>-<sys>-<abi>
    // TODO support sub for ARM
    // NOTE: very ugly way to split string
    char arch_str[32];
    char vendor_str[32];
    char sys_str[32];
    char abi_str[32];
    char* matches[4] = { arch_str, vendor_str, sys_str, abi_str };
    int match = 0;
    const char* start = triple.c_str();
    const char* cp = start;
    while (*cp) {
        if (*cp == '-') {
            unsigned len = cp - start;
            // TODO check len
            strncpy(matches[match], start, len);
            matches[match][len] = 0;
            start = cp+1;
            match++;
            if (match == 5) return false;
        }
        cp++;
    }
    if (cp != start) {
        unsigned len = cp - start;
        // TODO check len
        strncpy(matches[match], start, len);
        matches[match][len] = 0;
        match++;
    }
    if (match != 4) return false;

    info.arch = str2arch(arch_str);
    if (info.arch == ARCH_UNKNOWN) {
        fprintf(stderr, "unsupported arch: %s\n", arch_str);
        return false;
    }
    info.vendor = str2vendor(vendor_str);
    info.sys = str2sys(sys_str);
    if (info.sys == SYS_UNKNOWN) {
        fprintf(stderr, "unsupported system: %s\n", sys_str);
        return false;
    }
    info.abi = str2abi(abi_str);
    if (info.abi == ABI_UNKNOWN) {
        fprintf(stderr, "unsupported ABI: %s\n", abi_str);
        return false;
    }
    return true;
}

const char* C2::Str(const TargetInfo& info) {
    static char result[80];
    sprintf(result, "%s-%s-%s-%s",
        Str(info.arch), Str(info.vendor), Str(info.sys), Str(info.abi));
    return result;
}

static llvm::StringRef removeGCCRegisterPrefix(llvm::StringRef Name) {
  if (Name[0] == '%' || Name[0] == '#')
    Name = Name.substr(1);

  return Name;
}


bool TargetInfo::isValidClobber(llvm::StringRef Name) const {
    return (isValidGCCRegisterName(Name) || Name == "memory" || Name == "cc");
}

bool TargetInfo::isValidGCCRegisterName(llvm::StringRef Name) const {
    if (Name.empty()) return false;

  // Get rid of any register prefix.
  Name = removeGCCRegisterPrefix(Name);
  if (Name.empty())
    return false;

#if 0
  llvm::ArrayRef<const char *> Names = getGCCRegNames();

  // If we have a number it maps to an entry in the register name array.
  if (isdigit(Name[0])) {
    unsigned n;
    if (!Name.getAsInteger(0, n))
      return n < Names.size();
  }

  // Check register names.
  if (std::find(Names.begin(), Names.end(), Name) != Names.end())
    return true;

  // Check any additional names that we have.
  for (const AddlRegName &ARN : getGCCAddlRegNames())
    for (const char *AN : ARN.Names) {
      if (!AN)
        break;
      // Make sure the register that the additional name is for is within
      // the bounds of the register names from above.
      if (AN == Name && ARN.RegNum < Names.size())
        return true;
    }

  // Now check aliases.
  for (const GCCRegAlias &GRA : getGCCRegAliases())
    for (const char *A : GRA.Aliases) {
      if (!A)
        break;
      if (A == Name)
        return true;
    }
#endif

    // TODO

  return true;
}

bool TargetInfo::validateOutputConstraint(ConstraintInfo& Info) const
{
  const char *Name = Info.getConstraintStr().c_str();
  // An output constraint must start with '=' or '+'
  if (*Name != '=' && *Name != '+')
    return false;

  if (*Name == '+')
    Info.setIsReadWrite();

    Name++;

    // TODO see Clang:: TargetInfo:validateOutputConstraint()
    return true;
}

//bool TargetInfo::validateInputConstraint(MutableArrayRef<ConstaintInfo> OutputConstraints,
bool TargetInfo::validateInputConstraint(ConstraintInfo& Info) const
{
    return true;
}

