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

/*
Codegen Hi-Level implementation
*/
#include "operands.h"
#include "codegen.h"
#include "codegen_load_entrypoint.h"
#include "encode_visitor.h"
#include "compiler_options.h"
#include "optimizer/ir/inst.h"
#include "relocations.h"
#include "include/compiler_interface.h"
#include "irtoc/backend/compiler/constants.h"
#include "ir-dyn-base-types.h"
#include "runtime/include/coretypes/string.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/locations.h"
#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "optimizer/code_generator/method_properties.h"
#include "libarkbase/events/events.h"
#include "libarkbase/utils/tsan_interface.h"
#include "libarkbase/utils/utils.h"
#include "libarkbase/utils/arena_containers.h"
#include <algorithm>
#include <iomanip>

namespace ark::compiler {

class OsrEntryStub {
    void FixIntervals(Codegen *codegen, Encoder *encoder)
    {
        auto &la = codegen->GetGraph()->GetAnalysis<LivenessAnalyzer>();
        la.EnumerateLiveIntervalsForInst(saveState_, [this, codegen, encoder](const auto &li) {
            auto inst = li->GetInst();
            auto location = li->GetLocation();
            // Skip live registers that are already in the input list of the OsrSaveState
            const auto &ssInputs = saveState_->GetInputs();
            if (std::find_if(ssInputs.begin(), ssInputs.end(),
                             [inst](auto &input) { return input.GetInst() == inst; }) != ssInputs.end()) {
                return;
            }
            // Only constants allowed
            switch (inst->GetOpcode()) {
                case Opcode::LoadAndInitClass: {
                    auto klass = reinterpret_cast<uintptr_t>(inst->CastToLoadAndInitClass()->GetClass());
                    encoder->EncodeMov(codegen->ConvertRegister(inst->GetDstReg(), inst->GetType()),
                                       Imm(reinterpret_cast<uintptr_t>(klass)));
                    break;
                }
                case Opcode::Constant: {
                    if (location.IsFixedRegister()) {
                        EncodeConstantMove(li, encoder);
                    } else if (location.IsStack()) {
                        auto slot = location.GetValue();
                        encoder->EncodeSti(
                            bit_cast<int64_t>(inst->CastToConstant()->GetRawValue()), DOUBLE_WORD_SIZE_BYTES,
                            MemRef(codegen->SpReg(), codegen->GetFrameLayout().GetSpillOffsetFromSpInBytes(slot)));
                    } else {
                        ASSERT(location.IsConstant());
                    }
                    break;
                }
                // NOTE (ekudriashov): UnresolvedLoadAndInitClass
                default:
                    break;
            }
        });
    }

    static void EncodeConstantMove(const LifeIntervals *li, Encoder *encoder)
    {
        auto inst = li->GetInst();
        switch (li->GetType()) {
            case DataType::FLOAT64:
                encoder->EncodeMov(Reg(li->GetReg(), FLOAT64_TYPE),
                                   Imm(bit_cast<double>(inst->CastToConstant()->GetDoubleValue())));
                break;
            case DataType::FLOAT32:
                encoder->EncodeMov(Reg(li->GetReg(), FLOAT32_TYPE),
                                   Imm(bit_cast<float>(inst->CastToConstant()->GetFloatValue())));
                break;
            case DataType::UINT32:
                encoder->EncodeMov(Reg(li->GetReg(), INT32_TYPE), Imm(inst->CastToConstant()->GetRawValue()));
                break;
            case DataType::UINT64:
                encoder->EncodeMov(Reg(li->GetReg(), INT64_TYPE), Imm(inst->CastToConstant()->GetRawValue()));
                break;
            default:
                UNREACHABLE();
        }
    }

public:
    OsrEntryStub(Codegen *codegen, SaveStateInst *inst) : label_(codegen->GetEncoder()->CreateLabel()), saveState_(inst)
    {
    }

    DEFAULT_MOVE_SEMANTIC(OsrEntryStub);
    DEFAULT_COPY_SEMANTIC(OsrEntryStub);
    ~OsrEntryStub() = default;

    void Generate(Codegen *codegen)
    {
        auto encoder = codegen->GetEncoder();
        auto lr = codegen->GetTarget().GetLinkReg();
        auto fl = codegen->GetFrameLayout();
        codegen->CreateStackMap(saveState_->CastToSaveStateOsr());
        auto slot = static_cast<ssize_t>(CFrameLayout::LOCALS_START_SLOT + CFrameLayout::GetLocalsCount() - 1U);
        encoder->EncodeStp(
            codegen->FpReg(), lr,
            MemRef(codegen->FpReg(),
                   -fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::BYTES>(slot)));

        FixIntervals(codegen, encoder);
        encoder->EncodeJump(label_);
    }

    SaveStateInst *GetInst()
    {
        return saveState_;
    }

    auto &GetLabel()
    {
        return label_;
    }

private:
    LabelHolder::LabelId label_;
    SaveStateInst *saveState_ {nullptr};
};

Codegen::Codegen(Graph *graph)
    : Optimization(graph),
      allocator_(graph->GetAllocator()),
      localAllocator_(graph->GetLocalAllocator()),
      codeBuilder_(allocator_->New<CodeInfoBuilder>(graph->GetArch(), allocator_)),
      slowPaths_(graph->GetLocalAllocator()->Adapter()),
      slowPathsMap_(graph->GetLocalAllocator()->Adapter()),
      frameLayout_(CFrameLayout(graph->GetArch(), graph->GetStackSlotsCount(), !graph->GetMode().IsFastPath())),
      osrEntries_(graph->GetLocalAllocator()->Adapter()),
      vregIndices_(GetAllocator()->Adapter()),
      runtime_(graph->GetRuntime()),
      target_(graph->GetArch()),
      liveOuts_(graph->GetLocalAllocator()->Adapter()),
      disasm_(this),
      spillFillsResolver_(graph),
      initInputInstructions_(graph->GetLocalAllocator()->Adapter())
{
    graph->SetCodeBuilder(codeBuilder_);
    regfile_ = graph->GetRegisters();
    if (regfile_ != nullptr) {
        ASSERT(regfile_->IsValid());
        ArenaVector<Reg> regsUsage(allocator_->Adapter());
        Convert(&regsUsage, graph->GetUsedRegs<DataType::INT64>(), INT64_TYPE);
        Convert(&regsUsage, graph->GetUsedRegs<DataType::FLOAT64>(), FLOAT64_TYPE);
        regfile_->SetUsedRegs(regsUsage);
#ifndef NDEBUG
        COMPILER_LOG(DEBUG, CODEGEN) << "Regalloc used registers scalar " << graph->GetUsedRegs<DataType::INT64>();
        COMPILER_LOG(DEBUG, CODEGEN) << "Regalloc used registers vector " << graph->GetUsedRegs<DataType::FLOAT64>();
#endif
    }

    enc_ = graph->GetEncoder();
    ASSERT(enc_ != nullptr && enc_->IsValid());
    enc_->SetRegfile(regfile_);
    if (enc_->InitMasm()) {
        enc_->SetFrameLayout(GetFrameLayout());
    }

    callconv_ = graph->GetCallingConvention();
    if (callconv_ != nullptr) {
        ASSERT(callconv_->IsValid());
        if (callconv_->GetEncoder() == nullptr) {
            callconv_->SetEncoder(enc_);
        }
    }

    auto method = graph->GetMethod();
    // workaround for test
    if (method != nullptr) {
        methodId_ = graph->GetRuntime()->GetMethodId(method);
    }
    GetDisasm()->Init();
    GetDisasm()->SetEncoder(GetEncoder());
}

const char *Codegen::GetPassName() const
{
    return "Codegen";
}

bool Codegen::AbortIfFailed() const
{
    return true;
}

void Codegen::CreateFrameInfo()
{
    // Create FrameInfo for CFrame
    auto &fl = GetFrameLayout();
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(true) |
        FrameInfo::CallersRelativeFp::Encode(false) | FrameInfo::CalleesRelativeFp::Encode(true));
    ASSERT(frame != nullptr);
    frame->SetFrameSize(fl.GetFrameSize<CFrameLayout::OffsetUnit::BYTES>());
    frame->SetSpillsCount(fl.GetSpillsCount());

    frame->SetCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    frame->SetSetupFrame(true);
    frame->SetSaveFrameAndLinkRegs(true);
    frame->SetSaveUnusedCalleeRegs(!GetGraph()->GetMethodProperties().GetCompactPrologueAllowed());
    frame->SetAdjustSpReg(true);
    frame->SetHasFloatRegs(GetGraph()->HasFloatRegs());

    GetCodeBuilder()->SetHasFloatRegs(GetGraph()->HasFloatRegs());

    SetFrameInfo(frame);
}

void Codegen::FillOnlyParameters(RegMask *liveRegs, uint32_t numParams, bool isFastpath) const
{
    ASSERT(numParams <= 6U);
    if (GetArch() == Arch::AARCH64 && !isFastpath) {
        numParams = AlignUp(numParams, 2U);
    }
    *liveRegs &= GetTarget().GetParamRegsMask(numParams);
}

void Codegen::Convert(ArenaVector<Reg> *regsUsage, const ArenaVector<bool> *mask, TypeInfo typeInfo)
{
    ASSERT(regsUsage != nullptr);
    // There are no used registers
    if (mask == nullptr) {
        return;
    }
    ASSERT(mask->size() == MAX_NUM_REGS);
    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if ((*mask)[i]) {
            regsUsage->emplace_back(i, typeInfo);
        }
    }
}

#ifdef IRTOC_INTRINSICS_ENABLED
void Codegen::EmitSimdIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitReverseIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                   [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitJsCastDoubleToCharIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                              [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitMarkWordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                    [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitDataMemoryBarrierFullIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                 [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitWriteTlabStatsSafeIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                              [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitExpandU8ToU16Intrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                         [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitAtomicByteOrIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                        [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitSaveOrRestoreRegsEpIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                               [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitTailCallIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                    [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitSlowPathEntryIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                         [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

/* It is the same for CodegenFastPath and CodegenInterpreter */
void Codegen::EmitUnreachableIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                       [[maybe_unused]] SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_UNREACHABLE);
    GetEncoder()->EncodeAbort();
}

/* It is the same for CodegenFastPath and CodegenInterpreter */
void Codegen::EmitInterpreterReturnIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                             [[maybe_unused]] SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN);
    auto enc = GetEncoder();
    // Removes all call records
    ssize_t offset = GetIrtocCFrameSizeBytesBelowFP(GetArch());
    enc->EncodeAdd(SpReg(), FpReg(), Imm(-offset));
    GetCallingConvention()->GenerateNativeEpilogue(*GetFrameInfo(), []() {});
}

void Codegen::EmitInitInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                     [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitPushInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                     [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitPopInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                    [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitPopMultipleInterpreterCallRecordsIntrinsic([[maybe_unused]] IntrinsicInst *inst,
                                                             [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}

void Codegen::EmitUpdateInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                       [[maybe_unused]] SRCREGS src)
{
    ASSERT(0);
    GetEncoder()->SetFalseResult();
}
#endif  // IRTOC_INTRINSICS_ENABLED

bool Codegen::BeginMethod()
{
    // Do not try to encode too large graph
    auto estimatedCodeSize = GetGraph()->EstimateCodeSize();
    if (estimatedCodeSize > g_options.GetCompilerMaxGenCodeSize()) {
        COMPILER_LOG(WARNING, CODEGEN) << "Code is too large - estimated code size: " << estimatedCodeSize;
        return false;
    }
    // Limit encoder to match estimated to actual code size.
    // After this - encoder aborted, if allocated too much size.
    GetEncoder()->SetMaxAllocatedBytes(g_options.GetCompilerMaxGenCodeSize());

    if (!IsCompressedStringsEnabled()) {
#ifndef NDEBUG
        LOG(FATAL, COMPILER) << "String compression must be enabled";
#else
        LOG(ERROR, COMPILER) << "String compression must be enabled";
#endif
        return false;
    }

    if (GetGraph()->IsAotMode()) {
        GetEncoder()->SetCodeOffset(GetGraph()->GetAotData()->GetCodeOffset() + CodeInfo::GetCodeOffset(GetArch()));
    } else {
        GetEncoder()->SetCodeOffset(0);
    }

    codeBuilder_->BeginMethod(GetFrameLayout().GetFrameSize<CFrameLayout::OffsetUnit::BYTES>(),
                              GetGraph()->GetVRegsCount() + GetGraph()->GetEnvCount());

    GetEncoder()->BindLabel(GetLabelEntry());
    SetStartCodeOffset(GetEncoder()->GetCursorOffset());

    GeneratePrologue();

    return GetEncoder()->GetResult();
}

void Codegen::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Method Prologue");

    GetCallingConvention()->GeneratePrologue(*frameInfo_);

    if (!(GetGraph()->GetMode().IsNative() || GetGraph()->GetMode().IsNativePlus())) {
        // Set the frame kind to "Compiled"
        GetEncoder()->EncodeSti(1, 1, MemRef(ThreadReg(), GetRuntime()->GetTlsFrameKindOffset(GetArch())));
        // Create stack overflow check
        GetEncoder()->EncodeStackOverflowCheck(-GetRuntime()->GetStackOverflowCheckOffset());
        // Create empty stackmap for the stack overflow check
        SaveStateInst ss(Opcode::SaveState, 0, nullptr, nullptr, 0);
        GetCodeBuilder()->BeginStackMap(0, 0, &ss, false);
        GetCodeBuilder()->EndStackMap();
    }

#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
    MakeTrace();
#endif
}

#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
void Codegen::MakeTrace()
{
    if (GetGraph()->IsAotMode()) {
        SCOPED_DISASM_STR(this, "LoadMethod for trace");
        ScopedTmpReg method_reg(GetEncoder());
        LoadMethod(method_reg);
        InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_ENTER)), method_reg,
                    Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED)));
    } else {
        InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                    Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                    Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED)));
    }
}
#endif

void Codegen::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "Method Epilogue");

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    GetCallingConvention()->GenerateEpilogue(*frame_info_, MakeTrace);
#else
    GetCallingConvention()->GenerateEpilogue(*frameInfo_, []() {});
#endif
}

bool Codegen::VisitGraph()
{
    EncodeVisitor visitor(this);

    const auto &blocks = GetGraph()->GetBlocksLinearOrder();

    for (auto bb : blocks) {
        GetEncoder()->BindLabel(bb->GetId());
        for (auto inst : bb->AllInsts()) {
            SCOPED_DISASM_INST(this, inst);

#ifdef PANDA_COMPILER_DEBUG_INFO
            auto debugInfo = inst->GetDebugInfo();
            if (GetGraph()->IsLineDebugInfoEnabled() && debugInfo != nullptr) {
                debugInfo->SetOffset(GetEncoder()->GetCursorOffset());
            }
#endif
            visitor.VisitInstruction(inst);
            if (!visitor.GetResult()) {
                COMPILER_LOG(DEBUG, CODEGEN)
                    << "Can't encode instruction: " << GetOpcodeString(inst->GetOpcode()) << *inst;
                break;
            }
        }

        if (bb->NeedsJump()) {
            EmitJump(bb);
        }

        if (!visitor.GetResult()) {
            return false;
        }
    }

    auto estimatedSlowPathsSize = slowPaths_.size() * SLOW_PATH_SIZE;
    auto estimatedCodeSize = GetGraph()->EstimateCodeSize<true>() + estimatedSlowPathsSize;
    if (estimatedCodeSize > g_options.GetCompilerMaxGenCodeSize()) {
        COMPILER_LOG(WARNING, CODEGEN) << "Code is too large - code size: " << GetGraph()->EstimateCodeSize<true>()
                                       << ", slow paths' code estimation: " << estimatedSlowPathsSize;
        return false;
    }

    EmitSlowPaths();

    return true;
}

void Codegen::EmitJump(const BasicBlock *bb)
{
    BasicBlock *sucBb = nullptr;

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        sucBb = bb->GetSuccsBlocks()[0];
    } else {
        switch (bb->GetLastInst()->GetOpcode()) {
            case Opcode::If:
            case Opcode::IfImm:
                ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
                sucBb = bb->GetFalseSuccessor();
                break;
            default:
                sucBb = bb->GetSuccsBlocks()[0];
                break;
        }
    }
    SCOPED_DISASM_STR(this, std::string("Jump from  BB_") + std::to_string(bb->GetId()) + " to BB_" +
                                std::to_string(sucBb->GetId()));

    auto label = sucBb->GetId();
    GetEncoder()->EncodeJump(label);
}

