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

#ifndef CPP_ABCKIT_VALUE_IMPL_H
#define CPP_ABCKIT_VALUE_IMPL_H

#include "value.h"
#include "type.h"
#include "literal_array.h"

namespace abckit {

inline Type Value::GetType() const
{
    auto value = GetApiConfig()->cIapi_->valueGetType(GetView());
    CheckError(GetApiConfig());
    return Type(value, GetApiConfig(), GetResource());
}

inline bool Value::GetU1() const
{
    bool ret = GetApiConfig()->cIapi_->valueGetU1(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline int Value::GetInt() const
{
    int ret = GetApiConfig()->cIapi_->valueGetInt(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline double Value::GetDouble() const
{
    double ret = GetApiConfig()->cIapi_->valueGetDouble(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline std::string Value::GetString() const
{
    AbckitString *abcStr = GetApiConfig()->cIapi_->valueGetString(GetView());
    CheckError(GetApiConfig());
    std::string str = GetApiConfig()->cIapi_->abckitStringToString(abcStr);
    CheckError(GetApiConfig());
    return str;
}

inline const File *Value::GetFile() const
{
    CheckError(GetApiConfig());
    return GetResource();
}

inline LiteralArray Value::GetLiteralArray() const
{
    auto ret = GetApiConfig()->cIapi_->arrayValueGetLiteralArray(GetView());
    CheckError(GetApiConfig());
    return LiteralArray(ret, GetApiConfig(), GetResource());
}

}  // namespace abckit

#endif
