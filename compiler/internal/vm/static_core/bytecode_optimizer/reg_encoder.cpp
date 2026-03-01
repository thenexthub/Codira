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

#include "reg_encoder.h"
#include "common.h"
#include "compiler/optimizer/ir/basicblock.h"
#if defined(ENABLE_LIBABCKIT)
#include "generated/abckit_intrinsics_vreg_width.h"
#else
static bool IsDstRegNeedRenumbering([[maybe_unused]] ark::compiler::Inst *inst)
{
    UNREACHABLE();
}
static void CheckWidthAbcKitIntrinsic([[maybe_unused]] ark::bytecodeopt::RegEncoder *re,
                                      [[maybe_unused]] ark::compiler::Inst *inst)
{
    UNREACHABLE();
}
#endif

namespace ark::bytecodeopt {

static bool IsIntrinsicRange(Inst *inst)
{
    if (inst->GetOpcode() != compiler::Opcode::Intrinsic) {
        return false;
    }
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit()) {
        return IsAbcKitIntrinsicRange(inst->CastToIntrinsic()->GetIntrinsicId());
    }
#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
#ifdef ARK_INTRINSIC_SET
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_CALLI_RANGE_DYN:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_CALLI_THIS_RANGE_DYN:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_NEWOBJ_DYNRANGE:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_SUPER_CALL:
#else
        case compiler::RuntimeInterface::IntrinsicId::ECMA_CALLIRANGEDYN_PREF_IMM16_V8:
        case compiler::RuntimeInterface::IntrinsicId::ECMA_CALLITHISRANGEDYN_PREF_IMM16_V8:
        case compiler::RuntimeInterface::IntrinsicId::ECMA_NEWOBJDYNRANGE_PREF_IMM16_V8:
        case compiler::RuntimeInterface::IntrinsicId::ECMA_SUPERCALL_PREF_IMM16_V8:
        case compiler::RuntimeInterface::IntrinsicId::ECMA_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8:
#endif
            return true;
        default:
            return false;
    }
#endif
    return false;
}

static bool CanHoldRange(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case compiler::Opcode::CallStatic:
        case compiler::Opcode::CallVirtual:
        case compiler::Opcode::InitObject:
        case compiler::Opcode::Intrinsic:
            return true;
        default:
            return false;
    }
}

static compiler::Register CalculateNumNeededRangeTemps(const compiler::Graph *graph)
{
    compiler::Register ret = 0;

    for (auto bb : graph->GetBlocksRPO()) {
        for (const auto &inst : bb->AllInsts()) {
            if (!CanHoldRange(inst)) {
                continue;
            }
            auto nargs = inst->GetInputsCount() - (inst->RequireState() ? 1U : 0U);
            if (inst->GetOpcode() == compiler::Opcode::InitObject) {
                ASSERT(nargs > 0U);
                nargs -= 1;  // exclude LoadAndInitClass
            }
            if (ret < nargs && (nargs > MAX_NUM_NON_RANGE_ARGS || IsIntrinsicRange(inst))) {
                ret = nargs;
            }
        }
    }

    return ret;
}

static compiler::Register RegsDiff(compiler::Register r1, compiler::Register r2)
{
    if (r1 >= r2) {
        return r1 - r2;
    }
    if (compiler::IsFrameSizeLarge()) {
        return compiler::GetInvalidReg() + 1 - (r2 - r1);
    }
    return static_cast<uint8_t>(r1 - r2);
}

static compiler::Register RegsSum(compiler::Register r1, compiler::Register r2)
{
    auto sum = static_cast<size_t>(r1 + r2);
    if (sum <= compiler::GetInvalidReg()) {
        return sum;
    }
    if (compiler::IsFrameSizeLarge()) {
        return sum - (compiler::GetInvalidReg() + 1);
    }
    return static_cast<uint8_t>(r1 + r2);
}

