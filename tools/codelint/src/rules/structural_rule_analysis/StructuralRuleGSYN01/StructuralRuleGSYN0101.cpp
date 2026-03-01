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

#include "StructuralRuleGSYN0101.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

constexpr const char* JSONPATH = "/config/structural_rule_G_SYN_01.json";

void StructuralRuleGSYN0101::InitDisabledSyntaxCheckMap()
{
    disabledSyntaxCheckMap["PrimitiveType"] = handlePrimitiveType;
}

// Collect keywords for disabling syntax
void StructuralRuleGSYN0101::CollectDisabledSyntax()
{
    std::unordered_set<std::string> disabledSyntaxKeywords;
    if (!jsonInfo.contains("SyntaxKeyword")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    // Ensure consistency with the parameter requirements for the JsonArrayStringValueGet() interface
    for (const auto& item : jsonInfo["SyntaxKeyword"]) {
        std::string keyWord = item.get<std::string>();
        disabledSyntaxKeywords.insert(keyWord);
    }
    for (auto& item : disabledSyntaxKeywords) {
        if (disabledSyntaxCheckMap.count(item) != 0) {
            syntaxList.insert(item);
        }
    }
}

void StructuralRuleGSYN0101::FindSyntax(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        // Check whether the syntax is violated based on the check on some kind of AST nodes.
        for (auto& syntaxName : syntaxList) {
            disabledSyntaxCheckMap[syntaxName](*node);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGSYN0101::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    InitDisabledSyntaxCheckMap();
    CollectDisabledSyntax();
    FindSyntax(node);
}
} // namespace Codira::CodeCheck
