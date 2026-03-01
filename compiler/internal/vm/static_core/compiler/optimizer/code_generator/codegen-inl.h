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

#ifndef COMPILER_OPTIMIZER_CODEGEN_CODEGEN_INL_H
#define COMPILER_OPTIMIZER_CODEGEN_CODEGEN_INL_H

namespace ark::compiler {

/// 'live_inputs' shows that inst's source registers should be added the the mask
template <bool LIVE_INPUTS>
std::pair<RegMask, VRegMask> Codegen::GetLiveRegisters(Inst *inst)
{
    RegMask liveRegs;
    VRegMask liveFpRegs;
    if (!g_options.IsCompilerSaveOnlyLiveRegisters() || inst == nullptr) {
        liveRegs.set();
        liveFpRegs.set();
        return {liveRegs, liveFpRegs};
    }
    // Run LiveRegisters pass only if it is actually required
    if (!GetGraph()->IsAnalysisValid<LiveRegisters>()) {
        GetGraph()->RunPass<LiveRegisters>();
    }

    // Add registers from intervals that are live at inst's definition
    auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
    lr.VisitIntervalsWithLiveRegisters<LIVE_INPUTS>(inst, [&liveRegs, &liveFpRegs, this](const auto &li) {
        auto reg = ConvertRegister(li->GetReg(), li->GetType());
        GetEncoder()->SetRegister(&liveRegs, &liveFpRegs, reg);
    });

    // Add live temp registers
    liveRegs |= GetEncoder()->GetLiveTmpRegMask();
    liveFpRegs |= GetEncoder()->GetLiveTmpFpRegMask();

    return {liveRegs, liveFpRegs};
}

template <typename T, typename... Args>
T *Codegen::CreateSlowPath(Inst *inst, Args &&...args)
{
    static_assert(std::is_base_of_v<SlowPathBase, T>);
    auto label = GetEncoder()->CreateLabel();
    auto slowPath = GetLocalAllocator()->New<T>(label, inst, std::forward<Args>(args)...);
    slowPaths_.push_back(slowPath);
    return slowPath;
}

/**
 * Insert tracing code to the generated code. See `Trace` method in the `runtime/entrypoints.cpp`.
 * NOTE(compiler): we should rework parameters assigning algorithm, that is duplicated here.
 * @param params parameters to be passed to the TRACE entrypoint, first parameter must be TraceId value.
 */
template <typename... Args>
void Codegen::InsertTrace(Args &&...params)
{
    SCOPED_DISASM_STR(this, "Trace");
    [[maybe_unused]] constexpr size_t MAX_PARAM_NUM = 8;
    static_assert(sizeof...(Args) <= MAX_PARAM_NUM);
    auto regfile = GetRegfile();
    auto saveRegs = regfile->GetCallerSavedRegMask();
    saveRegs.set(GetTarget().GetReturnRegId());
    auto saveVregs = regfile->GetCallerSavedVRegMask();
    saveVregs.set(GetTarget().GetReturnFpRegId());

    SaveCallerRegisters(saveRegs, saveVregs, false);
    FillCallParams(std::forward<Args>(params)...);
    EmitCallRuntimeCode(nullptr, EntrypointId::TRACE);
    LoadCallerRegisters(saveRegs, saveVregs, false);
}

template <bool IS_FASTPATH, typename... Args>
void Codegen::CallEntrypoint(Inst *inst, EntrypointId id, Reg dstReg, RegMask preservedRegs, Args &&...params)
{
    ASSERT(inst != nullptr);
    CHECK_EQ(sizeof...(Args), GetRuntime()->GetEntrypointArgsNum(id));
    if (GetArch() == Arch::AARCH32) {
        // There is a problem with 64-bit parameters:
        // params number passed from entrypoints_gen.S.erb will be inconsistent with Aarch32 ABI.
        // Thus, runtime bridges will have wrong params number (\paramsnum macro argument).
        ASSERT(EnsureParamsFitIn32Bit({params...}));
        ASSERT(!dstReg.IsValid() || dstReg.GetSize() <= WORD_SIZE);
    }

    SCOPED_DISASM_STR(this, std::string("CallEntrypoint: ") + GetRuntime()->GetEntrypointName(id));
    RegMask liveRegs {preservedRegs | GetLiveRegisters(inst).first};
    RegMask paramsMask;
    if (inst->HasImplicitRuntimeCall() && !GetRuntime()->IsEntrypointNoreturn(id)) {
        SaveRegistersForImplicitRuntime(inst, &paramsMask, &liveRegs);
    }

    ASSERT(IS_FASTPATH == GetRuntime()->IsEntrypointFastPath(id));
    bool retRegAlive {liveRegs.Test(GetTarget().GetReturnRegId())};
    // parameter regs: their initial values must be stored by the caller
    // Other caller regs stored in bridges
    FillOnlyParameters(&liveRegs, sizeof...(Args), IS_FASTPATH);
    // When value stored in target return register outlives current call, it must be stored too
    if (retRegAlive && dstReg.IsValid()) {
        Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
        if (dstReg.GetId() != retReg.GetId()) {
            GetEncoder()->SetRegister(&liveRegs, nullptr, retReg, true);
        }
    }

    SaveCallerRegisters(liveRegs, VRegMask(), true);

    if (sizeof...(Args) != 0) {
        FillCallParams(std::forward<Args>(params)...);
    }

    // Call Code
    if (!EmitCallRuntimeCode(inst, id)) {
        return;
    }
    if (dstReg.IsValid()) {
        ASSERT(dstReg.IsScalar());
        GetEncoder()->SetRegister(&liveRegs, nullptr, dstReg, false);
        Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
        // We must:
        //  sign extended INT8 and INT16 to INT32
        //  zero extended UINT8 and UINT16 to UINT32
        if (dstReg.GetSize() < WORD_SIZE) {
            bool isSigned = DataType::IsTypeSigned(inst->GetType());
            GetEncoder()->EncodeCast(dstReg.As(INT32_TYPE), isSigned, retReg, isSigned);
        } else {
            GetEncoder()->EncodeMov(dstReg, retReg);
        }
    }
    CallEntrypointFinalize(liveRegs, paramsMask, inst);
}

// The function is used for calling runtime functions through special bridges.
// !NOTE Don't use the function for calling runtime without bridges(it save only parameters on stack)
template <typename... Args>
void Codegen::CallRuntime(Inst *inst, EntrypointId id, Reg dstReg, RegMask preservedRegs, Args &&...params)
{
    CallEntrypoint<false>(inst, id, dstReg, preservedRegs, std::forward<Args>(params)...);
}

template <typename... Args>
void Codegen::CallFastPath(Inst *inst, EntrypointId id, Reg dstReg, RegMask preservedRegs, Args &&...params)
{
#ifdef SAFEPOINT_TIME_CHECKER_ENABLED
    // predicate for irtoc intrinsics
    if (GetRuntime()->IsEntrypointFastPath(id)) {
        TypedImm entryId(static_cast<uint32_t>(id));
        InsertCallChecker(EntrypointId::REGISTER_SAFEPOINT_TIMER, entryId);
        CallEntrypoint<true>(inst, id, dstReg, preservedRegs, std::forward<Args>(params)...);
        InsertCallChecker(EntrypointId::DISARM_SAFEPOINT_TIMER, entryId);
    } else {
        CallEntrypoint<true>(inst, id, dstReg, preservedRegs, std::forward<Args>(params)...);
    }
#else
    CallEntrypoint<true>(inst, id, dstReg, preservedRegs, std::forward<Args>(params)...);
#endif
}

template <typename... Args>
void Codegen::CallRuntimeWithMethod(Inst *inst, void *method, EntrypointId eid, Reg dstReg, Args &&...params)
{
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg methodReg(GetEncoder());
        LoadMethod(methodReg);
        CallRuntime(inst, eid, dstReg, RegMask::GetZeroMask(), methodReg, std::forward<Args>(params)...);
    } else {
        if (Is64BitsArch(GetArch())) {
            CallRuntime(inst, eid, dstReg, RegMask::GetZeroMask(), TypedImm(reinterpret_cast<uint64_t>(method)),
                        std::forward<Args>(params)...);
        } else {
            // uintptr_t causes problems on host cross-jit compilation
            CallRuntime(inst, eid, dstReg, RegMask::GetZeroMask(), TypedImm(down_cast<uint32_t>(method)),
                        std::forward<Args>(params)...);
        }
    }
}

