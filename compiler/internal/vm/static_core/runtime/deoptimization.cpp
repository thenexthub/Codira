/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "runtime/deoptimization.h"

#include "include/cframe.h"
#include "include/managed_thread.h"
#include "include/stack_walker.h"
#include "libarkbase/events/events.h"
#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/profiling/profiling-inl.h"
#include "runtime/mem/rendezvous.h"

namespace ark {

/**
 * @brief Deoptimize CFrame that lies after another CFrame.
 * @param thread       Pointer to thread
 * @param pc           PC from which interpreter starts execution
 * @param frame        Pointer to the first interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param cframe_fp    Pointer to Cframe to be deoptimized
 * @param last_frame   Pointer to the last interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param callee_regs  Pointer to a callee-saved registers buffer from StackWalker
 */
// CC-OFFNXT(readability-function-size_parameters) asm bridge
extern "C" [[noreturn]] void DeoptimizeAfterCFrame(ManagedThread *thread, const uint8_t *pc, Frame *frame,
                                                   void *cframeFp, Frame *lastFrame, void *calleeRegs);
/**
 * @brief Deoptimize CFrame that lies after interpreter frame.
 * @param thread       Pointer to thread
 * @param pc           PC from which interpreter starts execution
 * @param frame        Pointer to the first interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param cframe_fp    Pointer to Cframe to be deoptimized
 * @param last_frame   Pointer to the last interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param callee_regs  Pointer to a callee-saved registers buffer from StackWalker
 */
// CC-OFFNXT(readability-function-size_parameters) asm bridge
extern "C" [[noreturn]] void DeoptimizeAfterIFrame(ManagedThread *thread, const uint8_t *pc, Frame *frame,
                                                   void *cframeFp, Frame *lastFrame, void *calleeRegs);
/**
 * @brief Drop given CFrame and return to its caller.
 * Drop means that we set stack pointer to the top of given CFrame, restores its return address and invoke `return`
 * instruction.
 * @param cframe_fp    Pointer to Cframe to be dropped
 * @param callee_regs  Pointer to a callee-saved registers buffer from StackWalker
 */
extern "C" [[noreturn]] void DropCompiledFrameAndReturn(void *cframeFp, void *calleeVregs);

static void UnpoisonAsanStack([[maybe_unused]] void *ptr)
{
#ifdef PANDA_ASAN_ON
    uint8_t sp;
    ASAN_UNPOISON_MEMORY_REGION(&sp, reinterpret_cast<uint8_t *>(ptr) - &sp);
#endif  // PANDA_ASAN_ON
}

#ifdef PANDA_EVENTS_ENABLED
static bool InvalidateCompiledMethod(ManagedThread *thread, Method *method, bool isCha,
                                     size_t &inStackCount) NO_THREAD_SAFETY_ANALYSIS
#else
static bool InvalidateCompiledMethod(ManagedThread *thread, Method *method, bool isCha) NO_THREAD_SAFETY_ANALYSIS
#endif
{
    ASSERT(thread != nullptr);
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        if (stack.IsCFrame() && stack.GetMethod() == method) {
            auto &cframe = stack.GetCFrame();
            cframe.SetShouldDeoptimize(true);
            cframe.SetDeoptCodeEntry(stack.GetCompiledCodeEntry());
            if (isCha) {
                LOG(DEBUG, CLASS_LINKER) << "[CHA]   Set ShouldDeoptimize for method: "
                                         << cframe.GetMethod()->GetFullName();
            } else {
                LOG(DEBUG, CLASS_LINKER) << "[IC]   Set ShouldDeoptimize for method: "
                                         << cframe.GetMethod()->GetFullName();
            }
#ifdef PANDA_EVENTS_ENABLED
            inStackCount++;
#endif
        }
    }
    return true;
}

