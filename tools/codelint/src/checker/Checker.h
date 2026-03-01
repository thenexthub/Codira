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

#ifndef CHECKER_H
#define CHECKER_H

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include "Codelint.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "rules/Rule.h"

namespace Codira::CodeCheck {
using RulesMap = std::map<CompileStage, std::map<const std::string, std::shared_ptr<Rule>>>;
class Checker {
public:
    explicit Checker(CodeCheckDiagnosticEngine* diagEngine);
    ~Checker() = default;
    int CheckCode();

private:
    template <typename T> void RegisterRule(const std::string& name, CompileStage stage)
    {
        rulesMap[stage][name] = std::make_shared<T>(diagEngine);
    }

    static void DoAnalysis(Rule* rule, CODELintCompilerInstance* instance) { rule->DoAnalysis(instance); }

    std::string fileDir;
    std::string modulesDir;
    CodeCheckDiagnosticEngine* diagEngine;
    RulesMap rulesMap;

    void CreateRuleThreads(nlohmann::json jsonInfo, const std::unique_ptr<CODELintCompilerInstance>& instance,
        CompileStage compileStage, int& readJsonCode);
};
} // namespace Codira::CodeCheck

#endif // CHECKER_H
