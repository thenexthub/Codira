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

// Frame* CreateFrameForMethodWithActualArgsDyn(uint32_t num_actual_args, Method* method, Frame* prev);
.extern CreateFrameForMethodWithActualArgsDyn
// void InterpreterEntryPoint(Method* pc, Frame* frame, bool is_dynamic);
.extern InterpreterEntryPoint
// void FreeFrame(Frame* frame);
.extern FreeFrame
// bool DecrementHotnessCounterDyn(Method* method, TaggedValue func_obj);
.extern DecrementHotnessCounterDyn

// CompiledCodeToInterpreterBridgeDyn, follows DynamicMethod calling convention
.global CompiledCodeToInterpreterBridgeDyn
TYPE_FUNCTION(CompiledCodeToInterpreterBridgeDyn)
CompiledCodeToInterpreterBridgeDyn:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // method:      %rdi
    // num_args:    %rsi
    // arg_i:       24(%rbp, i, 8)

    // Save return address to the TLS field
    movq (%rsp), %rax
    movq %rax, MANAGED_THREAD_NATIVE_PC_OFFSET(%THREAD_REG)

    pushq $COMPILED_CODE_TO_INTERPRETER_BRIDGE
    CFI_ADJUST_CFA_OFFSET(8)

    // construct the frame
    pushq %rbp
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(rbp, 0)
    movq %rsp, %rbp // set frame pointer
    CFI_DEF_CFA_REGISTER(rbp)

    subq $8, %rsp

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    pushq %r15
    CFI_REL_OFFSET(r15, -(2 * 8))
    pushq %r14
    CFI_REL_OFFSET(r14, -(3 * 8))
    pushq %r13
    CFI_REL_OFFSET(r13, -(4 * 8))
    pushq %r12
    CFI_REL_OFFSET(r12, -(5 * 8))
    pushq %rbx
    CFI_REL_OFFSET(rbx, -(6 * 8))

    subq $8, %rsp

    // Before we call DecrementHotnessCounterDyn we should set pointer to C2I frame in the TLS,
    // because compilation may fall into safepoint, so we need to make caller's callee registers
    // visible for the stack walker.
    movq %rbp, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)

    PUSH_GENERAL_REGS
    // Pass this function as second arg
    movq 24(%rbp), %rsi
    movq %THREAD_REG, %rdx
    callq DecrementHotnessCounterDyn@plt
    cmpb $0, %al
    je .Lnot_compiled

    // Compilation successfully done, so recover caller's frame in the TLS, since C2I is not needed.
    movq (%rbp), %r8
    movq %r8, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)

    POP_GENERAL_REGS

    movq %rbp, %rsp
    addq $16, %rsp
    movq (%rbp), %rbp
    CFI_REMEMBER_STATE
    CFI_RESTORE(rbp)
    CFI_DEF_CFA(rsp, 8)

    movq METHOD_COMPILED_ENTRY_POINT_OFFSET(%rdi), %rax
    jmp *%rax
    CFI_RESTORE_STATE
    CFI_DEF_CFA(rbp, (3 * 8))

.Lnot_compiled:
    RELOAD_GENERAL_REGS

    // Create an interpreter frame
    // %r12         - iframe*
    xchgq %rsi, %rdi
    movq %rbp, %rdx
    // Frame* CreateFrameForMethodWithActualArgsDyn(uint32_t num_actual_args, Method* method, Frame* prev);
    callq CreateFrameForMethodWithActualArgsDyn@plt
    movq %rax, %r12

    POP_GENERAL_REGS

    // setup regs as follows
    // %r12         - iframe*
    // %rdi         - method*
    // %rsi         - num_actual_args

    // %r15d        - num_iframe_args = max(num_actual_args, mehtod->num_args_)
    // %rax         - iframe.vregs_ + num_vregs_
    movl METHOD_NUM_ARGS_OFFSET(%rdi), %r15d
    cmpl %r15d, %esi
    cmovge %esi, %r15d
    movl FRAME_NUM_VREGS_OFFSET(%r12), %eax
    subl %r15d, %eax
    leaq FRAME_VREGS_OFFSET(%r12, %rax, FRAME_VREGISTER_SIZE), %rax

    test %rsi, %rsi
    jz .Linit_rest

    // copy actual args
    // %rdx         - incoming stack arguments
    leaq 24(%rbp), %rdx
    leal -FRAME_VREGISTER_SIZE(, %esi, FRAME_VREGISTER_SIZE), %r14d
