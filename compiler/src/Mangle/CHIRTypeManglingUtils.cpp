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

/**
 * @file
 *
 * This file implements some utils for CHIR type mangling.
 */

#include "Codira/Mangle/CHIRTypeManglingUtils.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/EnumDef.h"
#include "Codira/CHIR/Type/StructDef.h"
#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Utils/CastingTemplate.h"

#include <sstream>
#include <string>

using namespace Codira::CHIR;
using namespace Codira::MangleUtils;
using ChirTypeKind = Codira::CHIR::Type::TypeKind;
namespace {
// Type mangling look-up table
const std::unordered_map<ChirTypeKind, std::string> TYPE_MANGLING_LUT = {
    {ChirTypeKind::TYPE_INT8, "a"},
    {ChirTypeKind::TYPE_INT16, "s"},
    {ChirTypeKind::TYPE_INT32, "i"},
    {ChirTypeKind::TYPE_INT64, "l"},
    {ChirTypeKind::TYPE_INT_NATIVE, "q"},
    {ChirTypeKind::TYPE_UINT8, "h"},
    {ChirTypeKind::TYPE_UINT16, "t"},
    {ChirTypeKind::TYPE_UINT32, "j"},
    {ChirTypeKind::TYPE_UINT64, "m"},
    {ChirTypeKind::TYPE_UINT_NATIVE, "r"},
    {ChirTypeKind::TYPE_FLOAT16, "Dh"},
    {ChirTypeKind::TYPE_FLOAT32, "f"},
    {ChirTypeKind::TYPE_FLOAT64, "d"},
    {ChirTypeKind::TYPE_RUNE, "c"},
    {ChirTypeKind::TYPE_BOOLEAN, "b"},
    {ChirTypeKind::TYPE_UNIT, "u"},
    {ChirTypeKind::TYPE_NOTHING, "n"},
    {ChirTypeKind::TYPE_CSTRING, "k"},
    {ChirTypeKind::TYPE_VOID, "u"},
};
} // namespace

