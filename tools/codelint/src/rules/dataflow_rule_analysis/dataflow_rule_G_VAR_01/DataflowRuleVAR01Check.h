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

#ifndef DATAFLOW_RULE_VAR_01_CHECK_H
#define DATAFLOW_RULE_VAR_01_CHECK_H

#include "../DataflowRule.h"
#include "Codira/AST/Node.h"
#include "Codira/Utils/Utils.h"
#include "common/CommonData.h"
#include "common/DiagnosticEngine.h"

#define TRY_GET_EXPR(VAR, EXPR)                                                                                        \
        if (!(VAR)->IsLocalVar()) {                                         \
            return;                                                                                                    \
        }                                                                                                              \
        auto LOCALVAR = StaticCast<CHIR::LocalVar*>(VAR);                                                             \
        auto EXPR = LOCALVAR->GetExpr()

namespace Codira::CodeCheck {
class DataflowRuleVAR01Check : public DataflowRule {
public:
    DataflowRuleVAR01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleVAR01Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleVAR01Check() override = default;
    void DoAnalysis(CODELintCompilerInstance* instance) override;
protected:
    void CheckBasedOnCHIR(CHIR::Package& package) override;

private:
    using GlobalVarAssignCountMap = std::unordered_map<Ptr<CHIR::GlobalVar>, uint64_t>;
    using LocalVarAssignCountMap = std::unordered_map<Ptr<CHIR::LocalVar>, int>;
    using ClassMemberVarsMap = std::unordered_map<Ptr<CHIR::CustomTypeDef>, std::map<uint64_t, uint64_t>>;
    using PositionPair = std::pair<Codira::Position, Codira::Position>;

    void CheckBasedOnCHIRFunc(CHIR::BlockGroup& body, ClassMemberVarsMap& memberVarMap);
    void CollectMemberVar(Ptr<CHIR::CustomTypeDef> customTypeDef,
        std::vector<Codira::CHIR::MemberVarInfo>& memberVars, ClassMemberVarsMap& memberVarMap);
    void CollectMemberVar(Ptr<CHIR::ClassDef> classDef, ClassMemberVarsMap& memberVarMap);
    void CheckCustomTypeDef(Ptr<CHIR::CustomTypeDef> customTypeDef, ClassMemberVarsMap& memberVarMap);
    void CheckGetElementRef(Ptr<CHIR::Expression> expr, ClassMemberVarsMap& memberVarMap);
    void CheckAllTypesInPackage(CHIR::Package& package, ClassMemberVarsMap& memberVarMap);
    void CheckExprHelper(Ptr<Codira::CHIR::Expression> expr, GlobalVarAssignCountMap& globalVarAssignCountMap,
        LocalVarAssignCountMap& localVarAssignCountMap);
    void AddOrEraseElement(Ptr<CHIR::Value> var, GlobalVarAssignCountMap& globalVarAssignCountMap,
        LocalVarAssignCountMap& localVarAssignCountMap);
    void CheckApply(Ptr<CHIR::Expression> expr, LocalVarAssignCountMap& localVarAssignCountMap);
    void CheckApplyHelper(Ptr<CHIR::LocalVar> localVar, LocalVarAssignCountMap& localVarAssignCountMap,
        std::map<std::string, std::set<CHIR::LocalVar*>>& varMap);

    void CheckGlobalVar(const CHIR::GlobalVar* var);
    void CheckLocalVar(const CHIR::LocalVar* var, LocalVarAssignCountMap& localVarAssignCountMap);
    void CheckMemberVarHelper(
        Codira::CHIR::CustomType* customTy, std::vector<uint64_t>& path, ClassMemberVarsMap& memberVarMap);
    void CheckMemberVar(const CHIR::Store* store, ClassMemberVarsMap& memberVarMap);
    void CheckMemberVar(const CHIR::StoreElementRef* storeElementRef, ClassMemberVarsMap& memberVarMap);

    std::set<const CHIR::Value*> varHasChanged;
    bool CheckVarHelper(const CHIR::Value* var, int& count);
    std::map<const CHIR::CustomTypeDef*, std::pair<PositionPair, std::string>> genericDefMap;
    std::set<const CHIR::CustomTypeDef*> genericDefSet;
    std::set<const CHIR::GlobalVar*> staticVarInGeneric;
    std::set<const CHIR::GlobalVar*> staticVarhasChanged;
    void PrintDiagnoseInfo(const ClassMemberVarsMap& memberVarMap);
    void PrintDiagnoseInfo(const LocalVarAssignCountMap& localVarAssignCountMap);
    std::set<std::pair<Codira::Position, Codira::Position>> unsafeBlock;
    void FindAllUnSafeBlock(AST::Package& package);
    bool IsInUnSafeBlock(Codira::Position pos);
};
} // namespace Codira::CodeCheck
#endif // DATAFLOW_RULE_VAR_01_CHECK_H
