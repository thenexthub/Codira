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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_FRAME_LOWERING_FRAME_BUILDER_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_FRAME_LOWERING_FRAME_BUILDER_H

#include <functional>

#include <llvm/CodeGen/MachineFunction.h>

struct FrameInfo {
    using RegMasks = std::tuple<uint32_t, uint32_t>;

    // This field used only in aarch64 FrameBuilder
    uint32_t stackSize {};
    // Mask of used callee saved registers in Ark terms
    RegMasks regMasks {};
    bool hasCalls = false;
    ssize_t soOffset = 0;
    // True, if the function uses stack in blocks except prologue and epilogue.
    // E.g. allocates a stack object
    bool usesStack = false;
    // True, if the function uses float register in some way.
    // It is needed to notify runtime about usage of float registers
    bool usesFloatRegs = false;
};

// CC-OFFNXT(G.NAM.01) false positive
enum class FrameConstantDescriptor {
    FRAME_FLAGS,
    TLS_FRAME_OFFSET,
};

struct InlineAsmBuilder;

struct FrameBuilderInterface {
    using ConstantPoolHandler = std::function<ssize_t(FrameConstantDescriptor)>;

public:
    FrameBuilderInterface(FrameInfo frameInfo, ConstantPoolHandler handler)
        : frameInfo_ {std::move(frameInfo)}, constantPool_ {std::move(handler)}
    {
    }
    virtual ~FrameBuilderInterface() = default;

    virtual bool RemovePrologue(llvm::MachineBasicBlock &mblock) = 0;
    virtual bool RemoveEpilogue(llvm::MachineBasicBlock &mblock) = 0;
    virtual void InsertPrologue(llvm::MachineBasicBlock &mblock) = 0;
    virtual void InsertEpilogue(llvm::MachineBasicBlock &mblock) = 0;

    FrameInfo &GetFrameInfo()
    {
        return frameInfo_;
    }

    FrameBuilderInterface(const FrameBuilderInterface &) = delete;
    void operator=(const FrameBuilderInterface &) = delete;
    FrameBuilderInterface(FrameBuilderInterface &&) = delete;
    FrameBuilderInterface &operator=(FrameBuilderInterface &&) = delete;

protected:
    FrameInfo frameInfo_;               // NOLINT(misc-non-private-member-variables-in-classes)
    ConstantPoolHandler constantPool_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

struct AMD64FrameBuilder final : FrameBuilderInterface {
    AMD64FrameBuilder(FrameInfo frameInfo, ConstantPoolHandler handler)
        : FrameBuilderInterface(std::move(frameInfo), std::move(handler))
    {
    }
    ~AMD64FrameBuilder() override = default;

    bool RemovePrologue(llvm::MachineBasicBlock &mblock) override;
    bool RemoveEpilogue(llvm::MachineBasicBlock &mblock) override;
    void InsertPrologue(llvm::MachineBasicBlock &mblock) override;
    void InsertEpilogue(llvm::MachineBasicBlock &mblock) override;

    AMD64FrameBuilder(const AMD64FrameBuilder &) = delete;
    void operator=(const AMD64FrameBuilder &) = delete;
    AMD64FrameBuilder(AMD64FrameBuilder &&) = delete;
    AMD64FrameBuilder &operator=(AMD64FrameBuilder &&) = delete;
};

struct ARM64FrameBuilder final : FrameBuilderInterface {
    ARM64FrameBuilder(FrameInfo frameInfo, ConstantPoolHandler handler)
        : FrameBuilderInterface(std::move(frameInfo), std::move(handler))
    {
    }
    ~ARM64FrameBuilder() override = default;

    bool RemovePrologue(llvm::MachineBasicBlock &mblock) override;
    bool RemoveEpilogue(llvm::MachineBasicBlock &mblock) override;
    void InsertPrologue(llvm::MachineBasicBlock &mblock) override;
    void InsertEpilogue(llvm::MachineBasicBlock &mblock) override;

    ARM64FrameBuilder(const ARM64FrameBuilder &) = delete;
    void operator=(const ARM64FrameBuilder &) = delete;
    ARM64FrameBuilder(ARM64FrameBuilder &&) = delete;
    ARM64FrameBuilder &operator=(ARM64FrameBuilder &&) = delete;

private:
    void EmitCSRSaveRestoreCode(InlineAsmBuilder *builder, uint32_t regsMask, std::string_view asmSingleReg,
                                std::string_view asmPairRegs, ssize_t calleeOffset);
};

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_FRAME_LOWERING_FRAME_BUILDER_H
