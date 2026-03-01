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
#include "arch/aarch64/shorty.S"
#include "shorty_values.h"

// uint32_t EtsNapiCalcStackArgsSpaceSize(Method *method, bool is_critical)
.extern MAKE_ASM_NAME(EtsNapiCalcStackArgsSpaceSize)
// void EtsNapiBeginCritical(Method *method, uintptr_t args, uintptr_t stack_args, uintptr_t out_args)
.extern MAKE_ASM_NAME(EtsNapiBeginCritical)
// void EtsNapiBegin(Method *method, uintptr_t args, uintptr_t stack_args, uintptr_t out_args)
.extern MAKE_ASM_NAME(EtsNapiBegin)
// void EtsNapiEnd(Method *method)
.extern MAKE_ASM_NAME(EtsNapiEnd)
// ObjectHeader *EtsNapiObjEnd(Method *method, Reference *ref)
.extern MAKE_ASM_NAME(EtsNapiObjEnd)
// bool IsEtsMethodFastNative(Method *method)
.extern MAKE_ASM_NAME(IsEtsMethodFastNative)


// The entrypoint for EtsNapi critical method.
// there are two nuances:
// 1. Each panda method accepts Method* in the first argument.
//    We have to drop this argument and shift others back
// 2. We have to convert double arguments and the return value to floats
//    Panda runtime operates only with doubles.
// The function handles all the cases above.

