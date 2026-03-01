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

#include "assembler/assembly-type.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"
#include "plugins/ets/runtime/types/ets_type.h"

namespace ark::ets::ani {

static size_t ParseArrayBody(const std::string_view data, PandaString &str);

/*static*/
std::optional<PandaString> Mangle::ConvertDescriptor(const std::string_view descriptor, bool allowArray)
{
    if (descriptor.empty() || descriptor.back() == ';') {
        // The 'descriptor' has incorrect format, report error to user
        LOG(WARNING, ANI) << "Incorrect mangling: " << descriptor;
        return std::nullopt;
    }

    PandaString str;
    if (allowArray) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (descriptor.size() >= 3U && descriptor[0] == 'A' && descriptor[1] == '{') {
            auto bodySize = ParseArrayBody(descriptor.substr(1), str);
            if (bodySize == std::string_view::npos) {
                // The 'descriptor' has wrong format, so can't be convert
                LOG(WARNING, ANI) << "Incorrect mangling: " << descriptor;
                return std::nullopt;
            }
            return str;
        }
    }

    str.reserve(descriptor.size() + 2U);
    str.push_back('L');
    for (size_t pos = 0; pos < descriptor.size(); ++pos) {
        if (descriptor[pos] == '/') {
            // The new format 'descriptor' can't contain '/'
            LOG(WARNING, ANI) << "Incorrect mangling: " << descriptor;
            return std::nullopt;
        }
        str.push_back(descriptor[pos] == '.' ? '/' : descriptor[pos]);
    }
    str.push_back(';');

    return str;
}

static constexpr size_t MIN_BODY_SIZE = sizeof('{') + 1 + sizeof('}');

static size_t ParseType(char type, const std::string_view data, PandaString &str);

static size_t ParseUnionBody(const std::string_view data, PandaString &str)
{
    if (data.size() < MIN_BODY_SIZE || data[0] != '{') {
        return std::string_view::npos;
    }
    PandaString unionStr;
    unionStr.append("{U");

    std::string_view previousConstituentTypes;
    size_t size = 1;
    while (size < data.size() && data[size] != '}') {
        std::string_view substr = data.substr(size);
        size_t sz = ParseType(data[size], substr, unionStr);
        if (sz == std::string_view::npos) {
            // The 'descriptor' does not have a new format, so no conversion is required.
            return std::string_view::npos;
        }
        std::string_view parsedType = substr.substr(0, sz);
        // NOTE(dslynko, #26223): move constituent types order check into VerifyANI
        if (previousConstituentTypes > parsedType) {
            // Constituent types must be ordered in alphabetical order with respect to ANI encodings.
            return std::string_view::npos;
        }
        size += sz;
    }
    if (size >= data.size() || data[size] != '}') {
        // Union descriptor must end with '}'.
        return std::string_view::npos;
    }
    unionStr.push_back('}');

    str.append(pandasm::Type::CanonicalizeDescriptor(unionStr));
    return size + sizeof('}');
}

static size_t ParseArrayBody(const std::string_view data, PandaString &str)
{
    if (data.size() < MIN_BODY_SIZE || data[0] != '{') {
        return std::string_view::npos;
    }
    str.push_back('[');

    char type = data[1];
    const std::string_view typeData = data.substr(1);
    size_t size = ParseType(type, typeData, str);
    if (size == std::string_view::npos || size >= typeData.size() || typeData[size] != '}') {
        return std::string_view::npos;
    }
    return sizeof('{') + size + sizeof('}');
}

static size_t ParseBody(char type, const std::string_view data, PandaString &str)
{
    ASSERT(type != 'A');
    ASSERT(type != 'X');

    if (data.size() < MIN_BODY_SIZE || data[0] != '{') {
        return std::string_view::npos;
    }
    auto end = data.find('}', 1);
    if (end != std::string_view::npos) {
        str.push_back('L');
        for (size_t pos = 1; pos < end; ++pos) {
            if (data[pos] == '/' || data[pos] == ':') {
                // The new format 'descriptor' can't contain '/'
                return std::string_view::npos;
            }
            str.push_back(data[pos] == '.' ? '/' : data[pos]);
        }
        if (type == 'P') {
            // e.g. "La/b/c/X;" -> "La/b/c/%%partial-X;"
            size_t lastPos = str.find_last_of('/') + 1;
            str.replace(lastPos, str.length(), "%%partial-" + str.substr(lastPos));
        }
        str.push_back(';');
        return end + 1;
    }
    return std::string_view::npos;
}

