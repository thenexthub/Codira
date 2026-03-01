/*
 * Copyright (c)2023 YAMAMOTO Takashi.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <toolchain-c/TargetMachine.h>
#if LLVM_VERSION_MAJOR < 17
#include <toolchain/ADT/None.h>
#include <toolchain/ADT/Optional.h>
#endif
#include <toolchain/IR/Instructions.h>
#if LLVM_VERSION_MAJOR >= 14
#include <toolchain/MC/TargetRegistry.h>
#else
#include <toolchain/Support/TargetRegistry.h>
#endif
#include <toolchain/Target/TargetMachine.h>

#include "bh_assert.h"

#include "aot_llvm_extra2.h"

#if LLVM_VERSION_MAJOR >= 17
namespace vm::core {
template<typename T>
using Optional = std::optional<T>;
}
#endif

static toolchain::Optional<toolchain::Reloc::Model>
convert(LLVMRelocMode reloc_mode)
{
    switch (reloc_mode) {
        case LLVMRelocDefault:
#if LLVM_VERSION_MAJOR >= 16
            return std::nullopt;
#else
            return toolchain::None;
#endif
        case LLVMRelocStatic:
            return toolchain::Reloc::Static;
        case LLVMRelocPIC:
            return toolchain::Reloc::PIC_;
        case LLVMRelocDynamicNoPic:
            return toolchain::Reloc::DynamicNoPIC;
        case LLVMRelocROPI:
            return toolchain::Reloc::ROPI;
        case LLVMRelocRWPI:
            return toolchain::Reloc::RWPI;
        case LLVMRelocROPI_RWPI:
            return toolchain::Reloc::ROPI_RWPI;
    }
    bh_assert(0);
#if LLVM_VERSION_MAJOR >= 16
    return std::nullopt;
#else
    return toolchain::None;
#endif
}

#if LLVM_VERSION_MAJOR < 18
static toolchain::CodeGenOpt::Level
convert(LLVMCodeGenOptLevel opt_level)
{
    switch (opt_level) {
        case LLVMCodeGenLevelNone:
            return toolchain::CodeGenOpt::None;
        case LLVMCodeGenLevelLess:
            return toolchain::CodeGenOpt::Less;
        case LLVMCodeGenLevelDefault:
            return toolchain::CodeGenOpt::Default;
        case LLVMCodeGenLevelAggressive:
            return toolchain::CodeGenOpt::Aggressive;
    }
    bh_assert(0);
    return toolchain::CodeGenOpt::None;
}
#else
static toolchain::CodeGenOptLevel
convert(LLVMCodeGenOptLevel opt_level)
{
    switch (opt_level) {
        case LLVMCodeGenLevelNone:
            return toolchain::CodeGenOptLevel::None;
        case LLVMCodeGenLevelLess:
            return toolchain::CodeGenOptLevel::Less;
        case LLVMCodeGenLevelDefault:
            return toolchain::CodeGenOptLevel::Default;
        case LLVMCodeGenLevelAggressive:
            return toolchain::CodeGenOptLevel::Aggressive;
    }
    bh_assert(0);
    return toolchain::CodeGenOptLevel::None;
}
#endif

static toolchain::Optional<toolchain::CodeModel::Model>
convert(LLVMCodeModel code_model, bool *jit)
{
    *jit = false;
    switch (code_model) {
        case LLVMCodeModelDefault:
#if LLVM_VERSION_MAJOR >= 16
            return std::nullopt;
#else
            return toolchain::None;
#endif
        case LLVMCodeModelJITDefault:
            *jit = true;
#if LLVM_VERSION_MAJOR >= 16
            return std::nullopt;
#else
            return toolchain::None;
#endif
        case LLVMCodeModelTiny:
            return toolchain::CodeModel::Tiny;
        case LLVMCodeModelSmall:
            return toolchain::CodeModel::Small;
        case LLVMCodeModelKernel:
            return toolchain::CodeModel::Kernel;
        case LLVMCodeModelMedium:
            return toolchain::CodeModel::Medium;
        case LLVMCodeModelLarge:
            return toolchain::CodeModel::Large;
    }
    bh_assert(0);
#if LLVM_VERSION_MAJOR >= 16
    return std::nullopt;
#else
    return toolchain::None;
#endif
}

LLVMTargetMachineRef
LLVMCreateTargetMachineWithOpts(LLVMTargetRef ctarget, const char *triple,
                                const char *cpu, const char *features,
                                LLVMCodeGenOptLevel opt_level,
                                LLVMRelocMode reloc_mode,
                                LLVMCodeModel code_model,
                                bool EmitStackSizeSection,
                                const char *StackUsageOutput)
{
    toolchain::TargetOptions opts;

    // -fstack-size-section equiv
    // emit it to ".stack_sizes" section in case of ELF
    // you can read it with "toolchain-readobj --stack-sizes"
    opts.EmitStackSizeSection = EmitStackSizeSection;

    // -fstack-usage equiv
    if (StackUsageOutput != NULL) {
        opts.StackUsageOutput = StackUsageOutput;
    }

    auto target = reinterpret_cast<toolchain::Target *>(ctarget);
    auto rm = convert(reloc_mode);
    auto ol = convert(opt_level);
    bool jit;
    auto cm = convert(code_model, &jit);
#if LLVM_VERSION_MAJOR >= 21
    auto targetmachine = target->createTargetMachine(
        toolchain::Triple(triple), cpu, features, opts, rm, cm, ol, jit);
#else
    auto targetmachine = target->createTargetMachine(triple, cpu, features,
                                                     opts, rm, cm, ol, jit);
#endif
#if LLVM_VERSION_MAJOR >= 18
    // always place data in normal data section.
    //
    // note that:
    // - our aot file emitter/loader doesn't support x86-64 large data
    //   sections. (eg .lrodata)
    // - for our purposes, "data" is usually something the compiler
    //   generated. (eg. jump tables) we probably never benefit from
    //   large data sections.
    targetmachine->setLargeDataThreshold(UINT64_MAX);
#endif
    return reinterpret_cast<LLVMTargetMachineRef>(targetmachine);
}

/* https://reviews.toolchain.org/D153107 */
#if LLVM_VERSION_MAJOR < 18
using namespace vm::core;

LLVMTailCallKind
LLVMGetTailCallKind(LLVMValueRef Call)
{
    return (LLVMTailCallKind)unwrap<CallInst>(Call)->getTailCallKind();
}

void
LLVMSetTailCallKind(LLVMValueRef Call, LLVMTailCallKind kind)
{
    unwrap<CallInst>(Call)->setTailCallKind((CallInst::TailCallKind)kind);
}
#endif