template <typename... Args>
void Codegen::CallBarrier(RegMask liveRegs, VRegMask liveVregs, std::variant<EntrypointId, Reg> entrypoint, Reg dstReg,
                          Args &&...params)
{
    bool isFastpath = GetGraph()->GetMode().IsFastPath();
    if (isFastpath) {
        // irtoc fastpath needs to save all caller registers in case of call native function
        liveRegs = GetCallerRegsMask(GetArch(), false);
        liveVregs = GetCallerRegsMask(GetArch(), true);
    }
    GetEncoder()->SetRegister(&liveRegs, nullptr, dstReg, false);
    SaveCallerRegisters(liveRegs, liveVregs, !isFastpath);
    FillCallParams(std::forward<Args>(params)...);
    EmitCallRuntimeCode(nullptr, entrypoint);
    if (dstReg.IsValid()) {
        ASSERT(dstReg.IsScalar());
        Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
        GetEncoder()->EncodeMov(dstReg, retReg);
    }
    LoadCallerRegisters(liveRegs, liveVregs, !isFastpath);
}

template <typename T>
void Codegen::CreateUnaryCheck(Inst *inst, RuntimeInterface::EntrypointId id, DeoptimizeType type, Condition cc)
{
    [[maybe_unused]] auto ss = inst->GetSaveState();
    ASSERT(ss != nullptr && (ss->GetOpcode() == Opcode::SaveState || ss->GetOpcode() == Opcode::SaveStateDeoptimize));

    LabelHolder::LabelId slowPath;
    if (inst->CanDeoptimize()) {
        slowPath = CreateSlowPath<SlowPathDeoptimize>(inst, type)->GetLabel();
    } else {
        slowPath = CreateSlowPath<T>(inst, id)->GetLabel();
    }
    auto srcType = inst->GetInputType(0);
    auto src = ConvertRegister(inst->GetSrcReg(0), srcType);
    GetEncoder()->EncodeJump(slowPath, src, cc);
}