void Codegen::EndMethod()
{
    for (auto &osrStub : osrEntries_) {
        SCOPED_DISASM_STR(this,
                          std::string("Osr stub for OsrStateStump ") + std::to_string(osrStub->GetInst()->GetId()));
        osrStub->Generate(this);
    }

    GetEncoder()->Finalize();
}

// Allocates memory, copies generated code to it, sets the code to the graph's codegen data. Also this function
// encodes the code info and sets it to the graph.
bool Codegen::CopyToCodeCache()
{
    auto codeEntry = reinterpret_cast<char *>(GetEncoder()->GetLabelAddress(GetLabelEntry()));
    auto codeSize = GetEncoder()->GetCursorOffset();
    bool saveAllCalleeRegisters = !GetGraph()->GetMethodProperties().GetCompactPrologueAllowed();

    auto code = reinterpret_cast<uint8_t *>(GetAllocator()->Alloc(codeSize));
    if (code == nullptr) {
        return false;
    }
    std::copy_n(codeEntry, codeSize, code);
    GetGraph()->SetCode(EncodeDataType(code, codeSize));

    RegMask calleeRegs;
    VRegMask calleeVregs;
    GetRegfile()->FillUsedCalleeSavedRegisters(&calleeRegs, &calleeVregs, saveAllCalleeRegisters);
    constexpr size_t MAX_NUM_REGISTERS = 32;
    static_assert(MAX_NUM_REGS <= MAX_NUM_REGISTERS && MAX_NUM_VREGS <= MAX_NUM_REGISTERS);
    codeBuilder_->SetSavedCalleeRegsMask(static_cast<uint32_t>(calleeRegs.to_ulong()),
                                         static_cast<uint32_t>(calleeVregs.to_ulong()));

    ArenaVector<uint8_t> codeInfoData(GetGraph()->GetAllocator()->Adapter());
    codeBuilder_->Encode(&codeInfoData);
    GetGraph()->SetCodeInfo(Span(codeInfoData));

    return true;
}

void Codegen::IssueDisasm()
{
    if (GetGraph()->GetMode().SupportManagedCode() && g_options.IsCompilerDisasmDumpCodeInfo()) {
        GetDisasm()->PrintCodeInfo(this);
    }
    GetDisasm()->PrintCodeStatistics(this);
    auto &stream = GetDisasm()->GetStream();
    for (auto &item : GetDisasm()->GetItems()) {
        if (std::holds_alternative<CodeItem>(item)) {
            auto &code = std::get<CodeItem>(item);
            for (size_t pc = code.GetPosition(); pc < code.GetCursorOffset();) {
                stream << GetDisasm()->GetIndent(code.GetDepth());
                auto newPc = GetEncoder()->DisasmInstr(stream, pc, 0);
                stream << "\n";
                pc = newPc;
            }
        } else if (std::holds_alternative<std::string>(item)) {
            stream << (std::get<std::string>(item));
        }
    }
}

bool Codegen::RunImpl()
{
    Initialize();

    auto encoder = GetEncoder();
    encoder->GetLabels()->CreateLabels(GetGraph()->GetVectorBlocks().size());
    labelEntry_ = encoder->CreateLabel();
    labelExit_ = encoder->CreateLabel();

#ifndef NDEBUG
    if (g_options.IsCompilerNonOptimizing()) {
        // In case of non-optimizing compiler lowering pass is not run but low-level instructions may
        // also appear on codegen so, to satisfy GraphChecker, the flag should be raised.
        GetGraph()->SetLowLevelInstructionsEnabled();
    }
#endif  // NDEBUG

    if ((GetCallingConvention() == nullptr) || (GetEncoder() == nullptr)) {
        return false;
    }

    if (GetCallingConvention()->IsDynCallMode()) {
        auto numExpectedArgs = GetRuntime()->GetMethodTotalArgumentsCount(GetGraph()->GetMethod());
        if (numExpectedArgs > GetRuntime()->GetDynamicNumFixedArgs()) {
            CallConvDynInfo dynInfo(numExpectedArgs, GetRuntime()->GetEntrypointsTablePointerTlsOffset(GetArch()),
                                    GetRuntime()->GetEntrypointOffset(
                                        GetArch(), RuntimeInterface::EntrypointId::EXPAND_COMPILED_CODE_ARGS_DYN));
            GetCallingConvention()->SetDynInfo(dynInfo);
            frameInfo_->SetSaveFrameAndLinkRegs(true);
        } else {
            GetCallingConvention()->SetDynInfo(CallConvDynInfo());
        }
    }

    if (!GetEncoder()->GetResult()) {
        return false;
    }

    // Remove registers from the temp registers, if they are in the regalloc mask, i.e. available for regalloc.
    auto usedRegs = ~GetGraph()->GetArchUsedRegs();
    auto forbiddenTemps = usedRegs & GetTarget().GetTempRegsMask();
    if (forbiddenTemps.Any()) {
        for (size_t i = forbiddenTemps.GetMinRegister(); i <= forbiddenTemps.GetMaxRegister(); i++) {
            if (forbiddenTemps[i] && encoder->IsScratchRegisterReleased(Reg(i, INT64_TYPE))) {
                encoder->AcquireScratchRegister(Reg(i, INT64_TYPE));
            }
        }
    }

    return Finalize();
}

void Codegen::Initialize()
{
    CreateFrameInfo();

    GetRegfile()->SetCalleeSaved(GetRegfile()->GetCalleeSaved());

    if (!GetGraph()->SupportManagedCode()) {
        for (auto inst : GetGraph()->GetStartBlock()->AllInsts()) {
            if (inst->GetOpcode() == Opcode::LiveIn && GetTarget().GetTempRegsMask().Test(inst->GetDstReg())) {
                GetEncoder()->AcquireScratchRegister(Reg(inst->GetDstReg(), INT64_TYPE));
            }
        }
    }

    bool hasCalls = false;

    for (auto bb : GetGraph()->GetBlocksLinearOrder()) {
        // Calls may be in the middle of method
        for (auto inst : bb->Insts()) {
            // For throw instruction need jump2runtime same way
            if (inst->IsCall() || inst->GetOpcode() == Opcode::Throw) {
                hasCalls = true;
                break;
            }
        }
        if (hasCalls) {
            break;
        }
    }

    /* Convert Graph::GetUsedRegs(), which is std::vector<bool>, to simple
     * RegMask and save it in the Codegen. These masks are used to determine
     * which registers we need to save in prologue.
     *
     * NB! It's related to IRTOC specific prologue generation (see CodegenFastPath etc.).
     * Arch specific CallingConvention::GenerateProlog() relies on reg usage information
     * prepared in the Codegen constructor (before Initialize() is called).
     */
    auto fillMask = [](RegMask *mask, auto *vector) {
        if (vector == nullptr) {
            return;
        }
        ASSERT(mask->size() >= vector->size());
        mask->reset();
        for (size_t i = 0; i < mask->size(); i++) {
            if ((*vector)[i]) {
                mask->set(i);
            }
        }
    };
    fillMask(&usedRegs_, GetGraph()->GetUsedRegs<DataType::INT64>());
    fillMask(&usedVregs_, GetGraph()->GetUsedRegs<DataType::FLOAT64>());
    usedVregs_ &= GetTarget().GetAvailableVRegsMask();
    usedRegs_ &= GetTarget().GetAvailableRegsMask();
    usedRegs_ &= ~GetGraph()->GetArchUsedRegs();
    usedVregs_ &= ~GetGraph()->GetArchUsedVRegs();

    /* Remove used registers from Encoder's scratch registers */
    RegMask usedTemps = usedRegs_ & GetTarget().GetTempRegsMask();
    if (usedTemps.any()) {
        for (size_t i = 0; i < usedTemps.size(); i++) {
            if (usedTemps[i]) {
                GetEncoder()->AcquireScratchRegister(Reg(i, INT64_TYPE));
            }
        }
    }
}

bool Codegen::Finalize()
{
    if (GetDisasm()->IsEnabled()) {
        GetDisasm()->PrintMethodEntry(this);
    }

    if (!BeginMethod()) {
        return false;
    }

    if (!VisitGraph()) {
        return false;
    }
    EndMethod();

    if (!CopyToCodeCache()) {
        return false;
    }

    if (GetDisasm()->IsEnabled()) {
        IssueDisasm();
    }

    return true;
}

Reg Codegen::ConvertRegister(Register r, DataType::Type type)
{
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8: {
            return Reg(r, INT8_TYPE);
        }
        case DataType::UINT16:
        case DataType::INT16: {
            return Reg(r, INT16_TYPE);
        }
        case DataType::UINT32:
        case DataType::INT32: {
            return Reg(r, INT32_TYPE);
        }
        case DataType::UINT64:
        case DataType::INT64:
        case DataType::ANY: {
            return Reg(r, INT64_TYPE);
        }
        case DataType::FLOAT32: {
            return Reg(r, FLOAT32_TYPE);
        }
        case DataType::FLOAT64: {
            return Reg(r, FLOAT64_TYPE);
        }
        case DataType::REFERENCE: {
            return ConvertRegister(r, DataType::GetIntTypeForReference(GetArch()));
        }
        case DataType::POINTER: {
            return Reg(r, ConvertDataType(DataType::POINTER, GetArch()));
        }
        default:
            // Invalid converted register
            return INVALID_REGISTER;
    }
}

// Panda don't support types less then 32, so we need sign or zero extended to 32
Imm Codegen::ConvertImmWithExtend(uint64_t imm, DataType::Type type)
{
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
            return Imm(static_cast<uint32_t>(static_cast<uint8_t>(imm)));
        case DataType::INT8:
            return Imm(static_cast<int32_t>(bit_cast<int8_t, uint8_t>(imm)));
        case DataType::UINT16:
            return Imm(static_cast<uint32_t>(static_cast<uint16_t>(imm)));
        case DataType::INT16:
            return Imm(static_cast<int32_t>(bit_cast<int16_t, uint16_t>(imm)));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT32:
            return Imm(bit_cast<int32_t, uint32_t>(imm));
        case DataType::INT32:
            return Imm(bit_cast<int32_t, uint32_t>(imm));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT64:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        case DataType::INT64:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        case DataType::FLOAT32:
            return Imm(bit_cast<float, uint32_t>(static_cast<uint32_t>(imm)));
        case DataType::FLOAT64:
            return Imm(bit_cast<double, uint64_t>(imm));
        case DataType::ANY:
            return Imm(bit_cast<uint64_t, uint64_t>(imm));
        case DataType::REFERENCE:
            if (imm == 0) {
                return Imm(0);
            }
            [[fallthrough]]; /* fall-through */
        default:
            // Invalid converted immediate
            UNREACHABLE();
    }
    return Imm(0);
}

Condition Codegen::ConvertCc(ConditionCode cc)
{
    switch (cc) {
        case CC_EQ:
            return Condition::EQ;
        case CC_NE:
            return Condition::NE;
        case CC_LT:
            return Condition::LT;
        case CC_LE:
            return Condition::LE;
        case CC_GT:
            return Condition::GT;
        case CC_GE:
            return Condition::GE;
        case CC_B:
            return Condition::LO;
        case CC_BE:
            return Condition::LS;
        case CC_A:
            return Condition::HI;
        case CC_AE:
            return Condition::HS;
        case CC_TST_EQ:
            return Condition::TST_EQ;
        case CC_TST_NE:
            return Condition::TST_NE;
        default:
            UNREACHABLE();
            return Condition::EQ;
    }
    return Condition::EQ;
}

Condition Codegen::ConvertCcOverflow(ConditionCode cc)
{
    switch (cc) {
        case CC_EQ:
            return Condition::VS;
        case CC_NE:
            return Condition::VC;
        default:
            UNREACHABLE();
            return Condition::VS;
    }
    return Condition::VS;
}

void Codegen::EmitSlowPaths()
{
    for (auto slowPath : slowPaths_) {
        slowPath->Generate(this);
    }
}

uint32_t Codegen::GetAOTBinaryFileSnapshotIndexForInst([[maybe_unused]] const Inst *inst) const
{
    ASSERT(GetGraph()->IsAotMode());
    // If external methods are not inlined we always use original logic (current method's file)
    if (!g_options.IsCompilerInlineExternalMethods() || !g_options.IsCompilerInlineExternalMethodsAot()) {
        return panda_file::File::INVALID_FILE_INDEX;
    }
    const SaveStateInst *saveState =
        !inst->IsSaveState() ? inst->GetSaveState() : static_cast<const SaveStateInst *>(inst);
    ASSERT(saveState != nullptr);

    uint32_t index = panda_file::File::INVALID_FILE_INDEX;
    if (auto callInst = saveState->GetCallerInst()) {
        auto callMethod = callInst->GetCallMethod();
        ASSERT(callMethod != nullptr);
        auto runtime = GetGraph()->GetRuntime();
        // Write index only if inst is externally inlined
        if (runtime->GetBinaryFileForMethod(callMethod) == runtime->GetBinaryFileForMethod(GetGraph()->GetMethod())) {
            index = panda_file::File::INVALID_FILE_INDEX;
        } else {
            index = GetGraph()->GetRuntime()->GetAOTBinaryFileSnapshotIndexForMethod(callMethod);
        }
    }

    return index;
}

TypedImm Codegen::GetTypeIdImm(const Inst *inst, uint32_t typeId) const
{
    if (!GetGraph()->IsAotMode() || GetArch() == Arch::AARCH32) {
        return TypedImm(typeId);
    }
    return TypedImm(panda_file::File::PackFileEntityIdWithIndex(typeId, GetAOTBinaryFileSnapshotIndexForInst(inst)));
}

void Codegen::CreateStackMap(Inst *inst, Inst *user)
{
    SaveStateInst *saveState = nullptr;
    if (inst->IsSaveState()) {
        saveState = static_cast<SaveStateInst *>(inst);
    } else {
        saveState = inst->GetSaveState();
    }
    ASSERT(saveState != nullptr);

    bool requireVregMap = inst->RequireRegMap();
    uint32_t outerBpc = inst->GetPc();
    for (auto callInst = saveState->GetCallerInst(); callInst != nullptr;
         callInst = callInst->GetSaveState()->GetCallerInst()) {
        outerBpc = callInst->GetPc();
    }
    codeBuilder_->BeginStackMap(outerBpc, GetEncoder()->GetCursorOffset(), saveState, requireVregMap);
    if (user == nullptr) {
        user = inst;
        if (inst == saveState && inst->HasUsers()) {
            auto users = inst->GetUsers();
            auto it = std::find_if(users.begin(), users.end(),
                                   [](auto &u) { return u.GetInst()->GetOpcode() != Opcode::ReturnInlined; });
            user = it->GetInst();
        }
    }
    CreateStackMapRec(saveState, requireVregMap, user);

    codeBuilder_->EndStackMap();
    if (GetDisasm()->IsEnabled()) {
        GetDisasm()->PrintStackMap(this);
    }
}

void Codegen::CreateStackMapRec(SaveStateInst *saveState, bool requireVregMap, Inst *targetSite)
{
    bool hasInlineInfo = saveState->GetCallerInst() != nullptr;
    size_t vregsCount = 0;
    if (requireVregMap) {
        auto runtime = GetRuntime();
        if (auto caller = saveState->GetCallerInst()) {
            vregsCount = runtime->GetMethodRegistersCount(caller->GetCallMethod()) +
                         runtime->GetMethodArgumentsCount(caller->GetCallMethod());
        } else {
            vregsCount = runtime->GetMethodRegistersCount(saveState->GetMethod()) +
                         runtime->GetMethodArgumentsCount(saveState->GetMethod());
        }
        ASSERT(!GetGraph()->IsBytecodeOptimizer());
        // 1 for acc, number of env regs for dynamic method
        vregsCount += 1U + GetGraph()->GetEnvCount();
#ifndef NDEBUG
        ASSERT_PRINT(!saveState->GetInputsWereDeleted() || saveState->GetInputsWereDeletedSafely(),
                     "Some vregs were deleted from the save state");
#endif
    }

    if (auto callInst = saveState->GetCallerInst()) {
        CreateStackMapRec(callInst->GetSaveState(), requireVregMap, targetSite);
        auto method = GetGraph()->IsAotMode() ? nullptr : callInst->GetCallMethod();
        auto pandaFileIndex = GetGraph()->IsAotMode() ? GetAOTBinaryFileSnapshotIndexForInst(saveState)
                                                      : panda_file::File::INVALID_FILE_INDEX;
        codeBuilder_->BeginInlineInfo(method, GetRuntime()->GetMethodId(callInst->GetCallMethod()), saveState->GetPc(),
                                      vregsCount, pandaFileIndex);
    }

    if (requireVregMap) {
        CreateVRegMap(saveState, vregsCount, targetSite);
    }

    if (hasInlineInfo) {
        codeBuilder_->EndInlineInfo();
    }
}

