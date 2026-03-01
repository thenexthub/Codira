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

#ifndef COMMON_INTERFACES_OBJECTS_UTILS_FIELD_MACRO_H
#define COMMON_INTERFACES_OBJECTS_UTILS_FIELD_MACRO_H

#include "common_interfaces/objects/utils/objects_traits.h"
#include "common_interfaces/base_runtime.h"
#include "common_interfaces/base/mem.h"
#include <type_traits>
#include <utility>

// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PRIMITIVE_FIELD(name, type, offset, endOffset)                   \
    static constexpr size_t endOffset = (offset) + sizeof(type);         \
    inline void Set##name(type value)                                    \
    {                                                                    \
        /* CC-OFFNXT(G.PRE.02) code readability */                       \
        auto *addr = reinterpret_cast<type *>(ToUintPtr(this) + offset); \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */            \
        *addr = value;                                                   \
    }                                                                    \
    inline type Get##name() const                                        \
    {                                                                    \
        /* CC-OFFNXT(G.PRE.02) code readability */                       \
        auto *addr = reinterpret_cast<type *>(ToUintPtr(this) + offset); \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */            \
        return *addr;                                                    \
    }

#if !defined(PANDA_32_BIT_MANAGED_POINTER)
// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMMON_POINTER_SIZE sizeof(uint64_t)
#else
// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMMON_POINTER_SIZE sizeof(uint32_t)
#endif

// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define POINTER_FIELD(name, offset, endOffset)                                                       \
    static constexpr size_t endOffset = (offset) + COMMON_POINTER_SIZE;                              \
    template <typename PointerType, typename WriteBarrier,                                           \
              objects_traits::enable_if_is_write_barrier<std::decay_t<WriteBarrier>> = 0>            \
    inline void Set##name(WriteBarrier &&writeBarrier, PointerType value)                            \
    {                                                                                                \
        /* CC-OFFNXT(G.PRE.02) code readability */                                                   \
        void *obj = static_cast<void *>(this);                                                       \
        std::invoke(std::forward<WriteBarrier>(writeBarrier), obj, offset, value);                   \
    }                                                                                                \
                                                                                                     \
    template <typename PointerType, typename ReadBarrier,                                            \
              objects_traits::enable_if_is_read_barrier<std::decay_t<ReadBarrier>, PointerType> = 0> \
    inline PointerType Get##name(ReadBarrier &&readBarrier) const                                    \
    {                                                                                                \
        /* CC-OFFNXT(G.PRE.02) code readability */                                                   \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */                                        \
        return std::invoke(std::forward<ReadBarrier>(readBarrier),                                   \
                           const_cast<void *>(static_cast<const void *>(this)), offset);             \
    }

#if !defined(NDEBUG)
// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASE_CAST_CHECK(CAST_TYPE, CHECK_METHOD)                       \
    static inline CAST_TYPE *Cast(BaseObject *object)                  \
    {                                                                  \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        if (!object->GetBaseClass()->CHECK_METHOD()) {                 \
            std::abort();                                              \
        }                                                              \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */          \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        return static_cast<CAST_TYPE *>(object);                       \
    }                                                                  \
    static inline const CAST_TYPE *ConstCast(const BaseObject *object) \
    {                                                                  \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        if (!object->GetBaseClass()->CHECK_METHOD()) {                 \
            std::abort();                                              \
        }                                                              \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */          \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        return static_cast<const CAST_TYPE *>(object);                 \
    }
#else
// CC-OFFNXT(C_RULE_ID_DEFINE_LENGTH_LIMIT) solid logic
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASE_CAST_CHECK(CAST_TYPE, CHECK_METHOD)                       \
    static inline CAST_TYPE *Cast(BaseObject *object)                  \
    {                                                                  \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        DCHECK_CC(object->CHECK_METHOD());                             \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */          \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        return static_cast<CAST_TYPE *>(object);                       \
    }                                                                  \
    static const inline CAST_TYPE *ConstCast(const BaseObject *object) \
    {                                                                  \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        DCHECK_CC(object->CHECK_METHOD());                             \
        /* CC-OFFNXT(G.PRE.05) C_RULE_ID_KEYWORD_IN_DEFINE */          \
        /* CC-OFFNXT(G.PRE.02) code readability */                     \
        return static_cast<const CAST_TYPE *>(object);                 \
    }
#endif
#endif  // COMMON_INTERFACES_OBJECTS_UTILS_FIELD_MACRO_H