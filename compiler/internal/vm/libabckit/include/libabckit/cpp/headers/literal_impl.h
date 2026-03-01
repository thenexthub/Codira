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

#ifndef CPP_ABCKIT_LITERAL_IMPL_H
#define CPP_ABCKIT_LITERAL_IMPL_H

#include <cstdint>
#include "literal_array.h"
#include "literal.h"

namespace abckit {

inline bool Literal::GetBool() const
{
    bool ret = GetApiConfig()->cIapi_->literalGetBool(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline abckit::LiteralArray Literal::GetLiteralArray() const
{
    LiteralArray ret(GetApiConfig()->cIapi_->literalGetLiteralArray(GetView()), GetApiConfig(), GetResource());
    CheckError(GetApiConfig());
    return ret;
}

inline uint8_t Literal::GetU8() const
{
    auto val = GetApiConfig()->cIapi_->literalGetU8(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline uint16_t Literal::GetU16() const
{
    auto val = GetApiConfig()->cIapi_->literalGetU16(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline uint32_t Literal::GetU32() const
{
    auto val = GetApiConfig()->cIapi_->literalGetU32(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline uint64_t Literal::GetU64() const
{
    auto val = GetApiConfig()->cIapi_->literalGetU64(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline float Literal::GetFloat() const
{
    auto val = GetApiConfig()->cIapi_->literalGetFloat(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline double Literal::GetDouble() const
{
    auto val = GetApiConfig()->cIapi_->literalGetDouble(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline uint16_t Literal::GetMethodAffiliate() const
{
    auto val = GetApiConfig()->cIapi_->literalGetMethodAffiliate(GetView());
    CheckError(GetApiConfig());
    return val;
}

inline std::string Literal::GetString() const
{
    auto val = GetApiConfig()->cIapi_->literalGetString(GetView());
    CheckError(GetApiConfig());
    auto str = GetApiConfig()->cIapi_->abckitStringToString(val);
    CheckError(GetApiConfig());
    return str;
}

inline std::string Literal::GetMethod() const
{
    auto val = GetApiConfig()->cIapi_->literalGetMethod(GetView());
    CheckError(GetApiConfig());
    auto str = GetApiConfig()->cIapi_->abckitStringToString(val);
    CheckError(GetApiConfig());
    return str;
}

inline enum AbckitLiteralTag Literal::GetTag() const
{
    auto tag = GetApiConfig()->cIapi_->literalGetTag(GetView());
    CheckError(GetApiConfig());
    return tag;
}

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_IMPL_H
