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
#include "arch/amd64/helpers_amd64.S"
#include "arch/amd64/shorty.S"
#include "shorty_values.h"

// uint32_t EtsNapiCalcStackArgsSpaceSize(Method *method, bool is_critical)
.extern EtsNapiCalcStackArgsSpaceSize
// void EtsNapiBeginCritical(Method *method, uintptr_t args, uintptr_t stack_args, uintptr_t out_args)
.extern EtsNapiBeginCritical
// void EtsNapiBegin(Method *method, uintptr_t args, uintptr_t stack_args, uintptr_t out_args)
.extern EtsNapiBegin
// void EtsNapiEnd(Method *method)
.extern EtsNapiEnd
// ObjectHeader *EtsNapiObjEnd(Method *method, Reference *ref)
.extern EtsNapiObjEnd
// bool IsEtsMethodFastNative(Method *method)
.extern IsEtsMethodFastNative


// The entrypoint for EtsNapi critical method.
// there are two nuances:
// 1. Each panda method accepts Method* in the first argument.
//    We have to drop this argument and shift others back
// 2. We have to convert double arguments and the return value to floats
//    Panda runtime operates only with doubles.
// The function handles all the cases above.
.global EtsNapiCriticalNativeEntryPoint
.type EtsNapiCriticalNativeEntryPoint, %function
EtsNapiCriticalNativeEntryPoint:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // %rsp % 16 == 8
    pushq %rbp
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(rbp, 0)

    movq %rsp, %rbp // frame pointer
    CFI_DEF_CFA_REGISTER(rbp)

    pushq %rdi
    pushq $CFRAME_KIND_NATIVE

    // callee-saved
    pushq %rbx
    CFI_REL_OFFSET(rbx, -(3 * 8))
    pushq %r12
    CFI_REL_OFFSET(r12, -(4 * 8))

    // Skip locals
    subq $(CFRAME_LOCALS_COUNT * 8), %rsp

    // %rsp % 16 == 0

    // save arguments to the stack
    PUSH_FP_REGS

    PUSH_GENERAL_REGS
    // %rsp % 16 == 0

    movq %rdi, %rbx // save method to %rbx
    movb MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG), %r12b

    // Update current frame in the thread
    movq %rbp, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)
    movb $1, MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG)

    // Check num args. If all args could be settle in regs
    // don't calculate stack size
    mov METHOD_NUM_ARGS_OFFSET(%rbx), %eax
    cmp $6, %eax
    // reserve space for all GRP and float regs
    mov $112, %eax
    jle .Lprepare_stack_critical

    // Update native_stack_args_space_size_ if native_stack_args_space_size_ == 0
    movq %rbx, %rdi
    movq $1, %rsi
    callq EtsNapiCalcStackArgsSpaceSize@plt

.Lprepare_stack_critical:
    movq %rsp, %rsi
    leaq 16(%rbp), %rdx
    subq %rax, %rsp // %rsp % 16 == 0
    movq %rsp, %rcx
    movq %rbx, %rdi
    movq %THREAD_REG, %r8
    callq EtsNapiBeginCritical@plt

    // load the arguments
    POP_GENERAL_REGS

    POP_FP_REGS
    // %rsp % 16 == 0

    // call the method
    movq METHOD_NATIVE_POINTER_OFFSET(%rbx), %rax
    testq %rax, %rax
    je .Ldone
    callq *%rax

