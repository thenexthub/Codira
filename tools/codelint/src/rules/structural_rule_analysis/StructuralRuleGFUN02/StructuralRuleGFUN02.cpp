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

#include "StructuralRuleGFUN02.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

/*
 * The function that find the FuncExpr's params and insert them to param Map.
 *
 */
void StructuralRuleGFUN02::FuncParamFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                std::map<std::string, PositionPair> paramMap;
                for (auto& paramList : funcDecl.funcBody->paramLists) {
                    for (auto& param : paramList->params) {
                        paramMap[param->identifier] =
                            std::make_pair(param->identifier.Begin(), param->identifier.End());
                    }
                }

                FuncParamFinderHelper(funcDecl.funcBody->body.get(), paramMap);
                if (funcDecl.funcBody->body == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                for (auto& item : funcDecl.funcBody->body->body) {
                    if (item->astKind == ASTKind::FUNC_DECL) {
                        FuncParamFinder(item.get());
                    }
                }
                return VisitAction::SKIP_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGFUN02::EraseItemOfMap(const std::string& id, std::map<std::string, PositionPair>& paramMap)
{
    auto iter = paramMap.find(id);
    if (iter != paramMap.end()) {
        (void)paramMap.erase(id);
    }
}
/*
 * The function that find the param which is used and delete it from param map.
 *
 */
void StructuralRuleGFUN02::FuncParamFinderHelper(Ptr<Node> node, std::map<std::string, PositionPair>& paramMap)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this, &paramMap](Ptr<Node> node) -> VisitAction {
        match (*node)(
            [&paramMap, this](const RefExpr& refExpr) {
                EraseItemOfMap(refExpr.ref.identifier.GetRawText(), paramMap);
            },
            [&paramMap, this](const MacroExpandExpr& macroExpandExpr) {
                for (auto& arg : macroExpandExpr.invocation.args) {
                    if (arg.kind != TokenKind::IDENTIFIER) {
                        continue;
                    }
                    EraseItemOfMap(arg.Value(), paramMap);
                }
            });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();

    // if the paramMap is not empty that means the param which in map is not be used, throw error.
    if (!paramMap.empty()) {
        for (auto& param : paramMap) {
            if (param.first == "_") {
                continue;
            }
            Diagnose(param.second.first, param.second.second, CodeCheckDiagKind::G_FUN_02_unused_param_information,
                param.first.c_str());
        }
    }
}

void StructuralRuleGFUN02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FuncParamFinder(node);
}
} // namespace Codira::CodeCheck