inline ssize_t Codegen::GetStackOffset(Location location)
{
    if (location.GetKind() == LocationType::STACK_ARGUMENT) {
        return location.GetValue() * GetFrameLayout().GetSlotSize();
    }

    if (location.GetKind() == LocationType::STACK_PARAMETER) {
        return GetFrameLayout().GetFrameSize<CFrameLayout::OffsetUnit::BYTES>() +
               (location.GetValue() * GetFrameLayout().GetSlotSize());
    }

    ASSERT(location.GetKind() == LocationType::STACK);
    return GetFrameLayout().GetSpillOffsetFromSpInBytes(location.GetValue());
}

inline ssize_t Codegen::GetBaseOffset(Location location)
{
    if (location.IsAnyRegister()) {
        ASSERT(location.IsRegisterValid());
        auto *frame = GetFrameInfo();
        auto offset = location.IsFpRegister() ? frame->GetFpCalleesOffset() : frame->GetCalleesOffset();
        offset += GetCalleeRegsMask(GetArch(), location.IsFpRegister()).GetDistanceFromTail(location.GetValue());
        offset *= GetFrameLayout().GetSlotSize();
        return offset;
    }
    ASSERT(location.GetKind() == LocationType::STACK);
    return GetFrameLayout().GetSpillOffsetFromFpInBytes(location.GetValue());
}

inline MemRef Codegen::GetMemRefForSlot(CFrameLayout::OffsetOrigin basedOn, Location location)
{
    // Stack arguments shall be always SP-based
    if (location.GetKind() == LocationType::STACK_ARGUMENT || basedOn == CFrameLayout::OffsetOrigin::SP) {
        return MemRef(SpReg(), GetStackOffset(location));
    }
    ASSERT(location.IsAnyStack());
    return MemRef(FpReg(), -GetBaseOffset(location));
}

inline Reg Codegen::SpReg() const
{
    return GetTarget().GetStackReg();
}

inline Reg Codegen::FpReg() const
{
    return GetTarget().GetFrameReg();
}

inline const Disassembly *Codegen::GetDisasm() const
{
    return &disasm_;
}

inline Disassembly *Codegen::GetDisasm()
{
    return &disasm_;
}

inline void Codegen::AddLiveOut(const BasicBlock *bb, const Register reg)
{
    liveOuts_[bb].Set(reg);
}

inline RegMask Codegen::GetLiveOut(const BasicBlock *bb) const
{
    auto it = liveOuts_.find(bb);
    return it != liveOuts_.end() ? it->second : RegMask();
}

inline Reg Codegen::ThreadReg() const
{
    return Reg(GetThreadReg(GetArch()), GetTarget().GetPtrRegType());
}

inline bool Codegen::OffsetFitReferenceTypeSize(uint64_t offset) const
{
    // -1 because some arch uses signed offset
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    uint64_t maxOffset = 1ULL << (DataType::GetTypeSize(DataType::REFERENCE, GetArch()) - 1);
    return offset < maxOffset;
}

