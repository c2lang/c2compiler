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

#ifndef SCOPE_ANALYSER_H
#define SCOPE_ANALYSER_H

#include "ASTVisitor.h"
#include "Package.h"
#include "Type.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class IdentifierExpr;
class FileScope;
class Decl;

class ScopeAnalyser : public ASTVisitor {
public:
    ScopeAnalyser(FileScope& scope_, clang::DiagnosticsEngine& Diags_);
    virtual ~ScopeAnalyser();

    virtual bool handle(Decl* decl);
    unsigned int getErrors() const { return errors; }
private:
    void checkDecl(Decl* decl, bool used_public);
    void checkType(QualType type, bool used_public = false);
    void checkStructType(StructTypeDecl* S, bool used_public);
    void checkUse(Decl* decl);

    FileScope& globals;
    clang::DiagnosticsEngine& Diags;
    unsigned int errors;

    ScopeAnalyser(const ScopeAnalyser&);
    ScopeAnalyser& operator= (const ScopeAnalyser&);
};

}

#endif