.Ldone:
    movb %r12b, %r10b

    // return
    leaq -32(%rbp), %rsp
    popq %r12
    CFI_RESTORE(r12)
    popq %rbx
    CFI_RESTORE(rbx)
    addq $16, %rsp
    popq %rbp
    CFI_RESTORE(rbp)
    CFI_DEF_CFA(rsp, (1 * 8))

    movb %r10b, MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG)

    // check native exception
    movq MANAGED_THREAD_EXCEPTION_OFFSET(%THREAD_REG), %r11
    testq %r11, %r11
    jz .Lexit_

    // check frame is compiled
    testb %r10b, %r10b
    jz .Lexit_

    // check prev frame is true CFRAME and not BYPASS
    cmpq $BYPASS_BRIDGE, (SLOT_SIZE * COMP_METHOD_OFFSET)(%rbp)
    je .Lexit_

    movq (-CALLER_REG0_OFFSET + 0)(%rbp), %rax
    movq (-CALLER_REG0_OFFSET + 8)(%rbp), %rcx
    movq (-CALLER_REG0_OFFSET + 16)(%rbp), %rdx
    movq (-CALLER_REG0_OFFSET + 24)(%rbp), %r11
    movq (-CALLER_REG0_OFFSET + 32)(%rbp), %r10
    movq (-CALLER_REG0_OFFSET + 40)(%rbp), %r9
    movq (-CALLER_REG0_OFFSET + 48)(%rbp), %rsi
    movq (-CALLER_REG0_OFFSET + 56)(%rbp), %rdi
    movq (-CALLER_REG0_OFFSET + 64)(%rbp), %r8

    testq $CFRAME_HAS_FLOAT_REGS_FLAG_MASK, (-CFRAME_FLAGS_SLOT * SLOT_SIZE)(%rbp)
    jz 1f

    movsd (-CALLER_VREG0_OFFSET + 0)(%rbp), %xmm0
    movsd (-CALLER_VREG0_OFFSET + 8)(%rbp), %xmm1
    movsd (-CALLER_VREG0_OFFSET + 16)(%rbp), %xmm2
    movsd (-CALLER_VREG0_OFFSET + 24)(%rbp), %xmm3
    movsd (-CALLER_VREG0_OFFSET + 32)(%rbp), %xmm4
    movsd (-CALLER_VREG0_OFFSET + 40)(%rbp), %xmm5
    movsd (-CALLER_VREG0_OFFSET + 48)(%rbp), %xmm6
    movsd (-CALLER_VREG0_OFFSET + 56)(%rbp), %xmm7
    movsd (-CALLER_VREG0_OFFSET + 64)(%rbp), %xmm8
    movsd (-CALLER_VREG0_OFFSET + 72)(%rbp), %xmm9
    movsd (-CALLER_VREG0_OFFSET + 80)(%rbp), %xmm10
    movsd (-CALLER_VREG0_OFFSET + 88)(%rbp), %xmm11
    movsd (-CALLER_VREG0_OFFSET + 96)(%rbp), %xmm12
    movsd (-CALLER_VREG0_OFFSET + 104)(%rbp), %xmm13
    movsd (-CALLER_VREG0_OFFSET + 112)(%rbp), %xmm14
    movsd (-CALLER_VREG0_OFFSET + 120)(%rbp), %xmm15

1:
    jmp ThrowNativeExceptionBridge@plt

.Lexit_:
    retq
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
// |   | f |        |   r15 | 0
// |   | r |        |   r14 | 1
// |   | a |        |   r13 | 2
// | N | m |        |   r12 | 3
// | a | e |        |   rbx | 4
// | p |   |        | <skip>| 5
// | i +---+--------+-------+
// |   |            |  xmm7 | 6
// | f |            |  xmm6 | 7
// | r |            |  xmm5 | 8
// | a |            |  xmm4 | 9
// | m |            |  xmm3 | 10
// | e |            |  xmm2 | 11
// |   |            |  xmm1 | 12
// |   |     args   |  xmm0 | 13
// |   |            +-------+
// |   |            |    r9 | 14
// |   |            |    r8 | 15
// |   |            |   rcx | 16
// |   |            |   rdx | 17
// |   |            |   rsi | 18
// |   |            |   rdi | 19
// |   +-------+----+-------+
// |   |       |  Napi xmm7 | 20
// |   |       |  Napi xmm6 | 21
// |   |       |  Napi xmm5 | 22
// |   |       |  Napi xmm4 | 23
// |   |       |  Napi xmm3 | 24
// |   |       |  Napi xmm2 | 25
// |   |       |  Napi xmm1 | 26
// |   |       |  Napi xmm0 | 27
// |   |       +------------+
// |   |       |   Napi  r9 | 28
// |   | Napi  |   Napi  r8 | 29
// |   | args  |   Napi rcx | 30
// |   |       |   Napi rdx | 31
// |   |       |   Napi rsi | 32
// |   |       |   Napi rdi | 33
// |   |       +------------+
// |   |       | stack_argN | 34
// |   |       |   ...      |
// |   |       | stack_arg1 | 34+(N-1)
// |   |       | stack_arg0 | 34+(N-0)
// +---+-------+------------+
// |                        |

