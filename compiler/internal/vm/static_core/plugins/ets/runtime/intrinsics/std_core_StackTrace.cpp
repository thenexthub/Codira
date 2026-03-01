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

#include "ets_handle.h"
#include "include/mem/panda_containers.h"
#include "runtime/runtime_helpers.h"
#include "types/ets_method.h"
#include "types/ets_stacktrace_element.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

#include "runtime/include/stack_walker.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"

namespace ark::ets::intrinsics {

EtsStackTraceElement *CreateStackTraceElement(StackWalker *stack)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);

    EtsMethod *method = EtsMethod::FromRuntimeMethod(stack->GetMethod());
    auto className = EtsHandle<EtsString>(coroutine, method->GetClass()->GetName());
    if (UNLIKELY(className.GetPtr() == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        return nullptr;
    }
    auto methodName = EtsHandle<EtsString>(coroutine, method->GetNameString());
    if (UNLIKELY(methodName.GetPtr() == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        return nullptr;
    }

    const auto lineNumber = method->GetLineNumFromBytecodeOffset(stack->GetBytecodePc());
    auto *sourceFile = reinterpret_cast<const char *>(method->GetClassSourceFile().data);
    if (sourceFile == nullptr) {
        sourceFile = "<unknown>";
    }
    auto *stackTraceElement = EtsStackTraceElement::Create(coroutine);
    if (UNLIKELY(stackTraceElement == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        return nullptr;
    }
    auto element = EtsHandle<EtsStackTraceElement>(coroutine, stackTraceElement);
    element->SetClassName(className.GetPtr());
    element->SetMethodName(methodName.GetPtr());

    EtsString *sourceFileName = EtsString::CreateFromMUtf8(sourceFile);
    if (UNLIKELY(sourceFileName == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        return nullptr;
    }
    element->SetSourceFileName(sourceFileName);
    element->SetLineNumber(lineNumber);
    return element.GetPtr();
}

extern "C" EtsObjectArray *StdCoreStackTraceProvisionStackTrace()
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);

    auto stackTraceElementClass = PlatformTypes(coroutine)->coreStackTraceElement;

    auto walker = StackWalker::Create(coroutine);

    PandaVector<EtsHandle<EtsStackTraceElement>> stackTraceElements;
    for (auto stack = StackWalker::Create(coroutine); stack.HasFrame(); stack.NextFrame()) {
        auto element = EtsHandle<EtsStackTraceElement>(coroutine, CreateStackTraceElement(&stack));
        if (UNLIKELY(element.GetPtr() == nullptr)) {
            ASSERT(coroutine->HasPendingException());
            return nullptr;
        }
        stackTraceElements.push_back(element);
    }

    const auto linesSize = static_cast<uint32_t>(stackTraceElements.size());

    auto *resultArray = EtsObjectArray::Create(stackTraceElementClass, linesSize);
    if (UNLIKELY(resultArray == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        return nullptr;
    }
    EtsHandle<EtsObjectArray> resultArrayHandle(coroutine, resultArray);
    for (uint32_t i = 0; i < linesSize; i++) {
        resultArrayHandle.GetPtr()->Set(i, stackTraceElements[i]->AsObject());
    }
    return resultArrayHandle.GetPtr();
}

}  // namespace ark::ets::intrinsics