.global MAKE_ASM_NAME(EtsNapiCriticalNativeEntryPoint)
TYPE_FUNCTION(EtsNapiCriticalNativeEntryPoint)
MAKE_ASM_NAME(EtsNapiCriticalNativeEntryPoint):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(2 * 8)
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    mov fp, sp
    CFI_DEF_CFA_REGISTER(fp)
    sub sp, sp, #16
    str x0, [sp, #8]
    mov x16, #CFRAME_KIND_NATIVE
    str x16, [sp]
    stp x19, x20, [sp, #-16]!
    CFI_REL_OFFSET(x20, -(3 * 8))
    CFI_REL_OFFSET(x19, -(4 * 8))

    // Skip locals
    sub sp, sp, #(CFRAME_LOCALS_COUNT * 8)

    // save arguments to the stack
    stp d6, d7, [sp, #-16]!
    stp d4, d5, [sp, #-16]!
    stp d2, d3, [sp, #-16]!
    stp d0, d1, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!

    mov x19, x0 // save method to x19
    ldrb w20, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]
    mov w1, #1
    strb w1, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // Check num args. If all args could be settle in regs
    // don't calculate stack size
    ldr w0, [x19, #METHOD_NUM_ARGS_OFFSET]
    cmp w0, #8
    // reserve space for GPR and FPR args
    mov w0, #128
    ble .Lprepare_stack_critical

    mov x0, x19
    mov w1, #1
    bl MAKE_ASM_NAME(EtsNapiCalcStackArgsSpaceSize)

.Lprepare_stack_critical:
    mov x1, sp
    add x2, fp, #16
    sub sp, sp, x0
    mov x3, sp
    mov x4, THREAD_REG
    mov x0, x19
    bl MAKE_ASM_NAME(EtsNapiBeginCritical)

    // load the arguments
    ldp x0, x1, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x6, x7, [sp], #16
    ldp d0, d1, [sp], #16
    ldp d2, d3, [sp], #16
    ldp d4, d5, [sp], #16
    ldp d6, d7, [sp], #16

    // call the method
    ldr lr, [x19, #METHOD_NATIVE_POINTER_OFFSET]
    cbz lr, .Ldone
    blr lr

.Ldone:
    // return
    // signal handler of the sampling profiler use stack space below sp, 
    // so change it carefully only after registers restoration
    sub sp, fp, #32

    mov w10, w20

    ldp x19, x20, [sp], #32
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
    ldp fp, lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)

    strb w10, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // check native exception
    ldr x11, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp x11, #0
    // check frame is compiled
    ccmp w10, #0, #4, ne
    // check prev frame is true CFRAME and not BYPASS
    ldr x9, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    ccmp x9, #BYPASS_BRIDGE, 4, ne
    beq .Lexit_

    ldr x13, [fp, #(-CFRAME_FLAGS_SLOT * 8)]
    tbz x13, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, 1f

    add x12, fp, #-CALLER_VREG0_OFFSET
    ldp d0, d1, [x12]
    ldp d2, d3, [x12, #8*2]
    ldp d4, d5, [x12, #8*4]
    ldp d6, d7, [x12, #8*6]
    ldp d16, d17, [x12, #8*8]
    ldp d18, d19, [x12, #8*10]
    ldp d20, d21, [x12, #8*12]
    ldp d22, d23, [x12, #8*14]
    ldp d24, d25, [x12, #8*16]
    ldp d26, d27, [x12, #8*18]
    ldp d28, d29, [x12, #8*20]
    ldp d30, d31, [x12, #8*22]

1:
    ldp x0,  x1,  [fp, #-CALLER_REG0_OFFSET+8*0]
    ldp x2,  x3,  [fp, #-CALLER_REG0_OFFSET+8*2]
    ldp x4,  x5,  [fp, #-CALLER_REG0_OFFSET+8*4]
    ldp x6,  x7,  [fp, #-CALLER_REG0_OFFSET+8*6]
    ldp x8,  x9,  [fp, #-CALLER_REG0_OFFSET+8*8]
    ldp x10, x11, [fp, #-CALLER_REG0_OFFSET+8*10]
    ldp x12, x13, [fp, #-CALLER_REG0_OFFSET+8*12]
    ldp x14, x15, [fp, #-CALLER_REG0_OFFSET+8*14]
    ldp x16, x17, [fp, #-CALLER_REG0_OFFSET+8*16]
    ldr x18, [fp, #-CALLER_REG0_OFFSET+8*18]

    b MAKE_ASM_NAME(ThrowNativeExceptionBridge)

.Lexit_:
    ret
    CFI_ENDPROC


// |                        |
// |       Prev Frame       |
// |                        |
// +------------------------+
// |          ...           |
// |       stack args       |
// |          ...           |-11
// +---+---+----------------+
// |   |   |       LR       |-10
// |   |   |       FP       |-9
// |   |   |     Method *   |-8
// |   |   |      FLAGS     |-7
// |   |   +----------------+
// |   |   |       ...      |-6
// |   |   |      locals    |
// |   |   |       ...      |-1
// |   | c +--------+-------+
// |   | f |        |   x28 | 0
// |   | r |        |   x27 | 1
// |   | a |        |   x26 | 2
// |   | m |        |   x25 | 3
// |   | e |        |   x24 | 4
// |   |   |        |   x23 | 5
// |   |   |        |   x22 | 6
// |   |   |        |   x21 | 7
// |   |   |        |   x20 | 8
// |   |   |        |   x19 | 9
// |   |   |        +-------+
// |   |   |        |   d15 | 10
// |   |   |        |   d14 | 11
// |   |   |        |   d13 | 12
// |   |   |        |   d12 | 13
// |   |   |        |   d11 | 14
// | N |   |        |   d10 | 15
// | a |   |        |    d9 | 16
// | p |   |        |    d8 | 17
// | i +---+--------+-------+
// |   |            |    d7 | 18
// | f |            |    d6 | 19
// | r |            |    d5 | 20
// | a |            |    d4 | 21
// | m |            |    d3 | 22
// | e |            |    d2 | 23
// |   |            |    d1 | 24
// |   |            |    d0 | 25
// |   |     args   +-------+
// |   |            |    x7 | 26
// |   |            |    x6 | 27
// |   |            |    x5 | 28
// |   |            |    x4 | 29
// |   |            |    x3 | 30
// |   |            |    x2 | 31
// |   |            |    x1 | 32
// |   |            |    x0 | 33
// |   +-------+----+-------+
// |   |       |    Napi d7 | 34
// |   |       |    Napi d6 | 35
// |   |       |    Napi d5 | 36
// |   |       |    Napi d4 | 37
// |   |       |    Napi d3 | 38
// |   |       |    Napi d2 | 39
// |   |       |    Napi d1 | 40
// |   |       |    Napi d0 | 41
// |   |       +------------+
// |   |       |    Napi x7 | 42
// |   |  Napi |    Napi x6 | 43
// |   |  args |    Napi x5 | 44
// |   |       |    Napi x4 | 45
// |   |       |    Napi x3 | 46
// |   |       |    Napi x2 | 47
// |   |       |    Napi x1 | 48
// |   |       |    Napi x0 | 49
// |   |       +------------+
// |   |       | stack_argN | 50
// |   |       |   ...      |
// |   |       | stack_arg1 | 50+(N-1)
// |   |       | stack_arg0 | 50+(N-0)
// +---+-------+------------+
// |                        |

// The entrypoint for EtsNapi method
// Each panda method accepts Method* in the first argument
// The goal of this function is just drop this argument and
// shift other arguments back
.global MAKE_ASM_NAME(EtsNapiEntryPoint)
TYPE_FUNCTION(EtsNapiEntryPoint)
MAKE_ASM_NAME(EtsNapiEntryPoint):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(2 * 8)
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    mov fp, sp
    CFI_DEF_CFA_REGISTER(fp)
    sub sp, sp, #16
    str x0, [sp, #8]
    mov x16, #CFRAME_KIND_NATIVE
    str x16, [sp]

    // Skip locals
    sub sp, sp, #(CFRAME_LOCALS_COUNT * 8)

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    stp x27, x28, [sp, #-16]!
    CFI_REL_OFFSET(THREAD_REG, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
    CFI_REL_OFFSET(x27, -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8))
    stp x25, x26, [sp, #-16]!
    CFI_REL_OFFSET(x26, -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8))
    CFI_REL_OFFSET(x25, -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8))
    stp x23, x24, [sp, #-16]!
    CFI_REL_OFFSET(x24, -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8))
    CFI_REL_OFFSET(x23, -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 8))
    stp x21, x22, [sp, #-16]!
    CFI_REL_OFFSET(x22, -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 8))
    CFI_REL_OFFSET(x21, -((CFRAME_CALLEE_REGS_START_SLOT + 7) * 8))
    stp x19, x20, [sp, #-16]!
    CFI_REL_OFFSET(x20, -((CFRAME_CALLEE_REGS_START_SLOT + 8) * 8))
    CFI_REL_OFFSET(x19, -((CFRAME_CALLEE_REGS_START_SLOT + 9) * 8))
    stp d14, d15, [sp, #-16]!
    CFI_REL_OFFSET(d15, -((CFRAME_CALLEE_REGS_START_SLOT + 10) * 8))
    CFI_REL_OFFSET(d14, -((CFRAME_CALLEE_REGS_START_SLOT + 11) * 8))
    stp d12, d13, [sp, #-16]!
    CFI_REL_OFFSET(d13, -((CFRAME_CALLEE_REGS_START_SLOT + 12) * 8))
    CFI_REL_OFFSET(d12, -((CFRAME_CALLEE_REGS_START_SLOT + 13) * 8))
    stp d10, d11, [sp, #-16]!
    CFI_REL_OFFSET(d11, -((CFRAME_CALLEE_REGS_START_SLOT + 14) * 8))
    CFI_REL_OFFSET(d10, -((CFRAME_CALLEE_REGS_START_SLOT + 15) * 8))
    stp d8, d9, [sp, #-16]!
    CFI_REL_OFFSET(d9, -((CFRAME_CALLEE_REGS_START_SLOT + 16) * 8))
    CFI_REL_OFFSET(d8, -((CFRAME_CALLEE_REGS_START_SLOT + 17) * 8))

    // save arguments to the stack
    stp d6, d7, [sp, #-16]!
    stp d4, d5, [sp, #-16]!
    stp d2, d3, [sp, #-16]!
    stp d0, d1, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!

    mov x19, x0 // save method to x19
    ldrb w20, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // save shorty return type to x27 (w27)
    ldr x27, [x19, #METHOD_SHORTY_OFFSET]
    ldr w27, [x27]

    // save IsFastNative to x26
    mov x0, x19
    bl MAKE_ASM_NAME(IsEtsMethodFastNative)
    mov x26, x0

    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]
    mov w1, #1
    strb w1, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // Check num args. If all args could be settle in regs
    // don't calculate stack size
    ldr w0, [x19, #METHOD_NUM_ARGS_OFFSET]
    cmp w0, #6
    // reserve space for GPR and FPR args
    mov w0, #128
    ble .Lprepare_stack

    mov x0, x19
    mov w1, #0
    bl MAKE_ASM_NAME(EtsNapiCalcStackArgsSpaceSize)

.Lprepare_stack:
    mov x1, sp
    add x2, fp, #16
    sub sp, sp, x0
    mov x3, sp
    mov x0, x19
    mov x4, THREAD_REG
    bl MAKE_ASM_NAME(EtsNapiBegin)

    mov lr, x0
    // load the arguments
    ldp x0, x1, [lr], #16
    ldp x2, x3, [lr], #16
    ldp x4, x5, [lr], #16
    ldp x6, x7, [lr], #16
    ldp d0, d1, [lr], #16
    ldp d2, d3, [lr], #16
    ldp d4, d5, [lr], #16
    ldp d6, d7, [lr], #16

1:  // call the method
    ldr lr, [x19, #METHOD_NATIVE_POINTER_OFFSET]
    cbz lr, .Lend
    blr lr

    // handle the result
    and w27, w27, #0xF
    cmp w27, #SHORTY_VOID
    bne 1f
    // void method
    mov x0, x19
    mov x1, THREAD_REG
    mov x2, x26
    bl MAKE_ASM_NAME(EtsNapiEnd)
    b .Lreturn

1:  cmp w27, #SHORTY_REFERENCE
    bne 4f
    // ref method
    mov x3, x26
    mov x2, THREAD_REG
    mov x1, x0
    mov x0, x19
    bl MAKE_ASM_NAME(EtsNapiObjEnd)
    b .Lreturn
4:  sub w27, w27, #SHORTY_FIRST_FLOAT
    cmp w27, #(SHORTY_NUM_FLOAT_TYPES - 1)
    bls 2f
    // the return value is integer
    mov x1, x19
    mov x19, x0
    mov x0, x1
    mov x1, THREAD_REG
    mov x2, x26
    bl MAKE_ASM_NAME(EtsNapiEnd)
    mov x0, x19
    b .Lreturn
2:  mov x0, x19
    fmov x19, d0
    mov x1, THREAD_REG
    mov x2, x26
    bl MAKE_ASM_NAME(EtsNapiEnd)
    fmov d0, x19
    b .Lreturn

.Lend:
    mov x1, THREAD_REG
    mov x0, x19
    mov x2, x26
    bl MAKE_ASM_NAME(EtsNapiEnd)
    b .Lreturn

.Lreturn:
    // Restore callee registers, since GC may change its values while moving objects.
    // Signal handler of the sampling profiler use stack space below sp, 
    // so change it carefully only after registers restoration
    sub sp, fp, #(((CFRAME_LR_SLOT - CFRAME_PREV_FRAME_SLOT) + CFRAME_HEADER_SIZE + CFRAME_LOCALS_COUNT + GP_CALLEE_REGS_COUNT - 1) * 8)

    mov w10, w20

    ldp x19, x20, [sp], #16
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
    ldp x21, x22, [sp], #16
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    ldp x23, x24, [sp], #16
    CFI_RESTORE(x24)
    CFI_RESTORE(x2)
    ldp x25, x26, [sp], #16
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    ldp x27, x28, [sp], #16
    CFI_RESTORE(THREAD_REG)
    CFI_RESTORE(x27)
    
    mov sp, fp
    ldp fp, lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)

    strb w10, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    // check native exception
    ldr x11, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp x11, #0
    // check frame is compiled
    ccmp w10, #0, #4, ne
    // check prev frame is true CFRAME and not BYPASS
    ldr x9, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    ccmp x9, #BYPASS_BRIDGE, 4, ne
    beq .Lexit

    ldr x13, [fp, #(-CFRAME_FLAGS_SLOT * 8)]
    tbz x13, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, 2f

    add x12, fp, #-CALLER_VREG0_OFFSET
    ldp d0, d1, [x12]
    ldp d2, d3, [x12, #8*2]
    ldp d4, d5, [x12, #8*4]
    ldp d6, d7, [x12, #8*6]
    ldp d16, d17, [x12, #8*8]
    ldp d18, d19, [x12, #8*10]
    ldp d20, d21, [x12, #8*12]
    ldp d22, d23, [x12, #8*14]
    ldp d24, d25, [x12, #8*16]
    ldp d26, d27, [x12, #8*18]
    ldp d28, d29, [x12, #8*20]
    ldp d30, d31, [x12, #8*22]

2:
    ldp x0,  x1,  [fp, #-CALLER_REG0_OFFSET+8*0]
    ldp x2,  x3,  [fp, #-CALLER_REG0_OFFSET+8*2]
    ldp x4,  x5,  [fp, #-CALLER_REG0_OFFSET+8*4]
    ldp x6,  x7,  [fp, #-CALLER_REG0_OFFSET+8*6]
    ldp x8,  x9,  [fp, #-CALLER_REG0_OFFSET+8*8]
    ldp x10, x11, [fp, #-CALLER_REG0_OFFSET+8*10]
    ldp x12, x13, [fp, #-CALLER_REG0_OFFSET+8*12]
    ldp x14, x15, [fp, #-CALLER_REG0_OFFSET+8*14]
    ldp x16, x17, [fp, #-CALLER_REG0_OFFSET+8*16]
    ldr x18, [fp, #-CALLER_REG0_OFFSET+8*18]

    b MAKE_ASM_NAME(ThrowNativeExceptionBridge)

.Lexit:
    ret
    CFI_ENDPROC
