/* Copyright 2013-2018 Bas van den Berg
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

#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#include "Clang/Preprocessor.h"
#include "Parser/Ownership.h"

#define TYPE_CONST      (1<<1)
#define TYPE_VOLATILE   (1<<2)
#define TYPE_LOCAL      (1<<3)

namespace C2 {

class Decl;
class Expr;
class Stmt;
class VarDecl;

typedef c2lang::ActionResult<C2::Expr*> ExprResult;
typedef c2lang::ActionResult<C2::Stmt*> StmtResult;
typedef c2lang::ActionResult<C2::Decl*> DeclResult;
typedef c2lang::ActionResult<C2::VarDecl*> VarDeclResult;

}

#endif

