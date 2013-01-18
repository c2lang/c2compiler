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

#include <clang/Sema/SemaDiagnostic.h>
#include "FileAnalyser.h"
#include "Decl.h"

using namespace C2;
using namespace clang;

FileAnalyser::FileAnalyser(clang::DiagnosticsEngine& Diags_)
    : Diags(Diags_)
    , hasErrors_(false)
{}

FileAnalyser::~FileAnalyser() {}

bool FileAnalyser::handle(Decl* decl) {
    Decl* Old = getSymbol(decl->getName());
    if (Old) {
        Diag(decl->getLocation(), diag::err_redefinition)
        << decl->getName();
        Diag(Old->getLocation(), diag::note_previous_definition);
        hasErrors_ = true;
        return true;
    }
    symbols[decl->getName()] = decl;
    return false;
}

bool FileAnalyser::hasSymbol(const std::string& name) const {
    SymbolsConstIter iter = symbols.find(name);
    return (iter != symbols.end());
}

Decl* FileAnalyser::getSymbol(const std::string& name) const {
    SymbolsConstIter iter = symbols.find(name);
    if (iter == symbols.end()) return 0;
    else return iter->second;
}

clang::DiagnosticBuilder FileAnalyser::Diag(clang::SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