inline RegMask Codegen::GetUsedRegs() const
{
    return usedRegs_;
}
inline RegMask Codegen::GetUsedVRegs() const
{
    return usedVregs_;
}

inline uint32_t Codegen::GetVtableShift()
{
    // The size of the VTable element is equal to the size of pointers for the architecture
    // (not the size of pointer to objects)
    constexpr uint32_t SHIFT_64_BITS = 3;
    constexpr uint32_t SHIFT_32_BITS = 2;
    return Is64BitsArch(GetGraph()->GetArch()) ? SHIFT_64_BITS : SHIFT_32_BITS;
}

template <typename Arg, typename... Args>
ALWAYS_INLINE void Codegen::AddParamRegsInLiveMasksHandleArgs(ParameterInfo *paramInfo, RegMask *liveRegs,
                                                              VRegMask *liveVregs, Arg param, Args &&...params)
{
    auto currDst = paramInfo->GetNativeParam(param.GetType());
    if (std::holds_alternative<Reg>(currDst)) {
        auto reg = std::get<Reg>(currDst);
        if (reg.IsScalar()) {
            liveRegs->set(reg.GetId());
        } else {
            liveVregs->set(reg.GetId());
        }
    } else {
        GetEncoder()->SetFalseResult();
        UNREACHABLE();
    }
    if constexpr (sizeof...(Args) != 0) {
        AddParamRegsInLiveMasksHandleArgs(paramInfo, liveRegs, liveVregs, std::forward<Args>(params)...);
    }
}

template <typename... Args>
void Codegen::AddParamRegsInLiveMasks(RegMask *liveRegs, VRegMask *liveVregs, Args &&...params)
{
    auto callconv = GetCallingConvention();
    auto paramInfo = callconv->GetParameterInfo(0);
    AddParamRegsInLiveMasksHandleArgs(paramInfo, liveRegs, liveVregs, std::forward<Args>(params)...);
}

template <typename... Args>
void Codegen::CreateStubCall(Inst *inst, RuntimeInterface::IntrinsicId intrinsicId, Reg dst, Args &&...params)
{
    VRegMask liveVregs;
    RegMask liveRegs;
    auto enc = GetEncoder();

    AddParamRegsInLiveMasks(&liveRegs, &liveVregs, params...);

    if (dst.IsValid()) {
        ASSERT(dst.IsScalar());
        enc->SetRegister(&liveRegs, &liveVregs, dst, false);
        Reg retVal = GetTarget().GetReturnReg(dst.GetType());
        if (dst.GetId() != retVal.GetId()) {
            enc->SetRegister(&liveRegs, &liveVregs, retVal, true);
        }
    }

    SaveCallerRegisters(liveRegs, liveVregs, true);

    FillCallParams(std::forward<Args>(params)...);
    CallIntrinsic(inst, intrinsicId);

    if (inst->GetSaveState() != nullptr) {
        CreateStackMap(inst);
    }

    if (dst.IsValid()) {
        Reg retVal = GetTarget().GetReturnReg(dst.GetType());
        enc->EncodeMov(dst, retVal);
    }

    LoadCallerRegisters(liveRegs, liveVregs, true);
}

template <typename T>
void Codegen::EncodeImms(const T &imms, bool skipFirstLocation)
{
    auto paramInfo = GetCallingConvention()->GetParameterInfo(0);
    auto immType = DataType::INT32;
    if (skipFirstLocation) {
        paramInfo->GetNextLocation(immType);
    }
    for (auto imm : imms) {
        auto location = paramInfo->GetNextLocation(immType);
        ASSERT(location.IsFixedRegister());
        auto dstReg = ConvertRegister(location.GetValue(), immType);
        GetEncoder()->EncodeMov(dstReg, Imm(imm));
    }
}

template <typename... Args>
void FillPostWrbCallParams(MemRef mem, Args &&...params);

template <size_t IMM_ARRAY_SIZE>
class Codegen::FillCallParamsHelper {
public:
    using ImmsIter = typename std::array<std::pair<Reg, Imm>, IMM_ARRAY_SIZE>::iterator;

    FillCallParamsHelper(Codegen *cg, ParameterInfo *paramInfo, SpillFillInst *regMoves, ArenaVector<Reg> *spMoves,
                         ImmsIter immsIter)
        : cg_(cg), paramInfo_(paramInfo), regMoves_(regMoves), spMoves_(spMoves), immsIter_(immsIter)
    {
    }

