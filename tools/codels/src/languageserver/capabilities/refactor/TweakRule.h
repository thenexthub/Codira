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

#ifndef CODIRA_LSP_TWEAKRULE_H
#define CODIRA_LSP_TWEAKRULE_H

#include "Tweak.h"

namespace ark {
// base class of Tweak Rule
class TweakRule {
public:
    enum class TweakError {
        TWEAK_FAIL = 0,
        ERROR_AST
    };

    virtual ~TweakRule() = default;

    // rule check
    virtual bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const = 0;

    static bool CommonCheck(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions);
};

// Rule Engine: Check all rules
class TweakRuleEngine {
public:
    void AddRule(std::unique_ptr<TweakRule> rule)
    {
        rules.push_back(std::move(rule));
    }

    bool CheckRules(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions);

private:
    std::vector<std::unique_ptr<TweakRule>> rules;
};
} // namespace ark

#endif // CODIRA_LSP_TWEAKRULE_H
