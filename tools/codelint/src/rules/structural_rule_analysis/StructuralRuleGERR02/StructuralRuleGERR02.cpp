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

#include "StructuralRuleGERR02.h"
#include <regex>
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

constexpr const char* JSONPATH = "/config/structural_rule_G_ERR_02.json";
const int G_ERR_02_FIRST_MATCH = 0;

void StructuralRuleGERR02::PrintDebugInfo() const
{
#ifndef NDEBUG
    if (debugInfo.size() == 0) {
        return;
    }
    std::cout << "---------------------------------------------" << std::endl;
    for (auto iter = debugInfo.begin(); iter != debugInfo.end(); ++iter) {
        std::cout << "------" << std::endl;
        std::cout << "line: " << iter->second << ", string: " << iter->first << std::endl;
        std::cout << "------" << std::endl;
    }

    std::cout << "---------------------------------------------" << std::endl;
#endif
    return;
}


bool StructuralRuleGERR02::IsSensitiveException(const std::string &input) const
{
    if (!jsonInfo.contains("SensitiveException")) {
        Errorln(JSONPATH, " read json data failed!");
        return false;
    }

    for (const auto& item : jsonInfo["SensitiveException"]) {
        if (item.get<std::string>() == input) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> StructuralRuleGERR02::GetSensitiveKeyWord(const std::string &input)
{
    std::vector<std::string> keyWords;

    if (!jsonInfo.contains("SensitiveKeyword")) {
        Errorln(JSONPATH, " read json data failed!");
        return keyWords;
    }

    for (const auto& item : jsonInfo["SensitiveKeyword"]) {
        std::string keyWord = item.get<std::string>();
        std::regex re(R"(\b)" + keyWord + R"(\b)", std::regex_constants::icase);
        std::smatch match;
        std::string::const_iterator iterStart = input.begin();
        std::string::const_iterator iterEnd = input.end();
        while (std::regex_search(iterStart, iterEnd, match, re)) {
            keyWords.push_back(match[G_ERR_02_FIRST_MATCH]);
            iterStart = match[G_ERR_02_FIRST_MATCH].second;
        }
    }

    std::sort(keyWords.begin(), keyWords.end());
    (void)keyWords.erase(std::unique(keyWords.begin(), keyWords.end()), keyWords.end());
    return keyWords;
}

std::string StructuralRuleGERR02::GetDeclStr(Ptr<Decl> decl)
{
    if (auto varDecl = DynamicCast<VarDecl*>(decl.get()); varDecl) {
        if (auto binaryExpr = DynamicCast<BinaryExpr*>(varDecl->initializer.get().get()); binaryExpr) {
            return GetBinaryStr(binaryExpr);
        } else if (auto litConstExpr = DynamicCast<LitConstExpr*>(varDecl->initializer.get().get()); litConstExpr) {
            return litConstExpr->stringValue;
        }
    }

    return "";
}

std::string StructuralRuleGERR02::GetBinaryStr(Ptr<BinaryExpr> binaryExpr)
{
    std::string str;
    if (auto node = DynamicCast<CallExpr*>(binaryExpr->desugarExpr.get().get()); node) {
        if (auto memberAccess = DynamicCast<MemberAccess*>(node->baseFunc.get().get()); memberAccess) {
            if (memberAccess->field != "+") {
                return "";
            }

            if (auto subBinaryNode = DynamicCast<BinaryExpr*>(memberAccess->baseExpr.get().get()); subBinaryNode) {
                str = GetBinaryStr(subBinaryNode);
            } else if (auto leftNode = DynamicCast<LitConstExpr*>(memberAccess->baseExpr.get().get()); leftNode) {
                str = leftNode->stringValue;
            } else if (auto subRefNode = DynamicCast<RefExpr*>(memberAccess->baseExpr.get().get()); subRefNode) {
                str = GetDeclStr(subRefNode->ref.target);
            }
        }

        if (auto rightNode = DynamicCast<LitConstExpr*>(node->args[0].get()->expr.get().get()); rightNode) {
            str += rightNode->stringValue;
        } else if (auto rightRefNode = DynamicCast<RefExpr*>(node->args[0].get()->expr.get().get()); rightRefNode) {
            str += GetDeclStr(rightRefNode->ref.target);
        }
    }

    return str;
}

std::string StructuralRuleGERR02::GetExceptionInfo(Ptr<CallExpr> callExpr)
{
    std::string infoStr;
    if (callExpr->args.size() != 1) {
        return "";
    }
    Ptr<Expr> node;
    if (callExpr->desugarExpr && callExpr->desugarExpr->astKind == ASTKind::CALL_EXPR) {
        auto desugarCallExpr = DynamicCast<CallExpr*>(callExpr->desugarExpr.get());
        if (!desugarCallExpr) {
            return "";
        }
        node = desugarCallExpr->args[0].get()->expr.get();
    } else {
        node = callExpr->args[0].get()->expr.get();
    }
    if (auto litConstExpr = DynamicCast<LitConstExpr*>(node); litConstExpr) {
        infoStr = litConstExpr->stringValue;
    } else if (auto binaExpr = DynamicCast<BinaryExpr*>(node); binaExpr) {
        infoStr = GetBinaryStr(binaExpr);
    } else if (auto refExpr = DynamicCast<RefExpr*>(node); refExpr) {
        infoStr = GetDeclStr(refExpr->ref.target);
    }

    return infoStr;
}

void StructuralRuleGERR02::CheckResult(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ThrowExpr& throwExpr) {
                if (auto throwCallExpr = DynamicCast<CallExpr*>(throwExpr.expr.get()); throwCallExpr) {
                    auto exceptionType = throwCallExpr->ty->name;
                    if (IsSensitiveException(exceptionType)) {
                        exceptionTypeDiag.emplace_back(
                            std::make_pair(throwCallExpr->begin, throwCallExpr->end), exceptionType);
                    }

                    auto exceptionStr = GetExceptionInfo(throwCallExpr);
                    auto SensitiveKeyWords = GetSensitiveKeyWord(exceptionStr);
                    for (auto &keyWord : SensitiveKeyWords) {
                        exceptionInfoDiag.emplace_back(
                            std::make_pair(throwCallExpr->begin, throwCallExpr->end), keyWord);
                    }

                    debugInfo.emplace_back(exceptionStr, throwCallExpr->begin.line);
                }

                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();

    std::sort(exceptionTypeDiag.begin(), exceptionTypeDiag.end());
    (void)exceptionTypeDiag.erase(
        std::unique(exceptionTypeDiag.begin(), exceptionTypeDiag.end()), exceptionTypeDiag.end());
    for (auto &typeDiag : exceptionTypeDiag) {
        Diagnose(typeDiag.first.first, typeDiag.first.second, CodeCheckDiagKind::G_ERR_02_sensitive_exception,
            typeDiag.second);
    }
    exceptionTypeDiag.clear();

    std::sort(exceptionInfoDiag.begin(), exceptionInfoDiag.end());
    (void)exceptionInfoDiag.erase(
        std::unique(exceptionInfoDiag.begin(), exceptionInfoDiag.end()), exceptionInfoDiag.end());
    for (auto &infoDiag : exceptionInfoDiag) {
        Diagnose(
            infoDiag.first.first, infoDiag.first.second, CodeCheckDiagKind::G_ERR_02_sensitive_info, infoDiag.second);
    }
    exceptionInfoDiag.clear();
}

void StructuralRuleGERR02::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;

    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    CheckResult(node);
    PrintDebugInfo();
}
}