    template <typename Arg, typename... Args>
    ALWAYS_INLINE void FillCallParamsHandleOperands(Arg &&arg, Args &&...params)
    {
        Location dst;
        auto type = arg.GetType().ToDataType();
        dst = paramInfo_->GetNextLocation(type);
        if (dst.IsStackArgument()) {
            cg_->GetEncoder()->SetFalseResult();
            UNREACHABLE();  // Move to BoundaryFrame
        }

        static_assert(std::is_same_v<std::decay_t<Arg>, TypedImm> || std::is_convertible_v<Arg, Reg>);
        if constexpr (std::is_same_v<std::decay_t<Arg>, TypedImm>) {
            auto reg = cg_->ConvertRegister(dst.GetValue(), type);
            *immsIter_ = {reg, arg.GetImm()};
            immsIter_++;
        } else {
            Reg reg(std::forward<Arg>(arg));
            if (reg == cg_->SpReg()) {
                // SP should be handled separately, since on the ARM64 target it has ID out of range
                spMoves_->emplace_back(cg_->ConvertRegister(dst.GetValue(), type));
            } else {
                regMoves_->AddSpillFill(Location::MakeRegister(reg.GetId(), type), dst, type);
            }
        }
        if constexpr (sizeof...(Args) != 0) {
            FillCallParamsHandleOperands(std::forward<Args>(params)...);
        }
    }

private:
    Codegen *cg_ {};
    ParameterInfo *paramInfo_ {};
    SpillFillInst *regMoves_ {};
    ArenaVector<Reg> *spMoves_ {};
    ImmsIter immsIter_ {};
};

template <typename T, typename... Args>
constexpr std::pair<size_t, size_t> CountParameters()
{
    static_assert(std::is_same_v<std::decay_t<T>, TypedImm> != std::is_convertible_v<T, Reg>);
    if constexpr (sizeof...(Args) != 0) {
        constexpr auto IMM_REG_COUNT = CountParameters<Args...>();

        if constexpr (std::is_same_v<std::decay_t<T>, TypedImm>) {
            return {IMM_REG_COUNT.first + 1, IMM_REG_COUNT.second};
        } else if constexpr (std::is_convertible_v<T, Reg>) {
            return {IMM_REG_COUNT.first, IMM_REG_COUNT.second + 1};
        }
    }
    return {std::is_same_v<std::decay_t<T>, TypedImm>, std::is_convertible_v<T, Reg>};
}

template <typename... Args>
void Codegen::FillCallParams(Args &&...params)
{
    SCOPED_DISASM_STR(this, "FillCallParams");
    if constexpr (sizeof...(Args) != 0) {
        constexpr size_t IMMEDIATES_COUNT = CountParameters<Args...>().first;
        constexpr size_t REGS_COUNT = CountParameters<Args...>().second;
        // Native call - do not add reserve parameters
        auto paramInfo = GetCallingConvention()->GetParameterInfo(0);
        std::array<std::pair<Reg, Imm>, IMMEDIATES_COUNT> immediates {};
        ArenaVector<Reg> spMoves(GetLocalAllocator()->Adapter());
        auto regMoves = GetGraph()->CreateInstSpillFill();
        spMoves.reserve(REGS_COUNT);
        regMoves->GetSpillFills().reserve(REGS_COUNT);

        FillCallParamsHelper<IMMEDIATES_COUNT> h {this, paramInfo, regMoves, &spMoves, immediates.begin()};
        h.FillCallParamsHandleOperands(std::forward<Args>(params)...);

        // Resolve registers move order and encode
        spillFillsResolver_.ResolveIfRequired(regMoves);
        SpillFillEncoder(this, regMoves).EncodeSpillFill();

        // Encode immediates moves
        for (auto &immValues : immediates) {
            GetEncoder()->EncodeMov(immValues.first, immValues.second);
        }

        // Encode moves from SP reg
        for (auto dst : spMoves) {
            GetEncoder()->EncodeMov(dst, SpReg());
        }
    }
}

template <typename... Args>
void Codegen::FillPostWrbCallParams(MemRef mem, Args &&...params)
{
    auto base {mem.GetBase().As(TypeInfo::FromDataType(DataType::REFERENCE, GetArch()))};
    if (mem.HasIndex()) {
        ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
        FillCallParams(base, mem.GetIndex(), std::forward<Args>(params)...);
    } else {
        FillCallParams(base, TypedImm(mem.GetDisp()), std::forward<Args>(params)...);
    }
}

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H