// NO_THREAD_SAFETY_ANALYSIS because it doesn't know about mutator_lock status in this scope
void InvalidateCompiledEntryPoint(const PandaSet<Method *> &methods, bool isCha) NO_THREAD_SAFETY_ANALYSIS
{
    PandaVM *vm = Thread::GetCurrent()->GetVM();
    ScopedSuspendAllThreadsRunning ssat(vm->GetRendezvous());
    // NOTE(msherstennikov): remove this loop and check `methods` contains frame's method in stack traversing
    for (const auto &method : methods) {
#ifdef PANDA_EVENTS_ENABLED
        size_t inStackCount = 0;
        vm->GetThreadManager()->EnumerateThreads([method, &inStackCount, isCha](ManagedThread *thread) {
            return InvalidateCompiledMethod(thread, method, isCha, inStackCount);
#else
        vm->GetThreadManager()->EnumerateThreads([method, isCha](ManagedThread *thread) {
            return InvalidateCompiledMethod(thread, method, isCha);
#endif
        });
        if (isCha) {
            EVENT_CHA_DEOPTIMIZE(std::string(method->GetFullName()), inStackCount);
        }
        // NOTE (Trubenkov)  clean up compiled code(See issue 1706)
        method->SetInterpreterEntryPoint();
        Thread::GetCurrent()->GetVM()->GetCompiler()->RemoveOsrCode(method);
        // If deoptimization ocure during OSR compilation, we reset status after finish the compilation
        if (method->GetCompilationStatus() != Method::COMPILATION) {
            method->SetCompilationStatus(Method::NOT_COMPILED);
        }
    }
}

void PrevFrameDeopt(FrameKind prevFrameKind, ManagedThread *thread, StackWalker *stack, const uint8_t *pc,
                    Frame *lastIframe, Frame *iframe, CFrame &cframe)
{
    switch (prevFrameKind) {
        case FrameKind::COMPILER:
            LOG(DEBUG, INTEROP) << "Deoptimize after cframe";
            EVENT_DEOPTIMIZATION(std::string(cframe.GetMethod()->GetFullName()),
                                 pc - stack->GetMethod()->GetInstructions(), events::DeoptimizationAfter::CFRAME);
            // We need to set current frame kind to `compiled` as it's possible that we came here from other Deoptimize
            // call and in this case frame kind will be `non-compiled`:
            //
            //     compiled code
            //          |
            //      Deoptimize
            //          |
            // DeoptimizeAfterCFrame
            //          |
            // (change frame kind to `non-compiled`) interpreter::Execute -- we don't return after this call
            //          |
            // FindCatchBlockInCallStack
            //          |
            //      Deoptimize
            thread->SetCurrentFrameIsCompiled(true);
            DeoptimizeAfterCFrame(thread, pc, iframe, cframe.GetFrameOrigin(), lastIframe,
                                  stack->GetCalleeRegsForDeoptimize().end());
        case FrameKind::NONE:
        case FrameKind::INTERPRETER:
            EVENT_DEOPTIMIZATION(std::string(cframe.GetMethod()->GetFullName()),
                                 pc - stack->GetMethod()->GetInstructions(),
                                 prevFrameKind == FrameKind::NONE ? events::DeoptimizationAfter::TOP
                                                                  : events::DeoptimizationAfter::IFRAME);
            LOG(DEBUG, INTEROP) << "Deoptimize after iframe";
            DeoptimizeAfterIFrame(thread, pc, iframe, cframe.GetFrameOrigin(), lastIframe,
                                  stack->GetCalleeRegsForDeoptimize().end());
    }
}

NO_ADDRESS_SANITIZE void DestroyMethodWithInvalidatingEP(Method *destroyMethod)
{
    LOG(DEBUG, INTEROP) << "Destroy compiled method: " << destroyMethod->GetFullName();
    destroyMethod->SetDestroyed();
    PandaSet<Method *> destroyMethods;
    destroyMethods.insert(destroyMethod);
    InvalidateCompiledEntryPoint(destroyMethods, false);
}

[[noreturn]] NO_ADDRESS_SANITIZE void Deoptimize(StackWalker *stack, const uint8_t *pc, bool hasException,
                                                 Method *destroyMethod)
{
    ASSERT(stack != nullptr);
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    ASSERT(stack->IsCFrame());
    auto &cframe = stack->GetCFrame();
    UnpoisonAsanStack(cframe.GetFrameOrigin());
    auto method = stack->GetMethod();
    if (pc == nullptr) {
        ASSERT(method != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pc = method->GetInstructions() + stack->GetBytecodePc();
    }

    LOG(INFO, INTEROP) << "Deoptimize frame: " << method->GetFullName() << ", pc=" << std::hex
                       << pc - method->GetInstructions() << std::dec;

    // NOTE(konstanting): a potential candidate for moving out of the core part
    thread->GetVM()->CleanupCompiledFrameResources(thread->GetCurrentFrame());

    auto context = thread->GetVM()->GetLanguageContext();
    // We must run InvalidateCompiledEntryPoint before we convert the frame, because GC is already may be in the
    // collecting phase and it can move some object in the deoptimized frame.
    if (destroyMethod != nullptr) {
        DestroyMethodWithInvalidatingEP(destroyMethod);
    }

    FrameKind prevFrameKind;
    // We need to execute(find catch block) in all inlined methods. For this we calculate the number of inlined method
    // Else we can execute previus interpreter frames and we will FreeFrames in incorrect order
    uint32_t numInlinedMethods = 0;
    Frame *iframe = stack->ConvertToIFrame(&prevFrameKind, &numInlinedMethods);
    ASSERT(iframe != nullptr);

    Frame *lastIframe = iframe;
    while (numInlinedMethods-- != 0) {
        EVENT_METHOD_EXIT(last_iframe->GetMethod()->GetFullName() + "(deopt)", events::MethodExitKind::INLINED,
                          thread->RecordMethodExit());
        lastIframe = lastIframe->GetPrevFrame();
        ASSERT(!StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(lastIframe));
    }

    EVENT_METHOD_EXIT(last_iframe->GetMethod()->GetFullName() + "(deopt)", events::MethodExitKind::COMPILED,
                      thread->RecordMethodExit());

    if (thread->HasPendingException()) {
        LOG(DEBUG, INTEROP) << "Deoptimization has pending exception: "
                            << thread->GetException()->ClassAddr<Class>()->GetName();
        context.SetExceptionToVReg(iframe->GetAcc(), thread->GetException());
    }

    if (!hasException) {
        thread->ClearException();
    } else {
        ASSERT(thread->HasPendingException());
    }

    PrevFrameDeopt(prevFrameKind, thread, stack, pc, lastIframe, iframe, cframe);
    UNREACHABLE();
}

[[noreturn]] void DropCompiledFrame(StackWalker *stack)
{
    LOG(DEBUG, INTEROP) << "Drop compiled frame: " << stack->GetMethod()->GetFullName();
    auto cframeFp = stack->GetCFrame().GetFrameOrigin();
    EVENT_METHOD_EXIT(stack->GetMethod()->GetFullName() + "(drop)", events::MethodExitKind::COMPILED,
                      ManagedThread::GetCurrent()->RecordMethodExit());
    UnpoisonAsanStack(cframeFp);
    DropCompiledFrameAndReturn(cframeFp, stack->GetCalleeRegsForDeoptimize().end());
    UNREACHABLE();
}

}  // namespace ark