bool RegEncoder::RunImpl()
{
    ASSERT(state_ == RegEncoderState::IDLE);

    numMaxRangeInput_ = CalculateNumNeededRangeTemps(GetGraph());

    state_ = RegEncoderState::RENUMBER_ARGS;
    if (!RenumberArgRegs()) {
        return false;
    }

    state_ = RegEncoderState::RESERVE_TEMPS;
    ASSERT(numTemps_ == 0U);

    const auto numRegs = GetNumRegs();

    auto maxNumTemps = numTemps_;
    CalculateNumNeededTemps();
    if (!CheckStatus()) {
        return false;
    }

    while (maxNumTemps != numTemps_) {
        ASSERT(numTemps_ > maxNumTemps);

        if (numRegs > compiler::GetFrameSize() - numTemps_) {  // to avoid overflow
            return false;                                      // no more free registers left in the frame
        }

        auto delta = RegsDiff(numTemps_, maxNumTemps);
        rangeTempsStart_ = RegsSum(rangeTempsStart_, delta);
        RenumberRegs(MIN_REGISTER_NUMBER, delta);

        maxNumTemps = numTemps_;
        CalculateNumNeededTemps();
        if (!CheckStatus()) {
            return false;
        }
        if (numTemps_ == 0U) {
            break;
        }
    }

    numTemps_ = numTemps_ > 0U ? numTemps_ : numChangedWidth_;
    if (numTemps_ > 0U || numMaxRangeInput_ > 0U) {
        state_ = RegEncoderState::INSERT_SPILLS;
        InsertSpills();
        if (!CheckStatus()) {
            return false;
        }

        auto usageMask = GetGraph()->GetUsedRegs<compiler::DataType::INT64>();
        for (compiler::Register r = 0; r < numRegs; r++) {
            usageMask->at(numRegs + numTemps_ - r - 1L) = usageMask->at(numRegs - r - 1L);
        }
        std::fill(usageMask->begin(), usageMask->begin() + numTemps_, true);
    }

    SaveNumLocalsToGraph(GetNumLocalsFromGraph() + numTemps_);
    state_ = RegEncoderState::IDLE;

    return true;
}

static ark::compiler::DataType::Type GetRegType(ark::compiler::DataType::Type type)
{
    if (type == ark::compiler::DataType::REFERENCE) {
        return type;
    }
    if (ark::compiler::DataType::Is32Bits(type, Arch::NONE)) {
        return ark::compiler::DataType::UINT32;
    }
    return ark::compiler::DataType::UINT64;
}

static bool RegNeedsRenumbering(ark::compiler::Register r)
{
    return r != ark::compiler::GetAccReg() && r != ark::compiler::GetInvalidReg();
}

static ark::compiler::Register RenumberReg(const ark::compiler::Register r, const ark::compiler::Register delta)
{
    if (r == ark::compiler::GetAccReg()) {
        return r;
    }
    return RegsSum(r, delta);
}

static void RenumberSpillFillRegs(ark::compiler::SpillFillInst *inst, const ark::compiler::Register minReg,
                                  const ark::compiler::Register delta)
{
    for (auto &sf : inst->GetSpillFills()) {
        if (sf.SrcType() == compiler::LocationType::REGISTER && sf.SrcValue() >= minReg) {
            sf.SetSrc(compiler::Location::MakeRegister(RenumberReg(sf.SrcValue(), delta)));
        }
        if (sf.DstType() == compiler::LocationType::REGISTER && sf.DstValue() >= minReg) {
            sf.SetDst(compiler::Location::MakeRegister(RenumberReg(sf.DstValue(), delta)));
        }
    }
}

static void RenumberRegsForInst(compiler::Inst *inst, const compiler::Register minReg, const compiler::Register delta)
{
    // Renumber output of any instruction, if applicable:
    if (RegNeedsRenumbering(inst->GetDstReg()) && inst->GetDstReg() >= minReg) {
        inst->SetDstReg(RenumberReg(inst->GetDstReg(), delta));
    }

    if (inst->IsPhi() || inst->IsCatchPhi()) {
        return;
    }

    // Renumber inputs and outputs of SpillFill instructions:
    if (inst->IsSpillFill()) {
        RenumberSpillFillRegs(inst->CastToSpillFill(), minReg, delta);
        return;
    }

    // Fix inputs of common instructions:
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        if (RegNeedsRenumbering(inst->GetSrcReg(i)) && inst->GetSrcReg(i) >= minReg) {
            inst->SetSrcReg(i, RenumberReg(inst->GetSrcReg(i), delta));
        }
    }
}

