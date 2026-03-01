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

#include "StructuralRuleP01.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

const int P_01_MIN_CROSS_SEQ_NUM = 2;
const int P_01_MIN_COMPARE_FUNCTION = 2;
const int P_01_TUPLE_PARAM_0 = 0;
const int P_01_TUPLE_PARAM_1 = 1;
const int P_01_TUPLE_PARAM_2 = 2;
const int P_01_TUPLE_PARAM_3 = 3;

void StructuralRuleP01::PrintDebugInfo() const
{
#ifndef NDEBUG
    if (funcLockSeq.size() == 0) {
        return;
    }

    std::cout << "---------------------------------------------" << std::endl;
    for (auto iter = funcLockSeq.begin(); iter != funcLockSeq.end(); ++iter) {
        std::cout << "function name: " << iter->first << std::endl;
        for (auto &block : iter->second) {
            std::cout << "------" << std::endl;
            std::cout << "block line: " << block.first.first.first.line << ", file name: " << block.first.second
                      << std::endl;
            for (auto &item : block.second) {
                std::cout << "object name: " << item.first << ", object line: " << item.second << std::endl;
            }
        }

        std::cout << std::endl;
    }
    std::cout << "---------------------------------------------" << std::endl;
#endif
    return;
}


template <typename K, typename V> std::vector<K> KeySet(std::unordered_map<K, V> map)
{
    std::vector<K> keys;
    for (auto it = map.begin(); it != map.end(); ++it) {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end());
    return keys;
}

template <typename T> bool HasSameSeq(const std::vector<T> &blockSeq1, const std::vector<T> &blockSeq2)
{
    // Get the synchronized objects that both input blocks have, and keeping their original order.
    std::vector<T> crossSeq1;
    std::vector<T> crossSeq2;
    for (auto &item1 : blockSeq1) {
        for (auto &item2 : blockSeq2) {
            if (item1 == item2) {
                crossSeq1.push_back(item1);
            }
        }
    }

    for (auto &item2 : blockSeq2) {
        for (auto &item1 : blockSeq1) {
            if (item2 == item1) {
                crossSeq2.push_back(item2);
            }
        }
    }

    // If more than two synchronized objects are the same and in a different order,
    // then the order is considered to be different.
    return (crossSeq1.size() >= P_01_MIN_CROSS_SEQ_NUM && (crossSeq1 != crossSeq2)) ? false : true;
}

void StructuralRuleP01::CompareTwoFuncBlocks(P01FuncBlocks &blocksI, P01FuncBlocks &blocksJ,
    std::vector<P01DiagFmt> &diagInfos)
{
    for (auto block1 : blocksI) {
        for (auto block2 : blocksJ) {
            if (HasSameSeq(block1.second, block2.second)) {
                continue;
            }

            auto position1 = block1.first.first;
            auto fileName1 = block1.first.second;
            auto position2 = block2.first.first;
            auto fileName2 = block2.first.second;
            if (position1.first.line > position2.first.line) {
                diagInfos.push_back(
                    std::make_tuple(position1, CodeCheckDiagKind::P_01_dead_lock, position2.first.line, fileName2));
            } else {
                diagInfos.push_back(
                    std::make_tuple(position2, CodeCheckDiagKind::P_01_dead_lock, position1.first.line, fileName1));
            }
        }
    }

    return;
}

void StructuralRuleP01::CheckResult()
{
    auto funcNames = KeySet(funcLockSeq);
    auto funcNum = funcNames.size();
    // Less than two functions do not need to be compared.
    if (funcNum < P_01_MIN_COMPARE_FUNCTION) {
        return;
    }

    std::vector<P01DiagFmt> diagInfos;

    for (size_t i = 0; i < funcNum - 1; ++i) {
        auto blocksI = funcLockSeq[funcNames[i]];
        for (size_t j = i + 1; j < funcNum; ++j) {
            auto blocksJ = funcLockSeq[funcNames[j]];
            CompareTwoFuncBlocks(blocksI, blocksJ, diagInfos);
        }
    }

    // Print the sorted unique warning information.
    std::sort(diagInfos.begin(), diagInfos.end());
    (void)diagInfos.erase(std::unique(diagInfos.begin(), diagInfos.end()), diagInfos.end());
    for (auto &diag : diagInfos) {
        Diagnose(std::get<P_01_TUPLE_PARAM_0>(diag).first, std::get<P_01_TUPLE_PARAM_0>(diag).second,
            std::get<P_01_TUPLE_PARAM_1>(diag), std::get<P_01_TUPLE_PARAM_2>(diag), std::get<P_01_TUPLE_PARAM_3>(diag));
    }

    return;
}

