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

#ifndef DATAFLOW_RULE_G_CHK_01_CHECK_H
#define DATAFLOW_RULE_G_CHK_01_CHECK_H

#include <utility>
#include "common/DiagnosticEngine.h"
#include "common/TaintData.h"
#include "../DataflowRule.h"

namespace Codira::CodeCheck {
class DataflowRuleGCHK01Check : public DataflowRule {
public:
    struct NodeInfo {
        bool isRoot = false;
        bool isHarmful = true;
        NodeInfo() {}
        NodeInfo(bool isRoot, bool isHarmful) : isRoot(isRoot), isHarmful(isHarmful) {}
        NodeInfo(const NodeInfo &other)
        {
            isRoot = other.isRoot;
            isHarmful = other.isHarmful;
        }
        NodeInfo &operator = (const NodeInfo &other)
        {
            if (&other != this) {
                this->isRoot = other.isRoot;
                this->isHarmful = other.isHarmful;
            }
            return *this;
        }
    };

    DataflowRuleGCHK01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGCHK01Check(CodeCheckDiagnosticEngine *diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleGCHK01Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package &package) override;

private:
    std::map<std::string, std::set<CHIR::Block *>> varInblockWithoutTainted;
    std::set<std::string> perilousGlobalVars;
    void CheckFuncBody(CHIR::Block& entryBlock);
    template <typename T> bool IsAlarmNotNeeded(T* apply);
    template <typename T> void CheckApply(T* apply, std::set<std::string>& taintedVars);
    void CheckExpr(CHIR::Value* value, CHIR::LocalVar* result, std::set<std::string>& taintedVars);
    template<typename T> void CheckApplyInUsers(T* apply, CHIR::Value* arg);
    template<typename T> void CheckInvokeInUsers(T* invoke, CHIR::Value* arg);
    void CheckStoreInUsers(CHIR::Store* store, std::set<std::string>& taintedVars);
    void CheckApplyUser(CHIR::Expression* user, CHIR::Value* result);
    template <typename T> void CheckApplyUsers(T* apply, std::set<std::string>& taintedVars);
    void GetPerilousGlobalVar(CHIR::GlobalVar *globalVar);
    void CheckStore(CHIR::Store *store, std::set<std::string> &taintedVars);
    void CheckTuple(CHIR::Tuple *tuple, std::set<std::string> &taintedVars);
    void CheckField(CHIR::Field* field, std::set<std::string>& taintedVars);
    void CheckBlock(CHIR::Block* block);
};
} // namespace Codira::CodeCheck

#endif // DATAFLOW_RULE_G_CHK_01_CHECK_H
