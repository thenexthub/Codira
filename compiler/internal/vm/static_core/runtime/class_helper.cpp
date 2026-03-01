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

#include "runtime/include/class_helper.h"

#include <algorithm>

#include "include/class_linker_extension.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/bit_utils.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/class_linker.h"

namespace ark {

/* static */
const uint8_t *ClassHelper::GetDescriptor(const uint8_t *name, PandaString *storage, bool strictPublicDescriptor)
{
    if (strictPublicDescriptor) {
        auto componentName = PandaString(utf::Mutf8AsCString(name));
        if (std::find(componentName.begin(), componentName.end(), '/') != componentName.end()) {
            return nullptr;
        }
    }

    return GetArrayDescriptor(name, 0, storage);
}

/* static */
const uint8_t *ClassHelper::GetArrayDescriptor(const uint8_t *componentName, size_t rank, PandaString *storage)
{
    storage->clear();
    storage->append(rank, '[');
    storage->push_back('L');
    storage->append(utf::Mutf8AsCString(componentName));
    storage->push_back(';');
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

/* static */
char ClassHelper::GetPrimitiveTypeDescriptorChar(panda_file::Type::TypeId typeId)
{
    // static_cast isn't necessary in most implementations but may be by standard
    return static_cast<char>(*GetPrimitiveTypeDescriptorStr(typeId));
}

/* static */
const uint8_t *ClassHelper::GetPrimitiveTypeDescriptorStr(panda_file::Type::TypeId typeId)
{
    if (typeId == panda_file::Type::TypeId::REFERENCE) {
        UNREACHABLE();
        return nullptr;
    }

    return utf::CStringAsMutf8(panda_file::Type::GetSignatureByTypeId(panda_file::Type(typeId)));
}

/* static */
const char *ClassHelper::GetPrimitiveTypeStr(panda_file::Type::TypeId typeId)
{
    switch (typeId) {
        case panda_file::Type::TypeId::VOID:
            return "void";
        case panda_file::Type::TypeId::U1:
            return "bool";
        case panda_file::Type::TypeId::I8:
            return "i8";
        case panda_file::Type::TypeId::U8:
            return "u8";
        case panda_file::Type::TypeId::I16:
            return "i16";
        case panda_file::Type::TypeId::U16:
            return "u16";
        case panda_file::Type::TypeId::I32:
            return "i32";
        case panda_file::Type::TypeId::U32:
            return "u32";
        case panda_file::Type::TypeId::I64:
            return "i64";
        case panda_file::Type::TypeId::U64:
            return "u64";
        case panda_file::Type::TypeId::F32:
            return "f32";
        case panda_file::Type::TypeId::F64:
            return "f64";
        case panda_file::Type::TypeId::TAGGED:
            return "any";
        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

/* static */
const uint8_t *ClassHelper::GetPrimitiveDescriptor(panda_file::Type type, PandaString *storage)
{
    return GetPrimitiveArrayDescriptor(type, 0, storage);
}

/* static */
const uint8_t *ClassHelper::GetPrimitiveArrayDescriptor(panda_file::Type type, size_t rank, PandaString *storage)
{
    storage->clear();
    storage->append(rank, '[');
    storage->push_back(GetPrimitiveTypeDescriptorChar(type.GetId()));
    return utf::CStringAsMutf8(storage->c_str());
}

/* static */
const uint8_t *ClassHelper::GetTypeDescriptor(const PandaString &name, PandaString *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

/* static */
bool ClassHelper::IsPrimitive(const uint8_t *descriptor)
{
    switch (*descriptor) {
        case 'V':
        case 'Z':
        case 'B':
        case 'H':
        case 'S':
        case 'C':
        case 'I':
        case 'U':
        case 'J':
        case 'Q':
        case 'F':
        case 'D':
        case 'A':
            return true;
        default:
            return false;
    }
}

/* static */
bool ClassHelper::IsReference(const uint8_t *descriptor)
{
    Span<const uint8_t> sp(descriptor, 1);
    return sp[0] == 'L';
}

/* static */
bool ClassHelper::IsUnionDescriptor(const uint8_t *descriptor)
{
    Span<const uint8_t> sp(descriptor, 2);
    return sp[0] == '{' && sp[1] == 'U';
}

/* static */
bool ClassHelper::IsUnionOrArrayUnionDescriptor(const uint8_t *descriptor)
{
    if (IsUnionDescriptor(descriptor)) {
        return true;
    }
    if (IsArrayDescriptor(descriptor)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return IsUnionDescriptor(&descriptor[GetDimensionality(descriptor)]);
    }
    return false;
}

static size_t GetUnionTypeComponentsNumber(const uint8_t *descriptor)
{
    size_t length = 1;
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    while (descriptor[length] != '}') {
        if (descriptor[length] == '{') {
            length += GetUnionTypeComponentsNumber(&(descriptor[length]));
        } else {
            length += 1;
        }
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return length + 1;
}

/* static */
Span<const uint8_t> ClassHelper::GetUnionComponent(const uint8_t *descriptor)
{
    if (IsPrimitive(descriptor)) {
        return Span(descriptor, 1);
    }
    if (IsArrayDescriptor(descriptor)) {
        auto dim = GetDimensionality(descriptor);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return Span(descriptor, dim + GetUnionComponent(&(descriptor[dim])).Size());
    }
    if (IsUnionDescriptor(descriptor)) {
        return Span(descriptor, GetUnionTypeComponentsNumber(descriptor));
    }
    ASSERT(IsReference(descriptor));
    return Span(descriptor, std::string(utf::Mutf8AsCString(descriptor)).find(';') + 1);
}

}  // namespace ark
