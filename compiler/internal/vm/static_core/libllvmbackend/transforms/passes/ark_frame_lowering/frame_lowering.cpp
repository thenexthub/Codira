/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "frame_lowering.h"
#include "frame_builder.h"
#include "llvm_ark_interface.h"
#include "compiler/optimizer/code_generator/target_info.h"

#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/MachineFunctionPass.h>

#define DEBUG_TYPE "frame-builder"

namespace {
// See https://github.com/ARM-software/abi-aa/blob/main/aadwarf64/aadwarf64.rst#dwarf-register-names
constexpr int32_t X0 = 0;
constexpr int32_t D0 = 64;
constexpr int32_t D31 = 95;

// See amd64 abi spec
constexpr int32_t RAX = 0;
constexpr int32_t XMM0 = 17;
constexpr int32_t XMM15 = 32;

constexpr size_t ToDwarfReg(ark::Arch arch, bool fp, size_t arkReg)
{
    ASSERT(arkReg <= 32U);
    if (arch == ark::Arch::AARCH64) {
        return !fp ? arkReg : D0 + arkReg;
    }
    ASSERT(arch == ark::Arch::X86_64);
    constexpr int32_t SIXTEEN = 16U;
    ASSERT(!fp || arkReg < SIXTEEN);
    return fp ? 1U + SIXTEEN + arkReg : ark::llvmbackend::LLVMArkInterface::ToDwarfRegNumX86(arkReg);
}

class FrameLoweringPass : public llvm::MachineFunctionPass {
public:
    static constexpr llvm::StringRef PASS_NAME = "ARK-LLVM Frame Lowering";
    static constexpr llvm::StringRef ARG_NAME = "ark-llvm-frame-lowering";

    explicit FrameLoweringPass(ark::llvmbackend::LLVMArkInterface *arkInterface = nullptr)
        : llvm::MachineFunctionPass(ID), arkInterface_ {arkInterface}
    {
    }

    llvm::StringRef getPassName() const override
    {
        return PASS_NAME;
    }

    bool runOnMachineFunction(llvm::MachineFunction &mfunc) override
    {
        if (mfunc.getFunction().getMetadata("use-ark-frame") == nullptr) {
            return false;
        }
        // Collect information about current function
        FrameInfo frameInfo;
        frameInfo.regMasks = GetUsedRegs(mfunc);
        frameInfo.hasCalls = HasCalls(mfunc);
        frameInfo.stackSize = mfunc.getFrameInfo().getStackSize();
        frameInfo.soOffset = -arkInterface_->GetStackOverflowCheckOffset();
        frameInfo.usesStack = IsStackUsed(&mfunc);
        frameInfo.usesFloatRegs = FloatRegsUsed(&mfunc);
        if (arkInterface_->IsArm64()) {
            constexpr uint32_t SLOT_SIZE = 8U;
            ASSERT(frameInfo.stackSize >= SLOT_SIZE);
            frameInfo.stackSize -= 2U * SLOT_SIZE;
        }
        if (arkInterface_->IsArm64()) {
            ARM64FrameBuilder frameBuilder(
                frameInfo, [this](FrameConstantDescriptor descr) { return GetConstantFromRuntime(descr); });
            return VisitMachineFunction(mfunc, &frameBuilder);
        }
        AMD64FrameBuilder frameBuilder(frameInfo,
                                       [this](FrameConstantDescriptor descr) { return GetConstantFromRuntime(descr); });
        return VisitMachineFunction(mfunc, &frameBuilder);
    }

private:
    ssize_t GetConstantFromRuntime(FrameConstantDescriptor descr)
    {
        switch (descr) {
            case FrameConstantDescriptor::TLS_FRAME_OFFSET:
                return arkInterface_->GetTlsFrameKindOffset();
            case FrameConstantDescriptor::FRAME_FLAGS:
                return arkInterface_->GetCFrameHasFloatRegsFlagMask();
            default:
                llvm_unreachable("Undefined FrameConstantDescriptor in constant_pool_handler");
        }
    }

