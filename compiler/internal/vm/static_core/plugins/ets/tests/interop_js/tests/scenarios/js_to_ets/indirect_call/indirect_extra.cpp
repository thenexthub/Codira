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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropScenariosJsToEtsIndirectCallExtra : public EtsInteropTest {};

// NOTE #18313 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsIndirectCallExtra, DISABLED_Test_indirect_call_type_extra_set_call)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "Test_indirect_call_type_extra_set_call");
    ASSERT_EQ(ret, true);
}

// NOTE #18313 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsIndirectCallExtra, DISABLED_Test_indirect_call_type_extra_set_apply)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "Test_indirect_call_type_extra_set_apply");
    ASSERT_EQ(ret, true);
}

// NOTE #18313 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsIndirectCallExtra, DISABLED_Test_indirect_call_type_extra_set_bind_with_arg)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "Test_indirect_call_type_extra_set_bind_with_arg");
    ASSERT_EQ(ret, true);
}

// NOTE #18313 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsIndirectCallExtra, DISABLED_Test_indirect_call_type_extra_set_bind_without_arg)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "Test_indirect_call_type_extra_set_bind_without_arg");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
