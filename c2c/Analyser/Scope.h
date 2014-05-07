/* Copyright 2013,2014 Bas van den Berg
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

#ifndef ANALYSER_SCOPE_H
#define ANALYSER_SCOPE_H

#include <string>
#include <map>
#include <vector>
#include <stdint.h>

#include "AST/Package.h"

#define MAX_SCOPE_DEPTH 15

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Decl;
class VarDecl;
class UseDecl;
class DepAnalyser;

struct DynamicScope {
    DynamicScope();
    void Init(unsigned flags_);

    unsigned Flags;

    // local decls (in scope), no ownership
    typedef std::vector<VarDecl*> Decls;
    typedef Decls::const_iterator DeclsConstIter;
    Decls decls;
};


class Scope {
public:
    /// ScopeFlags - These are bitfields that are or'd together when creating a
    /// scope, which defines the sorts of things the scope contains.
    enum ScopeFlags {
        /// FnScope - This indicates that the scope corresponds to a function, which
        /// means that labels are set here.
        FnScope       = 0x01,

        /// BreakScope - This is a while,do,switch,for, etc that can have break
        /// stmts embedded into it.
        BreakScope    = 0x02,

        /// ContinueScope - This is a while,do,for, which can have continue
        /// stmt embedded into it.
        ContinueScope = 0x04,

        /// DeclScope - This is a scope that can contain a declaration.  Some scopes
        /// just contain loop constructs but don't contain decls.
        DeclScope = 0x08,

        /// ControlScope - The controlling scope in a if/switch/while/for statement.
        ControlScope = 0x10,

        /// BlockScope - This is a scope that corresponds to a block/closure object.
        /// Blocks serve as top-level scopes for some objects like labels, they
        /// also prevent things like break and continue.  BlockScopes always have
        /// the FnScope and DeclScope flags set as well.
        BlockScope = 0x40,

        /// SwitchScope - This is a scope that corresponds to a switch statement.
        SwitchScope = 0x800,
    };

    Scope(const std::string& name_, const Pkgs& pkgs_, clang::DiagnosticsEngine& Diags_, unsigned id);

    // adding symbols
    bool addUseDecl(UseDecl* useDecl);
    bool checkScopedSymbol(const VarDecl* V) const;
    void addScopedSymbol(VarDecl* V);

    // searching
    const Package* findUsedPackage(const std::string& name, clang::SourceLocation loc) const;
    Decl* findSymbol(const std::string& name, clang::SourceLocation loc, bool isType) const;
    Decl* findSymbolInPackage(const std::string& name, clang::SourceLocation loc, const Package* pkg) const;

    // Scopes
    void EnterScope(unsigned flags);
    void ExitScope();

    bool allowBreak()    const { return curScope->Flags & BreakScope; }
    bool allowContinue() const { return curScope->Flags & ContinueScope; }

    bool isExternal(const Package* pkg) const {
        return (pkg && pkg != myPkg);
    }

    void getExternals(DepAnalyser& dep) const;
private:
    const Package* findAnyPackage(const std::string& name) const;
    Decl* findOwn(const std::string& symbol) const;
    void addExternal(const Decl* D) const;
    //Decl* findSymbolInUsed(const std::string& name) const;

    // Dynamic Scopes
    DynamicScope scopes[MAX_SCOPE_DEPTH];
    unsigned scopeIndex;    // first free scope (= count of scopes)
    DynamicScope* curScope;

    // Packages with local symbols (includes self pkg)
    typedef std::vector<const Package*> Locals;
    typedef Locals::const_iterator LocalsConstIter;
    Locals locals;

    // used Packages (use <as>)
    typedef std::map<std::string, UseDecl*> Packages;
    typedef Packages::const_iterator PackagesConstIter;
    typedef Packages::iterator PackagesIter;
    Packages usedPackages;

    // all packages
    const Pkgs& allPackages;

    unsigned file_id;
    const Package* myPkg;

    // Symbol caches
    typedef std::map<const std::string, Decl*> SymbolCache;
    typedef SymbolCache::const_iterator CacheConstIter;
    typedef SymbolCache::iterator CacheIter;
    mutable SymbolCache symbolCache;

    typedef std::vector<const Decl*> Externals;
    mutable Externals externals;

    clang::DiagnosticsEngine& Diags;
};

}

#endif