    bool VisitMachineFunction(llvm::MachineFunction &mfunc, FrameBuilderInterface *frameBuilder)
    {
        // Remove generated prolog/epilog and insert custom one
        for (auto &bb : mfunc) {
            auto isPrologue = frameBuilder->RemovePrologue(bb);
            auto isEpilogue = frameBuilder->RemoveEpilogue(bb);
            if (isPrologue) {
                frameBuilder->InsertPrologue(bb);
            }
            if (isEpilogue) {
                frameBuilder->InsertEpilogue(bb);
            }
        }

        // Remember info for CodeInfoProducer
        auto &func = mfunc.getFunction();
        arkInterface_->PutMethodStackSize(&func, mfunc.getFrameInfo().getStackSize());
        arkInterface_->PutCalleeSavedRegistersMask(func.getName(), frameBuilder->GetFrameInfo().regMasks);

        return true;
    }

    int32_t GetDwarfRegNum(llvm::Register reg, const llvm::TargetRegisterInfo *regInfo) const
    {
        ASSERT(reg.isPhysical());
        // dwarfId < 0 means, there is no dwarf mapping.
        // E.g. for status registers.
        // But need to check separately for x86
        int dwarfId = regInfo->getDwarfRegNum(reg, false);
        if (dwarfId < 0 && !arkInterface_->IsArm64()) {
            // Check super regs as X86 dwarf info in LLVM is weird
            for (llvm::MCSuperRegIterator sreg(reg, regInfo); sreg.isValid(); ++sreg) {
                dwarfId = regInfo->getDwarfRegNum(*sreg, false);
                if (dwarfId >= 0) {
                    break;
                }
            }
        }

        ASSERT(!regInfo->isConstantPhysReg(reg.asMCReg()));
        return dwarfId;
    }

    // first -- X-regs, second -- V-regs
    FrameInfo::RegMasks GetUsedRegs(const llvm::MachineFunction &mfunc) const
    {
        FrameInfo::RegMasks masks {};
        llvm::SmallVector<llvm::Register> usedRegisters;

        auto arch = arkInterface_->IsArm64() ? ark::Arch::AARCH64 : ark::Arch::X86_64;
        auto csrMask = ark::GetCalleeRegsMask(arch, false);
        auto csrVMask = ark::GetCalleeRegsMask(arch, true);
        auto regInfo = mfunc.getSubtarget().getRegisterInfo();
        // Convert ark mask of csr to vector of llvm registers
        for (size_t i = 0; i < csrMask.Size(); i++) {
            if (csrMask.Test(i)) {
                auto dwarf = ToDwarfReg(arch, false, i);
                auto llvm = *regInfo->getLLVMRegNum(dwarf, false);
                usedRegisters.push_back(llvm::Register {llvm});
            }
        }
        for (size_t i = 0; i < csrVMask.Size(); i++) {
            if (csrVMask.Test(i)) {
                auto dwarf = ToDwarfReg(arch, true, i);
                auto llvm = *regInfo->getLLVMRegNum(dwarf, false);
                usedRegisters.push_back(llvm::Register {llvm});
            }
        }

        for (auto &block : mfunc) {
            for (auto &inst : block) {
                FillMaskForInst(&masks, usedRegisters, inst);
            }
        }

        // Drop thread register from the mask since it is reserved
        auto threadReg = ark::GetThreadReg(arch);
        std::get<0>(masks) &= ~(1U << threadReg);

        return masks;
    }

    void FillMaskForInst(FrameInfo::RegMasks *masks, const llvm::SmallVector<llvm::Register> &usedRegisters,
                         const llvm::MachineInstr &inst) const
    {
        auto registerInfo = inst.getMF()->getSubtarget().getRegisterInfo();
        for (auto reg : usedRegisters) {
            if (inst.getFlag(llvm::MachineInstr::FrameSetup) || inst.getFlag(llvm::MachineInstr::FrameDestroy) ||
                !inst.modifiesRegister(reg, registerInfo) || registerInfo->isConstantPhysReg(reg)) {
                continue;
            }
            auto dwarfRegNum = GetDwarfRegNum(reg, registerInfo);
            ASSERT(dwarfRegNum >= 0);
            FillMask(dwarfRegNum, masks);
        }
    }

    bool HasCalls(const llvm::MachineFunction &mfunc)
    {
        for (auto &mblock : mfunc) {
            for (auto &minst : mblock) {
                bool isFrameSetup = minst.getFlag(llvm::MachineInstr::FrameSetup);
                bool isFrameDestroy = minst.getFlag(llvm::MachineInstr::FrameDestroy);
                if (isFrameSetup || isFrameDestroy) {
                    continue;
                }
                auto desc = minst.getDesc();
                if (!desc.isCall() || desc.isReturn()) {
                    continue;
                }
                return true;
            }
        }
        return false;
    }

