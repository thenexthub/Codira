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

#ifndef DATAFLOW_RULE_G_CHK_03_CHECK_H
#define DATAFLOW_RULE_G_CHK_03_CHECK_H

#include <set>
#include <string>
#include <utility>
#include "../DataflowRule.h"
#include "Codira/CHIR/Analysis/Engine.h"
#include "Codira/CHIR/Analysis/GenKillAnalysis.h"
#include "common/CommonFunc.h"
#include "common/DiagnosticEngine.h"

namespace Codira::CodeCheck {
class DataflowRuleGCHK03Check : public DataflowRule {
public:
    DataflowRuleGCHK03Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGCHK03Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleGCHK03Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package& package) override;
    void CheckGlobalFunc(CHIR::Func* func);
    void IsPathCanonicalized(CHIR::Func* func);
    void IsPathVerified(CHIR::Func* func);

private:
    nlohmann::json jsonInfo;
};
// This part is used to check "The file path constructed by external data must be canonicalized before being verified."
class CHK03CanonicalizeAnalysis;
class CHK03CanonicalizeDomain : public Codira::CHIR::GenKillDomain<CHK03CanonicalizeDomain> {
    friend class CHK03CanonicalizeAnalysis;

public:
    CHK03CanonicalizeDomain() = delete;
    explicit CHK03CanonicalizeDomain(size_t domainSize) : GenKillDomain(domainSize), allocateIdxMap(nullptr) {}
    CHK03CanonicalizeDomain(size_t domainSize, std::map<const std::string, size_t>* allocateIdxMap)
        : GenKillDomain(domainSize), allocateIdxMap(allocateIdxMap)
    {
    }
    ~CHK03CanonicalizeDomain() override = default;

    const std::map<const std::string, size_t>* GetMap() const { return allocateIdxMap; }

private:
    std::map<const std::string, size_t>* allocateIdxMap;
};

class CHK03CanonicalizeAnalysis final : public Codira::CHIR::GenKillAnalysis<CHK03CanonicalizeDomain> {
public:
    CHK03CanonicalizeAnalysis() = delete;
    explicit CHK03CanonicalizeAnalysis(Codira::CHIR::Func* func, nlohmann::json jsonInfo);
    ~CHK03CanonicalizeAnalysis() final {}

    void InitializeFuncEntryState(CHK03CanonicalizeDomain& state) override;
    void PropagateExpressionEffect(
        CHK03CanonicalizeDomain& state, const Codira::CHIR::Expression* expression) override;
    std::optional<Codira::CHIR::Block*> PropagateTerminatorEffect(
        CHK03CanonicalizeDomain& state, const Codira::CHIR::Terminator* terminator) override;
    CHK03CanonicalizeDomain Bottom() override;

private:
    nlohmann::json jsonInfo;
    std::vector<std::string> preFiles{};
    std::map<const std::string, size_t> allocateIdxMap;
};

// This part is used to check "The file path constructed by external data must be verified before being used."
class CHK03VerifyAnalysis;
class CHK03VerifyDomain : public Codira::CHIR::GenKillDomain<CHK03VerifyDomain> {
    friend class CHK03VerifyAnalysis;

public:
    CHK03VerifyDomain() = delete;
    explicit CHK03VerifyDomain(size_t domainSize) : GenKillDomain(domainSize), allocateIdxMap(nullptr) {}
    CHK03VerifyDomain(size_t domainSize, std::map<const std::string, size_t>* allocateIdxMap)
        : GenKillDomain(domainSize), allocateIdxMap(allocateIdxMap)
    {
    }
    ~CHK03VerifyDomain() override = default;

    const std::map<const std::string, size_t>* GetMap() const { return allocateIdxMap; }

private:
    std::map<const std::string, size_t>* allocateIdxMap;
};

class CHK03VerifyAnalysis final : public Codira::CHIR::GenKillAnalysis<CHK03VerifyDomain> {
public:
    CHK03VerifyAnalysis() = delete;
    explicit CHK03VerifyAnalysis(Codira::CHIR::Func* func, nlohmann::json jsonInfo);
    ~CHK03VerifyAnalysis() final {}

    void InitializeFuncEntryState(CHK03VerifyDomain& state) override;
    void PropagateExpressionEffect(CHK03VerifyDomain& state, const Codira::CHIR::Expression* expression) override;
    std::optional<Codira::CHIR::Block*> PropagateTerminatorEffect(
        CHK03VerifyDomain& state, const Codira::CHIR::Terminator* terminator) override;
    CHK03VerifyDomain Bottom() override;

private:
    nlohmann::json jsonInfo;
    std::vector<std::string> preFiles{};
    std::map<const std::string, size_t> allocateIdxMap;
};
} // namespace Codira::CodeCheck

#endif // DATAFLOW_RULE_G_CHK_03_CHECK_H
