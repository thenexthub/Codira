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
#include "hello.impl.hpp"
#include <iostream>
#include "hello.proj.hpp"
#include "stdexcept"
#include "taihe/optional.hpp"
#include "taihe/runtime.hpp"

namespace {
// To be implemented.

void setData(::hello::Data const &data) {}

::hello::Data getData()
{
    return {
        .a = taihe::optional<taihe::string> {std::in_place, "name"},
        .b = taihe::optional<double> {std::in_place, 2.5},
        .c = taihe::optional<int> {std::in_place, 3},
        .d = taihe::optional<bool> {std::in_place, true},
        .e = taihe::optional<bool> {std::in_place, true},
        .f = taihe::optional<bool> {std::in_place, false},
        .g = taihe::optional<bool> {std::in_place, false},
    };
}

void setRecord(::taihe::map_view<::taihe::string, ::taihe::string> rec) {}

static ::taihe::map<::taihe::string, ::taihe::string> global_rec = [] {
    ::taihe::map<::taihe::string, ::taihe::string> rec;
    rec.emplace("key0", "value0");
    rec.emplace("key1", "value1");
    rec.emplace("key2", "value2");
    rec.emplace("key3", "value3");
    rec.emplace("key4", "value4");
    rec.emplace("key5", "value5");
    rec.emplace("key6", "value6");
    rec.emplace("key7", "value7");
    return rec;
}();

::taihe::map<::taihe::string, ::taihe::string> getRecord()
{
    return global_rec;
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_setData(setData);
TH_EXPORT_CPP_API_getData(getData);
TH_EXPORT_CPP_API_setRecord(setRecord);
TH_EXPORT_CPP_API_getRecord(getRecord);
// NOLINTEND