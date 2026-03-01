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
#include "customizeData.impl.hpp"
#include <iostream>
#include "customizeData.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace customizeData;

namespace {

class CustomizeDataImpl {
public:
    string name_ = "bob";
    string value_ = "jack";
    string extra_ = "john";

    CustomizeDataImpl() {}

    void SetName(string_view name)
    {
        name_ = name;
    }

    string GetName()
    {
        return name_;
    }

    void SetValue(string_view value)
    {
        value_ = value;
    }

    string GetValue()
    {
        return value_;
    }

    void SetExtra(string_view extra)
    {
        extra_ = extra;
    }

    string GetExtra()
    {
        return extra_;
    }
};

CustomizeData GetCustomizeData()
{
    return make_holder<CustomizeDataImpl, CustomizeData>();
}
}  // namespace

TH_EXPORT_CPP_API_GetCustomizeData(GetCustomizeData);