void Codegen::CreateVRegMap(SaveStateInst *saveState, size_t vregsCount, Inst *targetSite)
{
    vregIndices_.clear();
    vregIndices_.resize(vregsCount, {-1, -1});
    FillVregIndices(saveState);

    ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto targetLifeNumber = la.GetInstLifeIntervals(targetSite)->GetBegin();

    for (auto &inputIndex : vregIndices_) {
        if (inputIndex.first == -1 && inputIndex.second == -1) {
            codeBuilder_->AddVReg(VRegInfo());
            continue;
        }
        if (inputIndex.second != -1) {
            auto imm = saveState->GetImmediate(inputIndex.second);
            codeBuilder_->AddConstant(imm.value, IrTypeToMetainfoType(imm.type), imm.vregType);
            continue;
        }
        ASSERT(inputIndex.first != -1);
        auto vreg = saveState->GetVirtualRegister(inputIndex.first);
        auto inputInst = saveState->GetDataFlowInput(inputIndex.first);
        auto interval = la.GetInstLifeIntervals(inputInst)->FindSiblingAt(targetLifeNumber);
        ASSERT(interval != nullptr);
        CreateVreg(interval->GetLocation(), inputInst, vreg);
    }
}

void Codegen::CreateVreg(const Location &location, Inst *inst, const VirtualRegister &vreg)
{
    switch (location.GetKind()) {
        case LocationType::FP_REGISTER:
        case LocationType::REGISTER: {
            CreateVRegForRegister(location, inst, vreg);
            break;
        }
        case LocationType::STACK_PARAMETER: {
            auto slot = location.GetValue();
            codeBuilder_->AddVReg(VRegInfo(GetFrameLayout().GetStackArgsStartSlot() - slot - CFrameSlots::Start(),
                                           VRegInfo::Location::SLOT, IrTypeToMetainfoType(inst->GetType()),
                                           vreg.GetVRegType()));
            break;
        }
        case LocationType::STACK: {
            auto slot = location.GetValue();
            if (!Is64BitsArch(GetArch())) {
                slot = ((location.GetValue() << 1U) + 1);
            }
            codeBuilder_->AddVReg(VRegInfo(GetFrameLayout().GetFirstSpillSlot() + slot, VRegInfo::Location::SLOT,
                                           IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType()));
            break;
        }
        case LocationType::IMMEDIATE: {
            ASSERT(inst->IsConst());
            auto type = inst->GetType();
            // There are no int64 type for dynamic methods.
            if (GetGraph()->IsDynamicMethod() && type == DataType::INT64) {
                type = DataType::INT32;
            }
            codeBuilder_->AddConstant(inst->CastToConstant()->GetRawValue(), IrTypeToMetainfoType(type),
                                      vreg.GetVRegType());
            break;
        }
        default:
            // Reg-to-reg spill fill must not occurs within SaveState
            UNREACHABLE();
    }
}

void Codegen::FillVregIndices(SaveStateInst *saveState)
{
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        size_t vregIndex = saveState->GetVirtualRegister(i).Value();
        if (vregIndex == VirtualRegister::BRIDGE) {
            continue;
        }
        ASSERT(vregIndex < vregIndices_.size());
        vregIndices_[vregIndex].first = i;
    }
    for (size_t i = 0; i < saveState->GetImmediatesCount(); i++) {
        auto vregImm = saveState->GetImmediate(i);
        if (vregImm.vreg == VirtualRegister::BRIDGE) {
            continue;
        }
        ASSERT(vregImm.vreg < vregIndices_.size());
        ASSERT(vregIndices_[vregImm.vreg].first == -1);
        vregIndices_[vregImm.vreg].second = i;
    }
}

void Codegen::CreateVRegForRegister(const Location &location, Inst *inst, const VirtualRegister &vreg)
{
    bool isOsr = GetGraph()->IsOsrMode();
    bool isFp = (location.GetKind() == LocationType::FP_REGISTER);
    auto regNum = location.GetValue();
    auto reg = Reg(regNum, isFp ? FLOAT64_TYPE : INT64_TYPE);
    if (!isOsr && GetRegfile()->GetZeroReg() == reg) {
        codeBuilder_->AddConstant(0, IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType());
    } else if (isOsr || GetRegfile()->IsCalleeRegister(reg)) {
        if (isFp) {
            ASSERT(inst->GetType() != DataType::REFERENCE);
            ASSERT(isOsr || regNum >= GetFirstCalleeReg(GetArch(), true));
            codeBuilder_->AddVReg(VRegInfo(regNum, VRegInfo::Location::FP_REGISTER,
                                           IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType()));
        } else {
            ASSERT(isOsr || regNum >= GetFirstCalleeReg(GetArch(), false));
            codeBuilder_->AddVReg(VRegInfo(regNum, VRegInfo::Location::REGISTER, IrTypeToMetainfoType(inst->GetType()),
                                           vreg.GetVRegType()));
        }
    } else {
        ASSERT(regNum >= GetFirstCallerReg(GetArch(), isFp));
        auto lastSlot = GetFrameLayout().GetCallerLastSlot(isFp);
        RegMask mask(GetCallerRegsMask(GetArch(), isFp));
        regNum = mask.GetDistanceFromTail(regNum);
        codeBuilder_->AddVReg(VRegInfo(lastSlot - regNum, VRegInfo::Location::SLOT,
                                       IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType()));
    }
}

void Codegen::CreateOsrEntry(SaveStateInst *saveState)
{
    auto &stub = osrEntries_.emplace_back(GetAllocator()->New<OsrEntryStub>(this, saveState));
    GetEncoder()->BindLabel(stub->GetLabel());
}

void Codegen::CallIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId id)
{
    SCOPED_DISASM_STR(this, "CallIntrinsic");
    // Call intrinsics isn't supported for IrToc, because we don't know address of intrinsics during IRToc compilation.
    // We store addresses of intrinsics in aot file in AOT mode.
    ASSERT(GetGraph()->SupportManagedCode());
    if (GetGraph()->IsAotMode()) {
        auto aotData = GetGraph()->GetAotData();
        intptr_t offset = aotData->GetEntrypointOffset(GetEncoder()->GetCursorOffset(), static_cast<int32_t>(id));
        GetEncoder()->MakeCallAot(offset);
    } else {
        GetEncoder()->MakeCall(reinterpret_cast<const void *>(GetRuntime()->GetIntrinsicAddress(
            inst->IsRuntimeCall(), GetRuntime()->GetMethodSourceLanguage(GetGraph()->GetMethod()), id)));
    }
}

bool Codegen::EmitCallRuntimeCode(Inst *inst, std::variant<EntrypointId, Reg> entrypoint)
{
    auto encoder = GetEncoder();
    if (std::holds_alternative<Reg>(entrypoint)) {
        auto reg = std::get<Reg>(entrypoint);
        ASSERT(reg.IsValid());
        encoder->MakeCall(reg);
    } else {
        auto id = std::get<EntrypointId>(entrypoint);
        ScopedTmpReg tmpReg(encoder, true);
        GetEntrypoint(tmpReg, id);
        encoder->MakeCall(tmpReg);
    }

    SaveStateInst *saveState =
        (inst == nullptr || inst->IsSaveState()) ? static_cast<SaveStateInst *>(inst) : inst->GetSaveState();
    // StackMap should follow the call as the bridge function expects retaddr points to the stackmap
    if (saveState != nullptr) {
        CreateStackMap(inst);
    }

    if (std::holds_alternative<EntrypointId>(entrypoint) &&
        GetRuntime()->IsEntrypointNoreturn(std::get<EntrypointId>(entrypoint))) {
        if constexpr (DEBUG_BUILD) {  // NOLINT
            encoder->EncodeAbort();
        }
        return false;
    }
    ASSERT(saveState == nullptr || inst->IsRuntimeCall());

    return true;
}

void Codegen::InsertCallChecker(EntrypointId checkerId, TypedImm entryId)
{
    SCOPED_DISASM_STR(this, "CallChecker");
    auto regfile = GetRegfile();
    auto saveRegs = regfile->GetCallerSavedRegMask();
    saveRegs.set(GetTarget().GetReturnRegId());
    auto saveVregs = regfile->GetCallerSavedVRegMask();
    saveVregs.set(GetTarget().GetReturnFpRegId());

    SaveCallerRegisters(saveRegs, saveVregs, false);
    FillCallParams(entryId);
    EmitCallRuntimeCode(nullptr, checkerId);
    LoadCallerRegisters(saveRegs, saveVregs, false);
}

void Codegen::SaveRegistersForImplicitRuntime(Inst *inst, RegMask *paramsMask, RegMask *mask)
{
    ASSERT(inst->GetSaveState() != nullptr);
    auto &rootsMask = inst->GetSaveState()->GetRootsRegsMask();
    for (auto i = 0U; i < inst->GetInputsCount(); i++) {
        auto input = inst->GetDataFlowInput(i);
        if (!input->IsMovableObject()) {
            continue;
        }
        auto location = inst->GetLocation(i);
        if (location.IsRegister() && location.IsRegisterValid()) {
            auto val = location.GetValue();
            auto reg = Reg(val, INT64_TYPE);
            GetEncoder()->SetRegister(mask, nullptr, reg, true);
            if (DataType::IsReference(inst->GetInputType(i)) && !rootsMask.test(val)) {
                paramsMask->set(val);
                rootsMask.set(val);
            }
        }
    }
}

void Codegen::CreateCheckForTLABWithConstSize([[maybe_unused]] Inst *inst, Reg regTlabStart, Reg regTlabSize,
                                              size_t size, LabelHolder::LabelId label)
{
    SCOPED_DISASM_STR(this, "CreateCheckForTLABWithConstSize");
    auto encoder = GetEncoder();
    if (encoder->CanEncodeImmAddSubCmp(size, WORD_SIZE, false)) {
        encoder->EncodeJump(label, regTlabSize, Imm(size), Condition::LO);
        // Change pointer to start free memory
        encoder->EncodeAdd(regTlabSize, regTlabStart, Imm(size));
    } else {
        ScopedTmpReg sizeReg(encoder);
        encoder->EncodeMov(sizeReg, Imm(size));
        encoder->EncodeJump(label, regTlabSize, sizeReg, Condition::LO);
        encoder->EncodeAdd(regTlabSize, regTlabStart, sizeReg);
    }
}

void Codegen::CreateDebugRuntimeCallsForNewObject(Inst *inst, [[maybe_unused]] Reg regTlabStart, size_t allocSize,
                                                  RegMask preserved)
{
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON)
    CallRuntime(inst, EntrypointId::ANNOTATE_SANITIZERS, INVALID_REGISTER, preserved, regTlabStart,
                TypedImm(allocSize));
#endif
    if (GetRuntime()->IsTrackTlabAlloc()) {
        CallRuntime(inst, EntrypointId::WRITE_TLAB_STATS, INVALID_REGISTER, preserved, regTlabStart,
                    TypedImm(allocSize));
    }
}

void GetOriginalInstInputs(Inst *inst, ArenaVector<Inst *> &v)
{
    switch (inst->GetOpcode()) {
        case Opcode::NullCheck:
            GetOriginalInstInputs(inst->GetInput(0).GetInst(), v);
            break;
        case Opcode::Select:
        case Opcode::SelectImm:
            GetOriginalInstInputs(inst->GetInput(0).GetInst(), v);
            GetOriginalInstInputs(inst->GetInput(1).GetInst(), v);
            break;
        case Opcode::Phi:
            for (auto &phiInput : inst->GetInputs()) {
                GetOriginalInstInputs(phiInput.GetInst(), v);
            }
            break;
        default:
            v.push_back(inst);
    }
}

bool Codegen::CheckInitClassInputsAreEqual(Inst *init)
{
    if (!init->IsOneOf(Opcode::NullCheck, Opcode::Select, Opcode::SelectImm, Opcode::Phi)) {
        return false;
    }
    // We can omit runtime call only if all inputs are of the same class,
    // see checks in CreateNewObjCall below.
    initInputInstructions_.clear();
    GetOriginalInstInputs(init, initInputInstructions_);

    auto checkOpcode = [](Opcode op) {
        // Fastpath only works for these opcodes
        return op == Opcode::LoadAndInitClass || op == Opcode::LoadImmediate;
    };

    auto checkInst = initInputInstructions_.front();
    if (!checkOpcode(checkInst->GetOpcode())) {
        return false;
    }

    auto getClass = [](Inst *inst) {
        if (inst->GetOpcode() == Opcode::LoadAndInitClass) {
            return inst->CastToLoadAndInitClass()->GetClass();
        }
        return inst->CastToLoadImmediate()->GetClass();
    };

    auto klass = getClass(checkInst);
    for (auto inst : initInputInstructions_) {
        if (!checkOpcode(inst->GetOpcode())) {
            return false;
        }
        // We don't care about opcode equality, if klass is the same
        if (klass != getClass(inst)) {
            return false;
        }
    }
    return true;
}

