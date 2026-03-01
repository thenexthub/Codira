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

#include "Codira/CHIR/StringWrapper.h"

using namespace Codira::CHIR;

StringWrapper::StringWrapper(const std::string& initVal) : value(initVal)
{
}

const std::string& StringWrapper::Str() const
{
    return value;
}

void StringWrapper::Append(const std::string& newValue)
{
    value += newValue;
}

void StringWrapper::Append(const std::string& newValue, const std::string& delimiter)
{
    if (!newValue.empty() && !value.empty()) {
        value += delimiter + " " + newValue;
    }
}

void StringWrapper::RemoveLastNChars(const size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        value.pop_back();
    }
}

StringWrapper& StringWrapper::AddDelimiterOrNot(const std::string& delimiter)
{
    if (!value.empty()) {
        value += delimiter;
    }
    return *this;
}

StringWrapper& StringWrapper::AppendOrClear(const std::string& newValue)
{
    if (newValue.empty()) {
        value.clear();
    } else {
        value += newValue;
    }
    return *this;
}
