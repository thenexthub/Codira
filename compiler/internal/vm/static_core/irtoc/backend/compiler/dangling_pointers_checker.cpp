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

#include "dangling_pointers_checker.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "runtime/interpreter/frame.h"
#include "runtime/include/managed_thread.h"

namespace ark::compiler {
DanglingPointersChecker::DanglingPointersChecker(Graph *graph)
    : Analysis {graph},
      objectsUsers_ {graph->GetLocalAllocator()->Adapter()},
      checkedBlocks_ {graph->GetLocalAllocator()->Adapter()},
      phiInsts_ {graph->GetLocalAllocator()->Adapter()},
      objectInsts_ {graph->GetLocalAllocator()->Adapter()}
{
}

bool DanglingPointersChecker::RunImpl()
{
    return CheckAccSyncCallRuntime();
}

static Inst *MoveToPrevInst(Inst *inst, BasicBlock *bb)
{
    if (inst == bb->GetFirstInst()) {
        return nullptr;
    }
    return inst->GetPrev();
}

static bool IsRefType(Inst *inst)
{
    return inst->GetType() == DataType::REFERENCE;
}

static bool IsPointerType(Inst *inst)
{
    return inst->GetType() == DataType::POINTER;
}

static bool IsObjectDef(Inst *inst)
{
    if (!IsRefType(inst) && !IsPointerType(inst)) {
        return false;
    }
    if (inst->IsPhi()) {
        return false;
    }
    if (inst->GetOpcode() != Opcode::AddI) {
        return true;
    }
    auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    return imm != static_cast<uint64_t>(cross_values::GetFrameAccOffset(inst->GetBasicBlock()->GetGraph()->GetArch()));
}

void DanglingPointersChecker::InitLiveIns()
{
    auto arch = GetGraph()->GetArch();
    for (auto inst : GetGraph()->GetStartBlock()->Insts()) {
        if (inst->GetOpcode() != Opcode::LiveIn) {
            continue;
        }
        if (inst->GetDstReg() == regmap_[arch]["acc"]) {
            accLivein_ = inst;
        }
        if (inst->GetDstReg() == regmap_[arch]["acc_tag"]) {
            accTagLivein_ = inst;
        }
        if (inst->GetDstReg() == regmap_[arch]["frame"]) {
            frameLivein_ = inst;
        }
        if (inst->GetDstReg() == regmap_[arch]["thread"]) {
            threadLivein_ = inst;
        }
    }
}

// Frame can be defined in three ways:
// 1. LiveIn(frame)
// 2. LoadI(LiveIn(thread)).Imm(ManagedThread::GetFrameOffset())
// 3. LoadI(<another_frame_def>).Imm(Frame::GetPrevFrameOffset())

bool DanglingPointersChecker::IsFrameDef(Inst *inst)
{
    //    inst := LiveIn(frame)
    if (inst == frameLivein_) {
        return true;
    }
    // or
    //    inst := LoadI(inst_input).Imm(imm)
    if (inst->GetOpcode() == Opcode::LoadI) {
        // where
        //       inst_input := LiveIn(thread)
        //       imm := ManagedThread::GetFrameOffset()
        auto instInput = inst->GetInput(0).GetInst();
        if (instInput == threadLivein_ &&
            static_cast<LoadInstI *>(inst)->GetImm() == ark::ManagedThread::GetFrameOffset()) {
            return true;
        }
        // or
        //       inst_input := <frame_def>
        //       imm := Frame::GetPrevFrameOffset()
        if (static_cast<LoadInstI *>(inst)->GetImm() == static_cast<uint64_t>(ark::Frame::GetPrevFrameOffset()) &&
            IsFrameDef(instInput)) {
            return true;
        }
    }
    return false;
}

bool DanglingPointersChecker::CheckSuccessors(BasicBlock *bb, bool prevRes)
{
    if (checkedBlocks_.find(bb) != checkedBlocks_.end()) {
        return prevRes;
    }
    for (auto succBb : bb->GetSuccsBlocks()) {
        if (!prevRes) {
            return false;
        }
        for (auto inst : succBb->AllInsts()) {
            auto user = std::find(objectsUsers_.begin(), objectsUsers_.end(), inst);
            if (user == objectsUsers_.end() || (*user)->IsPhi()) {
                continue;
            }
            return false;
        }
        checkedBlocks_.insert(bb);

        prevRes &= CheckSuccessors(succBb, prevRes);
    }

    return prevRes;
}

// Accumulator can be defined in three ways:
// 1. acc_def := LiveIn(acc)
// 2. acc_def := LoadI(last_frame_def).Imm(frame_acc_offset)
// 3. acc_ptr := AddI(last_frame_def).Imm(frame_acc_offset)
//    acc_def := LoadI(acc_ptr).Imm(0)

std::tuple<Inst *, Inst *> DanglingPointersChecker::GetAccAndFrameDefs(Inst *inst)
{
    auto arch = GetGraph()->GetArch();
    if (inst == accLivein_) {
        return std::make_pair(accLivein_, nullptr);
    }
    if (inst->GetOpcode() != Opcode::LoadI) {
        return std::make_pair(nullptr, nullptr);
    }

    auto instInput = inst->GetInput(0).GetInst();
    auto frameAccOffset = static_cast<uint64_t>(cross_values::GetFrameAccOffset(arch));
    auto loadImm = static_cast<LoadInstI *>(inst)->GetImm();
    if (loadImm == frameAccOffset && IsFrameDef(instInput)) {
        return std::make_pair(inst, instInput);
    }

    if (loadImm == 0 && IsAccPtr(instInput)) {
        return std::make_pair(inst, instInput->GetInput(0).GetInst());
    }

    return std::make_pair(nullptr, nullptr);
}

// Accumulator tag can be defined in three ways:
// 1. acc_tag_def := LiveIn(acc_tag)
// 2. acc_ptr     := AddI(last_frame_def).Imm(frame_acc_offset)
//    acc_tag_def := LoadI(acc_ptr).Imm(acc_tag_offset)
// 3. acc_ptr     := AddI(last_frame_def).Imm(frame_acc_offset)
//    acc_tag_ptr := AddI(acc_ptr).Imm(acc_tag_offset)
//    acc_tag_def := LoadI(acc_tag_ptr).Imm(0)

bool DanglingPointersChecker::IsAccTagDef(Inst *inst)
{
    if (inst == accTagLivein_) {
        return true;
    }
    if (inst->GetOpcode() != Opcode::LoadI) {
        return false;
    }

    auto instInput = inst->GetInput(0).GetInst();
    auto arch = GetGraph()->GetArch();
    auto accTagOffset = static_cast<uint64_t>(cross_values::GetFrameAccMirrorOffset(arch));
    auto loadImm = static_cast<LoadInstI *>(inst)->GetImm();
    if (loadImm == accTagOffset && IsAccPtr(instInput)) {
        return true;
    }

    if (loadImm == 0 && IsAccTagPtr(instInput)) {
        return true;
    }

    return false;
}

bool DanglingPointersChecker::IsAccTagPtr(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::AddI) {
        return false;
    }
    auto arch = GetGraph()->GetArch();
    auto instImm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    auto accTagOffset = static_cast<uint64_t>(cross_values::GetFrameAccMirrorOffset(arch));
    if (instImm != accTagOffset) {
        return false;
    }
    auto accPtrInst = inst->GetInput(0).GetInst();
    return IsAccPtr(accPtrInst);
}

