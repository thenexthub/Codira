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

class EtsInteropScenariosJsToEtsConflictTypes : public EtsInteropTest {};

// NOTE(splatov) #17937 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_array)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictArray");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_arraybuffer)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictArraybuffer");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17379 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_boolean)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictBoolean");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17939 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_dataview)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictDataview");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17939 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_date)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictDate");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17940 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_error)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictError");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17939 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_map)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictMap");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_object)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictObject");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_string)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeConflictString");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
