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

#ifndef DATAFLOW_RULE_P_02_CHECK_H
#define DATAFLOW_RULE_P_02_CHECK_H

#include "../DataflowRule.h"
#include "Codira/AST/Node.h"
#include "Codira/Utils/Utils.h"
#include "common/CommonData.h"
#include "common/DiagnosticEngine.h"

namespace Codira::CodeCheck {
class DataflowRuleP02Check : public DataflowRule {
public:
    DataflowRuleP02Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleP02Check(CodeCheckDiagnosticEngine *diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleP02Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package &package) override;

private:
    void CheckBasedOnCHIRFunc(CHIR::BlockGroup& body);
    struct MemberVarInChain {
        CHIR::Value* base;
        std::string path;
        MemberVarInChain(CHIR::Value* base, std::string path): base(base), path(path) {}
        MemberVarInChain(): base(nullptr), path("") {}
        bool operator==(const MemberVarInChain& rhs) const
        {
            return base == rhs.base && path == rhs.path;
        }
        bool operator<(const MemberVarInChain& rhs) const
        {
            return path < rhs.path;
        }
    };
    using VarWithMutexPosMap = std::map<const CHIR::Value *, std::set<Codira::Position>>;
    using VarInChainWithMutexPosMap = std::map<MemberVarInChain, std::set<Codira::Position>>;
    using VarWithDiffMutexMap = std::map<const CHIR::Value *, std::map<CHIR::Value *, std::set<Codira::Position>>>;
    using VarInChainWithDiffMutexMap = std::map<MemberVarInChain, std::map<CHIR::Value *,
        std::set<Codira::Position>>>;
    void CheckNonTerminal(const CHIR::Expression *expr, bool &isLock);
    void CheckSpawnClosure(const CHIR::Lambda *spawnClosure);
    VarWithMutexPosMap varWithoutLock;
    VarInChainWithMutexPosMap varInChainWithoutLock;
    std::set<MemberVarInChain> varInChainStoreInSpawn;
    std::set<const CHIR::Value*> varStoreInSpawn;
    VarWithDiffMutexMap varWithDiffLock;
    VarInChainWithDiffMutexMap varInChainWithDiffLock;
    void AddElementToMap(const CHIR::Value *value, Codira::Position pos, bool withLock = false);
    void AddElementToSet(const CHIR::Value *value);
    std::string PrintTips(const VarWithDiffMutexMap &varWithDiffLock);
    bool debug = false;
    CHIR::Value *mutexLock{nullptr};
    template <typename T> MemberVarInChain GetMemberVarInChain(const T* getElementRef);
    MemberVarInChain GetMemberVarInChain(CHIR::Expression *expr);
    template <typename T>
    void CheckApplyOrInVoke(const CHIR::Expression &expr);
    void PrintVarWithoutMutex();
    void PrintVarInChainWithoutMutex();
    void PrintVarWithDiffMutex();
    void PrintVarInChainWithDiffMutex();
    std::pair<Codira::Position, Codira::Position> FindPositionOfMemberVar(const MemberVarInChain &memberVarInChain);
};
} // namespace Codira::CodeCheck
#endif // DATAFLOW_RULE_P_02_CHECK_H
