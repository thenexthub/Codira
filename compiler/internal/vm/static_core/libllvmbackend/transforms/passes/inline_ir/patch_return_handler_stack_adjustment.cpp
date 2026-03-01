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

#include "llvm_ark_interface.h"
#include "patch_return_handler_stack_adjustment.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/CodeGen/GlobalISel/MachineIRBuilder.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/Debug.h>

#include "transforms/transform_utils.h"

#define DEBUG_TYPE "patch-return-handler-stack-adjustment"

namespace {

/**
 * Patch stack adjustment value in return handler
 *
 * We generate inline assembly with hardcoded constant for return handlers in LLVMEntry::EmitInterpreterReturn
 *
 * The inline assembly uses stack pointer and inserts own return. Examples of inline assemblies:
 *
 * 1. leaq  $0, %rsp - x86. We add a hardcoded value to %rsp and retq then
 * 2. add  sp, sp, $0 - aarch64. We add hardcoded value to sp and ret then
 *
 * LLVM does not know about our rets
 *
 * We use stack pointer in inline assemblies assuming that llvm does not touch sp itself.
 * For example, we assume that llvm does not spill any register value onto the stack
 * But llvm can do it, example: 'sub    $0x10,%rsp' in function prologue.
 * LLVM will insert corresponding 'add    $0x10,%rsp' before its own rets but not for ours.
 *
 * So we add the stack size of machine function to our "hardcoded value" in inline assemblies.
 * To find such assemblies the pass looks for a comment in the inline assembly template -
 * LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT
 */
class PatchReturnHandlerStackAdjustment : public llvm::MachineFunctionPass {
public:
    static constexpr llvm::StringRef PASS_NAME = "ARK-LLVM patch stack adjustment";
    static constexpr llvm::StringRef ARG_NAME = "patch-return-handler-stack-adjustment";

    explicit PatchReturnHandlerStackAdjustment(ark::llvmbackend::LLVMArkInterface *arkInterface = nullptr)
        : MachineFunctionPass(ID), arkInterface_(arkInterface)
    {
    }

    llvm::StringRef getPassName() const override
    {
        return PASS_NAME;
    }

    bool runOnMachineFunction(llvm::MachineFunction &machineFunction) override
    {
        ASSERT(arkInterface_ != nullptr);
        if (!arkInterface_->IsIrtocReturnHandler(machineFunction.getFunction())) {
            return false;
        }

        auto &frameInfo = machineFunction.getFrameInfo();
        if (frameInfo.hasVarSizedObjects()) {
            llvm::report_fatal_error(llvm::StringRef("Return handler '") + machineFunction.getName() +
                                     "' uses var sized objects");
            return false;
        }
        auto stackSize = frameInfo.getStackSize();
        if (stackSize == 0) {
            return false;
        }

        bool changed = false;
        for (auto &basicBlock : machineFunction) {
            for (auto &instruction : basicBlock) {
                if (!instruction.isInlineAsm()) {
                    continue;
                }
                static constexpr unsigned INLINE_ASM_INDEX = 0;
                static constexpr unsigned STACK_ADJUSTMENT_INDEX = 3;

                std::string_view inlineAsm {instruction.getOperand(INLINE_ASM_INDEX).getSymbolName()};
                if (inlineAsm.find(ark::llvmbackend::LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT) !=
                    std::string::npos) {
                    auto &stackAdjustment = instruction.getOperand(STACK_ADJUSTMENT_INDEX);
                    ASSERT(stackAdjustment.isImm());
                    auto oldStackSize = stackAdjustment.getImm();
                    auto newStackSize = oldStackSize + stackSize;
                    LLVM_DEBUG(llvm::dbgs() << "Replaced old_stack_size = " << oldStackSize
                                            << " with new_stack_size = " << newStackSize << " in inline_asm = '"
                                            << inlineAsm << "' because llvm used " << stackSize
                                            << " bytes of stack in function = '" << machineFunction.getName() << "'\n");
                    stackAdjustment.setImm(newStackSize);
                    changed = true;
                }
            }
        }

        return changed;
    }

    static inline char ID = 0;  // NOLINT(readability-identifier-naming)
private:
    ark::llvmbackend::LLVMArkInterface *arkInterface_ {nullptr};
};

}  // namespace

llvm::MachineFunctionPass *ark::llvmbackend::CreatePatchReturnHandlerStackAdjustmentPass(
    ark::llvmbackend::LLVMArkInterface *arkInterface)
{
    return new PatchReturnHandlerStackAdjustment(arkInterface);
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::RegisterPass<PatchReturnHandlerStackAdjustment> g_p1(PatchReturnHandlerStackAdjustment::ARG_NAME,
                                                                  PatchReturnHandlerStackAdjustment::PASS_NAME, false,
                                                                  false);
