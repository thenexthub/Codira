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
#ifndef PANDA_RUNTIME_KLASS_HELPER_H_
#define PANDA_RUNTIME_KLASS_HELPER_H_

#include <cstdint>

#include "libarkbase/utils/span.h"
#include "libarkfile/file_items.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/object_header_config.h"

namespace ark {

// Small helper
template <class Config>
class ClassConfig {
public:
    using ClassWordSize = typename Config::Size;
};

class Class;
class ClassLinker;
class ClassLinkerContext;
class ClassLinkerExtension;
class ClassLinkerErrorHandler;

class ClassHelper : private ClassConfig<MemoryModelConfig> {
public:
    using ClassWordSize = typename ClassConfig::ClassWordSize;  // To be visible outside

    static constexpr size_t OBJECT_POINTER_SIZE = sizeof(ClassWordSize);
    // In general for any T: sizeof(T*) != OBJECT_POINTER_SIZE
    static constexpr size_t POINTER_SIZE = sizeof(uintptr_t);

    PANDA_PUBLIC_API static const uint8_t *GetDescriptor(const uint8_t *name, PandaString *storage,
                                                         bool strictPublicDescriptor = false);

    static const uint8_t *GetTypeDescriptor(const PandaString &name, PandaString *storage);

    static const uint8_t *GetArrayDescriptor(const uint8_t *componentName, size_t rank, PandaString *storage);

    static char GetPrimitiveTypeDescriptorChar(panda_file::Type::TypeId typeId);

    static const uint8_t *GetPrimitiveTypeDescriptorStr(panda_file::Type::TypeId typeId);

    static const char *GetPrimitiveTypeStr(panda_file::Type::TypeId typeId);

    static const uint8_t *GetPrimitiveDescriptor(panda_file::Type type, PandaString *storage);

    static const uint8_t *GetPrimitiveArrayDescriptor(panda_file::Type type, size_t rank, PandaString *storage);

    template <typename Str = std::string>
    static Str GetName(const uint8_t *descriptor);

    template <typename Str = std::string>
    static Str GetNameUndecorated(const uint8_t *descriptor);

    static bool IsArrayDescriptor(const uint8_t *descriptor)
    {
        Span<const uint8_t> sp(descriptor, 1);
        return sp[0] == '[';
    }

    static const uint8_t *GetComponentDescriptor(const uint8_t *descriptor)
    {
        ASSERT(IsArrayDescriptor(descriptor));
        Span<const uint8_t> sp(descriptor, 1);
        return sp.cend();
    }

    static size_t GetDimensionality(const uint8_t *descriptor)
    {
        ASSERT(IsArrayDescriptor(descriptor));
        size_t dim = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (*descriptor++ == '[') {
            ++dim;
        }
        return dim;
    }

    static bool IsPrimitive(const uint8_t *descriptor);
    static bool IsReference(const uint8_t *descriptor);
    static bool IsUnionDescriptor(const uint8_t *descriptor);
    static bool IsUnionOrArrayUnionDescriptor(const uint8_t *descriptor);
    static Span<const uint8_t> GetUnionComponent(const uint8_t *descriptor);
    static Class *GetUnionLUBClass(const uint8_t *descriptor, ClassLinker *classLinker,
                                   ClassLinkerContext *classLinkerCtx, ClassLinkerExtension *ext,
                                   ClassLinkerErrorHandler *handler = nullptr);

private:
    template <typename Str = std::string>
    static void AppendType(const uint8_t *descriptor, int rank, Str &result);

    template <typename Str = std::string>
    static void AppendNonPrimitiveType(const uint8_t *descriptor, int rank, Str &result);
};

// Str is std::string or PandaString
/* static */
template <typename Str>
Str ClassHelper::GetName(const uint8_t *descriptor)
{
    switch (*descriptor) {
        case 'V':
            return "void";
        case 'Z':
            return "u1";
        case 'B':
            return "i8";
        case 'H':
            return "u8";
        case 'S':
            return "i16";
        case 'C':
            return "u16";
        case 'I':
            return "i32";
        case 'U':
            return "u32";
        case 'J':
            return "i64";
        case 'Q':
            return "u64";
        case 'F':
            return "f32";
        case 'D':
            return "f64";
        case 'A':
            return "any";
        case 'Y':
            return "Y";
        case 'N':
            return "N";
        default: {
            break;
        }
    }

    Str name = utf::Mutf8AsCString(descriptor);
    if (name[0] == '[') {
        return name;
    }

    std::replace(name.begin(), name.end(), '/', '.');

    ASSERT(name.size() > 2);  // 2 - L and ;

    name.erase(0, 1);
    name.pop_back();

    return name;
}

// Str is std::string or PandaString
/* static */
template <typename Str>
Str ClassHelper::GetNameUndecorated(const uint8_t *descriptor)
{
    Str result;

    auto rank = 0;
    while (descriptor[rank] == '[') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        rank++;
    }

    AppendType(descriptor, rank, result);

    while (rank > 0) {
        result.append("[]");
        rank--;
    }

    return result;
}

// Str is std::string or PandaString
/* static */
template <typename Str>
void ClassHelper::AppendType(const uint8_t *descriptor, int rank, Str &result)
{
    switch (descriptor[rank]) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        case 'V':
            result.append("void");
            break;
        case 'Z':
            result.append("u1");
            break;
        case 'B':
            result.append("i8");
            break;
        case 'H':
            result.append("u8");
            break;
        case 'S':
            result.append("i16");
            break;
        case 'C':
            result.append("u16");
            break;
        case 'I':
            result.append("i32");
            break;
        case 'U':
            result.append("u32");
            break;
        case 'J':
            result.append("i64");
            break;
        case 'Q':
            result.append("u64");
            break;
        case 'F':
            result.append("f32");
            break;
        case 'D':
            result.append("f64");
            break;
        case 'A':
            result.append("any");
            break;
        case 'L':
            AppendNonPrimitiveType(descriptor, rank, result);
            break;
        default:
            break;
    }
}

// Str is std::string or PandaString
/* static */
template <typename Str>
void ClassHelper::AppendNonPrimitiveType(const uint8_t *descriptor, int rank, Str &result)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.append(utf::Mutf8AsCString(descriptor + rank + 1));  // cut 'L' at the beginning
    ASSERT(result.size() > 0);
    result.pop_back();  // cut ';' at the end

    std::replace(result.begin(), result.end(), '/', '.');
}

}  // namespace ark

#endif  // PANDA_RUNTIME_KLASS_HELPER_H_
