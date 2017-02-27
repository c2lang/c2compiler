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

#ifndef AST_ATTR_H
#define AST_ATTR_H

#include <vector>
#include <map>
#include <assert.h>

#include <clang/Basic/SourceLocation.h>

namespace C2 {

class Decl;
class Expr;
class ASTContext;
class StringBuilder;

enum AttrKind {
    ATTR_UNKNOWN = 0,
    ATTR_EXPORT,
    ATTR_PACKED,
    ATTR_UNUSED,
    ATTR_UNUSED_PARAMS,
    ATTR_SECTION,
    ATTR_NORETURN,
    ATTR_INLINE,
    ATTR_ALIGNED,
    ATTR_WEAK,
    ATTR_OPAQUE,
};

// TODO make private
#define ATTR_TYPE 0x1
#define ATTR_FUNC 0x2
#define ATTR_VAR  0x4

struct AttrInfo {
    AttrKind kind;
    const char* name;
    bool requiresArgument;
    unsigned allowedTypes;

    inline bool isAllowedInFunction() const { return allowedTypes & ATTR_FUNC; }
    inline bool isAllowedInType() const { return allowedTypes & ATTR_TYPE; }
    inline bool isAllowedInVar() const { return allowedTypes & ATTR_VAR; }
};

class Attr {
public:
    Attr(AttrKind k, clang::SourceRange R, Expr* e) : Range(R), kind(k), arg(e) {}

    void* operator new(size_t bytes, const ASTContext& C, unsigned alignment = 8);

    AttrKind getKind() const { return kind; }
    clang::SourceRange getRange() const { return Range; }
    clang::SourceLocation getLocation() const { return Range.getBegin(); }

    static AttrKind name2kind(const char* name);
    static const AttrInfo& getInfo(AttrKind kind);
    Expr* getArg() const { return arg; }

    void print(StringBuilder& output) const;
    const char* kind2str() const;
private:
    void* operator new(size_t bytes) noexcept {
        assert(0 && "Attr cannot be allocated with regular 'new'");
        return 0;
    }
    void operator delete(void* data) {
        assert(0 && "Attr cannot be released with regular 'delete'");
    }

    clang::SourceRange Range;
    AttrKind kind;
    Expr* arg;
};

typedef std::vector<Attr*> AttrList;
typedef AttrList::const_iterator AttrListConstIter;
typedef AttrList::iterator AttrListIter;

typedef std::map<const Decl*, AttrList> AttrMap;
typedef AttrMap::const_iterator AttrMapConstIter;
typedef AttrMap::iterator AttrMapIter;

}

#endif

