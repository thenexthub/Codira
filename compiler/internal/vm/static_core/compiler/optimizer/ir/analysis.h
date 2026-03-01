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

#ifndef COMPILER_OPTIMIZER_IR_ANALYSIS_H
#define COMPILER_OPTIMIZER_IR_ANALYSIS_H

#include "graph.h"

#include <optional>

namespace ark::compiler {

/// The file contains small analysis functions which can be used in different passes
class Inst;
class BasicBlock;
// returns Store value, for StoreArrayPair and StoreArrayPairI saved not last store value in second_value
Inst *InstStoredValue(Inst *inst, Inst **secondValue);
Inst *InstStoredValue(Inst *inst);

template <typename T = Inst>
bool HasOsrEntryBetween(T *dominate, T *current);
bool HasTryBlockBetween(Inst *dominateInst, Inst *inst);
bool IsSuitableForImplicitNullCheck(const Inst *inst);
bool IsInstNotNull(const Inst *inst);
bool CheckFcmpInputs(Inst *input0, Inst *input1);
bool CheckFcmpWithConstInput(Inst *input0, Inst *input1);
int64_t GetPowerOfTwo(uint64_t n);
bool CanRemoveOverflowCheck(Inst *inst, Marker marker);
bool IsCastAllowedInBytecode(const Inst *inst);
bool IsInputTypeMismatch(Inst *inst, int32_t inputIndex, Arch arch);
bool ApplyForCastJoin(Inst *cast, Inst *input, Inst *origInst, Arch arch);
SaveStateInst *CopySaveState(Graph *graph, SaveStateInst *inst);
std::optional<bool> IsIfInverted(BasicBlock *phiBlock, IfImmInst *ifImm);
bool CheckTypedArrayLengthObject(Inst *inst);

// If object input has known class, return pointer to the class, else returns nullptr
RuntimeInterface::ClassPtr GetClassPtrForObject(Inst *inst, size_t inputNum = 0);
RuntimeInterface::ClassPtr GetObjectClass(Inst *inst);
RuntimeInterface::ClassPtr GetClass(Inst *inst);

inline bool IsInstInDifferentBlocks(Inst *i1, Inst *i2)
{
    return i1->GetBasicBlock() != i2->GetBasicBlock();
}

inline bool InstHasPseudoInputs(Inst *inst)
{
    return inst->GetOpcode() == Opcode::WrapObjectNative;
}

// This function bypass all blocks and delete 'SaveStateOSR' if the block is no longer the header of the loop
void CleanupGraphSaveStateOSR(Graph *graph);

class IsSaveState;
class IsSaveStateCanTriggerGc;
// returns true is there is SaveState/SafePoint between instructions
template <typename T = IsSaveState>
bool HasSaveStateBetween(Inst *domInst, Inst *inst);

bool IsSaveStateForGc(const Inst *inst);

/**
 * Functions below are using for create bridge in SaveStates between source instruction and target instruction.
 * It use in GVN etc. It inserts `source` instruction into `SaveStates` on each path between `source` and
 * `target` instructions to save the object in case GC is triggered on this path.
 * Instructions on how to use it: compiler/docs/bridges.md
 */
class SaveStateBridgesBuilder {
public:
    ArenaVector<Inst *> *SearchMissingObjInSaveStates(Graph *graph, Inst *source, Inst *target,
                                                      Inst *stopSearch = nullptr, BasicBlock *targetBlock = nullptr);
    void CreateBridgeInSS(Inst *source);
    void SearchAndCreateMissingObjInSaveState(Graph *graph, Inst *source, Inst *target, Inst *stopSearchInst = nullptr,
                                              BasicBlock *targetBlock = nullptr);
    void FixInstUsageInSS(Graph *graph, Inst *inst);
    void FixSaveStatesInBB(BasicBlock *block);
    void FixPhisWithCheckInputs(BasicBlock *block);
    void DumpBridges(std::ostream &out, Inst *source);

private:
    void SearchSSOnWay(BasicBlock *block, Inst *startFrom, Inst *sourceInst, Marker visited, Inst *stopSearch);
    bool IsSaveStateForGc(Inst *inst);
    void SearchInSaveStateAndFillBridgeVector(Inst *inst, Inst *searchedInst);
    void FixUsageInstInOtherBB(BasicBlock *block, Inst *inst);
    void FixUsagePhiInBB(BasicBlock *block, Inst *inst);
    void DeleteUnrealObjInSaveState(Inst *ss);

    template <typename C>
    void AllocOrClear(Graph *graph, C **pContainer)
    {
        if (*pContainer != nullptr) {
            (*pContainer)->clear();
            return;
        }

        auto *allocator = graph->GetAllocator();
        *pContainer = allocator->New<C>(allocator->Adapter());
    }

    /**
     * Pointer to moved out to class for reduce memory usage in each pair of equal instructions.
     * When using functions, it looks like we work as if every time get a new vector,
     * but one vector is always used and cleaned before use.
     */
    ArenaVector<Inst *> *bridges_ {nullptr};
    ArenaVector<User *> *users_ {nullptr};
};

class InstAppender {
public:
    explicit InstAppender(BasicBlock *block, Inst *insertAfter = nullptr) : block_(block), prev_(insertAfter) {}
    ~InstAppender() = default;
    DEFAULT_MOVE_SEMANTIC(InstAppender);
    NO_COPY_SEMANTIC(InstAppender);

    void Append(Inst *inst);
    void Append(std::initializer_list<Inst *> instructions);

private:
    BasicBlock *block_;
    Inst *prev_ {nullptr};
};

bool StoreValueCanBeObject(Inst *inst);

bool IsConditionEqual(const Inst *inst0, const Inst *inst1, bool inverted);

CallInst *FindCallerInst(BasicBlock *target, Inst *start = nullptr);

void PrepareUsers(Inst *inst, ArenaVector<User *> *users);
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_ANALYSIS_H
