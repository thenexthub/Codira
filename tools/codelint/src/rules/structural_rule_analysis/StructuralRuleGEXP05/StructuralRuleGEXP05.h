/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef CODIRACODECHECK_STRUCTURALRULEGEXP05_H
#define CODIRACODECHECK_STRUCTURALRULEGEXP05_H

#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGEXP05 : public StructuralRule {
public:
    explicit StructuralRuleGEXP05(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGEXP05() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    std::unordered_map<Codira::TokenKind, std::unordered_set<Codira::TokenKind>> ConfusOperMap = {
        {TokenKind::ADD,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::SUB,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::MUL,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::DIV,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::MOD,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::EXP,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LT, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LT, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LE, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::GT, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::GE, {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::EQUAL,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::NOTEQ,
            {TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::BITAND, TokenKind::BITOR, TokenKind::BITXOR}},
        {TokenKind::LSHIFT,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::RSHIFT,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::BITAND,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::BITOR,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}},
        {TokenKind::BITXOR,
            {TokenKind::ADD, TokenKind::SUB, TokenKind::MUL, TokenKind::DIV, TokenKind::MOD, TokenKind::EXP,
                TokenKind::LT, TokenKind::LE, TokenKind::GT, TokenKind::GE, TokenKind::EQUAL, TokenKind::NOTEQ}}};
    void FindParenExpr(Codira::AST::Node* node);
    void CheckParenExpr(const Codira::AST::ParenExpr& parenExpr);
    void CheckBinaryExpr(const Codira::AST::BinaryExpr& binaryExpr);
    void CheckSubBinaryExpr(AST::Expr* subExpr, const Codira::TokenKind& op);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURALRULEGEXP05_H
