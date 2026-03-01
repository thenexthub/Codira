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
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"

#include "libarkbase/utils/arch.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/thread.h"

namespace ark::ets::intrinsics {

namespace {

/* make sure the class the intrinsic being called from belongs to the boot context */
bool EnsureBootContext()
{
    auto *coro = EtsCoroutine::GetCurrent();
    if (coro != nullptr) {
        auto ctx = StackWalker::Create(coro).GetMethod()->GetClass()->GetLoadContext();
        if (ctx != nullptr && ctx->IsBootContext()) {
            return true;
        }
    }

    auto e = panda_file_items::class_descriptors::ILLEGAL_STATE_ERROR;
    auto msg = "Unsafe intrinsics: cannot ensure the boot context!";
    ThrowEtsException(coro, e, msg);
    return false;
}

template <typename T>
T UnsafeMemoryReadUnaligned(EtsLong addr)
{
    if (EnsureBootContext()) {
        if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
            T val {};
            memcpy_s(&val, sizeof(val), reinterpret_cast<void *>(addr), sizeof(val));
            return val;
        }
        return *reinterpret_cast<T *>(addr);
    }
    return static_cast<T>(-1);
}

template <typename T>
void UnsafeMemoryWriteUnaligned(EtsLong addr, T val)
{
    if (EnsureBootContext()) {
        if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
            memcpy_s(reinterpret_cast<void *>(addr), sizeof(val), &val, sizeof(val));
        } else {
            *reinterpret_cast<T *>(addr) = val;
        }
    }
}

template <typename T>
T UnsafeMemoryReadAligned(EtsLong addr)
{
    if (EnsureBootContext()) {
        return *reinterpret_cast<T *>(addr);
    }
    return static_cast<T>(-1);
}

template <typename T>
void UnsafeMemoryWriteAligned(EtsLong addr, T val)
{
    if (EnsureBootContext()) {
        *reinterpret_cast<T *>(addr) = val;
    }
}

template <typename T>
T UnsafeMemoryRead(EtsLong addr)
{
#ifdef PANDA_TARGET_ARM32
    if constexpr (alignof(T) != alignof(std::byte)) {
        return UnsafeMemoryReadUnaligned<T>(addr);
    }
#endif  // PANDA_TARGET_ARM32
    return UnsafeMemoryReadAligned<T>(addr);
}

template <typename T>
void UnsafeMemoryWrite(EtsLong addr, T val)
{
#ifdef PANDA_TARGET_ARM32
    if constexpr (alignof(T) != alignof(std::byte)) {
        return UnsafeMemoryWriteUnaligned(addr, val);
    }
#endif  // PANDA_TARGET_ARM32
    return UnsafeMemoryWriteAligned(addr, val);
}

}  // namespace

extern "C" EtsByte UnsafeMemoryReadBoolean(EtsLong addr)
{
    return UnsafeMemoryRead<EtsByte>(addr);
}

extern "C" EtsByte UnsafeMemoryReadInt8(EtsLong addr)
{
    return UnsafeMemoryRead<EtsByte>(addr);
}

extern "C" EtsShort UnsafeMemoryReadInt16(EtsLong addr)
{
    return UnsafeMemoryRead<EtsShort>(addr);
}

extern "C" EtsInt UnsafeMemoryReadInt32(EtsLong addr)
{
    return UnsafeMemoryRead<EtsInt>(addr);
}

extern "C" EtsLong UnsafeMemoryReadInt64(EtsLong addr)
{
    return UnsafeMemoryRead<EtsLong>(addr);
}

extern "C" EtsFloat UnsafeMemoryReadFloat32(EtsLong addr)
{
    return UnsafeMemoryRead<EtsFloat>(addr);
}

extern "C" EtsDouble UnsafeMemoryReadFloat64(EtsLong addr)
{
    return UnsafeMemoryRead<EtsDouble>(addr);
}

extern "C" EtsDouble UnsafeMemoryReadNumber(EtsLong addr)
{
    return UnsafeMemoryRead<EtsDouble>(addr);
}

extern "C" void UnsafeMemoryWriteBoolean(EtsLong addr, EtsBoolean val)
{
    UnsafeMemoryWrite<EtsByte>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt8(EtsLong addr, EtsByte val)
{
    UnsafeMemoryWrite<EtsByte>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt16(EtsLong addr, EtsShort val)
{
    UnsafeMemoryWrite<EtsShort>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt32(EtsLong addr, EtsInt val)
{
    UnsafeMemoryWrite<EtsInt>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt64(EtsLong addr, EtsLong val)
{
    UnsafeMemoryWrite<EtsLong>(addr, val);
}

extern "C" void UnsafeMemoryWriteFloat32(EtsLong addr, EtsFloat val)
{
    UnsafeMemoryWrite<EtsFloat>(addr, val);
}

extern "C" void UnsafeMemoryWriteFloat64(EtsLong addr, EtsDouble val)
{
    UnsafeMemoryWrite<EtsDouble>(addr, val);
}

extern "C" void UnsafeMemoryWriteNumber(EtsLong addr, EtsDouble val)
{
    UnsafeMemoryWrite<EtsDouble>(addr, val);
}

extern "C" int UnsafeMemoryStringGetSizeInBytes(EtsString *str)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    EtsHandle<EtsString> handle(coroutine, str);

    if (!EnsureBootContext()) {
        return -1;
    }

    return handle.GetPtr()->GetUtf8Length();
}

extern "C" EtsString *UnsafeMemoryReadString(EtsLong buf, int len)
{
    if (!EnsureBootContext()) {
        return nullptr;
    }

    return EtsString::CreateFromUtf8(reinterpret_cast<const char *>(buf), static_cast<uint32_t>(len));
}

extern "C" int32_t WriteStringToMem(int64_t buf, ObjectHeader *s);
extern "C" int UnsafeMemoryWriteString(EtsLong addrEts, EtsString *str)
{
    /* we need the scope because EnsureBootContext() will most likely trigger GC */
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    EtsHandle<EtsString> handle(coroutine, str);

    if (!EnsureBootContext()) {
        return -1;
    }

    str = handle.GetPtr();
    return WriteStringToMem(addrEts, reinterpret_cast<ObjectHeader *>(str));
}

}  // namespace ark::ets::intrinsics
