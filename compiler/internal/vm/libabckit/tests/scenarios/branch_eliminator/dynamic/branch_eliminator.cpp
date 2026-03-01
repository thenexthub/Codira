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

#include "libabckit/c/abckit.h"

#include "branch_eliminator.h"

#include <cassert>
#include <gtest/gtest.h>

BranchEliminator::BranchEliminator(enum AbckitApiVersion version, AbckitFile *file, std::vector<ConstantInfo> constants)
    : file_(file), constants_(std::move(constants))
{
    impl_ = AbckitGetApiImpl(version);
    implI_ = AbckitGetInspectApiImpl(version);
    implG_ = AbckitGetGraphApiImpl(version);
    dynG_ = AbckitGetIsaApiDynamicImpl(version);
    implM_ = AbckitGetModifyApiImpl(version);
    vh_ = VisitHelper(file_, impl_, implI_, implG_, dynG_);
}

void BranchEliminator::Run()
{
    if (constants_.empty()) {
        return;
    }
    vh_.EnumerateModules([&](AbckitCoreModule *mod) {
        SuspectsType suspects;
        if (!GetSuspects(mod, suspects)) {
            return;
        }
        vh_.EnumerateModuleFunctions(mod,
                                     [&](AbckitCoreFunction *func) { EliminateBranchWithSuspect(func, suspects); });
    });
}

bool BranchEliminator::GetSuspects(AbckitCoreModule *mod, SuspectsType &suspects)
{
    vh_.EnumerateModuleImports(mod, [&](AbckitCoreImportDescriptor *id) {
        auto importName = vh_.GetString(implI_->importDescriptorGetName(id));
        auto *importedModule = implI_->importDescriptorGetImportedModule(id);
        auto path = vh_.GetString(implI_->moduleGetName(importedModule));
        for (ConstantInfoIndexType i = 0; i < constants_.size(); ++i) {
            const auto &constInfo = constants_[i];
            if (constInfo.path != path || constInfo.objName != importName) {
                continue;
            }
            auto iter = suspects.find(id);
            if (iter == suspects.end()) {
                std::vector<ConstantInfoIndexType> vec = {i};
                suspects.emplace(id, vec);
            } else {
                iter->second.push_back(i);
            }
        }
    });
    return !suspects.empty();
}

bool BranchEliminator::IsEliminatableIfInst(AbckitInst *inst)
{
    if (dynG_->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_IF || implG_->iGetInputCount(inst) != 2U) {
        return false;
    }
    auto *ldBool = implG_->iGetInput(inst, 0);
    auto op = dynG_->iGetOpcode(ldBool);
    if (op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE && op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE) {
        return false;
    }
    auto *constInst = implG_->iGetInput(inst, 1);
    return dynG_->iGetOpcode(constInst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
}

void BranchEliminator::EliminateBranchWithSuspect(AbckitCoreFunction *method, const SuspectsType &suspects)
{
    vh_.TransformMethod(method, [&](AbckitFile *, AbckitCoreFunction *method) {
        AbckitGraph *graph = implI_->createGraphFromFunction(method);
        bool hasGraphChanged = ReplaceModuleVarByConstant(graph, suspects);
        // CC-OFFNXT(G.CTL.03) implementation feature
        do {
            hasGraphChanged |= LoweringConstants(graph);

            auto *ifInst = vh_.GraphInstsFindIf(graph, [&](AbckitInst *inst) { return IsEliminatableIfInst(inst); });

            if (ifInst == nullptr) {
                break;
            }

            DeleteUnreachableBranch(ifInst);

            hasGraphChanged = true;
        } while (true);  // until no branch can be eliminated, i.e., there is no such If inst

        if (hasGraphChanged) {
            implM_->functionSetGraph(method, graph);
        }
        impl_->destroyGraph(graph);
    });
}

bool BranchEliminator::ReplaceModuleVarByConstant(AbckitGraph *graph, const SuspectsType &suspects)
{
    // find the following patter:
    // ...
    // 1. ldExternalModuleVar (importdescriptor)
    // 2. ldObjByName v1, "isDebug"
    //
    // and replace it by
    // 3. ldtrue

    std::unordered_map<AbckitInst *, ConstantInfoIndexType> moduleVars;
    vh_.EnumerateGraphInsts(graph, [&](AbckitInst *inst) {
        auto constInfoIndex = GetConstantInfoIndex(inst, suspects);
        if (constInfoIndex != INVALID_INDEX) {
            moduleVars.emplace(inst, constInfoIndex);
        }
    });
    for (auto &[ldObjByName, constInfoIndex] : moduleVars) {
        EXPECT_TRUE(ldObjByName != nullptr);
        EXPECT_TRUE(constInfoIndex != INVALID_INDEX);
        ReplaceLdObjByNameWithBoolean(ldObjByName, constInfoIndex);
    }

    return !moduleVars.empty();
}

ConstantInfoIndexType BranchEliminator::GetConstantInfoIndex(AbckitInst *inst, const SuspectsType &suspects) const
{
    if (dynG_->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
        return INVALID_INDEX;
    }
    auto *ldExternalModuleVar = implG_->iGetInput(inst, 0);
    if (dynG_->iGetOpcode(ldExternalModuleVar) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return INVALID_INDEX;
    }
    auto *id = dynG_->iGetImportDescriptor(ldExternalModuleVar);
    auto iter = suspects.find(id);
    if (iter == suspects.end()) {
        return INVALID_INDEX;
    }
    const auto &constInfoIndexes = iter->second;
    const auto propName = vh_.GetString(implG_->iGetString(inst));  // "isDebug"
    for (auto index : constInfoIndexes) {
        EXPECT_TRUE(index >= 0);
        EXPECT_TRUE(index < constants_.size());
        if (constants_[index].fieldName == propName) {
            return index;
        }
    }

    return INVALID_INDEX;
}

void BranchEliminator::ReplaceLdObjByNameWithBoolean(AbckitInst *ldObjByName, ConstantInfoIndexType constInfoIndex)
{
    bool fieldVal = constants_[constInfoIndex].fieldValue;
    auto *bb = implG_->iGetBasicBlock(ldObjByName);
    auto *graph = implG_->bbGetGraph(bb);
    auto *value = fieldVal ? dynG_->iCreateLdtrue(graph) : dynG_->iCreateLdfalse(graph);
    implG_->iInsertAfter(value, ldObjByName);
    ReplaceUsers(ldObjByName, value);

    auto *ldExternalModuleVar = implG_->iGetInput(ldObjByName, 0);
    bool isRemovable = true;
    std::vector<AbckitInst *> checks;
    vh_.EnumerateInstUsers(ldExternalModuleVar, [&](AbckitInst *inst) {
        if (inst != ldObjByName) {
            if (dynG_->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME) {
                isRemovable = false;
            } else {
                checks.emplace_back(inst);
            }
        }
    });

    if (isRemovable) {
        for (auto *check : checks) {
            implG_->iRemove(check);
        }
        implG_->iRemove(ldExternalModuleVar);
    }
    implG_->iRemove(ldObjByName);
}

void BranchEliminator::ReplaceUsers(AbckitInst *oldInst, AbckitInst *newInst)
{
    vh_.EnumerateInstUsers(oldInst, [&](AbckitInst *user) {
        auto inputCount = implG_->iGetInputCount(user);
        for (uint64_t i = 0; i < inputCount; ++i) {
            if (implG_->iGetInput(user, i) == oldInst) {
                implG_->iSetInput(user, newInst, i);
            }
        }
    });
}

bool BranchEliminator::GetInstAsBool(AbckitInst *inst) const
{
    switch (dynG_->iGetOpcode(inst)) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT:
            return implG_->iGetConstantValueI64(inst) != 0;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE:
            return true;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE:
            return false;
        default:
            EXPECT_TRUE(false);  // UNREACHABLE
    }
    return false;
}

