/* Copyright 2013 Bas van den Berg
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

#ifndef FILE_ANALYSER_H
#define FILE_ANALYSER_H

#include <map>
#include <string>

#include <clang/Lex/Preprocessor.h>
#include "ASTVisitor.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Decl;

// analyses a single file after it's been parsed into AST.
class FileAnalyser : public ASTVisitor {
public:
    FileAnalyser(clang::DiagnosticsEngine& Diags_);
    virtual ~FileAnalyser();

    virtual bool handle(Decl* decl);
    bool hasErrors() const { return hasErrors_; }
private:
    bool hasSymbol(const std::string& name) const;
    Decl* getSymbol(const std::string& name) const;

    clang::DiagnosticBuilder Diag(clang::SourceLocation Loc, unsigned DiagID);

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    Symbols symbols;

    clang::DiagnosticsEngine& Diags;

    bool hasErrors_;

    FileAnalyser(const FileAnalyser&);
    FileAnalyser& operator= (const FileAnalyser&);
};

}

#endif