bool DanglingPointersChecker::IsAccPtr(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::AddI) {
        return false;
    }
    auto arch = GetGraph()->GetArch();
    auto instImm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    auto frameAccOffset = static_cast<uint64_t>(cross_values::GetFrameAccOffset(arch));
    if (instImm != frameAccOffset) {
        return false;
    }
    auto frameInst = inst->GetInput(0).GetInst();
    if (!IsFrameDef(frameInst)) {
        return false;
    }
    if (lastFrameDef_ == nullptr) {
        return true;
    }
    return lastFrameDef_ == frameInst;
}

void DanglingPointersChecker::UpdateLastAccAndFrameDef(Inst *inst)
{
    auto [acc_def, frame_def] = GetAccAndFrameDefs(inst);
    if (acc_def != nullptr) {
        // inst is acc definition
        if (lastAccDef_ == nullptr) {
            // don't have acc definition before
            lastAccDef_ = acc_def;
            lastFrameDef_ = frame_def;
        }
    } else {
        // inst isn't acc definition
        if (IsObjectDef(inst) && !IsPointerType(inst)) {
            // objects defs should be only ref type
            objectInsts_.insert(inst);
        }
    }
}

void DanglingPointersChecker::GetLastAccDefinition(CallInst *runtimeCallInst)
{
    auto block = runtimeCallInst->GetBasicBlock();
    auto prevInst = runtimeCallInst->GetPrev();

    phiInsts_.clear();
    objectInsts_.clear();
    while (block != GetGraph()->GetStartBlock()) {
        while (prevInst != nullptr) {
            UpdateLastAccAndFrameDef(prevInst);

            if (lastAccTagDef_ == nullptr && IsAccTagDef(prevInst)) {
                lastAccTagDef_ = prevInst;
            }

            prevInst = MoveToPrevInst(prevInst, block);
        }

        objectInsts_.insert(accLivein_);

        for (auto *phiInst : block->PhiInsts()) {
            phiInsts_.push_back(phiInst);
        }
        block = block->GetDominator();
        prevInst = block->GetLastInst();
    }

    // Check that accumulator has not been overwritten in any execution branch except restored acc
    auto [phi_def_acc, phi_def_frame] = GetPhiAccDef();
    if (phi_def_acc != nullptr) {
        lastAccDef_ = phi_def_acc;
        lastFrameDef_ = phi_def_frame;
    }
    if (lastAccTagDef_ == nullptr) {
        lastAccTagDef_ = GetPhiAccTagDef();
    }

    if (lastAccDef_ == nullptr) {
        lastAccDef_ = accLivein_;
    }

    if (lastAccTagDef_ == nullptr) {
        lastAccTagDef_ = accTagLivein_;
    }
}