void Codegen::CreateNewObjCall(NewObjectInst *newObj)
{
    auto dst = ConvertRegister(newObj->GetDstReg(), newObj->GetType());
    auto src = ConvertRegister(newObj->GetSrcReg(0), newObj->GetInputType(0));
    auto initClass = newObj->GetInput(0).GetInst();
    auto srcClass = ConvertRegister(newObj->GetSrcReg(0), DataType::POINTER);
    auto runtime = GetRuntime();

    if (CheckInitClassInputsAreEqual(initClass)) {
        initClass = initClass->GetInput(0).GetInst();
    }

    auto maxTlabSize = runtime->GetTLABMaxSize();
    if (maxTlabSize == 0 ||
        (initClass->GetOpcode() != Opcode::LoadAndInitClass && initClass->GetOpcode() != Opcode::LoadImmediate)) {
        EVENT_CODEGEN("CallRuntime for NewObj");
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    RuntimeInterface::ClassPtr klass;
    if (initClass->GetOpcode() == Opcode::LoadAndInitClass) {
        klass = initClass->CastToLoadAndInitClass()->GetClass();
    } else {
        ASSERT(initClass->GetOpcode() == Opcode::LoadImmediate);
        klass = initClass->CastToLoadImmediate()->GetClass();
    }
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        EVENT_CODEGEN("CallRuntime for NewObj");
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    auto classSize = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();

    classSize = (classSize & ~(alignment - 1U)) + ((classSize % alignment) != 0U ? alignment : 0U);
    if (classSize > maxTlabSize) {
        EVENT_CODEGEN("CallRuntime for NewObj");
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    EVENT_CODEGEN("CallFastPath for NewObj");
    CallFastPath(newObj, EntrypointId::ALLOCATE_OBJECT_TLAB, dst, RegMask::GetZeroMask(), srcClass,
                 TypedImm(classSize));
}

void Codegen::CreateNewObjCallOld(NewObjectInst *newObj)
{
    auto dst = ConvertRegister(newObj->GetDstReg(), newObj->GetType());
    auto src = ConvertRegister(newObj->GetSrcReg(0), newObj->GetInputType(0));
    auto initClass = newObj->GetInput(0).GetInst();
    auto runtime = GetRuntime();
    auto maxTlabSize = runtime->GetTLABMaxSize();
    auto encoder = GetEncoder();

    if (maxTlabSize == 0 ||
        (initClass->GetOpcode() != Opcode::LoadAndInitClass && initClass->GetOpcode() != Opcode::LoadImmediate)) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    RuntimeInterface::ClassPtr klass;
    if (initClass->GetOpcode() == Opcode::LoadAndInitClass) {
        klass = initClass->CastToLoadAndInitClass()->GetClass();
    } else {
        ASSERT(initClass->GetOpcode() == Opcode::LoadImmediate);
        klass = initClass->CastToLoadImmediate()->GetClass();
    }
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    auto classSize = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();

    classSize = (classSize & ~(alignment - 1U)) + ((classSize % alignment) != 0U ? alignment : 0U);
    if (classSize > maxTlabSize) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    ScopedLiveTmpReg regTlabStart(encoder);
    ScopedLiveTmpReg regTlabSize(encoder);

    auto slowPath = CreateSlowPath<SlowPathEntrypoint>(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS);
    CreateLoadTLABInformation(regTlabStart, regTlabSize);
    CreateCheckForTLABWithConstSize(newObj, regTlabStart, regTlabSize, classSize, slowPath->GetLabel());

    RegMask preservedRegs;
    encoder->SetRegister(&preservedRegs, nullptr, src);
    CreateDebugRuntimeCallsForNewObject(newObj, regTlabStart, reinterpret_cast<size_t>(classSize), preservedRegs);

    ScopedTmpReg tlabReg(encoder);
    // Load pointer to tlab
    encoder->EncodeLdr(tlabReg, false, MemRef(ThreadReg(), runtime->GetCurrentTLABOffset(GetArch())));

    // Store pointer to the class
    encoder->EncodeStr(src, MemRef(regTlabStart, runtime->GetObjClassOffset(GetArch())));
    encoder->EncodeMov(dst, regTlabStart);
    regTlabStart.Release();
    // Store new pointer to start free memory in TLAB
    encoder->EncodeStrRelease(regTlabSize, MemRef(tlabReg, runtime->GetTLABFreePointerOffset(GetArch())));
    slowPath->BindBackLabel(encoder);
}

// CC-OFFNXT(G.FUD.05) CodeCheck yields incorrect line count
void Codegen::LoadClassFromObject(Reg classReg, Reg objReg)
{
    Reg reg = ConvertRegister(classReg.GetId(), DataType::REFERENCE);
    GetEncoder()->EncodeLdr(reg, false, MemRef(objReg, GetRuntime()->GetObjClassOffset(GetArch())));
#if defined(ARK_HYBRID) && defined(PANDA_TARGET_64)
    // The high 16 bits are used as marking flags in Ark Hybrid mode. We take the lower 48 bits as object address.
    GetEncoder()->EncodeAnd(reg, reg, Imm(0x0000'ffff'ffff'ffffULL));
#endif
}

void Codegen::CreateMultiArrayCall(CallInst *callInst)
{
    SCOPED_DISASM_STR(this, "Create Call for MultiArray");

    auto dstReg = ConvertRegister(callInst->GetDstReg(), callInst->GetType());
    auto numArgs = callInst->GetInputsCount() - 2U;  // first is class, last is save_state

    ScopedTmpReg classReg(GetEncoder());
    auto classType = ConvertDataType(DataType::REFERENCE, GetArch());
    Reg classOrig = classReg.GetReg().As(classType);
    auto location = callInst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

    ASSERT(!GetFrameInfo()->GetSpillsRelativeFp());
    GetEncoder()->EncodeMov(classOrig, ConvertRegister(location.GetValue(), DataType::REFERENCE));
    CallRuntime(callInst, EntrypointId::CREATE_MULTI_ARRAY, dstReg, RegMask::GetZeroMask(), classReg, TypedImm(numArgs),
                SpReg());
    if (callInst->GetFlag(inst_flags::MEM_BARRIER)) {
        GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void Codegen::CreateJumpToClassResolverPltShared(Inst *inst, Reg tmpReg, RuntimeInterface::EntrypointId id)
{
    auto encoder = GetEncoder();
    auto graph = GetGraph();
    auto aotData = graph->GetAotData();
    auto offset = aotData->GetSharedSlowPathOffset(id, encoder->GetCursorOffset());
    if (offset == 0 || !encoder->CanMakeCallByOffset(offset)) {
        SlowPathShared *slowPath;
        auto search = slowPathsMap_.find(id);
        if (search != slowPathsMap_.end()) {
            slowPath = search->second;
            ASSERT(slowPath->GetTmpReg().GetId() == tmpReg.GetId());
        } else {
            slowPath = CreateSlowPath<SlowPathShared>(inst, id);
            slowPath->SetTmpReg(tmpReg);
            slowPathsMap_[id] = slowPath;
        }
        encoder->MakeCall(slowPath->GetLabel());
    } else {
        encoder->MakeCallByOffset(offset);
    }
    CreateStackMap(inst);
}

void Codegen::CreateLoadClassFromPLT(Inst *inst, Reg tmpReg, Reg dst, size_t classId)
{
    auto encoder = GetEncoder();
    auto graph = GetGraph();
    auto aotData = graph->GetAotData();
    intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, false,
                                                  GetAOTBinaryFileSnapshotIndexForInst(inst));
    auto label = encoder->CreateLabel();
    ASSERT(tmpReg.GetId() != dst.GetId());
    ASSERT(inst->IsRuntimeCall());
    encoder->MakeLoadAotTableAddr(offset, tmpReg, dst);
    encoder->EncodeJump(label, dst, Condition::NE);

    // PLT Class Resolver has special calling convention:
    // First encoder temporary (tmp_reg) works as parameter and return value
    CHECK_EQ(tmpReg.GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());

    CreateJumpToClassResolverPltShared(inst, tmpReg, EntrypointId::CLASS_RESOLVER);

    encoder->EncodeMov(dst, tmpReg);
    encoder->BindLabel(label);
}

void Codegen::CreateLoadTLABInformation(Reg regTlabStart, Reg regTlabSize)
{
    SCOPED_DISASM_STR(this, "LoadTLABInformation");
    auto runtime = GetRuntime();
    // Load pointer to tlab
    GetEncoder()->EncodeLdr(regTlabSize, false, MemRef(ThreadReg(), runtime->GetCurrentTLABOffset(GetArch())));
    // Load pointer to start free memory in TLAB
    GetEncoder()->EncodeLdr(regTlabStart, false, MemRef(regTlabSize, runtime->GetTLABFreePointerOffset(GetArch())));
    // Load pointer to end free memory in TLAB
    GetEncoder()->EncodeLdr(regTlabSize, false, MemRef(regTlabSize, runtime->GetTLABEndPointerOffset(GetArch())));
    // Calculate size of the free memory
    GetEncoder()->EncodeSub(regTlabSize, regTlabSize, regTlabStart);
}

// The function alignment up the value from alignment_reg using tmp_reg.
void Codegen::CreateAlignmentValue(Reg alignmentReg, Reg tmpReg, size_t alignment)
{
    auto andVal = ~(alignment - 1U);
    // zeroed lower bits
    GetEncoder()->EncodeAnd(tmpReg, alignmentReg, Imm(andVal));
    GetEncoder()->EncodeSub(alignmentReg, alignmentReg, tmpReg);

    auto endLabel = GetEncoder()->CreateLabel();

    // if zeroed value is different, add alignment
    GetEncoder()->EncodeJump(endLabel, alignmentReg, Condition::EQ);
    GetEncoder()->EncodeAdd(tmpReg, tmpReg, Imm(alignment));

    GetEncoder()->BindLabel(endLabel);
    GetEncoder()->EncodeMov(alignmentReg, tmpReg);
}

Reg GetObjectReg(Codegen *codegen, Inst *inst)
{
    ASSERT(inst->IsCall() || inst->GetOpcode() == Opcode::ResolveVirtual || inst->GetOpcode() == Opcode::ResolveByName);
    auto location = inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto objReg = codegen->ConvertRegister(location.GetValue(), inst->GetInputType(0));
    ASSERT(objReg != INVALID_REGISTER);
    return objReg;
}

void Codegen::CreateCallIntrinsic(IntrinsicInst *inst)
{
    if (inst->HasArgumentsOnStack()) {
        // Since for some intrinsics(f.e. TimedWaitNanos) we need SaveState instruction and
        // more than two arguments, we need to place arguments on the stack, but in same time we need to
        // create boundary frame
        LOG(WARNING, COMPILER) << "Intrinsics with arguments on stack are not supported";
        GetEncoder()->SetFalseResult();
        return;
    }

    ASSERT(!HasLiveCallerSavedRegs(inst));
    if (inst->HasImms() && GetGraph()->SupportManagedCode()) {
        EncodeImms(inst->GetImms(), (inst->HasIdInput() || inst->IsMethodFirstInput()));
    }

    size_t explicitArgs;
    if (IsStackRangeIntrinsic(inst->GetIntrinsicId(), &explicitArgs)) {
        auto paramInfo = GetCallingConvention()->GetParameterInfo(explicitArgs);
        auto rangePtrReg =
            ConvertRegister(paramInfo->GetNextLocation(DataType::POINTER).GetRegister(), DataType::POINTER);
        if (inst->GetInputsCount() > explicitArgs + (inst->RequireState() ? 1U : 0U)) {
            auto rangeSpOffs = GetStackOffset(inst->GetLocation(explicitArgs));
            GetEncoder()->EncodeAdd(rangePtrReg, GetTarget().GetStackReg(), Imm(rangeSpOffs));
        }
    }
    if (inst->IsMethodFirstInput()) {
        Reg param0 = GetTarget().GetParamReg(0);
        if (GetGraph()->IsAotMode()) {
            LoadMethod(param0);
        } else {
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<uintptr_t>(inst->GetMethod())));
        }
    }
    CallIntrinsic(inst, inst->GetIntrinsicId());

    if (inst->GetSaveState() != nullptr) {
        // StackMap should follow the call as the bridge function expects retaddr points to the stackmap
        CreateStackMap(inst);
    }

    if (inst->GetType() != DataType::VOID) {
        auto arch = GetArch();
        auto returnType = inst->GetType();
        auto dstReg = ConvertRegister(inst->GetDstReg(), inst->GetType());
        auto returnReg = GetTarget().GetReturnReg(dstReg.GetType());
        // We must:
        //  sign extended INT8 and INT16 to INT32
        //  zero extended UINT8 and UINT16 to UINT32
        if (DataType::ShiftByType(returnType, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool isSigned = DataType::IsTypeSigned(returnType);
            GetEncoder()->EncodeCast(dstReg, isSigned, Reg(returnReg.GetId(), INT32_TYPE), isSigned);
        } else {
            GetEncoder()->EncodeMov(dstReg, returnReg);
        }
    }
}

void Codegen::IntfInlineCachePass(ResolveVirtualInst *resolver, Reg methodReg, Reg tmpReg, Reg objReg)
{
    // Cache structure:（offset addr)/(class addr) 32bit/32bit
    // -----------------------------------------------
    // (.aot_got)
    //     ...
    //     cache:offset/class   <----------|
    //     ...                             |
    // (.text)                             |
    // interface call start                |
    // call runtime irtoc function
    // if call class == cache.class <------|
    //    use cache.offset(method)  <------|
    // else                                |
    //    call RESOLVE_VIRTUAL_CALL_AOT    |
    //    save method‘s offset to cache >--|
    // return to (.text)
    // call method
    // -----------------------------------------------
    auto aotData = GetGraph()->GetAotData();
    uint64_t intfInlineCacheIndex = aotData->GetIntfInlineCacheIndex();
    // NOTE(liyiming): do LoadMethod in irtoc to reduce use tmp reg
    if (objReg.GetId() != tmpReg.GetId()) {
        auto regTmp64 = tmpReg.As(INT64_TYPE);
        uint64_t offset = aotData->GetInfInlineCacheSlotOffset(GetEncoder()->GetCursorOffset(), intfInlineCacheIndex);
        GetEncoder()->MakeLoadAotTableAddr(offset, regTmp64, INVALID_REGISTER);
        LoadMethod(methodReg);
        CallFastPath(resolver, EntrypointId::INTF_INLINE_CACHE, methodReg, {}, methodReg, objReg,
                     GetTypeIdImm(resolver, resolver->GetCallMethodId()), regTmp64);
    } else {
        // we don't have tmp reg here, so use x3 directly
        Reg reg3 = Reg(3U, INT64_TYPE);
        ScopedTmpRegF64 vtmp(GetEncoder());
        GetEncoder()->EncodeMov(vtmp, reg3);
        uint64_t offset = aotData->GetInfInlineCacheSlotOffset(GetEncoder()->GetCursorOffset(), intfInlineCacheIndex);
        GetEncoder()->MakeLoadAotTableAddr(offset, reg3, INVALID_REGISTER);
        LoadMethod(methodReg);
        CallFastPath(resolver, EntrypointId::INTF_INLINE_CACHE, methodReg, {}, methodReg, objReg,
                     GetTypeIdImm(resolver, resolver->GetCallMethodId()), reg3);
        GetEncoder()->EncodeMov(reg3, vtmp);
    }

    intfInlineCacheIndex++;
    aotData->SetIntfInlineCacheIndex(intfInlineCacheIndex);
}

/**
 * Returns a pointer to a caller of a virtual/static resolver.
 *
 * InstBuilder uses UnresolvedTypesInterface::AddTableSlot(method_ptr, ...) to allocate cache slots
 * for unresolved methods. The codegen uses UnresolvedTypesInterface::GetTableSlot(method_ptr, ...)
 * to get corresponding slots for unresolved methods.
 *
 * Since there is no valid method_ptr for an unresolved method at the point of IR construction
 * InstBuilder::BuildCallInst uses the pointer to the caller (GetGraph()->GetMethod()) as an argument
 * of UnresolvedTypesInterface::AddTableSlot. So the codegen has to deduce the pointer to the caller
 * and pass it to UnresolvedTypesInterface::GetTableSlot. If the caller was not inlined then
 * GetGraph()->GetMethod() works just fine, otherwise it is done by means of SaveState,
 * which is capable of remebering the corresponding caller instruction.
 *
 * To clarify the details let's consider the following sequence of calls:
 *
 * main() -> foo() -> bar()
 *
 * Suppose that foo() is a static method and bar() is an unresolved virtual method.
 *
 * 1) foo() is going to be inlined
 *
 * To inline foo() the inlining pass creates its IR by means of IRBuilder(..., caller_inst),
 * where caller_inst (the third argument) represents "CallStatic foo" instruction.
 * The caller_inst gets remembered in the corresponding SaveState created for bar's resolver
 * (see InstBuilder::CreateSaveState for the details).
 * At the same time "CallStatic foo" instruction contains a valid method pointer to foo()
 * (it can not be nullptr because we do not inline Unresolved).
 *
 * So we go through the following chain: bar's resolver->SaveState->caller_inst->pointer to the method.
 *
 * 2) foo() is not inlined
 *
 * In this case SaveState.caller_inst_ == nullptr (because foo() is not inlined).
 * Thus we just use plain GetGraph()->GetMethod().
 */
template <typename T>
RuntimeInterface::MethodPtr Codegen::GetCallerOfUnresolvedMethod(T *resolver)
{
    ASSERT(resolver->GetCallMethod() == nullptr);
    auto saveState = resolver->GetSaveState();
    ASSERT(saveState != nullptr);
    auto caller = saveState->GetCallerInst();
    auto method = (caller == nullptr ? GetGraph()->GetMethod() : caller->GetCallMethod());
    ASSERT(method != nullptr);
    return method;
}

void Codegen::EmitResolveUnknownVirtual(ResolveVirtualInst *resolver, Reg methodReg)
{
    SCOPED_DISASM_STR(this, "Create runtime call to resolve an unknown virtual method");
    ASSERT(resolver->GetOpcode() == Opcode::ResolveVirtual);
    auto method = GetCallerOfUnresolvedMethod(resolver);
    ScopedTmpReg tmpReg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
    Reg objReg = GetObjectReg(this, resolver);
    if (GetGraph()->IsAotMode()) {
        LoadMethod(methodReg);
        CallRuntime(resolver, EntrypointId::RESOLVE_VIRTUAL_CALL_AOT, methodReg, {}, methodReg, objReg,
                    GetTypeIdImm(resolver, resolver->GetCallMethodId()), TypedImm(0));
    } else {
        auto runtime = GetRuntime();
        auto utypes = runtime->GetUnresolvedTypes();
        auto skind = UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
        // Try to load vtable index
        auto methodSlotAddr = utypes->GetTableSlot(method, resolver->GetCallMethodId(), skind);
        GetEncoder()->EncodeMov(methodReg, Imm(methodSlotAddr));
        GetEncoder()->EncodeLdr(methodReg, false, MemRef(methodReg));
        // 0 means the virtual call is uninitialized or it is an interface call
        auto entrypoint = EntrypointId::RESOLVE_UNKNOWN_VIRTUAL_CALL;
        auto slowPath = CreateSlowPath<SlowPathUnresolved>(resolver, entrypoint);
        slowPath->SetUnresolvedType(method, resolver->GetCallMethodId());
        slowPath->SetDstReg(methodReg);
        slowPath->SetArgReg(objReg);
        slowPath->SetSlotAddr(methodSlotAddr);
        GetEncoder()->EncodeJump(slowPath->GetLabel(), methodReg, Condition::EQ);
        // Load klass into tmp_reg
        LoadClassFromObject(tmpReg, objReg);
        // Load from VTable, address = (klass + (index << shift)) + vtable_offset
        auto tmpReg64 = Reg(tmpReg.GetReg().GetId(), INT64_TYPE);
        GetEncoder()->EncodeAdd(tmpReg64, tmpReg, Shift(methodReg, GetVtableShift()));
        GetEncoder()->EncodeLdr(methodReg, false,
                                MemRef(tmpReg64, runtime->GetVTableOffset(GetArch()) - (1U << GetVtableShift())));
        slowPath->BindBackLabel(GetEncoder());
    }
}

void Codegen::EmitResolveVirtualAot(ResolveVirtualInst *resolver, Reg methodReg)
{
    SCOPED_DISASM_STR(this, "AOT ResolveVirtual using PLT-GOT");
    ASSERT(resolver->IsRuntimeCall());
    ScopedTmpReg tmpReg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
    auto methodReg64 = Reg(methodReg.GetId(), INT64_TYPE);
    auto tmpReg64 = Reg(tmpReg.GetReg().GetId(), INT64_TYPE);
    auto aotData = GetGraph()->GetAotData();
    intptr_t offset = aotData->GetVirtIndexSlotOffset(GetEncoder()->GetCursorOffset(), resolver->GetCallMethodId(),
                                                      GetAOTBinaryFileSnapshotIndexForInst(resolver));
    GetEncoder()->MakeLoadAotTableAddr(offset, tmpReg64, methodReg64);
    auto label = GetEncoder()->CreateLabel();
    GetEncoder()->EncodeJump(label, methodReg, Condition::NE);
    GetEncoder()->EncodeMov(methodReg64, tmpReg64);
    // PLT CallVirtual Resolver has a very special calling convention:
    //   First encoder temporary (method_reg) works as a parameter and return value
    CHECK_EQ(methodReg64.GetId(), GetTarget().GetTempRegsMask().GetMinRegister());
    ScopedTmpReg tmpReg1(GetEncoder(), true);
    GetEntrypoint(tmpReg1, EntrypointId::CALL_VIRTUAL_RESOLVER);
    GetEncoder()->MakeCall(tmpReg1);
    // Need a stackmap to build correct boundary frame
    CreateStackMap(resolver);
    GetEncoder()->BindLabel(label);
    // Load class into method_reg
    Reg objReg = GetObjectReg(this, resolver);
    LoadClassFromObject(tmpReg, objReg);
    // Load from VTable, address = (klass + (index << shift)) + vtable_offset
    GetEncoder()->EncodeAdd(methodReg, tmpReg64, Shift(methodReg, GetVtableShift()));
    GetEncoder()->EncodeLdr(methodReg, false,
                            MemRef(methodReg, GetRuntime()->GetVTableOffset(GetArch()) - (1U << GetVtableShift())));
}

void Codegen::EmitResolveVirtual(ResolveVirtualInst *resolver)
{
    auto methodReg = ConvertRegister(resolver->GetDstReg(), resolver->GetType());
    auto objectReg = ConvertRegister(resolver->GetSrcReg(0), DataType::REFERENCE);
    ScopedTmpReg tmpMethodReg(GetEncoder());
    if (resolver->GetCallMethod() == nullptr) {
        EmitResolveUnknownVirtual(resolver, tmpMethodReg);
        GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    } else if (GetRuntime()->IsInterfaceMethod(resolver->GetCallMethod())) {
        SCOPED_DISASM_STR(this, "Create runtime call to resolve a known virtual call");
        if (GetGraph()->IsAotMode()) {
#ifdef PANDA_32_BIT_MANAGED_POINTER
            if (GetArch() == Arch::AARCH64) {
                EVENT_CODEGEN("Interface cache used");
                ScopedTmpReg tmpReg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
                IntfInlineCachePass(resolver, tmpMethodReg, tmpReg, objectReg);
            } else {
#endif
                EVENT_CODEGEN("Interface cache not used");
                LoadMethod(tmpMethodReg);
                CallRuntime(resolver, EntrypointId::RESOLVE_VIRTUAL_CALL_AOT, tmpMethodReg, {}, tmpMethodReg, objectReg,
                            GetTypeIdImm(resolver, resolver->GetCallMethodId()), TypedImm(0));
#ifdef PANDA_32_BIT_MANAGED_POINTER
            }
#endif
        } else {
            CallRuntime(resolver, EntrypointId::RESOLVE_VIRTUAL_CALL, tmpMethodReg, {},
                        TypedImm(reinterpret_cast<size_t>(resolver->GetCallMethod())), objectReg);
        }
        GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    } else if (GetGraph()->IsAotNoChaMode()) {
        // ResolveVirtualAot requires method_reg to be the first tmp register.
        EmitResolveVirtualAot(resolver, tmpMethodReg);
        GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    } else {
        UNREACHABLE();
    }
}

void Codegen::EmitCallResolvedVirtual(CallInst *call)
{
    SCOPED_DISASM_STR(this, "Create a call to resolved virtual method");
    ASSERT(call->GetOpcode() == Opcode::CallResolvedVirtual);
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    ASSERT(!HasLiveCallerSavedRegs(call));
    ASSERT(call->GetCallMethod() == nullptr || GetGraph()->GetRuntime()->IsInterfaceMethod(call->GetCallMethod()) ||
           GetGraph()->IsAotNoChaMode());
    Reg methodReg = ConvertRegister(call->GetSrcReg(0), DataType::POINTER);
    Reg param0 = GetTarget().GetParamReg(0);
    // Set method
    GetEncoder()->EncodeMov(param0, methodReg);
    GetEncoder()->MakeCall(MemRef(param0, GetRuntime()->GetCompiledEntryPointOffset(GetArch())));
    FinalizeCall(call);
}

void Codegen::EmitCallVirtual(CallInst *call)
{
    SCOPED_DISASM_STR(this, "Create a call to virtual method");
    ASSERT(call->GetOpcode() == Opcode::CallVirtual);
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    auto runtime = GetRuntime();
    auto method = call->GetCallMethod();
    ASSERT(!HasLiveCallerSavedRegs(call));
    ASSERT(!call->IsUnresolved() && !runtime->IsInterfaceMethod(method) && !GetGraph()->IsAotNoChaMode());
    Reg methodReg = GetTarget().GetParamReg(0);
    ASSERT(!RegisterKeepCallArgument(call, methodReg));
    LoadClassFromObject(methodReg, GetObjectReg(this, call));
    // Get index
    auto vtableIndex = runtime->GetVTableIndex(method);
    // Load from VTable, address = klass + ((index << shift) + vtable_offset)
    auto totalOffset = runtime->GetVTableOffset(GetArch()) + (vtableIndex << GetVtableShift());
    // Class ref was loaded to method_reg
    GetEncoder()->EncodeLdr(methodReg, false, MemRef(methodReg, totalOffset));
    // Set method
    GetEncoder()->MakeCall(MemRef(methodReg, runtime->GetCompiledEntryPointOffset(GetArch())));
    FinalizeCall(call);
}

bool Codegen::EnsureParamsFitIn32Bit(std::initializer_list<std::variant<Reg, TypedImm>> params)
{
    for (auto &param : params) {
        if (std::holds_alternative<Reg>(param)) {
            auto reg = std::get<Reg>(param);
            if (reg.GetSize() > WORD_SIZE) {
                return false;
            }
        } else {
            auto immType = std::get<TypedImm>(param).GetType();
            if (immType.GetSize() > WORD_SIZE) {
                return false;
            }
        }
    }
    return true;
}

void Codegen::EmitResolveStatic(ResolveStaticInst *resolver)
{
    auto methodReg = ConvertRegister(resolver->GetDstReg(), resolver->GetType());
    if (GetGraph()->IsAotMode()) {
        LoadMethod(methodReg);
        CallRuntime(resolver, EntrypointId::GET_UNKNOWN_CALLEE_METHOD, methodReg, RegMask::GetZeroMask(), methodReg,
                    GetTypeIdImm(resolver, resolver->GetCallMethodId()), TypedImm(0));
        return;
    }
    auto method = GetCallerOfUnresolvedMethod(resolver);
    ScopedTmpReg tmp(GetEncoder());
    auto utypes = GetRuntime()->GetUnresolvedTypes();
    ASSERT(utypes != nullptr);
    auto skind = UnresolvedTypesInterface::SlotKind::METHOD;
    auto methodAddr = utypes->GetTableSlot(method, resolver->GetCallMethodId(), skind);
    GetEncoder()->EncodeMov(tmp.GetReg(), Imm(methodAddr));
    GetEncoder()->EncodeLdr(tmp.GetReg(), false, MemRef(tmp.GetReg()));
    auto slowPath = CreateSlowPath<SlowPathUnresolved>(resolver, EntrypointId::GET_UNKNOWN_CALLEE_METHOD);
    slowPath->SetUnresolvedType(method, resolver->GetCallMethodId());
    slowPath->SetDstReg(tmp.GetReg());
    slowPath->SetSlotAddr(methodAddr);
    GetEncoder()->EncodeJump(slowPath->GetLabel(), tmp.GetReg(), Condition::EQ);
    slowPath->BindBackLabel(GetEncoder());
    GetEncoder()->EncodeMov(methodReg, tmp.GetReg());
}

void Codegen::EmitCallResolvedStatic(CallInst *call)
{
    ASSERT(call->GetOpcode() == Opcode::CallResolvedStatic && call->GetCallMethod() == nullptr);
    Reg methodReg = ConvertRegister(call->GetSrcReg(0), DataType::POINTER);
    Reg param0 = GetTarget().GetParamReg(0);
    GetEncoder()->EncodeMov(param0, methodReg);
    GetEncoder()->MakeCall(MemRef(param0, GetRuntime()->GetCompiledEntryPointOffset(GetArch())));
    FinalizeCall(call);
}

void Codegen::EmitCallStatic(CallInst *call)
{
    ASSERT(call->IsStaticCall() && !call->IsUnresolved());
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    SCOPED_DISASM_STR(this, "Create a call to static");
    ASSERT(!HasLiveCallerSavedRegs(call));
    // Now MakeCallByOffset is not supported in Arch32Encoder (support ADR instruction)
    Reg param0 = GetTarget().GetParamReg(0);
    if (call->GetCallMethod() == GetGraph()->GetMethod() && GetArch() != Arch::AARCH32 && !GetGraph()->IsOsrMode() &&
        !GetGraph()->GetMethodProperties().GetHasDeopt()) {
        if (GetGraph()->IsAotMode()) {
            LoadMethod(param0);
        } else {
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())));
        }
        GetEncoder()->MakeCallByOffset(GetStartCodeOffset() - GetEncoder()->GetCursorOffset());
    } else {
        if (GetGraph()->IsAotMode()) {
            auto aotData = GetGraph()->GetAotData();
            intptr_t offset = aotData->GetPltSlotOffset(GetEncoder()->GetCursorOffset(), call->GetCallMethodId(),
                                                        GetAOTBinaryFileSnapshotIndexForInst(call));
            // PLT CallStatic Resolver transparently uses param_0 (Method) register
            GetEncoder()->MakeLoadAotTable(offset, param0);
        } else {  // usual JIT case
            auto method = call->GetCallMethod();
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<size_t>(method)));
        }
        GetEncoder()->MakeCall(MemRef(param0, GetRuntime()->GetCompiledEntryPointOffset(GetArch())));
    }
    FinalizeCall(call);
}

