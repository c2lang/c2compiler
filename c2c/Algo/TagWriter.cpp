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

#include "Algo/TagWriter.h"
#include "Algo/ASTVisitor.h"
#include "AST/Module.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "AST/Type.h"
#include "AST/Expr.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"
#include "FileUtils/FileUtils.h"
#include "Analyser/AnalyserConstants.h"
#include "Analyser/AnalyserUtils.h"

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>

using namespace C2;
using namespace std;

namespace C2 {

class TagVisitor : public ASTVisitor {
public:
    TagVisitor(TagWriter& writer_, const Decl* D, const clang::SourceManager& SM_)
        : ASTVisitor(D), writer(writer_), SM(SM_)
    {}
    virtual ~TagVisitor() {}

    virtual void visitIdentifierExpr(const IdentifierExpr* I) {
        clang::PresumedLoc loc = SM.getPresumedLoc(I->getLocation());
        assert(!loc.isInvalid() && "Invalid location");
        const Decl* D = I->getDecl();
        assert(D);
        clang::PresumedLoc loc2 = SM.getPresumedLoc(D->getLocation());
        if (!loc2.isInvalid()) {
            std::string name = I->getName();
            if (I->isStructFunction()) {
                char structName[MAX_LEN_VARNAME];
                name = AnalyserUtils::splitStructFunctionName(structName, I->getName());
            }
            writer.addRef(loc.getLine(), loc.getColumn(), name,
                          loc2.getFilename(), loc2.getLine(), loc2.getColumn());
        }
    }
private:
    TagWriter& writer;
    const clang::SourceManager& SM;
};


struct TagEntry {
    uint32_t src_line;
    uint32_t src_column;
    uint32_t dst_fileid;
    uint32_t dst_line;
    uint32_t dst_column;
    std::string symbol;

    static bool compare(const TagEntry& lhs, const TagEntry& rhs) {
        uint64_t left = lhs.src_line;
        left <<= 32;
        left |= lhs.src_column;
        uint64_t right = rhs.src_line;
        right <<= 32;
        right |= rhs.src_column;
        return left < right;
    }
};


struct TagFile {
    TagFile(const std::string& filename_, int id_) : filename(filename_), id(id_) {}

    void addTag(const TagEntry& tag) {
        tags.push_back(tag);
    }
    void sort() {
        std::sort(tags.begin(), tags.end(), TagEntry::compare);
    }
    void write(StringBuilder& out) const {
        // syntax: file 0 foo.c2 {
        out << "file " << id << " {\n";
        for (TagsConstIter iter = tags.begin(); iter != tags.end(); ++iter) {
            // syntax: 9:5 a -> 0:5:7
            const TagEntry& T = (*iter);
            out << '\t' << T.src_line << ':' << T.src_column << ' ' << T.symbol;
            out << " -> ";
            out << T.dst_fileid << ':' << T.dst_line << ':' << T.dst_column << '\n';
        }
        out << "}\n";
    }

    std::string filename;
    int id;
    typedef std::vector<TagEntry> Tags;
    typedef Tags::const_iterator TagsConstIter;
    Tags tags;
};

}


TagWriter::TagWriter(const clang::SourceManager& SM_, const Components& components)
    : SM(SM_)
    , currentFile(0)
{
    for (unsigned c=0; c<components.size(); c++) {
        const ModuleList& mods = components[c]->getModules();
        for (unsigned m=0; m<mods.size(); m++) {
            const AstList& modFiles = mods[m]->getFiles();
            for (unsigned i=0; i<modFiles.size(); i++) {
                analyse(*modFiles[i]);
            }
        }
    }
}

TagWriter::~TagWriter() {
    for (unsigned i=0; i<files.size(); i++) {
        delete files[i];
    }
}

void TagWriter::analyse(const AST& ast) {
    currentFile = getFile(ast.getFileName());

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
    // TODO TypeDecls ArrayValueDecls
}

void TagWriter::addRef(unsigned src_line, unsigned src_col, const std::string& symbol,
                       const std::string& dst_file, unsigned dst_line, unsigned dst_col)
{
    TagFile* destFile = getFile(dst_file);

    TagEntry tag;
    tag.src_line = src_line;
    tag.src_column = src_col;
    tag.dst_fileid = destFile->id;
    tag.dst_line = dst_line;
    tag.dst_column = dst_col;
    tag.symbol = symbol;
    currentFile->addTag(tag);
}

TagFile* TagWriter::getFile(const std::string& filename) {
    FileMapIter iter = filemap.find(filename);
    if (iter != filemap.end()) {
        unsigned file_id = iter->second;
        return files[file_id];
    }

    unsigned file_id = files.size();
    TagFile* tf = new TagFile(filename, file_id);
    filemap[filename] = file_id;
    files.push_back(tf);
    return tf;
}

void TagWriter::write(const std::string& title, const std::string& path) const {
    StringBuilder buffer(4*1024*1024);
    const char* offsets[files.size()];
    memset(offsets, 0, sizeof(offsets));
    buffer << "files {\n";
    for (unsigned index = 0; index < files.size(); ++index) {
        buffer << '\t' << files[index]->id << ' ' << files[index]->filename << ' ';
        offsets[index] = (const char*)buffer + buffer.size();
        buffer << "        \n";
    }
    buffer << "}\n";
    for (unsigned index = 0; index < files.size(); ++index) {
        char* cp = (char*)offsets[index];
        sprintf(cp, "%d", buffer.size());
        while (*cp != 0) cp++;
        *cp = ' ';
        files[index]->sort();
        files[index]->write(buffer);
    }

    FileUtils::writeFile(path.c_str(), path + "refs", buffer);
}