namespace Codira::CHIR {
std::string MangleType(const Codira::CHIR::Type& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName);

// Type mangling for CHIR::TupleType
std::string MangleType(const CHIR::TupleType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    auto items = t.GetElementTypes();
    std::stringstream ss;
    ss << MANGLE_TUPLE_PREFIX << MangleUtils::DecimalToManglingNumber(std::to_string(items.size()));
    for (auto i : items) {
        ss << MangleType(*i, genericsTypeStack, useGenericName);
    }
    ss << MANGLE_SUFFIX;
    return ss.str();
}

// Type mangling for CHIR::StructType
std::string MangleType(const CHIR::StructType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    auto s = t.GetStructDef();
    CODEC_NULLPTR_CHECK(s);
    std::stringstream ss;
    std::string mangled = s->GetIdentifierWithoutPrefix();
    ss << MANGLE_TYPE_STRUCT_PREFIX << Codira::CHIR::StripCodiraPrefix(mangled);
    for (auto argTy : t.GetGenericArgs()) {
        ss << MangleType(*argTy, genericsTypeStack, useGenericName);
    }
    return ss.str();
}

// Type mangling for CHIR::EnumType
std::string MangleType(const CHIR::EnumType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    auto enumDef = t.GetEnumDef();
    CODEC_NULLPTR_CHECK(enumDef);
    std::stringstream ss;
    std::string mangled = enumDef->GetIdentifierWithoutPrefix();
    ss << MANGLE_NESTED_PREFIX << StripCodiraPrefix(mangled);
    for (auto argTy : t.GetGenericArgs()) {
        ss << MangleType(*argTy, genericsTypeStack, useGenericName);
    }
    return ss.str();
}

// Type mangling for CHIR::FuncType
std::string MangleType(const CHIR::FuncType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    std::stringstream ss;
    if (t.IsCFunc()) {
        // FC + return + param + E
        ss << MANGLE_CFUNC_PREFIX;
    } else {
        // F0 + return + param + E
        ss << MANGLE_GENERAL_FUNC_PREFIX;
    }
    std::string retTy = MangleType(*t.GetReturnType(), genericsTypeStack, useGenericName);
    if (retTy.empty()) {
        retTy = MANGLE_VOID_TY_SUFFIX;
    }
    ss << retTy;
    if (t.GetParamTypes().empty()) {
        return ss.str() + MANGLE_VOID_TY_SUFFIX + MANGLE_SUFFIX;
    }
    for (auto p : t.GetParamTypes()) {
        std::string mangled = MangleType(*p, genericsTypeStack, useGenericName);
        ss << mangled;
    }
    ss << MANGLE_SUFFIX;
    return ss.str();
}
// Type mangling for CHIR::ClassTpe
std::string MangleType(const CHIR::ClassType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    auto c = t.GetClassDef();
    CODEC_NULLPTR_CHECK(c);
    std::stringstream ss;
    std::string mangled = c->GetIdentifierWithoutPrefix();
    ss << "C" << StripCodiraPrefix(mangled);
    for (auto argTy : t.GetGenericArgs()) {
        ss << MangleType(*argTy, genericsTypeStack, useGenericName);
    }
    return ss.str();
}

// Type mangling for CHIR::RawArray
std::string MangleType(const CHIR::RawArrayType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    std::stringstream ss;
    ss << MANGLE_TYPE_ARRAY_PREFIX;
    ss << MangleUtils::DecimalToManglingNumber(std::to_string(t.GetDims()));
    ss << MangleType(*t.GetElementType(), genericsTypeStack, useGenericName);
    return ss.str();
}

// Type mangling for CHIR::RefType
std::string MangleType(const CHIR::RefType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    return MangleType(*t.GetBaseType(), genericsTypeStack, useGenericName);
}

std::string MangleType(const CHIR::VArrayType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    std::stringstream ss;
    ss << MANGLE_VARRAY_PREFIX;
    ss << MangleUtils::DecimalToManglingNumber(std::to_string(t.GetSize()));
    ss << MangleType(*t.GetElementType(), genericsTypeStack, useGenericName);
    return ss.str();
}

std::string MangleType(const CHIR::CPointerType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    std::stringstream ss;
    ss << MANGLE_POINTER_PREFIX;
    ss << MangleType(*t.GetElementType(), genericsTypeStack, useGenericName);
    return ss.str();
}

std::string MangleType(const CHIR::GenericType& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    std::stringstream ss;
    if (useGenericName) {
        ss << MANGLE_GENERIC_PREFIX << t.GetSrcCodeIdentifier() << std::endl;
        return ss.str();
    }
    auto result = std::find_if(genericsTypeStack.rbegin(), genericsTypeStack.rend(),
        [&t](const std::string& name) { return name == t.GetSrcCodeIdentifier(); });
    CODEC_ASSERT(result != genericsTypeStack.rend() && "Using undeclared generic type!");
    size_t index = static_cast<size_t>(std::distance(result, genericsTypeStack.rend())) - 1;
    ss << MANGLE_GENERIC_PREFIX << MangleUtils::DecimalToManglingNumber(std::to_string(index));
    return ss.str();
}

std::string ManglePrimitive(const ChirTypeKind& kind)
{
    if (TYPE_MANGLING_LUT.count(kind) != 0) {
        return TYPE_MANGLING_LUT.at(kind);
    }
    return "";
}

std::string MangleArraySliceType(const CHIR::Type& t, const std::vector<std::string>& genericsTypeStack,
    bool useGenericName)
{
    std::stringstream ss;
    ss << MANGLE_ARRAY_SLICE_PREFIX;
    ss << MangleType(*t.GetTypeArgs()[0], genericsTypeStack, useGenericName);
    ss << MANGLE_SUFFIX;
    return ss.str();
}

// Type mangling entry and dispatcher
std::string MangleType(const CHIR::Type& t)
{
    std::vector<std::string> genericsTypeStack;
    return MangleType(t, genericsTypeStack);
}

// Type mangling entry and dispatcher
std::string MangleType(const CHIR::Type& t, const std::vector<std::string>& genericsTypeStack, bool useGenericName)
{
    auto k = t.GetTypeKind();
    if (auto prim = ManglePrimitive(k); !prim.empty()) {
        return prim;
    }
    CODEC_ASSERT(k != ChirTypeKind::TYPE_INVALID);
    switch (k) {
        case ChirTypeKind::TYPE_TUPLE:
            return MangleType(StaticCast<const TupleType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_STRUCT:
            return MangleType(StaticCast<const StructType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_ENUM:
            return MangleType(StaticCast<const EnumType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_FUNC:
            return MangleType(StaticCast<const FuncType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_CLASS: {
            return MangleType(StaticCast<const ClassType&>(t), genericsTypeStack, useGenericName);
        }
        case ChirTypeKind::TYPE_RAWARRAY:
            return MangleType(StaticCast<const RawArrayType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_REFTYPE:
            return MangleType(StaticCast<const RefType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_VARRAY:
            return MangleType(StaticCast<const VArrayType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_CPOINTER:
            return MangleType(StaticCast<const CPointerType&>(t), genericsTypeStack, useGenericName);
        case ChirTypeKind::TYPE_GENERIC:
            return MangleType(StaticCast<const GenericType&>(t), genericsTypeStack, useGenericName);
        default:
            CODEC_ASSERT(false && "Unexpected type to be mangled.");
            return "";
    }
}

std::string MangleType(const CHIR::Type& t, const std::vector<std::string>& genericsTypeStack)
{
    return MangleType(t, genericsTypeStack, false);
}

std::string GetTypeArgsQualifiedName(const std::vector<Type*>& typeArgs, bool forNameFieldOfTi)
{
    std::string res;
    for (auto typeArg : typeArgs) {
        res += GetTypeQualifiedName(*typeArg, forNameFieldOfTi) + ',';
    }
    if (!typeArgs.empty()) {
        res.pop_back(); // The last ',' is redundant.
    }
    return res;
}

std::string GetTypeQualifiedNameOfCustomType(const CHIR::CustomType& type, bool forNameFieldOfTi)
{
    CODEC_ASSERT(!type.IsAutoEnvInstBase());
    std::stringstream ss;
    if (type.IsAutoEnvGenericBase()) {
        auto typeArgs = type.GetGenericArgs();
        CODEC_ASSERT(!typeArgs.empty());
        auto retIter = typeArgs.end() - 1U;
        std::vector<Type*> paramType(typeArgs.begin(), retIter);
        ss << MANGLE_CLOSURE_STR << MANGLE_LT_PREFIX;
        ss << '(' << GetTypeArgsQualifiedName(paramType, true) << ")->" << GetTypeQualifiedName(**retIter, true);
        ss << MANGLE_GT_PREFIX;
    } else {
        auto def = type.GetCustomTypeDef();
        auto packageName = def->GetGenericDecl() ? def->GetGenericDecl()->GetPackageName() : def->GetPackageName();
        // Replace "::" between organization name and package name with a single character "/"
        // e.g., com::pkga.b -> com/pkga.b
        if (auto splitterIt = packageName.find("::"); splitterIt != std::string::npos) {
            packageName.replace(splitterIt, 2U, "/");
        }
        ss << packageName << ':' << GetCustomTypeIdentifier(type);
        if (auto typeArgs = type.GetGenericArgs(); !typeArgs.empty()) {
            ss << MANGLE_LT_PREFIX << GetTypeArgsQualifiedName(typeArgs, forNameFieldOfTi) << MANGLE_GT_PREFIX;
        }
    }
    return ss.str();
}

std::string GetTypeQualifiedName(const CHIR::Type& t, bool forNameFieldOfTi)
{
    if (t.IsPrimitive()) {
        return t.ToString();
    }
    auto k = t.GetTypeKind();
    CODEC_ASSERT(k != ChirTypeKind::TYPE_INVALID);
    switch (k) {
        case ChirTypeKind::TYPE_RAWARRAY:
            return MANGLE_RAWARR_STR + MANGLE_LT_PREFIX + \
                GetTypeQualifiedName(*t.GetTypeArgs()[0], forNameFieldOfTi) + MANGLE_GT_PREFIX;
        case ChirTypeKind::TYPE_REFTYPE:
            return GetTypeQualifiedName(*static_cast<const RefType&>(t).GetBaseType(), forNameFieldOfTi);
        case ChirTypeKind::TYPE_VARRAY: {
            auto& varrType = StaticCast<const CHIR::VArrayType&>(t);
            return MANGLE_VARR_STR + MANGLE_LT_PREFIX + \
                GetTypeQualifiedName(*varrType.GetElementType(), forNameFieldOfTi) + ',' + \
                std::to_string(varrType.GetSize()) + MANGLE_GT_PREFIX;
        }
        case ChirTypeKind::TYPE_CPOINTER:
            return MANGLE_CPTR_STR + MANGLE_LT_PREFIX + \
                GetTypeQualifiedName(*t.GetTypeArgs()[0], forNameFieldOfTi) + MANGLE_GT_PREFIX;
        case ChirTypeKind::TYPE_CSTRING:
            return MANGLE_CSTRING_STR;
        case ChirTypeKind::TYPE_TUPLE: {
            auto& type = static_cast<const TupleType&>(t);
            return MANGLE_TUPLE_STR + MANGLE_LT_PREFIX + \
                GetTypeArgsQualifiedName(type.GetElementTypes(), forNameFieldOfTi) + MANGLE_GT_PREFIX;
        }
        case ChirTypeKind::TYPE_STRUCT:
        case ChirTypeKind::TYPE_ENUM:
        case ChirTypeKind::TYPE_CLASS: {
            auto& type = static_cast<const CustomType&>(t);
            return GetTypeQualifiedNameOfCustomType(type, forNameFieldOfTi);
        }
        case ChirTypeKind::TYPE_FUNC: {
            auto& ft = static_cast<const FuncType&>(t);
            std::string name = '(' + GetTypeArgsQualifiedName(ft.GetParamTypes(), forNameFieldOfTi) + ")->" +
                GetTypeQualifiedName(*ft.GetReturnType(), forNameFieldOfTi);
            return name;
        }
        case ChirTypeKind::TYPE_GENERIC:
            return static_cast<const GenericType&>(t).GetSrcCodeIdentifier();
        case ChirTypeKind::TYPE_VOID:
            return MANGLE_UNIT_STR;
        case ChirTypeKind::TYPE_BOXTYPE: {
            auto& type = static_cast<const BoxType&>(t);
            auto typeQualifiedName = GetTypeQualifiedName(*type.GetBaseType(), forNameFieldOfTi);
            return forNameFieldOfTi ? typeQualifiedName : (MANGLE_BOX_STR + MANGLE_LT_PREFIX + \
                typeQualifiedName + MANGLE_GT_PREFIX);
        }
        default:
            CODEC_ASSERT(false && "Should not reach here.");
            return "";
    }
}

std::string GetCustomTypeIdentifier(const CHIR::CustomType& type)
{
    return GetCustomTypeDefIdentifier(*type.GetCustomTypeDef());
}

std::string GetCustomTypeDefIdentifier(const CHIR::CustomTypeDef& def)
{
    CODEC_ASSERT(!def.GetType()->IsAutoEnvBase());
    auto name = def.GetSrcCodeIdentifier();
    if (name.empty() ||
        (def.TestAttr(CHIR::Attribute::PRIVATE) && def.Get<CHIR::LinkTypeInfo>() == Linkage::INTERNAL)) {
        return def.GetIdentifierWithoutPrefix();
    } else {
        return name;
    }
}
} // namespace Codira::CHIR
