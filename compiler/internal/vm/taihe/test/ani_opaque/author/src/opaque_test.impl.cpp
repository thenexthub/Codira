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
#include "opaque_test.impl.hpp"

#include <cstdint>
#include <iostream>

#include "opaque_test.Union.proj.1.hpp"
#include "stdexcept"
#include "taihe/array.hpp"
#include "taihe/runtime.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {

bool is_string(uintptr_t a)
{
    ani_boolean res;
    ani_class cls;
    ani_env *env = get_env();
    env->FindClass("std.core.String", &cls);
    env->Object_InstanceOf((ani_object)a, cls, &res);
    return res;
}

array<uintptr_t> get_objects()
{
    ani_env *env = get_env();
    ani_string ani_arr_0;
    int const stringLen = 3;
    env->String_NewUTF8("AAA", stringLen, &ani_arr_0);
    ani_ref ani_arr_1;
    env->GetUndefined(&ani_arr_1);
    return array<uintptr_t>({(uintptr_t)ani_arr_0, (uintptr_t)ani_arr_1});
}

uintptr_t get_object()
{
    ani_env *env = get_env();
    ani_string ani_arr_0;
    int const stringLen = 3;
    env->String_NewUTF8("BBB", stringLen, &ani_arr_0);
    return (uintptr_t)ani_arr_0;
}

bool is_opaque(::opaque_test::Union const &s)
{
    ani_boolean res;
    ani_class cls;
    ani_env *env = get_env();
    if (s.get_tag() == ::opaque_test::Union::tag_t::oValue) {
        return (ani_boolean) true;
    }
    return (ani_boolean) false;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_is_string(is_string);
TH_EXPORT_CPP_API_get_object(get_object);
TH_EXPORT_CPP_API_get_objects(get_objects);
TH_EXPORT_CPP_API_is_opaque(is_opaque);
// NOLINTEND