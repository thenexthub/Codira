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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_REG_ALLOC_VERIFIER_H
#define COMPILER_OPTIMIZER_ANALYSIS_REG_ALLOC_VERIFIER_H

#include "optimizer/pass.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/basicblock.h"

namespace ark::compiler {
class BlockState;
class LocationState;
/*
 Analysis aimed to check correctness of registers and stack slots assignment.
 To verify correctness the analysis stores a state for each possible location
 (GP register, FP register, stack slot or immediate table slot for spilled constants).
 The state represents knowledge about a value stored in a location: it could be unknown,
 known or invalid (conflicting). The "known" state is augmented with instruction id
 whose value the location is holding.
 During verification process verifier iterates through all basic blocks and for each
 instruction within basic block performs following actions:
 1) checks that each instruction's input register holds known value and corresponding
    instruction id (from the input location) is the same as the input's id;
 2) updates instruction's output location with its id (if an instruction has destination).
 If any input location contains unknown or conflicting value then verification fails.

 When block analysis is complete the state at the block's end have to be propagated to
 successor blocks. Propagation step perform following actions:
 1) process all successor's locations corresponding to phi instructions by checking that
    phi's location at the end of predecessor block holds result of the exact instruction
    that will be selected by the phi when control flows from the predecessor to successor.
    If this condition holds then phi's id will be written to successor's location.
 2) merge all locations (that were not updated at (1)) at the end of predecessor block
    to the same locations at the beginning of successor block.
 3) if any location was updated during two previous steps then successor block is marked
    as updated and should be verified.

 Merge action (2) updates destination (successor block's) location with data from
 source (predecessor block's) location using following rules:
 1) if source or destination has conflicting value then destination will have conflicting value;
 2) if both values are unknown then destination value will remain unknown;
 3) if either destination or source value is known (but not both values simultaneously) then
    destination will either remain known or will be updated to known value;
 4) if both source and destination values are known and holds result of the same instruction
    then the destination remain known;
 5) if both source and destination values are known but holds results of different instructions
    then destination will be updated to conflicting value.

 Verification process continues until there are no more blocks updated during value propagation.
 If there are no such blocks and there were no errors then verification passes.
 */
class RegAllocVerifier : public Analysis {
public:
    explicit RegAllocVerifier(Graph *graph, bool saveLiveRegsOnCall = true);
    NO_COPY_SEMANTIC(RegAllocVerifier);
    NO_MOVE_SEMANTIC(RegAllocVerifier);
    ~RegAllocVerifier() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "RegAllocVerifier";
    }

private:
    BasicBlock *currentBlock_ {nullptr};
    BlockState *currentState_ {nullptr};
    ArenaVector<LocationState> immediates_;
    // required to support Save/RestoreRegisters intrinsics
    ArenaVector<LocationState> savedRegs_;
    ArenaVector<LocationState> savedVregs_;
    bool success_ {true};
    Marker implicitNullCheckHandledMarker_ {};
    bool saveLiveRegs_ {false};

    void InitImmediates();
    bool IsZeroReg(Register reg, DataType::Type type) const;
    void HandleDest(Inst *inst);
    LocationState &GetLocationState(Location location);
    void UpdateLocation(Location location, DataType::Type type, uint32_t instId);
    template <typename T>
    bool ForEachLocation(Location location, DataType::Type type, T callback);
    void ProcessCurrentBlock();
    void HandleParameter(ParameterInst *inst);
    void HandleSpillFill(SpillFillInst *inst);
    void HandleConst(ConstantInst *inst);
    void HandleInst(Inst *inst);
#ifdef PANDA_WITH_IRTOC
    bool IsSaveRestoreRegisters(Inst *inst);
    void HandleSaveRestoreRegisters(Inst *inst);
#endif
    void TryHandleImplicitNullCheck(Inst *inst);
    void RestoreLiveRegisters(Inst *inst);
};

class LocationState {
public:
    enum class State : uint8_t { UNKNOWN, KNOWN, CONFLICT };
    static auto constexpr ZERO_INST = INVALID_ID;

    LocationState() = default;
    LocationState(LocationState::State state, uint32_t id) : state_(state), id_(id) {}
    ~LocationState() = default;
    DEFAULT_COPY_SEMANTIC(LocationState);
    DEFAULT_MOVE_SEMANTIC(LocationState);

    State GetState() const
    {
        return state_;
    }

    void SetState(State state)
    {
        state_ = state;
    }

    void SetId(uint32_t id)
    {
        state_ = State::KNOWN;
        id_ = id;
    }

    uint32_t GetId() const
    {
        return id_;
    }

    bool Merge(const LocationState &other);

    bool ShouldSkip() const
    {
        return skip_;
    }

    void SetSkip(bool skip)
    {
        skip_ = skip;
    }

    bool IsValid(const Inst *inst) const
    {
        return GetId() == inst->GetId() || (GetId() == ZERO_INST && inst->IsZeroRegInst());
    }

private:
    State state_ {UNKNOWN};
    uint32_t id_ {INVALID_ID};
    bool skip_ {false};
};

class BlockState {
public:
    BlockState(size_t regs, size_t vregs, size_t stackSlots, size_t stackParams, ArenaAllocator *alloc);
    ~BlockState() = default;
    NO_COPY_SEMANTIC(BlockState);
    NO_MOVE_SEMANTIC(BlockState);
    LocationState &GetReg(Register reg)
    {
        ASSERT(reg < regs_.size());
        return regs_[reg];
    }
    const LocationState &GetReg(Register reg) const
    {
        ASSERT(reg < regs_.size());
        return regs_[reg];
    }
    LocationState &GetVReg(Register reg)
    {
        ASSERT(reg < vregs_.size());
        return vregs_[reg];
    }
    const LocationState &GetVReg(Register reg) const
    {
        ASSERT(reg < vregs_.size());
        return vregs_[reg];
    }
    const LocationState &GetStack(StackSlot slot) const
    {
        ASSERT(slot < stack_.size());
        return stack_[slot];
    }
    LocationState &GetStack(StackSlot slot)
    {
        ASSERT(slot < stack_.size());
        return stack_[slot];
    }
    const LocationState &GetStackArg(StackSlot slot) const
    {
        ASSERT(slot < stackArg_.size());
        return stackArg_[slot];
    }
    LocationState &GetStackArg(StackSlot slot)
    {
        if (slot >= stackArg_.size()) {
            stackArg_.resize(slot + 1);
        }
        return stackArg_[slot];
    }
    const LocationState &GetStackParam(StackSlot slot) const
    {
        ASSERT(slot < stackParam_.size());
        return stackParam_[slot];
    }
    LocationState &GetStackParam(StackSlot slot)
    {
        if (slot >= stackParam_.size()) {
            stackParam_.resize(slot + 1);
        }
        return stackParam_[slot];
    }

    bool Merge(const BlockState &state, const PhiInstSafeIter &phis, BasicBlock *pred,
               const ArenaVector<LocationState> &immediates, const LivenessAnalyzer &la);
    void Copy(BlockState *state);

private:
    ArenaVector<LocationState> regs_;
    ArenaVector<LocationState> vregs_;
    ArenaVector<LocationState> stack_;
    ArenaVector<LocationState> stackParam_;
    ArenaVector<LocationState> stackArg_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_REG_ALLOC_VERIFIER_H
