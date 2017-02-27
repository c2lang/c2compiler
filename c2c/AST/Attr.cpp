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
#include <assert.h>

#include "AST/Attr.h"
#include "AST/Expr.h"
#include "AST/ASTContext.h"
#include "Utils/StringBuilder.h"

using namespace C2;

static const AttrInfo attrInfo[] {
    { ATTR_EXPORT,        "export",        false, ATTR_TYPE | ATTR_FUNC | ATTR_VAR },
    { ATTR_PACKED,        "packed",        false, ATTR_TYPE },
    { ATTR_UNUSED,        "unused",        false, ATTR_TYPE | ATTR_FUNC | ATTR_VAR },
    { ATTR_UNUSED_PARAMS, "unused_params", false, ATTR_FUNC },
    { ATTR_SECTION,       "section",       true,              ATTR_FUNC | ATTR_VAR },
    { ATTR_NORETURN,      "noreturn",      false,             ATTR_FUNC },
    { ATTR_INLINE,        "inline",        false,             ATTR_FUNC },
    { ATTR_ALIGNED,       "aligned",       true,  ATTR_TYPE | ATTR_FUNC | ATTR_VAR },
    { ATTR_WEAK,          "weak",          false,             ATTR_FUNC | ATTR_VAR },
    { ATTR_OPAQUE,        "opaque",        false, ATTR_TYPE },
};

const char* Attr::kind2str() const {
    switch (kind) {
    case ATTR_UNKNOWN:       return "unkown";
    case ATTR_EXPORT:        return "export";
    case ATTR_PACKED:        return "packed";
    case ATTR_UNUSED:        return "unused";
    case ATTR_UNUSED_PARAMS: return "unused_params";
    case ATTR_SECTION:       return "section";
    case ATTR_NORETURN:      return "noreturn";
    case ATTR_INLINE:        return "inline";
    case ATTR_ALIGNED:       return "aligned";
    case ATTR_WEAK:          return "weak";
    case ATTR_OPAQUE:        return "opaque";
    }
    return "?";
}

AttrKind Attr::name2kind(const char* name) {
    unsigned count = sizeof(attrInfo) / sizeof(attrInfo[0]);
    for (unsigned i=0; i<count; i++) {
        const AttrInfo* AI = &attrInfo[i];
        if (strcmp(AI->name, name) == 0) return AI->kind;
    }
    return ATTR_UNKNOWN;
}

void* Attr::operator new(size_t bytes, const C2::ASTContext& C, unsigned alignment) {
    return ::operator new(bytes, C, alignment);
}

const AttrInfo& Attr::getInfo(AttrKind kind) {
    unsigned count = sizeof(attrInfo) / sizeof(attrInfo[0]);
    for (unsigned i=0; i<count; i++) {
        const AttrInfo* AI = &attrInfo[i];
        if (AI->kind == kind) return *AI;
    }
    assert(0);
    static AttrInfo none;
    return none;
}

void Attr::print(StringBuilder& output) const {
    output << kind2str();
    if (arg) {
        output << '=';
        arg->printLiteral(output);
    }
}