void RegEncoder::RenumberRegs(const compiler::Register minReg, const compiler::Register delta)
{
    // Renumbering always advances register number `delta` positions forward,
    // wrapping around on overflows with well-defined behavour.
    // Hence the requirement to keep delta unsigned.
    static_assert(std::is_unsigned<compiler::Register>::value, "compiler::Register must be unsigned");
    ASSERT(delta > 0U);

    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            RenumberRegsForInst(inst, minReg, delta);
        }
    }
}

static compiler::Register CalculateNumLocals(const ArenaVector<bool> *usageMask, compiler::Register numNonArgs)
{
    compiler::Register numLocals = 0;
    if (numNonArgs != 0U) {
        while (numLocals != numNonArgs && usageMask->at(numLocals)) {
            ++numLocals;
        }
    }
    return numLocals;
}

static compiler::Register CalculateNumTemps(const ArenaVector<bool> *usageMask, compiler::Register numNonArgs,
                                            compiler::Register numLocals)
{
    compiler::Register numTemps = 0;
    if (numLocals != numNonArgs) {
        compiler::Register r = numNonArgs - 1L;
        while (r < numNonArgs && usageMask->at(r)) {
            ++numTemps;
            --r;
        }
    }
    return numTemps;
}

bool RegEncoder::RenumberArgRegs()
{
    const auto usageMask = GetGraph()->GetUsedRegs<compiler::DataType::INT64>();
    ASSERT(usageMask->size() == compiler::GetFrameSize());

    auto frameSize = static_cast<compiler::Register>(usageMask->size());
    const auto numArgs = GetNumArgsFromGraph();
    ASSERT(frameSize >= numArgs);

    auto numNonArgs = static_cast<compiler::Register>(frameSize - numArgs);
    if (numMaxRangeInput_ > numNonArgs) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "RegEncoder: The free regs for range call are not enough";
        return false;
    }

    auto numLocals = CalculateNumLocals(usageMask, numNonArgs);
    auto numTemps = CalculateNumTemps(usageMask, numNonArgs, numLocals);
    if (numLocals + numTemps > numNonArgs - numMaxRangeInput_) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "RegEncoder: The free regs for range call are not enough";
        return false;
    }

    rangeTempsStart_ = numLocals;
    SaveNumLocalsToGraph(numLocals + numTemps + numMaxRangeInput_);

    if (numNonArgs == 0U && numMaxRangeInput_ == 0U) {  // all registers are arguments: no need to renumber
        return true;
    }

    // All free regs will be just enough to encode call.range: no need to renumber
    if (numLocals + numTemps + numMaxRangeInput_ == numNonArgs) {
        return true;
    }

    if (numTemps + numArgs == 0U) {  // no temps and no args: nothing to renumber
        return true;
    }

    const auto minReg = RegsDiff(numNonArgs, numTemps);
    ASSERT(minReg > MIN_REGISTER_NUMBER);

    // Assert that if temps are present, they are marked allocated in the mask:
    for (compiler::Register r = minReg; r < minReg + numTemps; r++) {
        ASSERT(usageMask->at(r));
    }

    // Assert that there are no used regs between locals and temps + arguments:
    for (compiler::Register r = numLocals; r < minReg; r++) {
        ASSERT(!usageMask->at(r));
    }

    auto delta = RegsDiff(numLocals + numTemps + numMaxRangeInput_, numNonArgs);
    RenumberRegs(minReg, delta);

    for (compiler::Register r = minReg; r < frameSize; r++) {
        usageMask->at(RenumberReg(r, delta)) = usageMask->at(r);
        usageMask->at(r) = false;
    }
    return true;
}

void RegEncoder::InsertSpills()
{
    ASSERT(numMaxRangeInput_ > 0U || (numTemps_ > 0U && numTemps_ <= MAX_NUM_INPUTS));

    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInstsSafe()) {
            if (inst->GetInputsCount() == 0U) {
                continue;
            }

            VisitInstruction(inst);
            if (!CheckStatus()) {
                return;
            }
        }
    }
}

