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

#ifndef DATAFLOW_RULE_G_FIO_01_CHECK_H
#define DATAFLOW_RULE_G_FIO_01_CHECK_H

#include <set>
#include <string>
#include <utility>
#include "../DataflowRule.h"
#include "Codira/CHIR/Analysis/Engine.h"
#include "Codira/CHIR/Analysis/GenKillAnalysis.h"
#include "common/CommonFunc.h"
#include "common/DiagnosticEngine.h"

class FIO01Analysis;
class FIO01Domain : public Codira::CHIR::GenKillDomain<FIO01Domain> {
    friend class FIO01Analysis;

public:
    FIO01Domain() = delete;
    explicit FIO01Domain(size_t domainSize) : GenKillDomain(domainSize), allocateIdxMap(nullptr) {}
    FIO01Domain(size_t domainSize, std::map<const std::string, size_t>* allocateIdxMap)
        : GenKillDomain(domainSize), allocateIdxMap(allocateIdxMap)
    {
    }
    ~FIO01Domain() override = default;

    const std::map<const std::string, size_t>* GetMap() const { return allocateIdxMap; }

private:
    std::map<const std::string, size_t>* allocateIdxMap;
};

class FIO01Analysis final : public Codira::CHIR::GenKillAnalysis<FIO01Domain> {
public:
    FIO01Analysis() = delete;
    explicit FIO01Analysis(Codira::CHIR::Func* func, const std::vector<std::string>& files = {});
    ~FIO01Analysis() final {}

    void InitializeFuncEntryState(FIO01Domain& state) override;
    void PropagateExpressionEffect(FIO01Domain& state, const Codira::CHIR::Expression* expression) override;
    std::optional<Codira::CHIR::Block*> PropagateTerminatorEffect(
        FIO01Domain& state, const Codira::CHIR::Terminator* terminator) override;
    FIO01Domain Bottom() override;

private:
    std::vector<std::string> preFiles{};
    std::map<const std::string, size_t> allocateIdxMap;
};

namespace Codira::CodeCheck {
class DataflowRuleGFIO01Check : public DataflowRule {
public:
    DataflowRuleGFIO01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGFIO01Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleGFIO01Check() override = default;

protected:
    std::vector<std::string> CheckDefaultParamFunc(CHIR::Func* func);
    void CheckNormalFunc(CHIR::Func* func, const std::vector<std::string>& files,
        std::map<std::string, std::pair<Codira::Position, Codira::Position>>& posMap);
    void CheckBasedOnCHIR(CHIR::Package& package) override;
};
} // namespace Codira::CodeCheck

#endif // DATAFLOW_RULE_G_FIO_01_CHECK_H
