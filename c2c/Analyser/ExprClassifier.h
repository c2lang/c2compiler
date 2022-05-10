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

#ifndef ANALYSER_EXPR_CLASSIFIER_H
#define ANALYSER_EXPR_CLASSIFIER_H

namespace C2 {

class Expr;
class UnaryOperator;

enum ExprValueKind {
    VK_LValue,
    VK_RValue,
};

class ExprClassifier {
public:
    // NOTE: expr must be otherwise analysed first (Type set, etc)
    static ExprValueKind classifyExpr(const Expr* expr);
private:
    static ExprValueKind classifyUnaryOperator(const UnaryOperator* expr);

    ExprClassifier(const ExprClassifier&);
    ExprClassifier& operator= (const ExprClassifier&);
};

}

#endif

