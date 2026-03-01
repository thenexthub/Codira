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
#ifndef PANDA_RUNTIME_EXCEPTIONS_H_
#define PANDA_RUNTIME_EXCEPTIONS_H_

#include "runtime/include/class-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/language_context.h"
#include "runtime/include/method.h"
#include "runtime/include/stack_walker.h"

namespace ark {

void ThrowException(const LanguageContext &ctx, ManagedThread *thread, const uint8_t *mutf8Name,
                    const uint8_t *mutf8Msg);

PANDA_PUBLIC_API void ThrowNullPointerException();
// This function could be used in case there are no managed stack frames.
// For example when the main thread creates an instance of VM and calls
// a native function which throws an exception. In this case there is no need to
// get current executing method from the managed stack.
PANDA_PUBLIC_API void ThrowNullPointerException(const LanguageContext &ctx, ManagedThread *thread);

void ThrowStackOverflowException(ManagedThread *thread);

void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length);
void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length,
                                         const LanguageContext &ctx, ManagedThread *thread);

void ThrowIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySsizeT length);

void ThrowIllegalStateException(const PandaString &msg);

void ThrowStringIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length);

void ThrowNegativeArraySizeException(coretypes::ArraySsizeT size);

void ThrowNegativeArraySizeException(const PandaString &msg);

void ThrowArithmeticException();

void ThrowClassCastException(const Class *dstType, const Class *srcType);

void ThrowAbstractMethodError(const Method *method);

void ThrowIncompatibleClassChangeErrorForMethodConflict(const Method *method);

void ThrowArrayStoreException(const Class *arrayClass, const Class *elementClass);

void ThrowArrayStoreException(const PandaString &msg);

void ThrowRuntimeException(const PandaString &msg);

void ThrowFileNotFoundException(const PandaString &msg);

void ThrowIOException(const PandaString &msg);

void ThrowIllegalArgumentException(const PandaString &msg);

void ThrowClassCircularityError(const PandaString &className, const LanguageContext &ctx);

void ThrowCoroutinesLimitExceedError(const PandaString &msg);

void ThrowOutOfMemoryError(ManagedThread *thread, const PandaString &msg);

PANDA_PUBLIC_API void ThrowOutOfMemoryError(const PandaString &msg);

void FindCatchBlockInCallStack(ManagedThread *thread);

void DropCFrameIfNecessary(ManagedThread *thread, StackWalker *stack, Frame *origFrame, FrameAccessor nextFrame,
                           Method *method);

void FindCatchBlockInCFrames(ManagedThread *thread, StackWalker *stack, Frame *origFrame);

void ThrowIllegalAccessException(const PandaString &msg);

void ThrowUnsupportedOperationException();

void ThrowVerificationException(const PandaString &msg);

void ThrowVerificationException(const LanguageContext &ctx, const PandaString &msg);

void ThrowInstantiationError(const PandaString &msg);

void ThrowNoClassDefFoundError(const PandaString &msg);

void ThrowTypedErrorDyn(const std::string &msg);

void ThrowReferenceErrorDyn(const std::string &msg);

void ThrowIllegalMonitorStateException(const PandaString &msg);

void ThrowCloneNotSupportedException();

void HandlePendingException(UnwindPolicy policy = UnwindPolicy::ALL);

inline void SetExceptionEvent([[maybe_unused]] events::ExceptionType type, [[maybe_unused]] ManagedThread *thread)
{
#ifdef PANDA_EVENTS_ENABLED
    auto stack = StackWalker::Create(thread);
    EVENT_EXCEPTION(std::string(stack.GetMethod()->GetFullName()), stack.GetBytecodePc(), stack.GetNativePc(), type);
#endif
}

}  // namespace ark

#endif  // PANDA_RUNTIME_EXCEPTIONS_H_