void Codegen::EmitCallNative(CallInst *callNative)
{
    SCOPED_DISASM_STR(this, "CallNative");
    ASSERT(GetGraph()->SupportManagedCode());
    ASSERT(!HasLiveCallerSavedRegs(callNative));

    auto nativePointerReg = ConvertRegister(callNative->GetSrcReg(0), DataType::POINTER);
    GetEncoder()->MakeCall(nativePointerReg);

    if (callNative->GetType() != DataType::VOID) {
        auto arch = GetArch();
        auto returnType = callNative->GetType();
        auto dstReg = ConvertRegister(callNative->GetDstReg(), callNative->GetType());
        auto returnReg = GetTarget().GetReturnReg(dstReg.GetType());
        // We must:
        //  sign extend INT8 and INT16 to INT32
        //  zero extend UINT8 and UINT16 to UINT32
        if (DataType::ShiftByType(returnType, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool isSigned = DataType::IsTypeSigned(returnType);
            GetEncoder()->EncodeCast(Reg(dstReg.GetId(), INT32_TYPE), isSigned, returnReg, isSigned);
        } else {
            GetEncoder()->EncodeMov(dstReg, returnReg);
        }
    }
}

void Codegen::EmitCallDynamic(CallInst *call)
{
    SCOPED_DISASM_STR(this, "Create a dynamic call");
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                        Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                        Imm(static_cast<size_t>(events::MethodEnterKind::INLINED)));
        }
#endif
        return;
    }
    RuntimeInterface *runtime = GetRuntime();
    Encoder *encoder = GetEncoder();

    auto dstReg = ConvertRegister(call->GetDstReg(), call->GetType());
    Reg methodParamReg = GetTarget().GetParamReg(CallConvDynInfo::REG_METHOD).As(GetPtrRegType());
    Reg numArgsParamReg = GetTarget().GetParamReg(CallConvDynInfo::REG_NUM_ARGS);
    auto paramFuncObjLoc = Location::MakeStackArgument(CallConvDynInfo::SLOT_CALLEE);

    ASSERT(!HasLiveCallerSavedRegs(call));

    // Load method from callee object
    static_assert(coretypes::TaggedValue::TAG_OBJECT == 0);
    encoder->EncodeLdr(methodParamReg, false, MemRef(SpReg(), GetStackOffset(paramFuncObjLoc)));
    encoder->EncodeLdr(methodParamReg, false, MemRef(methodParamReg, runtime->GetFunctionTargetOffset(GetArch())));

    ASSERT(call->GetInputsCount() > 1);
    auto numArgs = static_cast<uint32_t>(call->GetInputsCount() - 1);  // '-1' means 1 for spill fill input
    encoder->EncodeMov(numArgsParamReg, Imm(numArgs));

    size_t entryPointOffset = runtime->GetCompiledEntryPointOffset(GetArch());
    encoder->MakeCall(MemRef(methodParamReg, entryPointOffset));

    CreateStackMap(call);
    // Dynamic callee may have moved sp if there was insufficient num_actual_args
    encoder->EncodeSub(
        SpReg(), FpReg(),
        Imm(GetFrameLayout().GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::BYTES>(0)));

    if (dstReg.IsValid()) {
        Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
        encoder->EncodeMov(dstReg, retReg);
    }
}

void Codegen::FinalizeCall(CallInst *call)
{
    ASSERT(!call->IsDynamicCall());
    CreateStackMap(call);
    auto returnType = call->GetType();
    auto dstReg = ConvertRegister(call->GetDstReg(), returnType);
    // Restore frame pointer in the TLS
    GetEncoder()->EncodeStr(FpReg(), MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));
    // Sign/Zero extend return_reg if necessary
    if (dstReg.IsValid()) {
        auto arch = GetArch();
        auto returnReg = GetTarget().GetReturnReg(dstReg.GetType());
        //  INT8  and INT16  must be sign extended to INT32
        //  UINT8 and UINT16 must be zero extended to UINT32
        if (DataType::ShiftByType(returnType, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool isSigned = DataType::IsTypeSigned(returnType);
            GetEncoder()->EncodeCast(dstReg, isSigned, Reg(returnReg.GetId(), INT32_TYPE), isSigned);
        } else {
            GetEncoder()->EncodeMov(dstReg, returnReg);
        }
    }
}

void Codegen::MemRefToOffset(Reg offsetReg, MemRef mem)
{
    constexpr unsigned HAS_INDEX = 4;  // CC-OFF(G.NAM.03-CPP) project code style
    constexpr unsigned HAS_SCALE = 2;  // CC-OFF(G.NAM.03-CPP) project code style
    constexpr unsigned HAS_DISP = 1;   // CC-OFF(G.NAM.03-CPP) project code style
    constexpr unsigned NONE = 0;       // CC-OFF(G.NAM.03-CPP) project code style

    unsigned caseIndex = HAS_INDEX * static_cast<unsigned>(mem.HasIndex()) +
                         HAS_SCALE * static_cast<unsigned>(mem.HasScale()) +
                         HAS_DISP * static_cast<unsigned>(mem.HasDisp());
    switch (caseIndex) {
        case NONE:
        case HAS_DISP:
        case HAS_SCALE:
        case HAS_SCALE | HAS_DISP:
            enc_->EncodeMov(offsetReg, Imm(mem.GetDisp()));
            break;
        case HAS_INDEX:
            enc_->EncodeMov(offsetReg, mem.GetIndex());
            break;
        case HAS_INDEX | HAS_DISP:
            enc_->EncodeAdd(offsetReg, mem.GetIndex(), Imm(mem.GetDisp()));
            break;
        case HAS_INDEX | HAS_SCALE:
        case HAS_INDEX | HAS_SCALE | HAS_DISP:
            enc_->EncodeShl(offsetReg, mem.GetIndex(), Imm(mem.GetScale()));
            if ((caseIndex & HAS_DISP) != 0) {
                enc_->EncodeAdd(offsetReg, offsetReg, Imm(mem.GetDisp()));
            }
            break;
        default:
            UNREACHABLE();
    }
}