void RegEncoder::CalculateNumNeededTemps()
{
    numTemps_ = 0;

    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInstsSafe()) {
            if (inst->GetInputsCount() == 0U) {
                continue;
            }

            VisitInstruction(inst);
            if (!CheckStatus()) {
                return;
            }
        }
    }

    LOG(DEBUG, BYTECODE_OPTIMIZER) << GetGraph()->GetRuntime()->GetMethodFullName(GetGraph()->GetMethod())
                                   << ": num_temps_ = " << std::to_string(numTemps_);
}

template <typename T>
static void AddMoveBefore(Inst *inst, const T &spContainer)
{
    if (spContainer.empty()) {
        return;
    }
    auto sfInst = inst->GetBasicBlock()->GetGraph()->CreateInstSpillFill();
    for (auto const &[src, dst] : spContainer) {
        ASSERT(src != compiler::GetAccReg());
        sfInst->AddMove(src, dst.reg, GetRegType(dst.type));
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "RegEncoder: Move v" << static_cast<int>(dst.reg) << " <- v"
                                       << static_cast<int>(src) << " was added";
    }
    inst->GetBasicBlock()->InsertBefore(sfInst, inst);
}

static bool IsAccReadPosition(compiler::Inst *inst, size_t pos)
{
    // Calls can have accumulator at any position, return false for them
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit() && inst->IsIntrinsic()) {
        return inst->IsAccRead() && pos == AccReadIndex(inst);
    }
    return !inst->IsCallOrIntrinsic() && inst->IsAccRead() && pos == AccReadIndex(inst);
}

static void AddMoveAfter(Inst *inst, compiler::Register src, RegContent dst)
{
    auto *sfInst = inst->GetBasicBlock()->GetGraph()->CreateInstSpillFill();
    sfInst->AddMove(src, dst.reg, dst.type);
    LOG(DEBUG, BYTECODE_OPTIMIZER) << "RegEncoder: Move v" << static_cast<int>(dst.reg) << " <- v"
                                   << static_cast<int>(src) << " was added";
    inst->GetBasicBlock()->InsertAfter(sfInst, inst);
}

static void AddMoveBefore(Inst *inst, compiler::Register src, RegContent dst)
{
    auto *sfInst = inst->GetBasicBlock()->GetGraph()->CreateInstSpillFill();
    sfInst->AddMove(src, dst.reg, dst.type);
    LOG(DEBUG, BYTECODE_OPTIMIZER) << "RegEncoder: Move v" << static_cast<int>(dst.reg) << " <- v"
                                   << static_cast<int>(src) << " was added";
    inst->GetBasicBlock()->InsertBefore(sfInst, inst);
}

void RegEncoder::RenumberDstReg(compiler::Inst *inst, size_t temp, size_t rangeTemp, bool largeTemp)
{
    if (!GetGraph()->IsAbcKit() || !IsDstRegNeedRenumbering(inst)) {
        return;
    }
    auto reg = inst->GetDstReg();
    if (!RegNeedsRenumbering(reg) || reg < NUM_COMPACTLY_ENCODED_REGS) {
        return;
    }
    auto type = GetRegType(inst->GetType());
    inst->SetDstReg(temp);
    AddMoveAfter(inst, temp, RegContent(reg, type));
    if (largeTemp) {
        AddMoveBefore(inst, temp, RegContent(rangeTemp, type));
    }
}