    bool IsStackUsed(llvm::MachineFunction *machineFunction)
    {
        for (auto &basicBlock : *machineFunction) {
            for (auto &instruction : basicBlock) {
                if (instruction.getFlag(llvm::MachineInstr::FrameSetup) ||
                    instruction.getFlag(llvm::MachineInstr::FrameDestroy)) {
                    continue;
                }
                if (HasOperandUsingStack(&instruction)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool HasOperandUsingStack(llvm::MachineInstr *machineInstr)
    {
        auto regInfo = machineInstr->getMF()->getSubtarget().getRegisterInfo();
        auto arch = arkInterface_->IsArm64() ? ark::Arch::AARCH64 : ark::Arch::X86_64;
        auto sp = GetDwarfSP(arch);
        auto fp = GetDwarfFP(arch);

        for (auto operand : machineInstr->operands()) {
            if (operand.isReg()) {
                llvm::MCRegister reg = operand.getReg().asMCReg();
                size_t dwarfId = regInfo->getDwarfRegNum(reg, false);
                // Aarch64 zero registers (xzr/wzr) have same dwarfId as sp, and should be skipped
                if (regInfo->isConstantPhysReg(reg)) {
                    ASSERT(arkInterface_->IsArm64() && dwarfId == sp);
                    continue;
                }
                if (dwarfId == sp || dwarfId == fp) {
                    return true;
                }
            }
        }
        return false;
    }

    void FillMask(int32_t index, FrameInfo::RegMasks *regMasks) const
    {
        if (arkInterface_->IsArm64()) {
            // Dwarf numbers from llvm/lib/Target/AArch64/AArch64RegisterInfo.td
            if (index >= D0) {
                std::get<1>(*regMasks) |= 1U << static_cast<unsigned>(index - D0);
            } else {
                std::get<0>(*regMasks) |= 1U << static_cast<unsigned>(index - X0);
            }
        } else {
            // Dwarf numbers from llvm/lib/Target/X86/X86RegisterInfo.td
            if (index >= XMM0 && index <= XMM15) {
                std::get<1>(*regMasks) |= 1U << static_cast<unsigned>(index - XMM0);
            } else {
                index = ark::llvmbackend::LLVMArkInterface::X86RegNumberConvert(index);
                std::get<0>(*regMasks) |= 1U << static_cast<unsigned>(index - RAX);
            }
        }
    }

    bool FloatRegsUsed(llvm::MachineFunction *function)
    {
        // Check if any float register is used in this function.
        // Usage here means either read or write to register
        for (auto &basicBlock : *function) {
            for (auto &inst : basicBlock) {
                if (inst.getFlag(llvm::MachineInstr::FrameSetup) || inst.getFlag(llvm::MachineInstr::FrameDestroy)) {
                    continue;
                }
                if (HasOperandUsingFloatReg(&inst)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool HasOperandUsingFloatReg(llvm::MachineInstr *instr)
    {
        auto regInfo = instr->getMF()->getSubtarget().getRegisterInfo();
        for (auto operand : instr->operands()) {
            if (operand.isReg() && operand.getReg().isPhysical() &&
                !regInfo->isConstantPhysReg(operand.getReg().asMCReg())) {
                auto dwarf = GetDwarfRegNum(operand.getReg(), regInfo);
                if (arkInterface_->IsArm64() && dwarf >= D0 && dwarf < D31) {
                    return true;
                }
                if (!arkInterface_->IsArm64() && dwarf >= XMM0 && dwarf <= XMM15) {
                    return true;
                }
            }
        }
        return false;
    }

public:
    static inline char ID = 0;  // NOLINT(readability-identifier-naming)
private:
    ark::llvmbackend::LLVMArkInterface *arkInterface_ {nullptr};
};
}  // namespace

llvm::MachineFunctionPass *ark::llvmbackend::CreateFrameLoweringPass(ark::llvmbackend::LLVMArkInterface *arkInterface)
{
    return new FrameLoweringPass(arkInterface);
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::RegisterPass<FrameLoweringPass> g_fl(FrameLoweringPass::ARG_NAME, FrameLoweringPass::PASS_NAME, false,
                                                  false);