template <bool IS_CLASS>
void Codegen::CreatePreWRB(Inst *inst, MemRef mem, RegMask preserved, bool storePair)
{
    auto runtime = GetRuntime();
    auto *enc = GetEncoder();
    if (!runtime->NeedsPreWriteBarrier()) {
        return;
    }

    SCOPED_DISASM_STR(this, "Pre WRB");
    ScopedTmpReg entrypointReg(enc, enc->IsLrAsTempRegEnabledAndReleased());
    GetEncoder()->EncodeLdr(entrypointReg, false,
                            MemRef(ThreadReg(), runtime->GetTlsPreWrbEntrypointOffset(GetArch())));

    // Check entrypoint address
    auto label = GetEncoder()->CreateLabel();
    enc->EncodeJump(label, entrypointReg, Condition::EQ);
    auto refType =
        inst->GetType() == DataType::REFERENCE ? DataType::GetIntTypeForReference(enc->GetArch()) : DataType::INT64;
    ScopedTmpReg tmpRef(enc, ConvertDataType(refType, GetArch()));
    auto prevOffset = enc->GetCursorOffset();
    // Load old value
    if (IsVolatileMemInst(inst)) {
        enc->EncodeLdrAcquire(tmpRef, false, mem);
    } else {
        enc->EncodeLdr(tmpRef, false, mem);
    }
    TryInsertImplicitNullCheck(inst, prevOffset);
    if constexpr (IS_CLASS) {
        enc->EncodeLdr(tmpRef, false, MemRef(tmpRef, runtime->GetManagedClassOffset(GetArch())));
    } else {
        CheckObject(tmpRef, label);
    }
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);
    live_regs |= preserved;
    CallBarrier(live_regs, live_vregs, entrypointReg.GetReg(), INVALID_REGISTER, tmpRef);

    if (storePair) {
        // store pair doesn't support index and scalar
        ASSERT(!mem.HasIndex() && !mem.HasScale());
        // calculate offset for second store
        // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
        auto secondOffset = static_cast<ssize_t>(1U << DataType::ShiftByType(DataType::REFERENCE, enc->GetArch()));
        if (mem.HasDisp()) {
            secondOffset += mem.GetDisp();
        }
        // Load old value
        if (IsVolatileMemInst(inst)) {
            enc->EncodeLdrAcquire(tmpRef, false, MemRef(mem.GetBase(), secondOffset));
        } else {
            enc->EncodeLdr(tmpRef, false, MemRef(mem.GetBase(), secondOffset));
        }
        CheckObject(tmpRef, label);
        CallBarrier(live_regs, live_vregs, entrypointReg.GetReg(), INVALID_REGISTER, tmpRef);
    }
    enc->BindLabel(label);
}

// Force template instantiation to avoid "undefined reference" error from ld.
template void Codegen::CreatePreWRB<true>(Inst *inst, MemRef mem, RegMask preserved, bool storePair);
template void Codegen::CreatePreWRB<false>(Inst *inst, MemRef mem, RegMask preserved, bool storePair);

/* RegMask 'preserved' is a way to explicitly specify registers that must be
 * saved before and restored after the PostWRB call.
 *
 * If codegen emits some new instructions after the PostWRB call and these
 * instructions use 'inst' operand registers, then the registers should live
 * through the PostWRB call. The problem is that the liveness analysis can't
 * detect if those registers are live because there are no instructions using
 * them at the moment.
 *
 * Note, only CALLER_REG_MASK registers are taken into account.
 */

void Codegen::CreateCmcPostWRBCall(Inst *inst, MemRef mem, Reg srcReg, RegMask preserved)
{
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);
    live_regs |= preserved;

    if (!mem.HasIndex()) {
        CallBarrier(live_regs, live_vregs, EntrypointId::CMC_POST_WRITE_BARRIER, INVALID_REGISTER, mem.GetBase(),
                    TypedImm(mem.GetDisp()), srcReg);
        return;
    }
    ScopedTmpReg offset(GetEncoder());
    MemRefToOffset(offset, mem);
    CallBarrier(live_regs, live_vregs, EntrypointId::CMC_POST_WRITE_BARRIER, INVALID_REGISTER, mem.GetBase(), offset,
                srcReg);
}

void Codegen::CreateCmcPostWRBCallWithPair(Inst *inst, MemRef mem, Reg srcReg1, Reg srcReg2, RegMask preserved)
{
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);
    live_regs |= preserved;

    ASSERT(!mem.HasIndex());
    CallBarrier(live_regs, live_vregs, EntrypointId::CMC_POST_WRITE_PAIR_BARRIER, INVALID_REGISTER, mem.GetBase(),
                TypedImm(mem.GetDisp()), srcReg1, srcReg2);
}

void Codegen::CreateCmcPostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2, RegMask preserved)
{
    SCOPED_DISASM_STR(this, "Post-write barrier with CMC-GC");
    if (reg2.IsValid()) {
        CreateCmcPostWRBCallWithPair(inst, mem, reg1, reg2, preserved);
    } else {
        CreateCmcPostWRBCall(inst, mem, reg1, preserved);
    }
}

void Codegen::CreatePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2, RegMask preserved)
{
    ASSERT(reg1 != INVALID_REGISTER);
    auto barrierType = GetRuntime()->GetPostType();
    if (!GetGraph()->SupportsIrtocBarriers() || !GetGraph()->IsOfflineCompilationMode()) {
        if (barrierType == ark::mem::BarrierType::POST_WRB_NONE) {
            return;
        }
        ASSERT(barrierType == ark::mem::BarrierType::POST_INTERREGION_BARRIER ||
               barrierType == ark::mem::BarrierType::POST_CMC_WRITE_BARRIER);
    }
    if (barrierType == ark::mem::BarrierType::POST_CMC_WRITE_BARRIER) {
        CreateCmcPostWRB(inst, mem, reg1, reg2, preserved);
        return;
    }
    // For dynamic methods, another check
    if (GetGraph()->IsDynamicMethod()) {
        CreatePostWRBForDynamicImpl(inst, mem, reg1, reg2, preserved);
    } else {
        CreatePostWRBImpl(inst, mem, reg1, reg2, preserved);
    }
}

void Codegen::CreatePostWRBImpl(Inst *inst, MemRef mem, Reg reg1, Reg reg2, RegMask preserved)
{
    SCOPED_DISASM_STR(this, "Post-write barrier with G1-GC");
    PostWriteBarrier pwb(this, inst);
    Inst *secondValue;
    Inst *val = InstStoredValue(inst, &secondValue);
    ASSERT(val != nullptr);
    ASSERT(secondValue == nullptr || reg2 != INVALID_REGISTER);
    if (val->GetOpcode() == Opcode::NullPtr) {
        // We can don't encode Post barrier for nullptr
        if (secondValue == nullptr || secondValue->GetOpcode() == Opcode::NullPtr) {
            return;
        }
        // CallPostWRB only for second reg
        auto secondMemOffset = static_cast<ssize_t>(reg1.GetSize() / BITS_PER_BYTE);
        if (!mem.HasIndex()) {
            MemRef secondMem(mem.GetBase(), mem.GetDisp() + secondMemOffset);
            pwb.Encode(secondMem, reg2, INVALID_REGISTER, !IsInstNotNull(secondValue), preserved);
            return;
        }
        ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
        ASSERT(GetTarget().GetTempRegsMask().Test(mem.GetIndex().GetId()));
        GetEncoder()->EncodeAdd(mem.GetIndex(), mem.GetIndex(), Imm(secondMemOffset));
        pwb.Encode(mem, reg2, INVALID_REGISTER, !IsInstNotNull(secondValue), preserved);
        return;
    }
    // Create PostWRB only for first value
    if (secondValue != nullptr && secondValue->GetOpcode() == Opcode::NullPtr) {
        reg2 = INVALID_REGISTER;
    }
    bool checkObject = true;
    if (reg2 == INVALID_REGISTER) {
        if (IsInstNotNull(val)) {
            checkObject = false;
        }
    } else {
        if (IsInstNotNull(val) && IsInstNotNull(secondValue)) {
            checkObject = false;
        }
    }
    pwb.Encode(mem, reg1, reg2, checkObject, preserved);
}

void Codegen::CreatePostWRBForDynamicImpl(Inst *inst, MemRef mem, Reg reg1, Reg reg2, RegMask preserved)
{
    SCOPED_DISASM_STR(this, "Post-write barrier for dynamic with G1-GC");
    PostWriteBarrier pwb(this, inst);
    if (reg2 == INVALID_REGISTER) {
        int storeIndex;
        if (inst->GetOpcode() == Opcode::StoreObject || inst->GetOpcode() == Opcode::StoreI ||
            inst->GetOpcode() == Opcode::StoreArrayI) {
            storeIndex = 1_I;
        } else {
            ASSERT(inst->GetOpcode() == Opcode::StoreArray || inst->GetOpcode() == Opcode::Store);
            storeIndex = 2_I;
        }
        if (StoreValueCanBeObject(inst->GetDataFlowInput(storeIndex))) {
            pwb.Encode(mem, reg1, reg2, true, preserved);
        }
    } else {
        int storeIndex;
        if (inst->GetOpcode() == Opcode::StoreArrayPairI) {
            storeIndex = 1_I;
        } else {
            ASSERT(inst->GetOpcode() == Opcode::StoreArrayPair);
            storeIndex = 2_I;
        }
        bool firstIsObject = StoreValueCanBeObject(inst->GetDataFlowInput(storeIndex));
        bool secondIsObject = StoreValueCanBeObject(inst->GetDataFlowInput(storeIndex + 1));
        if (firstIsObject || secondIsObject) {
            if (firstIsObject && !secondIsObject) {
                reg2 = INVALID_REGISTER;
            } else if (!firstIsObject && secondIsObject) {
                reg1 = reg2;
                reg2 = INVALID_REGISTER;
            }
            pwb.Encode(mem, reg1, reg2, true, preserved);
        }
    }
}

void Codegen::CreateCmcReadViaBarrierCall(Inst *inst, MemRef mem, Reg dstReg, bool isVolatile, RegMask preserved)
{
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);
    live_regs |= preserved;
    auto entrypointId = isVolatile ? EntrypointId::CMC_ATOMIC_READ_VIA_BARRIER : EntrypointId::CMC_READ_VIA_BARRIER;

    if (!mem.HasIndex()) {
        CallBarrier(live_regs, live_vregs, entrypointId, dstReg, mem.GetBase(), TypedImm(mem.GetDisp()));
    } else if (dstReg.GetId() != mem.GetBase().GetId()) {
        MemRefToOffset(dstReg /* as offset */, mem);
        CallBarrier(live_regs, live_vregs, entrypointId, dstReg, mem.GetBase(), dstReg /* as offset */);
    } else {
        ScopedTmpReg offset(GetEncoder());
        MemRefToOffset(offset, mem);
        CallBarrier(live_regs, live_vregs, entrypointId, dstReg, mem.GetBase(), offset);
    }
}

void Codegen::CreateReadViaBarrier(Inst *inst, MemRef mem, Reg dstReg, bool isVolatile, RegMask preserved)
{
    ASSERT(inst->GetType() == DataType::REFERENCE || inst->GetType() == DataType::ANY);
    if (!GetRuntime()->NeedsPreReadBarrier()) {
        // Fallback to load without barrier
        auto type = inst->GetType();
        if (isVolatile) {
            GetEncoder()->EncodeLdrAcquire(dstReg, IsTypeSigned(type), mem);
        } else {
            GetEncoder()->EncodeLdr(dstReg, IsTypeSigned(type), mem);
        }
        return;
    }
    ASSERT(GetRuntime()->GetPreReadType() == ark::mem::BarrierType::PRE_CMC_READ_BARRIER);
    SCOPED_DISASM_STR(this, "Load via CMC-GC read barrier.");
    CreateCmcReadViaBarrierCall(inst, mem, dstReg, isVolatile, preserved);
}

void Codegen::CreateReadPairViaBarrier(Inst *inst, MemRef mem, Reg dstReg1, Reg dstReg2, RegMask preserved)
{
    ASSERT(dstReg1.IsValid() && dstReg2.IsValid());
    ASSERT(dstReg1.GetId() != dstReg2.GetId());

    if (!GetRuntime()->NeedsPreReadBarrier()) {
        // Fallback to load without barrier
        GetEncoder()->EncodeLdp(dstReg1, dstReg2, IsTypeSigned(inst->GetType()), mem);
        return;
    }
    ASSERT(GetRuntime()->GetPreReadType() == ark::mem::BarrierType::PRE_CMC_READ_BARRIER);
    SCOPED_DISASM_STR(this, "Load pair via CMC-GC read barrier.");

    auto callBarrier1st = [this, inst, mem, dstReg1, preserved]() {
        CreateCmcReadViaBarrierCall(inst, mem, dstReg1, false, preserved);
    };
    auto callBarrier2nd = [this, inst, mem, dstReg2, preserved]() {
        auto secondDisp = mem.GetDisp() + (1U << DataType::ShiftByType(DataType::REFERENCE, GetArch()));
        auto secondMem = MemRef(mem.GetBase(), mem.GetIndex(), mem.GetScale(), secondDisp);
        CreateCmcReadViaBarrierCall(inst, secondMem, dstReg2, false, preserved);
    };
    // We need to flip the order when destination registers overlap with mem.GetBase().
    if (dstReg1.GetId() == mem.GetBase().GetId()) {
        callBarrier2nd();
        callBarrier1st();
    } else {
        callBarrier1st();
        callBarrier2nd();
    }
}

void Codegen::CheckObject(Reg reg, LabelHolder::LabelId label)
{
    auto graph = GetGraph();
    auto *enc = GetEncoder();

    // interpreter use x20 in IrToc and we don't have enough temporary registers
    // remove after PR 98 or PR 47
    if (graph->IsDynamicMethod()) {
        ASSERT(reg.IsScalar());
        reg = reg.As(INT64_TYPE);
        auto tagMask = graph->GetRuntime()->GetTaggedTagMask();
        // Check that the value is object(not int and not double)
        enc->EncodeJumpTest(label, reg, Imm(tagMask), Condition::TST_NE);
        auto specialMask = graph->GetRuntime()->GetTaggedSpecialMask();
        // Check that the value is not special value
        enc->EncodeJumpTest(label, reg, Imm(~specialMask), Condition::TST_EQ);
    } else {
        enc->EncodeJump(label, reg, Condition::EQ);
    }
}

bool Codegen::HasLiveCallerSavedRegs(Inst *inst)
{
    auto [live_regs, live_fp_regs] = GetLiveRegisters<false>(inst);
    live_regs &= GetCallerRegsMask(GetArch(), false);
    live_fp_regs &= GetCallerRegsMask(GetArch(), true);
    return live_regs.Any() || live_fp_regs.Any();
}

void Codegen::SaveCallerRegisters(RegMask liveRegs, VRegMask liveVregs, bool adjustRegs)
{
    SCOPED_DISASM_STR(this, "Save caller saved regs");
    auto base = GetFrameInfo()->GetCallersRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    liveRegs &= ~GetEncoder()->GetAvailableScratchRegisters();
    liveVregs &= ~GetEncoder()->GetAvailableScratchFpRegisters();
    if (adjustRegs) {
        liveRegs &= GetRegfile()->GetCallerSavedRegMask();
        liveVregs &= GetRegfile()->GetCallerSavedVRegMask();
    } else {
        liveRegs &= GetCallerRegsMask(GetArch(), false);
        liveVregs &= GetCallerRegsMask(GetArch(), true);
    }
    if (GetFrameInfo()->GetPushCallers()) {
        GetEncoder()->PushRegisters(liveRegs, liveVregs);
    } else {
        GetEncoder()->SaveRegisters(liveRegs, false, GetFrameInfo()->GetCallersOffset(), base,
                                    GetCallerRegsMask(GetArch(), false));
        GetEncoder()->SaveRegisters(liveVregs, true, GetFrameInfo()->GetFpCallersOffset(), base,
                                    GetFrameInfo()->GetPositionedCallers() ? GetCallerRegsMask(GetArch(), true)
                                                                           : RegMask());
    }
}