bool RegEncoder::InsertSpillsForDynRangeInst(compiler::Inst *inst, size_t nargs, size_t start)
{
    RegContentVec spillVec(GetGraph()->GetLocalAllocator()->Adapter());  // spill_vec is used to handle callrange
    compiler::Register temp = rangeTempsStart_;
    compiler::Register rangeTemp = rangeTempsStart_;
    bool largeTemp = false;
    if (temp > compiler::INVALID_REG) {
        ASSERT(GetGraph()->IsAbcKit());
        ASSERT(compiler::IsFrameSizeLarge());
        temp = 0;
        largeTemp = true;
    }

    for (size_t i = start; i < nargs; ++i) {
        auto srcReg = inst->GetSrcReg(i);
        auto type = inst->GetInputType(i);
        // do not spillfill for acc-read position. For example, Intrinsic.FSTARR32
        if (IsAccReadPosition(inst, i)) {
            continue;
        }
        if (largeTemp) {
            spillVec.emplace_back(temp, RegContent(rangeTemp, type));
            AddMoveAfter(inst, rangeTemp, RegContent(temp, type));
        }
        spillVec.emplace_back(srcReg, RegContent(temp, type));
        if (GetGraph()->IsAbcKit() && ((temp >= compiler::GetFrameSize()) || (rangeTemp >= compiler::GetFrameSize()))) {
            success_ = false;
            return success_;
        }
        inst->SetSrcReg(i, temp);

        temp++;
        rangeTemp++;
    }

    AddMoveBefore(inst, spillVec);

    RenumberDstReg(inst, temp, rangeTemp, largeTemp);

    return success_;
}

void RegEncoder::InsertSpillsForDynInputsInst(compiler::Inst *inst)
{
    ASSERT(state_ == RegEncoderState::INSERT_SPILLS);
    ASSERT(inst->IsStaticCall() || inst->IsVirtualCall() || inst->IsInitObject() || inst->IsIntrinsic());

    RegContentMap spillMap(GetGraph()->GetLocalAllocator()->Adapter());  // src -> (dst, src_type), non-callrange

    auto nargs = inst->GetInputsCount() - (inst->RequireState() ? 1U : 0U);
    auto start = GetStartInputIndex(inst);
    bool range = IsIntrinsicRange(inst) || (nargs - start > MAX_NUM_NON_RANGE_ARGS && CanHoldRange(inst));

    compiler::Register temp = range ? rangeTempsStart_ : 0U;
    if (range) {
        InsertSpillsForDynRangeInst(inst, nargs, start);
        return;
    }

    for (size_t i = start; i < nargs; ++i) {
        auto srcReg = inst->GetSrcReg(i);
        auto type = inst->GetInputType(i);

        // do not spillfill for acc-read position. For example, Intrinsic.FSTARR32
        if (IsAccReadPosition(inst, i)) {
            continue;
        }

        if (!RegNeedsRenumbering(srcReg) || srcReg < NUM_COMPACTLY_ENCODED_REGS) {
            continue;
        }

        auto res = spillMap.emplace(srcReg, RegContent(temp, type));
        if (res.second) {
            inst->SetSrcReg(i, temp++);
        } else {
            // Such register is already in map.
            // It can be ok for cases like: CallStatic v49, v49
            // Such instructions can be generated by optimizer too.
            const RegContent &regCont = res.first->second;
            inst->SetSrcReg(i, regCont.reg);
        }
    }

    AddMoveBefore(inst, spillMap);
    RenumberDstReg(inst, temp);
}

size_t RegEncoder::GetStartInputIndex(compiler::Inst *inst)
{
    return inst->GetOpcode() == compiler::Opcode::InitObject ? 1U : 0U;  // exclude LoadAndInitClass and NewObject
}

static bool IsBoundDstSrc(const compiler::Inst *inst)
{
    if (!inst->IsBinaryInst()) {
        return false;
    }
    auto src0 = inst->GetSrcReg(0U);
    auto src1 = inst->GetSrcReg(1U);
    auto dst = inst->GetDstReg();
    if (inst->IsCommutative()) {
        return src0 == dst || src1 == dst;
    }
    return src0 == dst;
}

static bool IsMoveAfter(const compiler::Inst *inst)
{
    auto writesToDest = inst->GetDstReg() != compiler::GetAccReg();
    if (inst->IsBinaryImmInst()) {
        return writesToDest;
    }
    if (inst->IsBinaryInst()) {
        return writesToDest && IsBoundDstSrc(inst);
    }
    switch (inst->GetOpcode()) {
        case compiler::Opcode::LoadObject:
            // Special case for LoadObject, because it can be register instruction.
            return writesToDest;
        case compiler::Opcode::NewArray:
            return true;
        default:
            return false;
    }
}

