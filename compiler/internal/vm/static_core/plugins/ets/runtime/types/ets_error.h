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

#ifndef PANDA_PLUGINS_ETS_ERROR_H
#define PANDA_PLUGINS_ETS_ERROR_H

#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_stacktrace_element.h"

namespace ark::ets {

namespace test {
class EtsErrorTest;
}  // namespace test

// class Error is mirror class for escompat.Error
// Currently, there is no necessity to declare this class as mirror in yaml files,
// this this class is auxiliary for describing std.core error classes as mirror classes.
class Error : public EtsObject {
public:
    NO_COPY_SEMANTIC(Error);
    NO_MOVE_SEMANTIC(Error);

    Error() = delete;
    ~Error() = delete;

    static Error *Create(EtsCoroutine *coro, EtsClass *klass)
    {
        return FromEtsObject(EtsObject::Create(coro, klass));
    }

    static Error *FromEtsObject(EtsObject *object)
    {
        return static_cast<Error *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    void SetName(EtsCoroutine *coro, EtsString *name)
    {
        ASSERT(name != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(Error, name_), name->GetCoreType());
    }

    void SetMessage(EtsCoroutine *coro, EtsString *msg)
    {
        ASSERT(msg != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(Error, message_), msg->GetCoreType());
    }

    void SetStack(EtsCoroutine *coro, EtsString *stack)
    {
        ASSERT(stack != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(Error, stack_), stack->GetCoreType());
    }

    void SetStackLines(EtsCoroutine *coro, EtsTypedObjectArray<EtsStackTraceElement> *stackLines)
    {
        ASSERT(stackLines != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(Error, stackLines_), stackLines->GetCoreType());
    }

private:
    ObjectPointer<EtsString> name_;
    ObjectPointer<EtsString> message_;
    ObjectPointer<EtsTypedObjectArray<EtsStackTraceElement>> stackLines_;
    ObjectPointer<EtsString> stack_;  // non-mandatory field in `class Error`
    ObjectPointer<EtsObject> cause_;  // non-mandatory field in `class Error`
    FIELD_UNUSED ObjectPointer<EtsInt> code_;

    friend class test::EtsErrorTest;
};

// Purpose of this class is to have preallocated OOM object in runtime, that can be useful in situation
// when all heap is filled and we don't have space to allocate stdlib OOM error.
class EtsOutOfMemoryError final : public Error {
public:
    static constexpr std::string_view OOM_ERROR_NAME = "OutOfMemoryError";
    // Default message should be empty string to avoid allocations in .toString()
    // NOLINTNEXTLINE(readability-redundant-string-init)
    static constexpr std::string_view DEFAULT_OOM_MSG = "";
    static constexpr std::string_view DEFAULT_OOM_STACK = "Heap is full, no space to collect stack";

public:
    NO_COPY_SEMANTIC(EtsOutOfMemoryError);
    NO_MOVE_SEMANTIC(EtsOutOfMemoryError);

    EtsOutOfMemoryError() = delete;
    ~EtsOutOfMemoryError() = delete;

    static EtsOutOfMemoryError *Create(EtsCoroutine *coro)
    {
        [[maybe_unused]] EtsHandleScope scope(coro);

        auto oomH = EtsHandle<EtsOutOfMemoryError>(
            coro, FromError(Error::Create(coro, PlatformTypes(coro)->coreOutOfMemoryError)));

        ASSERT(oomH.GetPtr() != nullptr);
        auto *name = EtsString::CreateFromMUtf8(OOM_ERROR_NAME.data());
        oomH->SetName(coro, name);

        auto *message = EtsString::CreateFromMUtf8(DEFAULT_OOM_MSG.data());
        oomH->SetMessage(coro, message);

        auto *stack = EtsString::CreateFromMUtf8(DEFAULT_OOM_STACK.data());
        oomH->SetStack(coro, stack);

        auto *stackLines =
            EtsTypedObjectArray<EtsStackTraceElement>::Create(PlatformTypes(coro)->coreStackTraceElement, 0U);
        oomH->SetStackLines(coro, stackLines);

        return oomH.GetPtr();
    }

    static EtsOutOfMemoryError *FromError(Error *error)
    {
        ASSERT(error->GetClass()->GetDescriptor() == panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR);
        return static_cast<EtsOutOfMemoryError *>(error);
    }
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_ERROR_H
