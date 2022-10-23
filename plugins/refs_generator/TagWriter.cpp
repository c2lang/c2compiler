/* Copyright 2013-2022 Bas van den Berg
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

#include "TagWriter.h"
#include "Algo/ASTVisitor.h"
#include "AST/Module.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "AST/Type.h"
#include "AST/Expr.h"

#include "Clang/SourceLocation.h"
#include "Clang/SourceManager.h"

#include <string.h>
#include <assert.h>

using namespace C2;
using namespace std;

namespace C2 {

class TagVisitor : public ASTVisitor {
public:
    TagVisitor(Refs* refs_, const Decl* D, const c2lang::SourceManager& SM_)
        : ASTVisitor(D), refs(refs_), SM(SM_)
    {}
    virtual ~TagVisitor() {}

    virtual void visitDecl(const Decl* d) {
        switch (d->getKind()) {
        case DECL_FUNC:
            break;
        case DECL_VAR: {
            const VarDecl* vd = cast<VarDecl>(d);
            if (!vd->isGlobal()) return;
            break;
        }
        case DECL_ENUMVALUE:
            return; // should not come here
        case DECL_ALIASTYPE:
            break;
        case DECL_STRUCTTYPE:
            break;
        case DECL_ENUMTYPE: {
            const EnumTypeDecl* e = cast<EnumTypeDecl>(d);
            for (unsigned i=0; i<e->numConstants(); i++) {
                const EnumConstantDecl* ecd = e->getConstant(i);
                {
                    c2lang::PresumedLoc loc = SM.getPresumedLoc(ecd->getLocation());
                    if (loc.isInvalid()) return;
                    RefDest dest = { loc.getFilename(), loc.getLine(), (uint16_t)loc.getColumn() };
                    refs_add_symbol(refs, ecd->getName(), &dest);
                }
            }
            // TODO get Constants
            break;
        }
        case DECL_FUNCTIONTYPE:
        case DECL_ARRAYVALUE:
            return;
        case DECL_IMPORT:
        case DECL_LABEL:
        case DECL_STATIC_ASSERT:
            return;
        }

        c2lang::PresumedLoc loc = SM.getPresumedLoc(d->getLocation());
        if (loc.isInvalid()) return;
        RefDest dest = { loc.getFilename(), loc.getLine(), (uint16_t)loc.getColumn() };
        refs_add_symbol(refs, d->getName(), &dest);
        //TODO for Enum, also add EnumConstantDecls
    }

    virtual void visitIdentifierExpr(const IdentifierExpr* I) {
        c2lang::PresumedLoc loc = SM.getPresumedLoc(I->getLocation());
        assert(!loc.isInvalid() && "Invalid location");
        const Decl* D = I->getDecl();
        assert(D);
        c2lang::PresumedLoc loc2 = SM.getPresumedLoc(D->getLocation());
        if (loc2.isInvalid()) return;

        std::string name = I->getName();

        if (I->isStructFunction()) {
            // NOTE: in definition, it is a StructTypeDecl not FunctionDecl
            if (isa<FunctionDecl>(D)) {
                const FunctionDecl* FD = cast<FunctionDecl>(D);
                name = FD->getMemberName();
            }
        }

        RefSrc src = { loc.getLine(), (uint16_t)loc.getColumn(), (uint16_t)strlen(name.c_str()) };
        RefDest dest = { loc2.getFilename(), loc2.getLine(), (uint16_t)loc2.getColumn() };
        refs_add_tag(refs, &src, &dest);
    }
private:
    Refs* refs;
    const c2lang::SourceManager& SM;
};

TagWriter::TagWriter(const c2lang::SourceManager& SM_, const Components& components)
    : SM(SM_)
    , refs(refs_create())
{
    for (unsigned c=0; c<components.size(); c++) {
        const ModuleList& mods = components[c]->getModules();
        for (unsigned m=0; m<mods.size(); m++) {
            const Module* M = mods[m];
            if (!M->isLoaded() || M->isPlugin()) continue;
            const AstList& modFiles = M->getFiles();
            for (unsigned i=0; i<modFiles.size(); i++) {
                analyse(*modFiles[i]);
            }
        }
    }
}

TagWriter::~TagWriter() {
    refs_free(refs);
}

void TagWriter::analyse(const AST& ast) {
    refs_add_file(refs, ast.getFileName().c_str());

    // TODO dont create a new Visitor every time, just set Decl (pass to run?)
    for (unsigned i=0; i<ast.numTypes(); i++) {
        TagVisitor visitor(refs, ast.getType(i), SM);
        visitor.run();
    }
    for (unsigned i=0; i<ast.numVars(); i++) {
        TagVisitor visitor(refs, ast.getVar(i), SM);
        visitor.run();
    }
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        TagVisitor visitor(refs, ast.getFunction(i), SM);
        visitor.run();
    }
    for (unsigned i=0; i<ast.numStaticAsserts(); i++) {
        TagVisitor visitor(refs, ast.getStaticAssert(i), SM);
        visitor.run();
    }
    // TODO TypeDecls ArrayValueDecls
}

void TagWriter::write(const std::string& title, const std::string& path) const {
    std::string refs2 = path + "refs";
    refs_trim(refs);
    refs_write(refs, refs2.c_str());
}

}