void StructuralRuleP01::SynchronizedExprHandler(SynchronizedExpr &expr, const std::string funcName)
{
    std::vector<std::pair<std::string, int>> blockSeq;
    PositionPair blockPos = std::make_pair(expr.begin, expr.end);
    std::string curFileName = (expr.curFile != nullptr) ? expr.curFile->fileName : "";

    Walker walker(&expr, [this, &blockSeq](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this, &blockSeq](const SynchronizedExpr& exprInner) {
                if (auto refExpr = DynamicCast<RefExpr*>(exprInner.mutex.get().get()); refExpr) {
                    auto targetLine = 0;
                    if (refExpr->ref.target != nullptr) {
                        targetLine = refExpr->ref.target->identifier.Begin().line;
                    }
                    blockSeq.push_back(std::make_pair(refExpr->ref.identifier.Val(), targetLine));
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();

    // The synchronized block in the lambda is scanned twice, which may be an AST bug, I will continue to track it
    // later. The current temporary solution is to de-duplicate the same blocks which have the same line number and the
    // same file name.
    for (auto &block : funcLockSeq[funcName]) {
        if (blockPos == block.first.first && curFileName == block.first.second) {
            return;
        }
    }

    if (blockSeq.size() > 1) {
        funcLockSeq[funcName].push_back(std::make_pair(std::make_pair(blockPos, curFileName), blockSeq));
    }

    return;
}

std::string GetFuncIdent(const FuncBody &funcBody)
{
    std::string fullPackageName = "";
    std::string outterName = "";
    if (funcBody.parentClassLike != nullptr) {
        outterName = funcBody.parentClassLike->identifier;
        fullPackageName = funcBody.parentClassLike->fullPackageName;
    } else if (funcBody.parentStruct != nullptr) {
        outterName = funcBody.parentStruct->identifier;
        fullPackageName = funcBody.parentStruct->fullPackageName;
    } else if (funcBody.parentEnum != nullptr) {
        outterName = funcBody.parentEnum->identifier;
        fullPackageName = funcBody.parentEnum->fullPackageName;
    } else if (funcBody.funcDecl != nullptr) {
        outterName = funcBody.funcDecl->identifier;
        fullPackageName = funcBody.funcDecl->fullPackageName;
    }

    if (fullPackageName.size() == 0 || outterName.size() == 0) {
        return "";
    }

    return funcBody.funcDecl->identifier + "_" + fullPackageName + "_" + outterName;
}

void StructuralRuleP01::FuncHandler(FuncBody &funcBody)
{
    auto funcIdent = GetFuncIdent(funcBody);
    if (funcIdent.size() == 0) {
        // lambda and spawn expressions are not processed in this process
        return;
    }

    Walker walker(&funcBody, [this, &funcIdent](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const LambdaExpr &lambdaExpr) {
                LambdaExprHandler((LambdaExpr &)lambdaExpr);
                return VisitAction::SKIP_CHILDREN;
            },
            [this, &funcIdent](const SynchronizedExpr &expr) {
                StructuralRuleP01::SynchronizedExprHandler((SynchronizedExpr &)expr, funcIdent);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleP01::LambdaExprHandler(const Codira::AST::LambdaExpr &lambdaExpr)
{
    std::string fileName = (lambdaExpr.curFile != nullptr) ? lambdaExpr.curFile->fileName : "";
    std::string funcName = "lambda_" + fileName + "_" + std::to_string(lambdaExpr.begin.line);
    Walker walker(lambdaExpr.funcBody->body.get(), [this, &funcName](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this, &funcName](const SynchronizedExpr &expr) {
                StructuralRuleP01::SynchronizedExprHandler((SynchronizedExpr &)expr, funcName);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();

    return;
}

void StructuralRuleP01::GetLockSeq(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncBody &funcBody) {
                FuncHandler((FuncBody &)funcBody);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const LambdaExpr &lambdaExpr) {
                LambdaExprHandler((LambdaExpr &)lambdaExpr);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleP01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;

    GetLockSeq(node);
}

void StructuralRuleP01::DoAnalysis(CODELintCompilerInstance *instance)
{
    auto packageList = instance->GetSourcePackages();
    for (auto package : packageList) {
        auto &ctx = *instance->GetASTContextByPackage(package);
        MatchPattern(ctx, package);
    }

    CheckResult();
    PrintDebugInfo();
}
}
