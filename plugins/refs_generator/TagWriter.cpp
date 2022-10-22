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
    TagVisitor(TagWriter& writer_, const Decl* D, const c2lang::SourceManager& SM_)
        : ASTVisitor(D), writer(writer_), SM(SM_)
    {}
    virtual ~TagVisitor() {}

    virtual void visitIdentifierExpr(const IdentifierExpr* I) {
        c2lang::PresumedLoc loc = SM.getPresumedLoc(I->getLocation());
        assert(!loc.isInvalid() && "Invalid location");
        const Decl* D = I->getDecl();
        assert(D);
        c2lang::PresumedLoc loc2 = SM.getPresumedLoc(D->getLocation());
        if (!loc2.isInvalid()) {
            std::string name = I->getName();

            if (I->isStructFunction()) {
                // NOTE: in definition, it is a StructTypeDecl not FunctionDecl
                if (isa<FunctionDecl>(D)) {
                    const FunctionDecl* FD = cast<FunctionDecl>(D);
                    name = FD->getMemberName();
                }
            }
            writer.addRef(loc.getLine(), loc.getColumn(), name,
                          loc2.getFilename(), loc2.getLine(), loc2.getColumn());
        }
    }
private:
    TagWriter& writer;
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

    for (unsigned i=0; i<ast.numTypes(); i++) {
        TagVisitor visitor(*this, ast.getType(i), SM);
        visitor.run();
    }
    for (unsigned i=0; i<ast.numVars(); i++) {
        TagVisitor visitor(*this, ast.getVar(i), SM);
        visitor.run();
    }
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        TagVisitor visitor(*this, ast.getFunction(i), SM);
        visitor.run();
    }
    for (unsigned i=0; i<ast.numStaticAsserts(); i++) {
        TagVisitor visitor(*this, ast.getStaticAssert(i), SM);
        visitor.run();
    }
    // TODO TypeDecls ArrayValueDecls
}

void TagWriter::addRef(unsigned src_line, unsigned src_col, const std::string& symbol,
                       const std::string& dst_file, unsigned dst_line, unsigned dst_col)
{
    RefSrc src = { src_line, (uint16_t)src_col, (uint16_t)strlen(symbol.c_str()) };
    RefDest dest = { dst_file.c_str(), dst_line, (uint16_t)dst_col };
    refs_add_tag(refs, &src, &dest);
}

void TagWriter::write(const std::string& title, const std::string& path) const {
    std::string refs2 = path + "refs";
    refs_trim(refs);
    refs_write(refs, refs2.c_str());
}

}
