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

#include <unordered_map>

#include "assembly-type.h"

namespace ark::pandasm {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::unordered_map<std::string_view, std::string_view> g_primitiveTypes = {
    {"u1", "Z"},  {"i8", "B"},  {"u8", "H"},  {"i16", "S"}, {"u16", "C"},  {"i32", "I"}, {"u32", "U"},
    {"f32", "F"}, {"f64", "D"}, {"i64", "J"}, {"u64", "Q"}, {"void", "V"}, {"any", "A"}};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::vector<std::string_view> g_syntheticReferenceTypes = {"Y", "N"};

std::string Type::GetComponentDescriptor(bool ignorePrimitive) const
{
    const std::string &componentName = GetComponentName();
    if (!ignorePrimitive) {
        auto it = g_primitiveTypes.find(componentName);
        if (it != g_primitiveTypes.cend()) {
            return it->second.data();
        }
    }

    const auto it = std::find(g_syntheticReferenceTypes.begin(), g_syntheticReferenceTypes.end(), (componentName));
    if (it != g_syntheticReferenceTypes.cend()) {
        return it->data();
    }

    auto res = "L" + componentName + ";";
    std::replace(res.begin(), res.end(), '.', '/');
    return res;
}

std::string GetDescriptorImpl(const std::string &componentName, bool ignorePrimitive)
{
    if (!ignorePrimitive) {
        auto it = g_primitiveTypes.find(componentName);
        if (it != g_primitiveTypes.cend()) {
            return it->second.data();
        }
    }

    auto compTypeRaw = Type::FromName(componentName);
    auto compType = Type(compTypeRaw.GetNameWithoutRank(), 0);
    auto res = std::string(compTypeRaw.GetRank(), '[');
    if (compType.IsUnion()) {
        return res + compType.GetDescriptor();
    }
    return res + compType.GetComponentDescriptor(ignorePrimitive);
}

std::string Type::GetDescriptor(bool ignorePrimitive) const
{
    std::string res = std::string(rank_, '[');
    if ((componentNames_.size() == 1)) {
        return res + GetDescriptorImpl(componentNames_[0], ignorePrimitive);
    }
    res += "{U";
    for (const auto &it : componentNames_) {
        res += GetDescriptorImpl(it, ignorePrimitive);
    }
    res += "}";
    return res;
}

void Type::Canonicalize()
{
    if (!IsUnion()) {
        return;
    }
    for (auto &componentName : componentNames_) {
        Type rawCompType = Type::FromName(componentName);
        Type compType = Type(rawCompType.GetNameWithoutRank(), 0);
        componentName = Type(compType.GetName(), rawCompType.GetRank()).GetName();
    }

    std::sort(componentNames_.begin(), componentNames_.end());
    auto duplicateBeginIt = unique(componentNames_.begin(), componentNames_.end());
    componentNames_.erase(duplicateBeginIt, componentNames_.end());
    name_ = GetName(GetComponentName(), GetRank());
}

/* static */
std::string Type::CanonicalizeDescriptor(std::string_view descriptor)
{
    auto type = Type::FromDescriptor(descriptor);
    type.Canonicalize();
    return type.GetDescriptor();
}

/* static */
panda_file::Type::TypeId Type::GetId(std::string_view name, bool ignorePrimitive)
{
    static std::unordered_map<std::string_view, panda_file::Type::TypeId> pandaTypes = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDATYPE(name, inst_code) {std::string_view(name), panda_file::Type::TypeId::inst_code},
        PANDA_ASSEMBLER_TYPES(PANDATYPE)
#undef PANDATYPE
    };

    if (!ignorePrimitive) {
        auto iter = pandaTypes.find(name);
        if (iter == pandaTypes.end()) {
            return panda_file::Type::TypeId::REFERENCE;
        }
        return iter->second;
    }

    return panda_file::Type::TypeId::REFERENCE;
}