// The entrypoint for EtsNapi method
// Each panda method accepts Method* in the first argument
// The goal of this function is just drop this argument and
// shift other arguments back
.global EtsNapiEntryPoint
.type EtsNapiEntryPoint, %function
EtsNapiEntryPoint:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // %rsp % 16 == 8
    pushq %rbp
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(rbp, 0)

    movq %rsp, %rbp // frame pointer
    CFI_DEF_CFA_REGISTER(rbp)

    pushq %rdi
    pushq $CFRAME_KIND_NATIVE

    // Skip locals
    subq $(CFRAME_LOCALS_COUNT * 8), %rsp

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    pushq %r15
    CFI_REL_OFFSET(r15, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
    pushq %r14
    CFI_REL_OFFSET(r14, -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8))
    pushq %r13
    CFI_REL_OFFSET(r13, -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8))
    pushq %r12
    CFI_REL_OFFSET(r12, -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8))
    pushq %rbx
    CFI_REL_OFFSET(rbx, -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8))

    subq $8, %rsp
    // %rsp % 16 == 0

    // save arguments to the stack
    PUSH_FP_REGS

    PUSH_GENERAL_REGS
    // %rsp % 16 == 0

    movq %rdi, %rbx // save method to %rbx
    movb MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG), %r12b

    // Update current frame in the thread
    movq %rbp, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)
    movb $1, MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG)

    // save return type to %r14d
    movq METHOD_SHORTY_OFFSET(%rbx), %rcx
    movl (%rcx), %r14d

    // save IsFastNative to %r13
    movq %rbx, %rdi
    callq IsEtsMethodFastNative@plt
    movq %rax, %r13

    // Check num args. If all args could be settle in regs
    // don't calculate stack size
    mov METHOD_NUM_ARGS_OFFSET(%rbx), %eax
    cmp $4, %eax
    // reserve space for all GRP and float regs
    mov $112, %eax
    jle .Lprepare_stack

    movq %rbx, %rdi
    xorq %rsi, %rsi
    callq EtsNapiCalcStackArgsSpaceSize@plt

.Lprepare_stack:
    movq %rsp, %rsi
    leaq 16(%rbp), %rdx
    subq %rax, %rsp // %rsp % 16 == 0
    movq %rsp, %rcx
    movq %rbx, %rdi
    movq %THREAD_REG, %r8
    callq EtsNapiBegin@plt

    // save SP
    movq %rsp, %r11
    movq %rax, %rsp

    // load the arguments
    POP_GENERAL_REGS

    POP_FP_REGS
    // %rsp % 16 == 0

    // restore SP
    movq %r11, %rsp

    // call the method
    movq METHOD_NATIVE_POINTER_OFFSET(%rbx), %rax
    testq %rax, %rax
    je .Lend
    callq *%rax

    // handle the result
    andl $0xF, %r14d
    cmpl $SHORTY_VOID, %r14d
    jne 1f

    // void method
    movq %rbx, %rdi
    movq %THREAD_REG, %rsi
    movq %r13, %rdx
    callq EtsNapiEnd@plt
    jmp .Lreturn

1:  cmpl $SHORTY_REFERENCE, %r14d
    jne 4f

    // ref method
    movq %rbx, %rdi
    movq %rax, %rsi
    movq %THREAD_REG, %rdx
    movq %r13, %rcx
    callq EtsNapiObjEnd@plt
    jmp .Lreturn

4:  subl $SHORTY_FIRST_FLOAT, %r14d
    cmpl $(SHORTY_NUM_FLOAT_TYPES - 1), %r14d
    jbe 2f

    // the return value is integer
    pushq %rax
    subq $8, %rsp
    movq %rbx, %rdi
    movq %THREAD_REG, %rsi
    movq %r13, %rdx
    callq EtsNapiEnd@plt
    addq $8, %rsp
    popq %rax
    jmp .Lreturn

