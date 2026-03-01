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

class EtsInteropJsTestFrontend : public EtsInteropTest {};

TEST_F(EtsInteropJsTestFrontend, test_newcall)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "test_newcall"));
}

TEST_F(EtsInteropJsTestFrontend, test_dyncall)
{
    auto ret = CallEtsFunction<uint32_t>(GetPackageName(), "test_dyncall");
    ASSERT_EQ(ret, 10U);
}

TEST_F(EtsInteropJsTestFrontend, test_dyncall_by_value)
{
    auto ret = CallEtsFunction<uint32_t>(GetPackageName(), "test_dyncall_by_value");
    ASSERT_EQ(ret, 43U);
}

}  // namespace ark::ets::interop::js::testing