static size_t ParseType(char type, const std::string_view data, PandaString &str)
{
    size_t bodySize = std::string_view::npos;
    // clang-format off
    switch (type) {
        case 'z': str.push_back('Z'); return 1;
        case 'c': str.push_back('C'); return 1;
        case 'b': str.push_back('B'); return 1;
        case 's': str.push_back('S'); return 1;
        case 'i': str.push_back('I'); return 1;
        case 'l': str.push_back('J'); return 1;
        case 'f': str.push_back('F'); return 1;
        case 'd': str.push_back('D'); return 1;
        case 'Y': str.append("Lstd/core/Object;"); return 1;
        case 'N': str.append("Lstd/core/Object;"); return 1;
        case 'U': str.append(panda_file_items::class_descriptors::OBJECT); return 1;
        case 'A': bodySize = ParseArrayBody(data.substr(1), str); break;
        case 'X': bodySize = ParseUnionBody(data.substr(1), str); break;
        case 'C':
        case 'E':
        case 'P': bodySize = ParseBody(type, data.substr(1), str); break;
        default:
            // The 'descriptor' does not have a new format, so no conversion is required.
            return std::string_view::npos;
    }
    // clang-format on

    if (bodySize == std::string_view::npos) {
        return std::string_view::npos;
    }
    return 1 + bodySize;
}

/*static*/
std::optional<PandaString> Mangle::ConvertSignature(const std::string_view descriptor)
{
    PandaString str;
    str.reserve(descriptor.size() * 2U);
    int nr = -1;
    int k = -1;
    for (size_t i = 0; i < descriptor.size(); ++i) {
        char type = descriptor[i];
        if (type == ':') {
            str.push_back(':');
            nr = 0;
            k = 1;
            continue;
        }
        size_t sz = ParseType(type, descriptor.substr(i), str);
        if (sz == std::string_view::npos) {
            // The 'descriptor' does not have a new format, so no conversion is required.
            LOG(WARNING, ANI) << "Incorrect mangling: " << descriptor;
            return std::nullopt;
        }
        i += sz - 1;
        nr += k;
    }
    if (k == -1) {
        // The 'descriptor' does not have a ':' symbol
        LOG(WARNING, ANI) << "Incorrect mangling: " << descriptor;
        return std::nullopt;
    }
    if (nr == 0) {
        str.push_back('V');
    }
    return str;
}

static EtsType GetTypeByFirstChar(char c)
{
    switch (c) {
        case 'Y':
        case 'N':
        case 'U':
        case 'A':
        case 'X':
        case 'C':
        case 'E':
        case 'P':
            return EtsType::OBJECT;
        case 'z':
            return EtsType::BOOLEAN;
        case 'b':
            return EtsType::BYTE;
        case 'c':
            return EtsType::CHAR;
        case 's':
            return EtsType::SHORT;
        case 'i':
            return EtsType::INT;
        case 'l':
            return EtsType::LONG;
        case 'f':
            return EtsType::FLOAT;
        case 'd':
            return EtsType::DOUBLE;
        default:
            return EtsType::UNKNOWN;
    }
}

static size_t ProcessObjectParameter(PandaSmallVector<PandaString> &paramTypes, const std::string_view signature,
                                     size_t i)
{
    PandaString str;
    str.reserve(signature.size() - i);
    size_t typeSize = ParseType(signature[i], signature.substr(i), str);
    if (typeSize == PandaString::npos) {
        return typeSize;
    }
    paramTypes.push_back(str);
    return typeSize + i;
}

static size_t ProcessParameter(Method::Proto &proto, PandaSmallVector<PandaString> &paramTypes,
                               const std::string_view signature, size_t i)
{
    EtsType paramType = GetTypeByFirstChar(signature[i]);
    // Check that type is valid
    if (paramType == EtsType::UNKNOWN) {
        return std::string_view::npos;
    }
    proto.GetShorty().push_back(ets::ConvertEtsTypeToPandaType(paramType));

    if (paramType != EtsType::OBJECT) {
        return i + 1;
    }

    return ProcessObjectParameter(paramTypes, signature, i);
}

static size_t ParseReturnValue(Method::Proto &proto, PandaSmallVector<PandaString> &paramTypes,
                               const std::string_view signature)
{
    size_t dots = signature.rfind(':');
    // Return if ':' wasn't founded
    if (dots == std::string_view::npos) {
        return dots;
    }

    if (dots == signature.size() - 1) {
        proto.GetShorty().push_back(ets::ConvertEtsTypeToPandaType(EtsType::VOID));
        return dots;
    }
    // Process return type after ':'
    auto nextPos = ProcessParameter(proto, paramTypes, signature, dots + 1);
    if (nextPos == std::string_view::npos || nextPos != signature.size()) {
        return std::string_view::npos;
    }

    return dots;
}

void Mangle::ConvertSignatureToProto(std::optional<EtsMethodSignature> &methodSignature,
                                     const std::string_view signature)
{
    Method::Proto proto;
    PandaSmallVector<PandaString> paramTypes;

    size_t dots = ParseReturnValue(proto, paramTypes, signature);
    if (dots == std::string_view::npos) {
        return;
    }

    // Process parameters before ':'
    for (size_t i = 0; i < dots;) {
        i = ProcessParameter(proto, paramTypes, signature, i);
        if (i == std::string_view::npos) {
            return;
        }
    }
    methodSignature.emplace(std::move(proto), std::move(paramTypes));
}

}  // namespace ark::ets::ani