void RegEncoder::InsertSpillsForInst(compiler::Inst *inst)
{
    ASSERT(state_ == RegEncoderState::INSERT_SPILLS);

    RegContentMap spillMap(GetGraph()->GetLocalAllocator()->Adapter());  // src -> (dst, src_type)

    if (inst->IsOperandsDynamic()) {
        InsertSpillsForDynInputsInst(inst);
        return;
    }

    compiler::Register temp = 0;
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        // NOTE(mbolshov): make a better solution to skip instructions, that are not relevant to bytecode_opt
        if (inst->GetInput(i).GetInst()->GetOpcode() == Opcode::LoadAndInitClass) {
            continue;
        }
        auto reg = inst->GetSrcReg(i);
        if (RegNeedsRenumbering(reg) && reg >= NUM_COMPACTLY_ENCODED_REGS) {
            auto res = spillMap.emplace(reg, RegContent(temp, GetRegType(inst->GetInputType(i))));
            if (res.second) {
                inst->SetSrcReg(i, temp++);
            } else {
                // Such register is already in map.
                // It can be ok for cases like: and v49, v49
                // Such instructions can be generated by optimizer too.
                const RegContent &regCont = res.first->second;
                inst->SetSrcReg(i, regCont.reg);
            }
        }
    }

    AddMoveBefore(inst, spillMap);
    if (IsMoveAfter(inst)) {
        auto reg = inst->GetDstReg();
        if (RegNeedsRenumbering(reg) && reg >= NUM_COMPACTLY_ENCODED_REGS) {
            inst->SetDstReg(temp);
            AddMoveAfter(inst, temp, RegContent(reg, GetRegType(inst->GetType())));
        }
    }
}

static void IncTempsIfNeeded(compiler::Inst *inst, const compiler::Register reg, compiler::Register &numTemps,
                             compiler::Register &numChangedWidth)
{
    if (RegNeedsRenumbering(reg) && reg >= NUM_COMPACTLY_ENCODED_REGS) {
        numTemps++;
        if (inst->IsBinaryInst()) {
            numChangedWidth++;
        }
    }
}

void RegEncoder::CalculateNumNeededTempsForInst(compiler::Inst *inst)
{
    ASSERT(state_ == RegEncoderState::RESERVE_TEMPS);

    compiler::Register numTemps = 0;
    compiler::Register numChangedWidth = 0;

    if (inst->IsOperandsDynamic()) {
        if (IsIntrinsicRange(inst)) {
            return;
        }
        ASSERT(inst->IsStaticCall() || inst->IsVirtualCall() || inst->IsInitObject() || inst->IsIntrinsic());

        auto nargs = inst->GetInputsCount() - (inst->RequireState() ? 1U : 0U);
        size_t start = inst->GetOpcode() == compiler::Opcode::InitObject ? 1U : 0U;
        if (nargs - start > MAX_NUM_NON_RANGE_ARGS) {  // is call.range
            return;
        }

        for (size_t i = start; i < nargs; i++) {
            if (IsAccReadPosition(inst, i)) {
                continue;
            }
            auto reg = inst->GetSrcReg(i);
            if (!RegNeedsRenumbering(reg) || reg < NUM_COMPACTLY_ENCODED_REGS) {
                continue;
            }
            numTemps++;
            if (inst->IsBinaryInst()) {
                numChangedWidth++;
            }
        }
    } else {
        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            // NOTE(mbolshov): make a better solution to skip instructions, that are not relevant to bytecode_opt
            if (inst->GetInput(i).GetInst()->GetOpcode() == Opcode::LoadAndInitClass) {
                continue;
            }
            IncTempsIfNeeded(inst, inst->GetSrcReg(i), numTemps, numChangedWidth);
        }

        if (IsMoveAfter(inst) && !IsBoundDstSrc(inst)) {
            IncTempsIfNeeded(inst, inst->GetDstReg(), numTemps, numChangedWidth);
        }
    }

    ASSERT(numTemps <= MAX_NUM_INPUTS);

    numTemps_ = std::max(numTemps, numTemps_);
    numChangedWidth_ = std::max(numChangedWidth, numChangedWidth_);
}

