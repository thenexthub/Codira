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

#include "TweakRule.h"
#include "../../logger/Logger.h"

namespace ark {
// 1. selected.start != selected.end
// 2. root != null
// 3. contain a SelectionTree::Complete node
bool TweakRule::CommonCheck(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions)
{
    if (!sel.arkAst || !sel.arkAst->file || !sel.arkAst->sourceManager) {
        return false;
    }
    if (sel.range.start.fileID == sel.range.end.fileID && sel.range.start == sel.range.end) {
        return false;
    }
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return false;
    }
    bool isValid = true;
    bool containComplete = false;
    SelectionTree::Walk(root, [&isValid, &containComplete]
        (const SelectionTree::SelectionTreeNode *node) {
            if (!node->node) {
                isValid = false;
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (node->selected == SelectionTree::Selection::Complete) {
                containComplete = true;
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
    if (!(isValid && containComplete)) {
        extraOptions.insert(std::make_pair("ErrorCode",
            std::to_string(static_cast<int>(TweakRule::TweakError::TWEAK_FAIL))));
    }
    return isValid && containComplete;
}

bool TweakRuleEngine::CheckRules(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions)
{
    if (rules.empty()) {
        return true;
    }
    return TweakRule::CommonCheck(sel, extraOptions)
           && std::all_of(rules.begin(), rules.end(),
                  [&](const auto& rule) { return rule->Check(sel, extraOptions); });
}
} // namespace ark
