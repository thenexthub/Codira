/**
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

#include "arch/asm_support.h"
#include "arch/aarch64/helpers_aarch64.S"
#include "arch/aarch64/call_runtime.S"

.macro BRIDGE_SELECTOR name, notcompiled_entry, compiled_entry
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \notcompiled_entry
.private_extern \compiled_entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \notcompiled_entry
.hidden \compiled_entry
#else
.protected \name
.protected \notcompiled_entry
.protected \compiled_entry
#endif
.type \name, %function
#endif
\name:
    ldrb w17, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    cbz w17, 1f
    b \compiled_entry
    // workround relocation R_AARCH64_CONDBR19 out of range
1:  b \notcompiled_entry
.endm

.macro RUNTIME_CALL_CHECKER name, entry
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
#endif
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    stp lr, x19, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(lr, 0)
    CFI_REL_OFFSET(x19, 8)

    ldr x19, [THREAD_REG, #MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET]

    str xzr, [THREAD_REG, #MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET]

    bl \entry

    str x19, [THREAD_REG, #MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET]

    ldp lr, x19, [sp], #16
    CFI_ADJUST_CFA_OFFSET(-16)
    CFI_RESTORE(lr)
    CFI_RESTORE(x19)

    ret
    CFI_ENDPROC
.endm

.macro ENTRYPOINT name, entry, paramsnum, ret_type
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
#endif
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    CALL_RUNTIME RUNTIME_MODE_DEFAULT, \entry, \paramsnum, \ret_type
    ret
    CFI_ENDPROC
.endm

.macro ENTRYPOINT_ODD_SAVED name, entry, paramsnum, ret_type
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
#endif
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    CALL_RUNTIME RUNTIME_MODE_ODD_SAVED, \entry, \paramsnum, \ret_type
    ret
    CFI_ENDPROC
.endm

.macro ENTRYPOINT_SLOW_PATH name, entry, paramsnum, ret_type
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
#endif
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    CALL_RUNTIME RUNTIME_MODE_SLOW_PATH, \entry, \paramsnum, \ret_type
    ret
    CFI_ENDPROC
.endm

.macro MethodEntrypointStub name, entry
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
#endif
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    // If the caller is not a compiled method, we need to call \entry
    // and return back after its execution
    ldrb w17, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    tst w17, w17
    beq 1f
    CFI_REMEMBER_STATE

    str lr, [sp, #-8]
    CFI_REL_OFFSET(lr, -(1 * 8))
    mov x17, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    str x17, [sp, #-16]
    str fp, [sp, #-24]
    CFI_REL_OFFSET(fp, -(3 * 8))
    str lr, [THREAD_REG, #MANAGED_THREAD_NATIVE_PC_OFFSET]
    sub x17, sp, #24
    str x17, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]
    sub sp, sp, #32
    CFI_ADJUST_CFA_OFFSET(32)

    // save callee regs
    stp x27, x28, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(x28, 8)
    CFI_REL_OFFSET(x27, 0)
    stp x25, x26, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(x26, 8)
    CFI_REL_OFFSET(x25, 0)
    stp x23, x24, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(x24, 8)
    CFI_REL_OFFSET(x23, 0)
    stp x21, x22, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(x22, 8)
    CFI_REL_OFFSET(x21, 0)
    stp x19, x20, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(x20, 8)
    CFI_REL_OFFSET(x19, 0)
    stp d14, d15, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(d15, 8)
    CFI_REL_OFFSET(d14, 0)
    stp d12, d13, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(d13, 8)
    CFI_REL_OFFSET(d12, 0)
    stp d10, d11, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(d11, 8)
    CFI_REL_OFFSET(d10, 0)
    stp d8, d9, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(d9, 8)
    CFI_REL_OFFSET(d8, 0)

    bl \entry
    // we're not going to return back here

1:
    CFI_RESTORE_STATE
    CFI_DEF_CFA(sp, 0)
    str lr, [sp, #-8]
    CFI_REL_OFFSET(lr, -(1 * 8))
    sub sp, sp, #16
    CFI_ADJUST_CFA_OFFSET(16)

    bl \entry
    
    add sp, sp, #16
    CFI_ADJUST_CFA_OFFSET(-16)
    ldr lr, [sp, #-8]
    CFI_RESTORE(lr)
    ret
    CFI_ENDPROC
.endm

#include "entrypoints_gen.S"
#include "entrypoints_bridge_asm_macro.inl"

MethodEntrypointStub MAKE_ASM_NAME(AbstractMethodStub), MAKE_ASM_NAME(AbstractMethodErrorEntrypoint)

MethodEntrypointStub MAKE_ASM_NAME(DefaultConflictMethodStub), MAKE_ASM_NAME(IncompatibleClassChangeErrorForMethodConflictEntrypoint)
