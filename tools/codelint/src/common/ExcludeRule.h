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

#ifndef EXCLUDE_RULE_H
#define EXCLUDE_RULE_H

#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <exception>
#include "Codira/Utils/FileUtil.h"

namespace Codira::CodeCheck {
class ExcludeRule {
public:
    enum class ExcludeRuleType {
        IGNORE = 0,
        INCLUDE
    };
    enum class ExcludeRuleTarget {
        FILE_AND_DIR = 0,
        DIRECTORY
    };
    ExcludeRule(std::string srcFileDir, std::string exclude);
    ~ExcludeRule() = default;
    const ExcludeRuleType GetRuleType() const;
    const ExcludeRuleTarget GetRuleTarget() const;
    bool IsMatched(const std::string &target);
    bool isValid = true;

private:
    void Preprocess(std::string &regexStr);
    void MatchSignModification(std::string &regexStr);

    ExcludeRuleType ruleType = ExcludeRuleType::IGNORE;
    ExcludeRuleTarget ruleTarget = ExcludeRuleTarget::FILE_AND_DIR;
    std::string excludeRegexStr;
    std::regex excludeRegex;
    bool nonTrailSlash = false;
};
} // namespace Codira::CodeCheck::ExcludeRule

#endif // EXCLUDE_RULE_H