void RegEncoder::Check4Width(compiler::Inst *inst)
{
    switch (state_) {
        case RegEncoderState::RESERVE_TEMPS: {
            CalculateNumNeededTempsForInst(inst);
            break;
        }
        case RegEncoderState::INSERT_SPILLS: {
            InsertSpillsForInst(inst);
            break;
        }
        default:
            UNREACHABLE();
    }
}

void RegEncoder::Check8Width([[maybe_unused]] compiler::Inst *inst)
{
    // NOTE(aantipina): implement after it became possible to use register numbers more than 256 (#2697)
}

void RegEncoder::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    CallHelper(visitor, inst);
}

void RegEncoder::VisitCallVirtual(GraphVisitor *visitor, Inst *inst)
{
    CallHelper(visitor, inst);
}

void RegEncoder::VisitInitObject(GraphVisitor *visitor, Inst *inst)
{
    CallHelper(visitor, inst);
}

void RegEncoder::VisitIntrinsic(GraphVisitor *visitor, Inst *inst)
{
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit() && inst->IsIntrinsic()) {
        auto re = static_cast<RegEncoder *>(visitor);
        if (IsIntrinsicRange(inst)) {
            re->Check4Width(inst);
            return;
        }
        if (IsAbcKitIntrinsic(inst->CastToIntrinsic()->GetIntrinsicId())) {
            CheckWidthAbcKitIntrinsic(re, inst);
        }
        CallHelper(visitor, inst);
        return;
    }
    if (inst->IsCallOrIntrinsic()) {
        CallHelper(visitor, inst);
        return;
    }
    auto re = static_cast<RegEncoder *>(visitor);
    if (IsIntrinsicRange(inst)) {
        re->Check4Width(inst);
        return;
    }

    re->Check8Width(inst);
}

void RegEncoder::VisitLoadObject(GraphVisitor *v, Inst *instBase)
{
    bool isAccType = instBase->GetDstReg() == compiler::GetAccReg();

    auto re = static_cast<RegEncoder *>(v);
    auto inst = instBase->CastToLoadObject();
    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT32:
        case compiler::DataType::FLOAT64:
        case compiler::DataType::REFERENCE:
            if (isAccType) {
                re->Check8Width(inst);
            } else {
                re->Check4Width(inst);
            }
            break;
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            re->success_ = false;
    }
}

void RegEncoder::VisitLoadStatic(GraphVisitor *v, Inst *instBase)
{
    auto re = static_cast<RegEncoder *>(v);
    auto inst = instBase->CastToLoadStatic();

    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT32:
        case compiler::DataType::FLOAT64:
        case compiler::DataType::REFERENCE:
            return;
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            re->success_ = false;
    }
}

void RegEncoder::VisitStoreObject(GraphVisitor *v, Inst *instBase)
{
    bool isAccType = instBase->GetSrcReg(1U) == compiler::GetAccReg();

    auto re = static_cast<RegEncoder *>(v);
    auto inst = instBase->CastToStoreObject();
    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT32:
        case compiler::DataType::FLOAT64:
        case compiler::DataType::REFERENCE:
            if (isAccType) {
                re->Check8Width(inst);
            } else {
                re->Check4Width(inst);
            }
            break;
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            re->success_ = false;
    }
}

void RegEncoder::VisitStoreStatic(GraphVisitor *v, Inst *instBase)
{
    auto re = static_cast<RegEncoder *>(v);
    auto inst = instBase->CastToStoreStatic();

    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT32:
        case compiler::DataType::FLOAT64:
        case compiler::DataType::REFERENCE:
            return;
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            re->success_ = false;
    }
}
void RegEncoder::VisitSpillFill([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void RegEncoder::VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void RegEncoder::VisitLoadString([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void RegEncoder::VisitReturn([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void RegEncoder::VisitCatchPhi([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void RegEncoder::VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}

#include "generated/check_width.cpp"
}  // namespace ark::bytecodeopt