void Codegen::LoadCallerRegisters(RegMask liveRegs, VRegMask liveVregs, bool adjustRegs)
{
    SCOPED_DISASM_STR(this, "Restore caller saved regs");
    auto base = GetFrameInfo()->GetCallersRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    liveRegs &= ~GetEncoder()->GetAvailableScratchRegisters();
    liveVregs &= ~GetEncoder()->GetAvailableScratchFpRegisters();
    if (adjustRegs) {
        liveRegs &= GetRegfile()->GetCallerSavedRegMask();
        liveVregs &= GetRegfile()->GetCallerSavedVRegMask();
    } else {
        liveRegs &= GetCallerRegsMask(GetArch(), false);
        liveVregs &= GetCallerRegsMask(GetArch(), true);
    }
    if (GetFrameInfo()->GetPushCallers()) {
        GetEncoder()->PopRegisters(liveRegs, liveVregs);
    } else {
        GetEncoder()->LoadRegisters(liveRegs, false, GetFrameInfo()->GetCallersOffset(), base,
                                    GetCallerRegsMask(GetArch(), false));
        GetEncoder()->LoadRegisters(liveVregs, true, GetFrameInfo()->GetFpCallersOffset(), base,
                                    GetCallerRegsMask(GetArch(), true));
    }
}

bool Codegen::RegisterKeepCallArgument(CallInst *callInst, Reg reg)
{
    for (auto i = 0U; i < callInst->GetInputsCount(); i++) {
        auto location = callInst->GetLocation(i);
        if (location.IsRegister() && location.GetValue() == reg.GetId()) {
            return true;
        }
    }
    return false;
}

void Codegen::LoadMethod(Reg dst)
{
    ASSERT((CFrameMethod::GetOffsetFromFpInBytes(GetFrameLayout()) -
            (GetFrameLayout().GetMethodOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::BYTES>())) ==
           0);
    auto fpOffset = CFrameMethod::GetOffsetFromFpInBytes(GetFrameLayout());
    auto mem = MemRef(FpReg(), -fpOffset);
    GetEncoder()->EncodeLdr(dst, false, mem);
}

void Codegen::StoreFreeSlot(Reg src)
{
    ASSERT(src.GetSize() <= (GetFrameLayout().GetSlotSize() << 3U));
    auto fpOffset =
        GetFrameLayout().GetFreeSlotOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::BYTES>();
    auto mem = MemRef(FpReg(), -fpOffset);
    GetEncoder()->EncodeStr(src, mem);
}

void Codegen::LoadFreeSlot(Reg dst)
{
    ASSERT(dst.GetSize() <= (GetFrameLayout().GetSlotSize() << 3U));
    auto fpOffset =
        GetFrameLayout().GetFreeSlotOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::BYTES>();
    auto mem = MemRef(FpReg(), -fpOffset);
    GetEncoder()->EncodeLdr(dst, false, mem);
}

void Codegen::CreateReturn(const Inst *inst)
{
    if (GetGraph()->GetMethodProperties().GetLastReturn() == inst) {
        GetEncoder()->BindLabel(GetLabelExit());
        GenerateEpilogue();
    } else {
        GetEncoder()->EncodeJump(GetLabelExit());
    }
}

void Codegen::EncodeDynamicCast(Inst *inst, Reg dst, bool dstSigned, Reg src)
{
    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_GE(dst.GetSize(), BITS_PER_UINT32);

    bool isDst64 {dst.GetSize() == BITS_PER_UINT64};
    dst = dst.As(INT32_TYPE);

    auto enc {GetEncoder()};
    if (g_options.IsCpuFeatureEnabled(CpuFeature::JSCVT)) {
        // no slow path intended
        enc->EncodeFastPathDynamicCast(dst, src, LabelHolder::INVALID_LABEL);
    } else {
        auto slowPath {CreateSlowPath<SlowPathJsCastDoubleToInt32>(inst)};
        slowPath->SetDstReg(dst);
        slowPath->SetSrcReg(src);

        enc->EncodeFastPathDynamicCast(dst, src, slowPath->GetLabel());
        slowPath->BindBackLabel(enc);
    }

    if (isDst64) {
        enc->EncodeCast(dst.As(INT64_TYPE), dstSigned, dst, dstSigned);
    }
}

// Next visitors use calling convention
void Codegen::VisitCallIndirect(CallIndirectInst *inst)
{
    auto location = inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto src = Reg(location.GetValue(), GetTarget().GetPtrRegType());
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());

    GetEncoder()->MakeCall(src);
    if (inst->HasUsers()) {
        GetEncoder()->EncodeMov(dst, GetTarget().GetReturnReg(dst.GetType()));
    }
}

void Codegen::VisitCall(CallInst *inst)
{
    ASSERT(GetGraph()->GetRelocationHandler() != nullptr);

    auto mode = GetGraph()->GetMode();
    ASSERT(mode.IsFastPath() || mode.IsInterpreter() || mode.IsNative() || mode.IsNativePlus());
    ASSERT(mode.IsFastPath() || !HasLiveCallerSavedRegs(inst));

    RegMask callerRegs;
    RegMask callerFpRegs;
    auto dstReg = ConvertRegister(inst->GetDstReg(), inst->GetType());

    if (mode.IsFastPath()) {
        // irtoc fastpath needs to save all caller registers in case of call native function
        callerRegs = GetCallerRegsMask(GetArch(), false);
        callerFpRegs = GetCallerRegsMask(GetArch(), true);
        GetEncoder()->SetRegister(&callerRegs, &callerFpRegs, dstReg, false);
        SaveCallerRegisters(callerRegs, callerFpRegs, false);
    }

    RelocationInfo relocation;
    relocation.data = inst->GetCallMethodId();
    GetEncoder()->MakeCall(&relocation);
    GetGraph()->GetRelocationHandler()->AddRelocation(relocation);

    if (inst->HasUsers()) {
        ASSERT(dstReg.IsValid());
        GetEncoder()->EncodeMov(dstReg, GetTarget().GetReturnReg(dstReg.GetType()));
    }

    if (mode.IsFastPath()) {
        LoadCallerRegisters(callerRegs, callerFpRegs, false);
    }
}

static void GetEntryPointId(uint64_t elementSize, RuntimeInterface::EntrypointId &eid)
{
    switch (elementSize) {
        case sizeof(uint8_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB8;
            break;
        case sizeof(uint16_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB16;
            break;
        case sizeof(uint32_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB32;
            break;
        case sizeof(uint64_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB64;
            break;
        default:
            UNREACHABLE();
    }
}

void Codegen::VisitNewArray(Inst *inst)
{
    auto method = inst->CastToNewArray()->GetMethod();
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto srcClass = ConvertRegister(inst->GetSrcReg(NewArrayInst::INDEX_CLASS), DataType::POINTER);
    auto srcSize = ConvertRegister(inst->GetSrcReg(NewArrayInst::INDEX_SIZE), DataType::Type::INT32);
    auto arrayType = inst->CastToNewArray()->GetTypeId();
    auto runtime = GetGraph()->GetRuntime();
    auto encoder = GetEncoder();

    auto maxTlabSize = runtime->GetTLABMaxSize();
    // NOTE(msherstennikov): support NewArray fast path for arm32
    if (maxTlabSize == 0 || GetArch() == Arch::AARCH32) {
        CallRuntime(inst, EntrypointId::CREATE_ARRAY, dst, RegMask::GetZeroMask(), srcClass, srcSize);
        if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
            encoder->EncodeMemoryBarrier(memory_order::RELEASE);
        }
        return;
    }

    auto lenInst = inst->GetDataFlowInput(0);
    auto classArraySize = runtime->GetClassArraySize(GetArch());
    uint64_t arraySize = 0;
    uint64_t elementSize = runtime->GetArrayElementSize(method, arrayType);
    uint64_t alignment = runtime->GetTLABAlignment();
    if (lenInst->GetOpcode() == Opcode::Constant) {
        ASSERT(lenInst->GetType() == DataType::INT64);
        arraySize = lenInst->CastToConstant()->GetIntValue() * elementSize + classArraySize;
        arraySize = (arraySize & ~(alignment - 1U)) + ((arraySize % alignment) != 0U ? alignment : 0U);
        if (arraySize > maxTlabSize) {
            CallRuntime(inst, EntrypointId::CREATE_ARRAY, dst, RegMask::GetZeroMask(), srcClass, srcSize);
            if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
                encoder->EncodeMemoryBarrier(memory_order::RELEASE);
            }
            return;
        }
    }

    EntrypointId eid;
    GetEntryPointId(elementSize, eid);
    CallFastPath(inst, eid, dst, RegMask::GetZeroMask(), srcClass, srcSize);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        encoder->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void Codegen::CreateMonitorCall(MonitorInst *inst)
{
    auto src = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto id = inst->IsExit() ? EntrypointId::MONITOR_EXIT_FAST_PATH : EntrypointId::MONITOR_ENTER_FAST_PATH;
    CallFastPath(inst, id, INVALID_REGISTER, RegMask::GetZeroMask(), src);
}

void Codegen::CreateMonitorCallOld(MonitorInst *inst)
{
    auto src = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto id = inst->IsExit() ? EntrypointId::UNLOCK_OBJECT : EntrypointId::LOCK_OBJECT;
    CallRuntime(inst, id, dst, RegMask::GetZeroMask(), src);
}

void Codegen::CreateCheckCastInterfaceCall(Inst *inst)
{
    auto obj = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto interface = ConvertRegister(inst->GetSrcReg(SECOND_OPERAND), DataType::REFERENCE);
    CallFastPath(inst, EntrypointId::CHECK_CAST_INTERFACE, INVALID_REGISTER, RegMask::GetZeroMask(), obj, interface);
}

void Codegen::TryInsertImplicitNullCheck(Inst *inst, size_t prevOffset)
{
    if (!IsSuitableForImplicitNullCheck(inst)) {
        return;
    }
    if (!inst->CanThrow()) {
        return;
    }

    auto nullcheck = inst->GetInput(0).GetInst();
    ASSERT(nullcheck->GetOpcode() == Opcode::NullCheck && nullcheck->CastToNullCheck()->IsImplicit());
    auto currOffset = GetEncoder()->GetCursorOffset();
    ASSERT(currOffset > prevOffset);
    GetCodeBuilder()->AddImplicitNullCheck(currOffset, currOffset - prevOffset);
    CreateStackMap(nullcheck, inst);
}

void Codegen::CreateMemmoveUnchecked([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    auto entrypointId = EntrypointId::INVALID;
    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_1_BYTE:
            entrypointId = EntrypointId::ARRAY_COPY_TO_UNCHECKED_1_BYTE;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_2_BYTE:
            entrypointId = EntrypointId::ARRAY_COPY_TO_UNCHECKED_2_BYTE;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_4_BYTE:
            entrypointId = EntrypointId::ARRAY_COPY_TO_UNCHECKED_4_BYTE;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_8_BYTE:
            entrypointId = EntrypointId::ARRAY_COPY_TO_UNCHECKED_8_BYTE;
            break;

        default:
            UNREACHABLE();
            break;
    }
    ASSERT(entrypointId != EntrypointId::INVALID);
    auto srcObj = src[FIRST_OPERAND];
    auto dstObj = src[SECOND_OPERAND];
    auto dstStart = src[THIRD_OPERAND];
    auto srcStart = src[FOURTH_OPERAND];
    auto srcEnd = src[FIFTH_OPERAND];
    CallFastPath(inst, entrypointId, INVALID_REGISTER, RegMask::GetZeroMask(), srcObj, dstObj, dstStart, srcStart,
                 srcEnd);
}

void Codegen::CreateFloatIsInf([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeIsInf(dst, src[0]);
}

void Codegen::CreateStringEquals([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    CallFastPath(inst, EntrypointId::STRING_EQUALS, dst, {}, src[0], src[1U]);
}

void Codegen::CreateMathCeil([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeCeil(dst, src[0]);
}

void Codegen::CreateMathFloor([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeFloor(dst, src[0]);
}

void Codegen::CreateCountLeadingZeroBits([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeCountLeadingZeroBits(Reg(dst.GetId(), src[0].GetType()), src[0]);
}

void Codegen::CreateStringBuilderChar(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    CallFastPath(inst, EntrypointId::STRING_BUILDER_CHAR, dst, RegMask::GetZeroMask(), src[0], src[1U]);
}

void Codegen::CreateStringBuilderBool(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    CallFastPath(inst, EntrypointId::STRING_BUILDER_BOOL, dst, RegMask::GetZeroMask(), src[0], src[1U]);
}

void Codegen::CreateStringBuilderString(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    auto entrypointId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_BUILDER_STRING_COMPRESSED
                                                                   : EntrypointId::STRING_BUILDER_STRING;

    CallFastPath(inst, entrypointId, dst, RegMask::GetZeroMask(), src[0], src[1U]);
}

void Codegen::CreateStringFromCharArrayTlab(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    CreateStringFromCharArrayTlab(static_cast<Inst *>(inst), dst, src);
}

void Codegen::CreateStringFromCharArrayTlab(Inst *inst, Reg dst, SRCREGS src)
{
    auto runtime = GetGraph()->GetRuntime();
    auto offset = src[FIRST_OPERAND];
    auto count = src[SECOND_OPERAND];
    auto array = src[THIRD_OPERAND];

    auto entryId = GetRuntime()->IsCompressedStringsEnabled()
                       ? EntrypointId::CREATE_STRING_FROM_CHAR_ARRAY_TLAB_COMPRESSED
                       : EntrypointId::CREATE_STRING_FROM_CHAR_ARRAY_TLAB;

    if (GetRegfile()->GetZeroReg().GetId() == offset.GetId()) {
        entryId = GetRuntime()->IsCompressedStringsEnabled()
                      ? EntrypointId::CREATE_STRING_FROM_ZERO_BASED_CHAR_ARRAY_TLAB_COMPRESSED
                      : EntrypointId::CREATE_STRING_FROM_ZERO_BASED_CHAR_ARRAY_TLAB;
    }

    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(klassReg, false,
                                MemRef(ThreadReg(), runtime->GetStringClassPointerTlsOffset(GetArch())));
        if (GetRegfile()->GetZeroReg().GetId() == offset.GetId()) {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), count, array, klassReg);
        } else {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), offset, count, array, klassReg);
        }
    } else {
        auto klassImm =
            TypedImm(reinterpret_cast<uintptr_t>(runtime->GetLineStringClass(GetGraph()->GetMethod(), nullptr)));
        if (GetRegfile()->GetZeroReg().GetId() == offset.GetId()) {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), count, array, klassImm);
        } else {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), offset, count, array, klassImm);
        }
    }
}

void Codegen::CreateStringFromStringTlab(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entryId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::CREATE_STRING_FROM_STRING_TLAB_COMPRESSED
                                                              : EntrypointId::CREATE_STRING_FROM_STRING_TLAB;
    auto srcStr = src[FIRST_OPERAND];
    CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), srcStr);
}

void Codegen::CreateStringGetCharsTlab([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypointId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_GET_CHARS_TLAB_COMPRESSED
                                                                   : EntrypointId::STRING_GET_CHARS_TLAB;
    auto runtime = GetGraph()->GetRuntime();
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(klassReg, false,
                                MemRef(ThreadReg(), runtime->GetArrayU16ClassPointerTlsOffset(GetArch())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassReg);
    } else {
        auto klassImm = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetArrayU16Class(GetGraph()->GetMethod())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassImm);
    }
}

