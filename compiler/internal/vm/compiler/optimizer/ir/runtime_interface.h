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

#ifndef COMPILER_OPTIMIZER_IR_RUNTIME_INTERFACE_H
#define COMPILER_OPTIMIZER_IR_RUNTIME_INTERFACE_H

#include <cstdint>
#include <cstddef>
#include "assembler/assembly-literals.h"
#include "constants.h"
#include "datatype.h"
#include "ir-dyn-base-types.h"
#include "mem/gc_barrier.h"
#include "source_languages.h"
#include "utils/arch.h"
#include "utils/span.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
enum class ClassType {
    UNRESOLVED_CLASS = 0,
    OBJECT_CLASS,
    ARRAY_CLASS,
    ARRAY_OBJECT_CLASS,
    INTERFACE_CLASS,
    OTHER_CLASS,
    FINAL_CLASS,
    COUNT
};

class Inst;

class RuntimeInterface {
public:
    using BinaryFilePtr = void *;
    using MethodPtr = void *;
    using FieldPtr = void *;
    using MethodId = uint32_t;
    using StringPtr = void *;
    using ClassPtr = void *;
    using IdType = uint32_t;
    using FieldId = uint32_t;
    using StringId = uint32_t;
    using LiteralArrayId = uint32_t;
    using MethodIndex = uint16_t;
    using FieldIndex = uint16_t;
    using TypeIndex = uint16_t;
    using StringIndex = uint16_t;
    using LiteralArrayIndex = uint16_t;

    static const uintptr_t RESOLVE_STRING_AOT_COUNTER_LIMIT = PANDA_32BITS_HEAP_START_ADDRESS;

    RuntimeInterface() = default;
    virtual ~RuntimeInterface() = default;

    virtual uint32_t ResolveOffsetByIndex([[maybe_unused]] MethodPtr parent_method,
                                          [[maybe_unused]] uint16_t index) const
    {
        return 0;
    }

    virtual BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    virtual MethodId GetMethodId([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    // Return total arguments count including this for instance method
    virtual size_t GetMethodTotalArgumentsCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id) const
    {
        return 0;
    }
    virtual size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual size_t GetMethodRegistersCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual const uint8_t *GetMethodCode([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }
    virtual size_t GetMethodCodeSize([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual SourceLanguage GetMethodSourceLanguage([[maybe_unused]] MethodPtr method) const
    {
        return SourceLanguage::PANDA_ASSEMBLY;
    }

    virtual std::string GetClassNameFromMethod([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownClass";
    }

    virtual ClassPtr GetClass([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    virtual std::string GetMethodName([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownMethod";
    }

    virtual std::string GetMethodFullName([[maybe_unused]] MethodPtr method, [[maybe_unused]] bool with_signature) const
    {
        return "UnknownMethod";
    }

    std::string GetMethodFullName(MethodPtr method) const
    {
        return GetMethodFullName(method, false);
    }

    /**************************************************************************
     * Class information
     */

    // returns Class Id for Method
    virtual size_t GetClassIdForMethod([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    #include <ecma_intrinsics_enum.inl>

    NO_COPY_SEMANTIC(RuntimeInterface);
    NO_MOVE_SEMANTIC(RuntimeInterface);

private:
    static constexpr uint32_t ARRAY_DEFAULT_ELEMENT_SIZE = 4;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_RUNTIME_INTERFACE_H