.Lloop_actual:
    movq (%rdx, %r14), %rcx
    movq %rcx, (%rax, %r14)
    subl $FRAME_VREGISTER_SIZE, %r14d
    jae .Lloop_actual

.Linit_rest:
    // r15d         - num_rest_args = num_iframe_args - num_actual_args
    subl %esi, %r15d
    jz .Lcall_interpreter

    // init rest args
    // %r14         - iframe.vregs_ + num_vregs_ + num_actual_args
    // %rax         - initial tagged value (TAGGED_VALUE_UNDEFINED)
    leaq (%rax, %rsi, FRAME_VREGISTER_SIZE), %r14
    movq $TAGGED_VALUE_UNDEFINED, %rax
    leal -FRAME_VREGISTER_SIZE(, %r15d, FRAME_VREGISTER_SIZE), %r15d
.Lloop_rest:
    movq %rax, (%r14, %r15)
    subl $FRAME_VREGISTER_SIZE, %r15d
    jae .Lloop_rest

.Lcall_interpreter:
#if defined(PANDA_WITH_ECMASCRIPT) && defined(ARK_INTRINSIC_SET)
    // Setup EcmascriptEnvironment
    // %r14       - this func (1st arg)
    // %rax       - iframe ext data
    movq 24(%rbp), %r14

    leaq (EXTFRAME_EXT_DATA_OFFSET-EXTFRAME_FRAME_OFFSET)(%r12), %rax
    
    // Store constant pool to iframe
    movq JSFUNCTION_CONSTANT_POOL_OFFSET(%r14), %r15
    movq %r15, ECMASCRIPT_ENVIRONMENT_CONSTANT_POOL_OFFSET(%rax)
    
    // Store lexical env to iframe
    movq JSFUNCTION_LEXICAL_ENV_OFFSET(%r14), %r15
    movq %r15, ECMASCRIPT_ENVIRONMENT_LEXICAL_ENV_OFFSET(%rax)

    // Store this func to iframe
    movq %r14, ECMASCRIPT_ENVIRONMENT_THIS_FUNC_OFFSET(%rax)
#endif

    // call InterpreterEntryPoint
    movq %r12, %rsi
    // void InterpreterEntryPoint(Method *method, Frame* frame);
    callq InterpreterEntryPoint@plt

    // handle the result
    // setup regs as follows
    // %r12         - iframe*
    // %r13/%r14    - result
    leaq FRAME_ACC_OFFSET(%r12), %rax
    movq (%rax), %r13
    movq FRAME_ACC_MIRROR_OFFSET(%rax), %r14

    movq %r12, %rdi
    // void FreeFrame(Frame* frame);
    callq FreeFrame@plt

    movq %r13, %rax
    movq %r14, %rdx

    leaq -48(%rbp), %rsp
    popq %rbx
    CFI_RESTORE(rbx)
    popq %r12
    CFI_RESTORE(r12)
    popq %r13
    CFI_RESTORE(r13)
    popq %r14
    CFI_RESTORE(r14)
    popq %r15
    CFI_RESTORE(r15)
    addq $8, %rsp

    popq %rbp
    CFI_RESTORE(rbp)
    CFI_DEF_CFA(rsp, (2 * 8))
    addq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(-(1 * 8))
    retq
    CFI_ENDPROC
