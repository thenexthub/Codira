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
#include "foo.impl.hpp"
#include "foo.DerivedMethodClass.impl.h"
#include "stdexcept"

namespace {
::foo::DerivedMethodClass MakeDerivedMethodClass()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<DerivedMethodClassImpl, ::foo::DerivedMethodClass>();
}

::foo::DerivedDataClass MakeDerivedDataClass()
{
    return {
        .base = {"base"},
        .foo = {"foo"},
        .bar = {"bar"},
        .x = 42,
        .y = 56,
    };
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_MakeDerivedMethodClass(MakeDerivedMethodClass);
TH_EXPORT_CPP_API_MakeDerivedDataClass(MakeDerivedDataClass);
// NOLINTEND