/*
 * Copyright (c)2023 YAMAMOTO Takashi.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <toolchain/Config/toolchain-config.h>
#include <toolchain-c/TargetMachine.h>

LLVM_C_EXTERN_C_BEGIN
LLVMTargetMachineRef
LLVMCreateTargetMachineWithOpts(LLVMTargetRef ctarget, const char *triple,
                                const char *cpu, const char *features,
                                LLVMCodeGenOptLevel opt_level,
                                LLVMRelocMode reloc_mode,
                                LLVMCodeModel code_model,
                                bool EmitStackSizeSection,
                                const char *StackUsageOutput);

/* https://reviews.toolchain.org/D153107 */
#if LLVM_VERSION_MAJOR < 18
typedef enum {
    LLVMTailCallKindNone = 0,
    LLVMTailCallKindTail = 1,
    LLVMTailCallKindMustTail = 2,
    LLVMTailCallKindNoTail = 3,
} LLVMTailCallKind;

LLVMTailCallKind
LLVMGetTailCallKind(LLVMValueRef CallInst);
void
LLVMSetTailCallKind(LLVMValueRef CallInst, LLVMTailCallKind kind);
#endif

LLVM_C_EXTERN_C_END
