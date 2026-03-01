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

// Frame* CreateFrameForMethodWithActualArgsDyn(uint32_t num_actual_args, Method* method, Frame* prev);
.extern MAKE_ASM_NAME(CreateFrameForMethodWithActualArgsDyn)
// void FreeFrame(Frame* frame)
.extern MAKE_ASM_NAME(FreeFrame)
// void InterpreterEntryPoint(Method *method, Frame* frame);
.extern MAKE_ASM_NAME(InterpreterEntryPoint)
// bool DecrementHotnessCounterDyn(Method*, TaggedValue func_obj)
.extern MAKE_ASM_NAME(DecrementHotnessCounterDyn)

.global MAKE_ASM_NAME(CompiledCodeToInterpreterBridgeDyn)
TYPE_FUNCTION(CompiledCodeToInterpreterBridgeDyn)
MAKE_ASM_NAME(CompiledCodeToInterpreterBridgeDyn):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    // method:      x0
    // num_args:    x1
    // arg_i:       24(fp, i, 8)

    // Construct bridge frame:
    // lr
    // COMPILED_CODE_TO_INTERPRETER_BRIDGE
    // fp  <- fp
    // ==  <- sp

    sub sp, sp, #32
    CFI_ADJUST_CFA_OFFSET(4 * 8)
    str lr, [sp, #24]
    CFI_REL_OFFSET(lr, 24)
    mov lr, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    stp fp, lr, [sp, #8]
    CFI_REL_OFFSET(fp, 8)
    add fp, sp, #8
    CFI_DEF_CFA(fp, (3 * 8))

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    PUSH_CALLEE_REGS sp
    CFI_REL_OFFSET(x28, -(2 * 8))
    CFI_REL_OFFSET(x27, -(3 * 8))
    CFI_REL_OFFSET(x26, -(4 * 8))
    CFI_REL_OFFSET(x25, -(5 * 8))
    CFI_REL_OFFSET(x24, -(6 * 8))
    CFI_REL_OFFSET(x23, -(7 * 8))
    CFI_REL_OFFSET(x22, -(8 * 8))
    CFI_REL_OFFSET(x21, -(9 * 8))
    CFI_REL_OFFSET(x20, -(10 * 8))
    CFI_REL_OFFSET(x19, -(11 * 8))
    CFI_REL_OFFSET(d15, -(12 * 8))
    CFI_REL_OFFSET(d14, -(13 * 8))
    CFI_REL_OFFSET(d13, -(14 * 8))
    CFI_REL_OFFSET(d12, -(15 * 8))
    CFI_REL_OFFSET(d11, -(16 * 8))
    CFI_REL_OFFSET(d10, -(17 * 8))
    CFI_REL_OFFSET(d9, -(18 * 8))
    CFI_REL_OFFSET(d8, -(19 * 8))

    // Before we call DecrementHotnessCounterDyn we should set pointer to C2I frame in the TLS,
    // because compilation may fall into safepoint, so we need to make caller's callee registers
    // visible for the stack walker.
    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]

    PUSH_ARGS_REGS

    // Pass this function as second arg
    ldr x1, [fp, #24]
    mov x2, THREAD_REG
    bl MAKE_ASM_NAME(DecrementHotnessCounterDyn)

    // Compilation finished, so recover caller's frame in the TLS.
    ldr lr, [fp]
    str lr, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]

    uxtb x0, w0
    cbz x0, .Lnot_compiled

    POP_ARGS_REGS

    sub sp, fp, #8
    ldr fp, [sp, #8]
    CFI_REMEMBER_STATE
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, (4 * 8))
    ldr lr, [sp, #24]
    CFI_RESTORE(lr)
    add sp, sp, #32
    CFI_ADJUST_CFA_OFFSET(-(4 * 8))

    // invoke the method
    ldr x16, [x0, #METHOD_COMPILED_ENTRY_POINT_OFFSET]
    br  x16
    CFI_RESTORE_STATE
    CFI_DEF_CFA(fp, (3 * 8))

.Lnot_compiled:
    // Restore x0 and x1 args since they contain Method* and actual_num_args
    ldp x0, x1, [sp]

    // setup regs as follows to survive the call
    // x19          - method*
    // x20          - num_actual_args
    mov x19, x0
    mov w20, w1

    // create an interpreter frame
    mov w0, w1
    mov x1, x19
    mov x2, fp
    bl MAKE_ASM_NAME(CreateFrameForMethodWithActualArgsDyn)
    mov x21, x0

    // setup regs as follows
    // x19          - method*
    // w20          - num_actual_args
    // x21          - iframe*

    // w22          - num_iframe_args = max(num_actual_args, mehtod->num_args_)
    // x23          - iframe.vregs_ + num_vregs_
    ldr w22, [x19, #METHOD_NUM_ARGS_OFFSET]
    cmp w22, w20
    csel w22, w22, w20, hs
    ldr w23, [x21, #FRAME_NUM_VREGS_OFFSET]
    sub w23, w23, w22
    add x23, x21, x23, lsl 3
    add x23, x23, FRAME_VREGS_OFFSET

    cbz w20, .Linit_rest

    // copy actual args
    // x0           - incoming stack arguments
    add x0, fp, 24
    sub w1, w20, 1
    lsl w1, w1, 3
.Lloop_actual:
    ldr x2, [x0, x1]
    str x2, [x23, x1]
    subs w1, w1, 8
    bhs .Lloop_actual

.Linit_rest:
    // w22          - num_rest_args = num_iframe_args - num_actual_args
    subs w22, w22, w20
    beq .Lcall_interpreter

    // init rest args
    // %x0          - iframe.vregs_ + num_vregs_ + num_actual_args
    // %x23         - initial tagged value (TAGGED_VALUE_UNDEFINED)
    lsl w0, w20, 3
    add x0, x0, x23
    mov x23, TAGGED_VALUE_UNDEFINED
    sub w22, w22, 1
    lsl w22, w22, 3
.Lloop_rest:
    str x23, [x0, x22]
    subs x22, x22, FRAME_VREGISTER_SIZE
    bhs .Lloop_rest

.Lcall_interpreter:
#if defined(PANDA_WITH_ECMASCRIPT) && defined(ARK_INTRINSIC_SET)
    // Setup EcmascriptEnvironment
    // %x0       - this func (1st arg)
    // %x23      - iframe ext data
    ldr x0, [fp, #24]
    sub x23, x21, #(EXTFRAME_FRAME_OFFSET - EXTFRAME_EXT_DATA_OFFSET)

    // Store constant pool to iframe
    ldr x22, [x0, JSFUNCTION_CONSTANT_POOL_OFFSET]
    str x22, [x23, ECMASCRIPT_ENVIRONMENT_CONSTANT_POOL_OFFSET]
    
    // Store lexical env to iframe
    ldr x22, [x0, JSFUNCTION_LEXICAL_ENV_OFFSET]
    str x22, [x23, ECMASCRIPT_ENVIRONMENT_LEXICAL_ENV_OFFSET]

    // Store this func to iframe
    str x0, [x23, ECMASCRIPT_ENVIRONMENT_THIS_FUNC_OFFSET]
#endif

    // call InterpreterEntryPoint
    mov x0, x19
    mov x1, x21
    bl MAKE_ASM_NAME(InterpreterEntryPoint)

    // handle the result
    // setup regs as follows
    // x21 - iframe
    // x19, x20 - result
    add x20, x21, #FRAME_ACC_OFFSET
    ldp x19, x20, [x20]

    mov x0, x21
    bl MAKE_ASM_NAME(FreeFrame)

    mov x0, x19
    mov x1, x20

    sub sp, fp, #8
    // Restore callee registers, since GC may change its values while moving objects.
    mov x16, sp
    ldp x27, x28, [x16, #-16]!
    CFI_RESTORE(x28)
    CFI_RESTORE(x27)
    ldp x25, x26, [x16, #-16]!
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    ldp x23, x24, [x16, #-16]!
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    ldp x21, x22, [x16, #-16]!
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    ldp x19, x20, [x16, #-16]!
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)

    ldr fp, [sp, #8]
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, (4 * 8))
    ldr lr, [sp, #24]
    CFI_RESTORE(lr)
    add sp, sp, #32
    CFI_ADJUST_CFA_OFFSET(-(4 * 8))
    ret
    CFI_ENDPROC