2:
    subq $16, %rsp
    movsd %xmm0, (%rsp)
    movq %rbx, %rdi
    movq %THREAD_REG, %rsi
    movq %r13, %rdx
    callq EtsNapiEnd@plt
    movsd (%rsp), %xmm0
    addq $16, %rsp
    jmp .Lreturn

.Lend:
    movq %rbx, %rdi
    movq %THREAD_REG, %rsi
    movq %r13, %rdx
    callq EtsNapiEnd@plt

.Lreturn:
    movb %r12b, %r10b

    // Restore callee registers, since GC may change its values while moving objects.
    movq -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8)(%rbp), %r15
    CFI_RESTORE(r15)
    movq -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8)(%rbp), %r14
    CFI_RESTORE(r14)
    movq -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8)(%rbp), %r13
    CFI_RESTORE(r13)
    movq -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8)(%rbp), %r12
    CFI_RESTORE(r12)
    movq -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8)(%rbp), %rbx
    CFI_RESTORE(rbx)
    movq %rbp, %rsp
    popq %rbp
    CFI_RESTORE(rbp)
    CFI_DEF_CFA(rsp, (1 * 8))

    movb %r10b, MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG)

    // check native exception
    movq MANAGED_THREAD_EXCEPTION_OFFSET(%THREAD_REG), %r11
    testq %r11, %r11
    jz .Lexit

    // check frame is compiled
    testb %r10b, %r10b
    jz .Lexit

    // check prev frame is true CFRAME and not BYPASS
    cmpq $BYPASS_BRIDGE, (SLOT_SIZE * COMP_METHOD_OFFSET)(%rbp)
    je .Lexit

    movq (-CALLER_REG0_OFFSET + 0)(%rbp), %rax
    movq (-CALLER_REG0_OFFSET + 8)(%rbp), %rcx
    movq (-CALLER_REG0_OFFSET + 16)(%rbp), %rdx
    movq (-CALLER_REG0_OFFSET + 24)(%rbp), %r11
    movq (-CALLER_REG0_OFFSET + 32)(%rbp), %r10
    movq (-CALLER_REG0_OFFSET + 40)(%rbp), %r9
    movq (-CALLER_REG0_OFFSET + 48)(%rbp), %rsi
    movq (-CALLER_REG0_OFFSET + 56)(%rbp), %rdi
    movq (-CALLER_REG0_OFFSET + 64)(%rbp), %r8

    testq $CFRAME_HAS_FLOAT_REGS_FLAG_MASK, (-CFRAME_FLAGS_SLOT * SLOT_SIZE)(%rbp)
    jz 1f

    movsd (-CALLER_VREG0_OFFSET + 0)(%rbp), %xmm0
    movsd (-CALLER_VREG0_OFFSET + 8)(%rbp), %xmm1
    movsd (-CALLER_VREG0_OFFSET + 16)(%rbp), %xmm2
    movsd (-CALLER_VREG0_OFFSET + 24)(%rbp), %xmm3
    movsd (-CALLER_VREG0_OFFSET + 32)(%rbp), %xmm4
    movsd (-CALLER_VREG0_OFFSET + 40)(%rbp), %xmm5
    movsd (-CALLER_VREG0_OFFSET + 48)(%rbp), %xmm6
    movsd (-CALLER_VREG0_OFFSET + 56)(%rbp), %xmm7
    movsd (-CALLER_VREG0_OFFSET + 64)(%rbp), %xmm8
    movsd (-CALLER_VREG0_OFFSET + 72)(%rbp), %xmm9
    movsd (-CALLER_VREG0_OFFSET + 80)(%rbp), %xmm10
    movsd (-CALLER_VREG0_OFFSET + 88)(%rbp), %xmm11
    movsd (-CALLER_VREG0_OFFSET + 96)(%rbp), %xmm12
    movsd (-CALLER_VREG0_OFFSET + 104)(%rbp), %xmm13
    movsd (-CALLER_VREG0_OFFSET + 112)(%rbp), %xmm14
    movsd (-CALLER_VREG0_OFFSET + 120)(%rbp), %xmm15

1:
    jmp ThrowNativeExceptionBridge@plt

.Lexit:
    retq
    CFI_ENDPROC
