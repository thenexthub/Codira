/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_DANGLING_POINTERS_CHECKER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_DANGLING_POINTERS_CHECKER_H

#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/compiler_options.h"

namespace ark::compiler {
class DanglingPointersChecker : public Analysis {
public:
    explicit DanglingPointersChecker(Graph *graph);

    NO_MOVE_SEMANTIC(DanglingPointersChecker);
    NO_COPY_SEMANTIC(DanglingPointersChecker);
    ~DanglingPointersChecker() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "DanglingPointersChecker";
    }
    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    inline static std::unordered_map<Arch, std::unordered_map<std::string, size_t>> regmap_ {
        // NOLINTNEXTLINE(readability-magic-numbers)
        {Arch::AARCH64, {{"thread", 28}, {"frame", 23}, {"acc", 21}, {"acc_tag", 22}}},
        // NOLINTNEXTLINE(readability-magic-numbers)
        {Arch::X86_64, {{"thread", 15}, {"frame", 5}, {"acc", 11}, {"acc_tag", 3}}}};
    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    inline static std::unordered_set<std::string> targetFuncs_ {
        "CreateArrayByIdEntrypoint",         "CreateObjectByClassInterpreter",  "CreateMultiArrayRecEntrypoint",
        "ResolveLiteralArrayByIdEntrypoint", "GetStaticFieldByIdEntrypoint",    "GetCalleeMethodFromBytecodeId",
        "ResolveTypeByIdEntrypoint",         "CreateMultiDimensionalArrayById", "SafepointEntrypointInterp"};

private:
    ArenaVector<const Inst *> objectsUsers_;
    ArenaSet<const BasicBlock *> checkedBlocks_;
    ArenaVector<Inst *> phiInsts_;
    ArenaSet<Inst *> objectInsts_;
    Inst *accLivein_ {nullptr};
    Inst *accTagLivein_ {nullptr};
    Inst *frameLivein_ {nullptr};
    Inst *threadLivein_ {nullptr};
    Inst *lastAccDef_ {nullptr};
    Inst *lastAccTagDef_ {nullptr};
    Inst *lastFrameDef_ {nullptr};

    bool CheckAccSyncCallRuntime();
    void InitLiveIns();
    void UpdateLastAccAndFrameDef(Inst *inst);
    void GetLastAccDefinition(CallInst *runtimeCallInst);
    std::tuple<Inst *, Inst *> GetAccAndFrameDefs(Inst *inst);
    std::tuple<Inst *, Inst *> GetPhiAccDef();
    std::tuple<Inst *, Inst *> GetAccDefFromInputs(Inst *inst);

    Inst *GetPhiAccTagDef();
    bool IsAccTagDefInInputs(Inst *inst);
    bool IsAccTagDef(Inst *inst);
    bool IsFrameDef(Inst *inst);
    bool IsAccPtr(Inst *inst);
    bool IsAccTagPtr(Inst *inst);
    bool IsSaveAcc(const Inst *inst);
    bool CheckObjectInputs(Inst *inst);
    bool CheckStoreAcc(CallInst *runtimeCallInst);
    bool CheckStoreAccTag(CallInst *runtimeCallInst);
    bool CheckAccUsers(CallInst *runtimeCallInst);
    bool CheckObjectsUsers(CallInst *runtimeCallInst);
    bool CheckUsers(CallInst *runtimeCallInst);
    bool CheckSuccessors(BasicBlock *bb, bool prevRes);
};
}  // namespace ark::compiler

#endif
