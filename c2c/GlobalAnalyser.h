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

#ifndef GLOBAL_ANALYSER_H
#define GLOBAL_ANALYSER_H

#include "ASTVisitor.h"
#include "Package.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Type;
class IdentifierExpr;
class GlobalScope;
class Decl;

class GlobalAnalyser : public ASTVisitor {
public:
    GlobalAnalyser(GlobalScope& scope_, clang::DiagnosticsEngine& Diags_);
    virtual ~GlobalAnalyser();

    virtual bool handle(Decl* decl);
    unsigned int getErrors() const { return errors; }
private:
    void checkType(Type* type, bool used_public = false);
    void checkUse(Decl* decl);

    GlobalScope& globalScope;
    clang::DiagnosticsEngine& Diags;
    unsigned int errors;

    GlobalAnalyser(const GlobalAnalyser&);
    GlobalAnalyser& operator= (const GlobalAnalyser&);
};

}

#endif