/* static */
pandasm::Type Type::FromPrimitiveId(panda_file::Type::TypeId id)
{
    static std::unordered_map<panda_file::Type::TypeId, std::string_view> pandaTypes = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDATYPE(name, inst_code) {panda_file::Type::TypeId::inst_code, std::string_view(name)},
        PANDA_ASSEMBLER_TYPES(PANDATYPE)
#undef PANDATYPE
    };
    auto iter = pandaTypes.find(id);
    ASSERT(iter != pandaTypes.end());
    pandasm::Type ret {iter->second, 0};
    ASSERT(ret.typeId_ == id);
    return ret;
}

/* static */
std::string Type::GetName(std::string_view componentName, size_t rank)
{
    std::string name(componentName);
    while (rank-- > 0) {
        name += "[]";
    }
    return name;
}

static std::pair<std::string_view, size_t> FromDescriptorComponent(std::string_view descriptor)
{
    static std::unordered_map<std::string_view, std::string_view> reversePrimitiveTypes = {
        {"Z", "u1"},  {"B", "i8"},  {"H", "u8"},  {"S", "i16"},  {"C", "u16"}, {"I", "i32"}, {"U", "u32"}, {"F", "f32"},
        {"D", "f64"}, {"J", "i64"}, {"Q", "u64"}, {"V", "void"}, {"A", "any"}, {"Y", "Y"},   {"N", "N"}};

    bool isRefType = descriptor[0] == 'L';
    if (isRefType) {
        auto len = descriptor.find(';');
        return {descriptor.substr(1, len - 1), len + 1};
    }

    auto prim = reversePrimitiveTypes.find(descriptor.substr(0, 1));
    if (prim == reversePrimitiveTypes.end()) {
        LOG(FATAL, ASSEMBLER) << "The map 'reversePrimitiveTypes' don't contain the descriptor [" << descriptor << "].";
        return {"", 0};
    }

    return {prim->second, 1};
}

static std::pair<std::string, size_t> FromDescriptorImpl(std::string_view descriptor)
{
    bool isUnionType = descriptor[0] == '{';
    if (!isUnionType) {
        auto [name, len] = FromDescriptorComponent(descriptor);
        return {std::string(name), len};
    }

    std::string name = "{U";
    size_t unionLen = 3;
    descriptor.remove_prefix(Type::UNION_PREFIX_LEN);
    while (descriptor[0] != '}') {
        size_t rank = 0;
        while (descriptor[rank] == '[') {
            ++rank;
        }
        unionLen += rank;
        descriptor.remove_prefix(rank);
        auto [componentDesc, len] = FromDescriptorImpl(descriptor);
        unionLen += len;
        name += componentDesc;
        while (rank-- > 0) {
            name += "[]";
        }
        name += ",";
        descriptor.remove_prefix(len);
    }
    name.pop_back();  // remove the extra comma
    name += "}";
    return {name, unionLen};
}

/* static */
Type Type::FromDescriptor(std::string_view descriptor)
{
    size_t i = 0;
    while (descriptor[i] == '[') {
        ++i;
    }

    size_t rank = i;
    descriptor.remove_prefix(rank);
    auto [name, _] = FromDescriptorImpl(descriptor);
    return Type(name, rank);
}

/* static */
Type Type::FromName(std::string_view name, bool ignorePrimitive)
{
    size_t size = name.size();
    size_t i = 0;

    while (name[size - i - 1] == ']') {
        i += Type::RANK_STEP;
    }

    name.remove_suffix(i);

    return Type(name, i / Type::RANK_STEP, ignorePrimitive);
}

/* static */
// NOLINTNEXTLINE(misc-unused-parameters)
bool Type::IsStringType(const std::string &name, ark::panda_file::SourceLang lang)
{
    auto stringType = Type::FromDescriptor(ark::panda_file::GetStringClassDescriptor(lang));
    return name == stringType.GetName();
}

/* static */
bool Type::IsPandaPrimitiveType(const std::string &name)
{
    auto it = g_primitiveTypes.find(name);
    return it != g_primitiveTypes.cend();
}

}  // namespace ark::pandasm