void Codegen::CreateStringHashCode([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypoint = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_HASH_CODE_COMPRESSED
                                                                 : EntrypointId::STRING_HASH_CODE;
    auto strReg = src[FIRST_OPERAND];
    auto mref = MemRef(strReg, ark::coretypes::String::GetHashcodeOffset());
    auto slowPath = CreateSlowPath<SlowPathStringHashCode>(inst, entrypoint);
    slowPath->SetDstReg(dst);
    slowPath->SetSrcReg(strReg);
    if (dst.GetId() != strReg.GetId()) {
        GetEncoder()->EncodeLdr(ConvertRegister(dst.GetId(), DataType::INT32), false, mref);
        GetEncoder()->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
    } else {
        ScopedTmpReg hashReg(GetEncoder(), INT32_TYPE);
        GetEncoder()->EncodeLdr(hashReg, false, mref);
        GetEncoder()->EncodeJump(slowPath->GetLabel(), hashReg, Condition::EQ);
        GetEncoder()->EncodeMov(dst, hashReg);
    }
    slowPath->BindBackLabel(GetEncoder());
}
void Codegen::CreateStringCompareTo([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto str1 = src[FIRST_OPERAND];
    auto str2 = src[SECOND_OPERAND];
    CallFastPath(inst, EntrypointId::STRING_COMPARE_TO, dst, {}, str1, str2);
}
#include "intrinsics_codegen.inl"

void Codegen::CreateBuiltinIntrinsic(IntrinsicInst *inst)
{
    Reg dst = INVALID_REGISTER;
    SRCREGS src;

    if (!inst->NoDest()) {
        dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    }

    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        if (inst->GetInput(i).GetInst()->IsSaveState()) {
            continue;
        }
        auto location = inst->GetLocation(i);
        auto type = inst->GetInputType(i);
        src[i] = ConvertRegister(location.GetValue(), type);
    }
    FillBuiltin(inst, src, dst);
}

static bool GetNeedBarrierProperty(const Inst *inst)
{
    Opcode op = inst->GetOpcode();
    if (op == Opcode::LoadObject) {
        return inst->CastToLoadObject()->GetNeedBarrier();
    }
    if (op == Opcode::StoreObject) {
        return inst->CastToStoreObject()->GetNeedBarrier();
    }
    if (op == Opcode::LoadObjectPair) {
        return inst->CastToLoadObjectPair()->GetNeedBarrier();
    }
    if (op == Opcode::StoreObjectPair) {
        return inst->CastToStoreObjectPair()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArray) {
        return inst->CastToLoadArray()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArray) {
        return inst->CastToStoreArray()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArrayI) {
        return inst->CastToLoadArrayI()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArrayI) {
        return inst->CastToStoreArrayI()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArrayPair) {
        return inst->CastToLoadArrayPair()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArrayPair) {
        return inst->CastToStoreArrayPair()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArrayPairI) {
        return inst->CastToLoadArrayPairI()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArrayPairI) {
        return inst->CastToStoreArrayPairI()->GetNeedBarrier();
    }
    if (op == Opcode::LoadStatic) {
        return inst->CastToLoadStatic()->GetNeedBarrier();
    }
    if (op == Opcode::StoreStatic) {
        return inst->CastToStoreStatic()->GetNeedBarrier();
    }
    return false;
}

/**
 * Returns true if codegen emits call(s) to some library function(s)
 * while processing the instruction.
 */
bool Codegen::InstEncodedWithLibCall(const Inst *inst, Arch arch)
{
    ASSERT(inst != nullptr);
    Opcode op = inst->GetOpcode();
    if (op == Opcode::Mod) {
        auto dstType = inst->GetType();
        if (arch == Arch::AARCH64 || arch == Arch::X86_64) {
            return dstType == DataType::FLOAT32 || dstType == DataType::FLOAT64;
        }
        return arch == Arch::AARCH32;
    }
    if (op == Opcode::Div && arch == Arch::AARCH32) {
        auto dstType = inst->GetType();
        return dstType == DataType::INT64 || dstType == DataType::UINT64;
    }
    if (op == Opcode::Cast && arch == Arch::AARCH32) {
        auto dstType = inst->GetType();
        auto srcType = inst->GetInputType(0);
        if (dstType == DataType::FLOAT32 || dstType == DataType::FLOAT64) {
            return srcType == DataType::INT64 || srcType == DataType::UINT64;
        }
        if (srcType == DataType::FLOAT32 || srcType == DataType::FLOAT64) {
            return dstType == DataType::INT64 || dstType == DataType::UINT64;
        }
        return false;
    }

    return GetNeedBarrierProperty(inst);
}

Reg Codegen::ConvertInstTmpReg(const Inst *inst, DataType::Type type) const
{
    ASSERT(inst->GetTmpLocation().IsFixedRegister());
    return Reg(inst->GetTmpLocation().GetValue(), ConvertDataType(type, GetArch()));
}

Reg Codegen::ConvertInstTmpReg(const Inst *inst) const
{
    return ConvertInstTmpReg(inst, Is64BitsArch(GetArch()) ? DataType::INT64 : DataType::INT32);
}

void Codegen::GetEntrypoint(Reg entry, intptr_t epOffset) const
{
    auto epTable = MemRef(ThreadReg(), GetRuntime()->GetEntrypointsTablePointerTlsOffset(GetArch()));
    LoadEntrypoint(entry, GetEncoder(), epTable, epOffset);
}

void Codegen::GetEntrypoint(Reg entry, EntrypointId id) const
{
    auto epTable = MemRef(ThreadReg(), GetRuntime()->GetEntrypointsTablePointerTlsOffset(GetArch()));
    auto epOffset = GetRuntime()->GetEntrypointOffset(GetArch(), id);
    LoadEntrypoint(entry, GetEncoder(), epTable, epOffset);
}

void PostWriteBarrier::Encode(MemRef mem, Reg reg1, Reg reg2, bool checkObject, RegMask preserved)
{
    ASSERT(reg1.IsValid());
    auto reference {TypeInfo::FromDataType(DataType::REFERENCE, cg_->GetArch())};
    PostWriteBarrier::Args args;
    args.mem = mem;
    args.reg1 = reg1.As(reference);
    args.reg2 = reg2.IsValid() ? reg2.As(reference) : INVALID_REGISTER;
    args.checkObject = checkObject;
    args.preserved = preserved;
    if (cg_->GetGraph()->SupportsIrtocBarriers()) {
        if (cg_->GetGraph()->IsOfflineCompilationMode()) {
            EncodeOfflineIrtocBarrier(args);
        } else {
            EncodeOnlineIrtocBarrier(args);
        }
    } else if (type_ == ark::mem::BarrierType::POST_INTERREGION_BARRIER) {
        EncodeInterRegionBarrier(args);
    } else {
        // Unknown GC barrier type
        UNREACHABLE();
    }
}

void PostWriteBarrier::EncodeOfflineIrtocBarrier(Args args)
{
    auto hasObj2 = HasObject2(args);
    auto paramRegs = GetParamRegs(hasObj2 ? 4U : 3U, args);
    cg_->SaveCallerRegisters(paramRegs, VRegMask(), false);
    if (hasObj2) {
        FillCallParams(args.mem, args.reg1, args.reg2);
    } else {
        FillCallParams(args.mem, args.reg1);
    }
    // load function pointer from tls field
    auto offset {hasObj2 ? cross_values::GetManagedThreadPostWrbTwoObjectsOffset(cg_->GetArch())
                         : cross_values::GetManagedThreadPostWrbOneObjectOffset(cg_->GetArch())};
    MemRef entry(cg_->ThreadReg(), offset);
    cg_->GetEncoder()->MakeCall(entry);
    cg_->LoadCallerRegisters(paramRegs, VRegMask(), false);
}

void PostWriteBarrier::EncodeOnlineIrtocBarrier(Args args)
{
    SCOPED_DISASM_STR(cg_, "Post Online Irtoc-WRB");
    auto hasObj2 = HasObject2(args);
    if (type_ == ark::mem::BarrierType::POST_INTERREGION_BARRIER) {
        if (hasObj2) {
            EncodeOnlineIrtocRegionTwoRegsBarrier(args);
        } else {
            EncodeOnlineIrtocRegionOneRegBarrier(args);
        }
    } else {
        // Unknown GC barrier type
        UNREACHABLE();
    }
}

/**
 * Post-write barrier for StorePair case.
 * The code below may mark 0, 1 or 2 cards. The idea is following:
 *  if (the second object isn't null and is allocated in other from base object's region)
 *      MarkOneCard(CardOfSecondField(mem))
 *      if (address of the second field isn't aligned at the size of a card)
 *          # i.e. each of store address (fields of the base objects) are related to the same card
 *          goto Done
 *
 *  if (the first object isn't null and is allocated in other from base object's region)
 *      MarkOneCard(CardOfFirstField(mem))
 *
 *  label: Done
 */
void PostWriteBarrier::EncodeOnlineIrtocRegionTwoRegsBarrier(Args args)
{
    static constexpr auto ENTRYPOINT_ID {RuntimeInterface::EntrypointId::POST_INTER_REGION_BARRIER_SLOW};
    auto *enc {cg_->GetEncoder()};
    auto base = GetBase(args);
    auto paramReg0 = enc->GetTarget().GetParamReg(0);
    auto paramRegs = GetParamRegs(1U, args);
    auto lblMarkCardAndExit = enc->CreateLabel();
    auto lblCheck1Obj = enc->CreateLabel();
    auto lblDone = enc->CreateLabel();
    EncodeCheckObject(base, args.reg2, lblCheck1Obj, args.checkObject);
    cg_->SaveCallerRegisters(paramRegs, VRegMask(), false);
    EncodeWrapOneArg(paramReg0, base, args.mem, args.reg1.GetSize() / BITS_PER_BYTE);
    {
        ScopedTmpReg tmp(enc, cg_->ConvertDataType(DataType::REFERENCE, cg_->GetArch()));
        enc->EncodeAnd(tmp, paramReg0, Imm(cross_values::GetCardAlignmentMask(cg_->GetArch())));
        enc->EncodeJump(lblMarkCardAndExit, tmp, Condition::NE);
    }
    ScopedTmpReg tmpReg(enc, true);
    cg_->GetEntrypoint(tmpReg, ENTRYPOINT_ID);
    enc->MakeCall(tmpReg);
    cg_->LoadCallerRegisters(paramRegs, VRegMask(), false);
    enc->BindLabel(lblCheck1Obj);
    EncodeCheckObject(base, args.reg1, lblDone, args.checkObject);
    cg_->SaveCallerRegisters(paramRegs, VRegMask(), false);
    EncodeWrapOneArg(paramReg0, base, args.mem);
    enc->BindLabel(lblMarkCardAndExit);
    cg_->GetEntrypoint(tmpReg, ENTRYPOINT_ID);
    enc->MakeCall(tmpReg);
    cg_->LoadCallerRegisters(paramRegs, VRegMask(), false);
    enc->BindLabel(lblDone);
}

void PostWriteBarrier::EncodeOnlineIrtocRegionOneRegBarrier(Args args)
{
    static constexpr auto ENTRYPOINT_ID {RuntimeInterface::EntrypointId::POST_INTER_REGION_BARRIER_SLOW};
    auto *enc {cg_->GetEncoder()};
    auto base = GetBase(args);
    auto paramReg0 = enc->GetTarget().GetParamReg(0);
    auto paramRegs = GetParamRegs(1U, args);
    auto skip = enc->CreateLabel();
    EncodeCheckObject(base, args.reg1, skip, args.checkObject);
    cg_->SaveCallerRegisters(paramRegs, VRegMask(), false);
    EncodeWrapOneArg(paramReg0, base, args.mem);
    ScopedTmpReg tmpReg(enc, true);
    cg_->GetEntrypoint(tmpReg, ENTRYPOINT_ID);
    enc->MakeCall(tmpReg);
    cg_->LoadCallerRegisters(paramRegs, VRegMask(), false);
    enc->BindLabel(skip);
}

void PostWriteBarrier::EncodeInterRegionBarrier(Args args)
{
    SCOPED_DISASM_STR(cg_, "Post IR-WRB");
    ASSERT(type_ == ark::mem::BarrierType::POST_INTERREGION_BARRIER);

    static constexpr auto ENTRYPOINT_ID {RuntimeInterface::EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE};
    auto *enc {cg_->GetEncoder()};
    auto label = enc->CreateLabel();
    if (args.checkObject) {
        cg_->CheckObject(args.reg1, label);
    }
    auto regionSizeBit = GetBarrierOperandValue<uint8_t>(BARRIER_POSITION, "REGION_SIZE_BITS");
    auto base = GetBase(args);
    ScopedTmpReg tmp(enc, cg_->ConvertDataType(DataType::REFERENCE, cg_->GetArch()));
    // Compare first store value with mem
    enc->EncodeXor(tmp, base, args.reg1);
    enc->EncodeShr(tmp, tmp, Imm(regionSizeBit));
    enc->EncodeJump(label, tmp, Condition::EQ);
    auto [live_regs, live_vregs] = cg_->GetLiveRegisters<true>(inst_);
    live_regs |= args.preserved;

    if (args.mem.HasIndex()) {
        ASSERT(args.mem.GetScale() == 0 && !args.mem.HasDisp());
        enc->EncodeAdd(tmp, base, args.mem.GetIndex());
        cg_->CallBarrier(live_regs, live_vregs, ENTRYPOINT_ID, INVALID_REGISTER, tmp, args.reg1);
    } else if (args.mem.HasDisp()) {
        ASSERT(!args.mem.HasIndex());
        enc->EncodeAdd(tmp, base, Imm(args.mem.GetDisp()));
        cg_->CallBarrier(live_regs, live_vregs, ENTRYPOINT_ID, INVALID_REGISTER, tmp, args.reg1);
    } else {
        cg_->CallBarrier(live_regs, live_vregs, ENTRYPOINT_ID, INVALID_REGISTER, base, args.reg1);
    }
    enc->BindLabel(label);

    if (HasObject2(args)) {
        auto label1 = enc->CreateLabel();
        if (args.checkObject) {
            cg_->CheckObject(args.reg2, label1);
        }
        // Compare second store value with mem
        enc->EncodeXor(tmp, base, args.reg2);
        enc->EncodeShr(tmp, tmp, Imm(regionSizeBit));
        enc->EncodeJump(label1, tmp, Condition::EQ);
        enc->EncodeAdd(tmp, base, Imm(args.reg1.GetSize() / BITS_PER_BYTE));
        if (args.mem.HasIndex()) {
            ASSERT(args.mem.GetScale() == 0 && !args.mem.HasDisp());
            enc->EncodeAdd(tmp, tmp, args.mem.GetIndex());
        } else if (args.mem.HasDisp()) {
            ASSERT(!args.mem.HasIndex());
            enc->EncodeAdd(tmp, tmp, Imm(args.mem.GetDisp()));
        }
        cg_->CallBarrier(live_regs, live_vregs, ENTRYPOINT_ID, INVALID_REGISTER, tmp, args.reg2);
        enc->BindLabel(label1);
    }
}

void PostWriteBarrier::EncodeCalculateCardIndex(Reg baseReg, ScopedTmpReg *tmp, ScopedTmpReg *tmp1)
{
    ASSERT(baseReg != INVALID_REGISTER);
    auto tmpType = tmp->GetReg().GetType();
    auto tmp1Type = tmp1->GetReg().GetType();
    if (baseReg.GetSize() < Reg(*tmp).GetSize()) {
        tmp->ChangeType(baseReg.GetType());
        tmp1->ChangeType(baseReg.GetType());
    }
    auto enc {cg_->GetEncoder()};
    enc->EncodeSub(*tmp, baseReg, *tmp);
    enc->EncodeShr(*tmp, *tmp, Imm(GetBarrierOperandValue<uint8_t>(BARRIER_POSITION, "CARD_BITS")));
    tmp->ChangeType(tmpType);
    tmp1->ChangeType(tmp1Type);
}

void PostWriteBarrier::EncodeCheckObject(Reg base, Reg reg1, LabelHolder::LabelId skipLabel, bool checkNull)
{
    auto enc {cg_->GetEncoder()};
    if (checkNull) {
        // Fast null check in-place for one register
        enc->EncodeJump(skipLabel, reg1, Condition::EQ);
    }
    ScopedTmpReg tmp(enc, cg_->ConvertDataType(DataType::REFERENCE, cg_->GetArch()));
    auto regionSizeBit = GetBarrierOperandValue<uint8_t>(BARRIER_POSITION, "REGION_SIZE_BITS");
    enc->EncodeXor(tmp, base, reg1);
    enc->EncodeShr(tmp, tmp, Imm(regionSizeBit));
    enc->EncodeJump(skipLabel, tmp, Condition::EQ);
}

void PostWriteBarrier::EncodeWrapOneArg(Reg param, Reg base, MemRef mem, size_t additionalOffset)
{
    auto enc {cg_->GetEncoder()};
    if (mem.HasIndex()) {
        ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
        enc->EncodeAdd(param, base, mem.GetIndex());
        if (additionalOffset != 0) {
            enc->EncodeAdd(param, param, Imm(additionalOffset));
        }
    } else if (mem.HasDisp()) {
        ASSERT(!mem.HasIndex());
        enc->EncodeAdd(param, base, Imm(mem.GetDisp() + additionalOffset));
    } else {
        enc->EncodeAdd(param, base, Imm(additionalOffset));
    }
}

}  // namespace ark::compiler
