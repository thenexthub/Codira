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
#ifndef PANDA_RUNTIME_MEM_REFERENCE_H
#define PANDA_RUNTIME_MEM_REFERENCE_H

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"

namespace ark::mem {
class ReferenceStorage;
class RefBlock;
namespace test {
class ReferenceStorageTest;
}  // namespace test
}  // namespace ark::mem

namespace ark::mem {

class GlobalObjectStorage;

class Reference {
public:
    enum class ObjectType : uint8_t {
        /// Used for objects on stack (arguments for methods)
        STACK = 0,
        /**
         * Local references which can be used inside Frame. If Frame was removed all references inside this Frame can't
         * be used anymore
         */
        LOCAL = 1,
        /// Global references which can be used without Frames
        GLOBAL = 2,
        /**
         * Weak references which work as Global references, but will be nullified when GC clears the object when the
         * object becomes unreachable (no guarantees when it will happen)
         */
        WEAK = 3,
        /// Global references which can be used without Frames. Addresses of references aren't changed
        GLOBAL_FIXED = LOCAL,
        ENUM_SIZE = 4
    };

    DEFAULT_COPY_SEMANTIC(Reference);
    DEFAULT_MOVE_SEMANTIC(Reference);
    ~Reference() = delete;
    Reference() = delete;

    bool IsStack() const
    {
        return GetType() == ObjectType::STACK;
    }

    bool IsLocal() const
    {
        ObjectType type = GetType();
        return type == ObjectType::STACK || type == ObjectType::LOCAL;
    }

    bool IsGlobal() const
    {
        return GetType() == ObjectType::GLOBAL;
    }

    bool IsGlobalFixed() const
    {
        return GetType() == ObjectType::GLOBAL_FIXED;
    }

    bool IsWeak() const
    {
        return GetType() == ObjectType::WEAK;
    }

private:
    static constexpr auto MASK_TYPE = 3U;
    static constexpr auto MASK_WITHOUT_TYPE = sizeof(uintptr_t) == sizeof(uint64_t)
                                                  ? std::numeric_limits<uint64_t>::max() - MASK_TYPE
                                                  : std::numeric_limits<uint32_t>::max() - MASK_TYPE;

    static Reference *CreateWithoutType(uintptr_t addr)
    {
        ASSERT((addr & MASK_TYPE) == 0);
        return reinterpret_cast<Reference *>(addr);
    }

    static Reference *Create(uintptr_t addr, ObjectType type)
    {
        ASSERT((addr & MASK_TYPE) == 0);
        return SetType(addr, type);
    }

    static ObjectType GetType(const Reference *ref)
    {
        auto addr = ToUintPtr(ref);
        return static_cast<ObjectType>(addr & MASK_TYPE);
    }

    static Reference *SetType(Reference *ref, ObjectType type)
    {
        auto addr = ToUintPtr(ref);
        return SetType(addr, type);
    }

    static Reference *SetType(uintptr_t addr, ObjectType type)
    {
        ASSERT((addr & MASK_TYPE) == 0);
        return reinterpret_cast<Reference *>(addr | static_cast<uintptr_t>(type));
    }

    ObjectType GetType() const
    {
        return Reference::GetType(this);
    }

    static Reference *GetRefWithoutType(const Reference *ref)
    {
        auto addr = ToUintPtr(ref);
        return reinterpret_cast<Reference *>(addr & MASK_WITHOUT_TYPE);
    }

    friend class ark::mem::ReferenceStorage;
    friend class ark::mem::GlobalObjectStorage;
    friend class ark::mem::RefBlock;
    friend class ark::mem::test::ReferenceStorageTest;
};
}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_REFERENCE_H