std::tuple<Inst *, Inst *> DanglingPointersChecker::GetPhiAccDef()
{
    // If any input isn't a definition (or there are no definitions among its inputs),
    // then the phi is not a definition.
    // Otherwise, if we have reached the last input and it is a definition (or there is a definition in among its
    // inputs), then the phi is a definition.
    for (auto *phiInst : phiInsts_) {
        bool isAccDefPhi = true;
        auto inputsCount = phiInst->GetInputsCount();
        Inst *accDef {nullptr};
        Inst *frameDef {nullptr};
        for (uint32_t inputIdx = 0; inputIdx < inputsCount; inputIdx++) {
            auto inputInst = phiInst->GetInput(inputIdx).GetInst();
            std::tie(accDef, frameDef) = GetAccAndFrameDefs(inputInst);
            if (accDef != nullptr || inputInst == nullptr) {
                continue;
            }
            if (inputInst->IsConst() ||
                (inputInst->GetOpcode() == Opcode::Bitcast && inputInst->GetInput(0).GetInst()->IsConst())) {
                accDef = inputInst;
                continue;
            }
            std::tie(accDef, frameDef) = GetAccDefFromInputs(inputInst);
            if (accDef == nullptr) {
                isAccDefPhi = false;
                break;
            }
        }
        if (!isAccDefPhi) {
            continue;
        }
        if (accDef != nullptr) {
            return std::make_pair(phiInst, frameDef);
        }
    }
    return std::make_pair(nullptr, nullptr);
}

std::tuple<Inst *, Inst *> DanglingPointersChecker::GetAccDefFromInputs(Inst *inst)
{
    auto inputsCount = inst->GetInputsCount();
    Inst *accDef {nullptr};
    Inst *frameDef {nullptr};
    for (uint32_t inputIdx = 0; inputIdx < inputsCount; inputIdx++) {
        auto inputInst = inst->GetInput(inputIdx).GetInst();

        std::tie(accDef, frameDef) = GetAccAndFrameDefs(inputInst);
        if (accDef != nullptr || inputInst == nullptr) {
            continue;
        }
        if (inputInst->IsConst() ||
            (inputInst->GetOpcode() == Opcode::Bitcast && inputInst->GetInput(0).GetInst()->IsConst())) {
            accDef = inputInst;
            continue;
        }
        std::tie(accDef, frameDef) = GetAccDefFromInputs(inputInst);
        if (accDef == nullptr) {
            return std::make_pair(nullptr, nullptr);
        }
    }
    return std::make_pair(accDef, frameDef);
}

