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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_INL_H

#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"

namespace ark::ets {

// CC-OFFNXT(G.PRE.02, G.PRE.06) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE(methodName, type, obj, ...)        \
    do {                                                                       \
        if constexpr (std::is_same_v<type, int8_t>) {                          \
            reinterpret_cast<EtsByteArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, int16_t>) {                  \
            reinterpret_cast<EtsShortArray *>(obj)->methodName(__VA_ARGS__);   \
        } else if constexpr (std::is_same_v<type, int32_t>) {                  \
            reinterpret_cast<EtsIntArray *>(obj)->methodName(__VA_ARGS__);     \
        } else if constexpr (std::is_same_v<type, int64_t>) {                  \
            reinterpret_cast<EtsLongArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint8_t>) {                  \
            reinterpret_cast<EtsBooleanArray *>(obj)->methodName(__VA_ARGS__); \
        } else if constexpr (std::is_same_v<type, uint16_t>) {                 \
            reinterpret_cast<EtsCharArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint32_t>) {                 \
            reinterpret_cast<EtsUintArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint64_t>) {                 \
            reinterpret_cast<EtsUlongArray *>(obj)->methodName(__VA_ARGS__);   \
        } else {                                                               \
            UNREACHABLE();                                                     \
        }                                                                      \
    } while (false)

// CC-OFFNXT(G.PRE.02, G.PRE.06) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXECUTE_METHOD_DEPENDS_ON_TYPE(varToStore, methodName, type, obj, ...)              \
    do {                                                                                    \
        if constexpr (std::is_same_v<type, int8_t>) {                                       \
            varToStore = reinterpret_cast<EtsByteArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, int16_t>) {                               \
            varToStore = reinterpret_cast<EtsShortArray *>(obj)->methodName(__VA_ARGS__);   \
        } else if constexpr (std::is_same_v<type, int32_t>) {                               \
            varToStore = reinterpret_cast<EtsIntArray *>(obj)->methodName(__VA_ARGS__);     \
        } else if constexpr (std::is_same_v<type, int64_t>) {                               \
            varToStore = reinterpret_cast<EtsLongArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint8_t>) {                               \
            varToStore = reinterpret_cast<EtsBooleanArray *>(obj)->methodName(__VA_ARGS__); \
        } else if constexpr (std::is_same_v<type, uint16_t>) {                              \
            varToStore = reinterpret_cast<EtsCharArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint32_t>) {                              \
            varToStore = reinterpret_cast<EtsUintArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint64_t>) {                              \
            varToStore = reinterpret_cast<EtsUlongArray *>(obj)->methodName(__VA_ARGS__);   \
        } else {                                                                            \
            UNREACHABLE();                                                                  \
        }                                                                                   \
    } while (false)

template <typename T>
T EtsEscompatArrayBuffer::GetElement(uint32_t index, uint32_t byteOffset)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetVolatile, T, obj, index, byteOffset);
    return val;
}

template <typename T>
void EtsEscompatArrayBuffer::SetElement(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE(SetVolatile, T, obj, index, byteOffset, element);
}

template <typename T>
T EtsEscompatArrayBuffer::GetVolatileElement(uint32_t index, uint32_t byteOffset)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetVolatile, T, obj, index, byteOffset);
    return val;
}

template <typename T>
void EtsEscompatArrayBuffer::SetVolatileElement(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE(SetVolatile, T, obj, index, byteOffset, element);
}

template <typename T>
std::pair<bool, T> EtsEscompatArrayBuffer::CompareAndExchangeElement(uint32_t index, uint32_t byteOffset, T oldElement,
                                                                     T newElement, bool strong)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    std::pair<bool, T> val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, CompareAndExchange, T, obj, index, byteOffset, oldElement, newElement, strong);
    return val;
}
template <typename T>
T EtsEscompatArrayBuffer::ExchangeElement(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, Exchange, T, obj, index, byteOffset, element);
    return val;
}

template <typename T>
T EtsEscompatArrayBuffer::GetAndAdd(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndAdd, T, obj, index, byteOffset, element);
    return val;
}

template <typename T>
T EtsEscompatArrayBuffer::GetAndSub(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndSub, T, obj, index, byteOffset, element);
    return val;
}

template <typename T>
T EtsEscompatArrayBuffer::GetAndBitwiseOr(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndBitwiseOr, T, obj, index, byteOffset, element);
    return val;
}

template <typename T>
T EtsEscompatArrayBuffer::GetAndBitwiseAnd(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndBitwiseAnd, T, obj, index, byteOffset, element);
    return val;
}

template <typename T>
T EtsEscompatArrayBuffer::GetAndBitwiseXor(uint32_t index, uint32_t byteOffset, T element)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_));
    T val;  // CC-OFF(G.EXP.09-CPP) variable is used in code gen macro
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndBitwiseXor, T, obj, index, byteOffset, element);
    return val;
}

#undef EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE
#undef EXECUTE_METHOD_DEPENDS_ON_TYPE

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_INL_H
