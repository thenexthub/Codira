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

#ifndef BRANCH_ELIMINATOR_H
#define BRANCH_ELIMINATOR_H

#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "helpers/visit_helper/visit_helper-inl.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

struct ConstantInfo {
    std::string path;
    std::string objName;    // Config
    std::string fieldName;  // isDebug
    bool fieldValue;        // field is constant true or false
};

using ConstantInfoIndexType = uint32_t;

using SuspectsType = std::unordered_map<AbckitCoreImportDescriptor *, std::vector<ConstantInfoIndexType>>;

class BranchEliminator {
public:
    BranchEliminator(enum AbckitApiVersion version, AbckitFile *file, std::vector<ConstantInfo> constants);

    void Run();

    constexpr static ConstantInfoIndexType INVALID_INDEX = std::numeric_limits<ConstantInfoIndexType>::max();

private:
    bool GetInstAsBool(AbckitInst *inst) const;
    bool GetSuspects(AbckitCoreModule *mod, SuspectsType &suspects);
    void EliminateBranchWithSuspect(AbckitCoreFunction *method, const SuspectsType &suspects);
    ConstantInfoIndexType GetConstantInfoIndex(AbckitInst *inst, const SuspectsType &suspects) const;
    void ReplaceUsers(AbckitInst *oldInst, AbckitInst *newInst);
    bool ReplaceModuleVarByConstant(AbckitGraph *graph, const SuspectsType &suspects);
    void ReplaceLdObjByNameWithBoolean(AbckitInst *ldObjByName, ConstantInfoIndexType constInfoIndex);
    void DeleteUnreachableBranch(AbckitInst *ifInst) const;
    bool LoweringConstants(AbckitGraph *graph);
    bool RemoveUnusedInsts(AbckitGraph *graph);
    bool CanBeRemoved(AbckitInst *inst);
    bool IsEliminatableIfInst(AbckitInst *ifInst);

private:
    const AbckitApi *impl_ = nullptr;
    const AbckitInspectApi *implI_ = nullptr;
    const AbckitGraphApi *implG_ = nullptr;
    const AbckitIsaApiDynamic *dynG_ = nullptr;
    const AbckitModifyApi *implM_ = nullptr;
    AbckitFile *file_ = nullptr;
    VisitHelper vh_;

    std::vector<ConstantInfo> constants_;
};

#endif /* BRANCH_ELIMINATOR_H */