Inst *DanglingPointersChecker::GetPhiAccTagDef()
{
    for (auto *phiInst : phiInsts_) {
        if (IsRefType(phiInst) || IsPointerType(phiInst)) {
            continue;
        }
        auto inputsCount = phiInst->GetInputsCount();
        for (uint32_t inputIdx = 0; inputIdx < inputsCount; inputIdx++) {
            auto inputInst = phiInst->GetInput(inputIdx).GetInst();
            auto isAccTagDef = IsAccTagDef(inputInst);
            if ((isAccTagDef || inputInst->IsConst()) && (inputIdx == inputsCount - 1)) {
                return phiInst;
            }

            if (isAccTagDef || inputInst->IsConst()) {
                continue;
            }

            if (!IsAccTagDefInInputs(inputInst)) {
                break;
            }
            return phiInst;
        }
    }
    return nullptr;
}

bool DanglingPointersChecker::IsAccTagDefInInputs(Inst *inst)
{
    auto inputsCount = inst->GetInputsCount();
    for (uint32_t inputIdx = 0; inputIdx < inputsCount; inputIdx++) {
        auto inputInst = inst->GetInput(inputIdx).GetInst();
        if (IsAccTagDef(inputInst)) {
            return true;
        }

        if ((inputIdx == inputsCount - 1) && inputInst->IsConst()) {
            return true;
        }

        if (IsAccTagDefInInputs(inputInst)) {
            return true;
        }
    }
    return false;
}

bool DanglingPointersChecker::IsSaveAcc(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::StoreI) {
        return false;
    }

    auto arch = GetGraph()->GetArch();
    auto frameAccOffset = static_cast<uint64_t>(cross_values::GetFrameAccOffset(arch));
    if (static_cast<const StoreInstI *>(inst)->GetImm() != frameAccOffset) {
        return false;
    }
    auto storeInput1 = inst->GetInput(1).GetInst();
    if (storeInput1 != lastAccDef_) {
        return false;
    }
    auto storeInput0 = inst->GetInput(0).GetInst();
    if (lastFrameDef_ == nullptr) {
        if (IsFrameDef(storeInput0)) {
            return true;
        }
    } else if (storeInput0 == lastFrameDef_) {
        return true;
    }
    return false;
}

// Accumulator is saved using the StoreI instruction:
// StoreI(last_frame_def, last_acc_def).Imm(cross_values::GetFrameAccOffset(GetArch()))

bool DanglingPointersChecker::CheckStoreAcc(CallInst *runtimeCallInst)
{
    auto prevInst = runtimeCallInst->GetPrev();
    auto block = runtimeCallInst->GetBasicBlock();
    while (block != GetGraph()->GetStartBlock()) {
        while (prevInst != nullptr && prevInst != lastAccDef_) {
            if (IsSaveAcc(prevInst)) {
                return true;
            }
            prevInst = MoveToPrevInst(prevInst, block);
        }
        block = block->GetDominator();
        prevInst = block->GetLastInst();
    }
    return false;
}

// Accumulator tag is saved using the StoreI instruction:
// StoreI(acc_ptr, last_acc_tag_def).Imm(cross_values::GetFrameAccMirrorOffset(GetArch()))