void BranchEliminator::DeleteUnreachableBranch(AbckitInst *ifInst) const
{
    // compute result of If compare
    auto *ldBool = implG_->iGetInput(ifInst, 0);     // 0: ldtrue or ldfalse
    auto *constInst = implG_->iGetInput(ifInst, 1);  // 1: ConstantInst
    auto valLeft = GetInstAsBool(ldBool);
    auto valRight = GetInstAsBool(constInst);
    auto conditionCode = dynG_->iGetConditionCode(ifInst);

    EXPECT_TRUE(conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ ||
                conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    bool result = (conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ && valLeft == valRight) ||
                  (conditionCode == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE && valLeft != valRight);

    auto *bb = implG_->iGetBasicBlock(ifInst);
    // if true ==> delete false branch
    implG_->iRemove(ifInst);

    implG_->bbDisconnectSuccBlock(bb, result ? 1 : 0);
    implG_->gRunPassRemoveUnreachableBlocks(implG_->bbGetGraph(bb));
}

bool BranchEliminator::LoweringConstants(AbckitGraph *graph)
{
    bool hasGraphChanged = false;
    auto doCb = [this, graph, &hasGraphChanged]() -> bool {
        std::vector<AbckitInst *> users;

        auto instFindUsers = [this, &users](AbckitInst *user) {
            auto userOp = dynG_->iGetOpcode(user);
            if (userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE || userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE ||
                userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE ||
                userOp == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE) {
                users.emplace_back(user);
            }
        };

        AbckitInst *ldBool = vh_.GraphInstsFindIf(graph, [&](AbckitInst *inst) {
            auto op = dynG_->iGetOpcode(inst);
            if (op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE && op != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE) {
                return false;
            }

            vh_.EnumerateInstUsers(inst, instFindUsers);

            return !users.empty();
        });

        if (ldBool == nullptr) {
            return false;
        }

        auto ldBoolVal = GetInstAsBool(ldBool);
        for (auto *isBool : users) {
            auto isBoolVal = GetInstAsBool(isBool);
            auto *ldBool = ldBoolVal == isBoolVal ? dynG_->iCreateLdtrue(graph) : dynG_->iCreateLdfalse(graph);
            implG_->iInsertAfter(ldBool, isBool);
            ReplaceUsers(isBool, ldBool);
            implG_->iRemove(isBool);
        }

        hasGraphChanged = true;

        return true;
    };

    while (doCb()) {
    }

    hasGraphChanged |= RemoveUnusedInsts(graph);

    return hasGraphChanged;
}

bool BranchEliminator::RemoveUnusedInsts(AbckitGraph *graph)
{
    bool hasGraphChanged = false;
    // CC-OFFNXT(G.CTL.03) implementation feature
    do {
        std::vector<AbckitInst *> removableInsts;
        vh_.EnumerateGraphInsts(graph, [&](AbckitInst *inst) {
            if (implG_->iGetUserCount(inst) == 0 && CanBeRemoved(inst)) {
                removableInsts.emplace_back(inst);
            }
        });

        if (removableInsts.empty()) {
            break;
        }

        EXPECT_TRUE(!removableInsts.empty());
        for (auto *inst : removableInsts) {
            implG_->iRemove(inst);
        }

        hasGraphChanged = true;
    } while (true);

    return hasGraphChanged;
}

bool BranchEliminator::CanBeRemoved(AbckitInst *inst)
{
    switch (dynG_->iGetOpcode(inst)) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE:
            return true;
        default:
            return false;
    }
}