bool DanglingPointersChecker::CheckStoreAccTag(CallInst *runtimeCallInst)
{
    bool isSaveAccTag = false;
    auto arch = GetGraph()->GetArch();
    auto prevInst = runtimeCallInst->GetPrev();
    auto block = runtimeCallInst->GetBasicBlock();
    auto accTagOffset = static_cast<uint64_t>(cross_values::GetFrameAccMirrorOffset(arch));
    while (block != GetGraph()->GetStartBlock()) {
        while (prevInst != nullptr && prevInst != lastAccDef_) {
            if (prevInst->GetOpcode() != Opcode::StoreI) {
                prevInst = MoveToPrevInst(prevInst, block);
                continue;
            }
            if (static_cast<StoreInstI *>(prevInst)->GetImm() != accTagOffset) {
                prevInst = MoveToPrevInst(prevInst, block);
                continue;
            }
            auto storeInput1 = prevInst->GetInput(1).GetInst();
            if (lastAccTagDef_ == nullptr) {
                lastAccTagDef_ = storeInput1;
            }
            if (storeInput1 != lastAccTagDef_ && !storeInput1->IsConst()) {
                prevInst = MoveToPrevInst(prevInst, block);
                continue;
            }
            auto storeInput0 = prevInst->GetInput(0).GetInst();
            if (IsAccPtr(storeInput0)) {
                isSaveAccTag = true;
                break;
            }

            prevInst = MoveToPrevInst(prevInst, block);
        }
        if (isSaveAccTag) {
            break;
        }
        block = block->GetDominator();
        prevInst = block->GetLastInst();
    }
    return isSaveAccTag;
}

bool DanglingPointersChecker::CheckAccUsers(CallInst *runtimeCallInst)
{
    objectsUsers_.clear();
    for (const auto &user : lastAccDef_->GetUsers()) {
        objectsUsers_.push_back(user.GetInst());
    }

    return CheckUsers(runtimeCallInst);
}

bool DanglingPointersChecker::CheckObjectsUsers(CallInst *runtimeCallInst)
{
    objectsUsers_.clear();
    for (auto *objectInst : objectInsts_) {
        for (const auto &user : objectInst->GetUsers()) {
            objectsUsers_.push_back(user.GetInst());
        }
    }

    return CheckUsers(runtimeCallInst);
}

bool DanglingPointersChecker::CheckUsers(CallInst *runtimeCallInst)
{
    bool checkObjectUsers = true;
    auto runtimeCallBlock = runtimeCallInst->GetBasicBlock();

    auto nextInst = runtimeCallInst->GetNext();
    while (nextInst != nullptr) {
        auto user = std::find(objectsUsers_.begin(), objectsUsers_.end(), nextInst);
        if (user == objectsUsers_.end() || (*user)->IsPhi()) {
            nextInst = nextInst->GetNext();
            continue;
        }
        return false;
    }

    checkedBlocks_.clear();
    return CheckSuccessors(runtimeCallBlock, checkObjectUsers);
}

bool DanglingPointersChecker::CheckAccSyncCallRuntime()
{
    if (regmap_.find(GetGraph()->GetArch()) == regmap_.end()) {
        return true;
    }

    if (GetGraph()->GetRelocationHandler() == nullptr) {
        return true;
    }

    // collect runtime calls
    ArenaVector<CallInst *> runtimeCalls(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (!inst->IsCall()) {
                continue;
            }
            auto callInst = static_cast<CallInst *>(inst);
            auto callFuncName =
                GetGraph()->GetRuntime()->GetExternalMethodName(GetGraph()->GetMethod(), callInst->GetCallMethodId());
            if (targetFuncs_.find(callFuncName) == targetFuncs_.end()) {
                continue;
            }
            runtimeCalls.push_back(callInst);
        }
    }
    if (runtimeCalls.empty()) {
        return true;
    }

    // find LiveIns for acc and frame
    InitLiveIns();

    for (auto runtimeCallInst : runtimeCalls) {
        lastAccDef_ = nullptr;
        lastAccTagDef_ = nullptr;
        GetLastAccDefinition(runtimeCallInst);

        if (!IsRefType(lastAccDef_) && !IsPointerType(lastAccDef_)) {
            continue;
        }

        // check that acc has been stored in the frame before call
        if (!CheckStoreAcc(runtimeCallInst)) {
            return false;
        }

        if (!GetGraph()->IsDynamicMethod() && !CheckStoreAccTag(runtimeCallInst)) {
            return false;
        }

        // check that acc isn't used after call
        if (!CheckAccUsers(runtimeCallInst)) {
            return false;
        }

        // check that other objects aren't used after call
        if (!CheckObjectsUsers(runtimeCallInst)) {
            return false;
        }
    }
    return true;
}
}  // namespace ark::compiler
