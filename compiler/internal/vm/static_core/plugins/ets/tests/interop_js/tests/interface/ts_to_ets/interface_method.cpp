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

class EtsInterfaceTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeInterfaceClassString"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeInterfaceClassInt"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_bool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeInterfaceClassBool"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_array)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeInterfaceClassArray"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_any_type_interface_class_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeInterfaceClassObject"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassAnyTypeMethodString"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassAnyTypeMethodInt"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_bool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassAnyTypeMethodBool"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_array)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassAnyTypeMethodArray"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_any_type_method_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassAnyTypeMethodObject"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_union_type_interface_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUnionTypeInterfaceClassString"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_union_type_interface_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUnionTypeInterfaceClassInt"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_union_type_method_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassUnionTypeMethodString"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_interface_class_union_type_method_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassUnionTypeMethodInt"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_subset_by_ref_interface)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSubsetByRefInterface"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_subset_by_ref_interface_error)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSubsetByRefInterfaceError"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_subset_by_value_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSubsetByValueClass"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_subset_by_value_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateSubsetByValueClassFromTs"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_interface_class_union_type_method_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceInterfaceClassUnionTypeMethodInt"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_interface_class_union_type_method_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceInterfaceClassUnionTypeMethodString"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_instance_subset_by_value_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceSubsetByValueClassFromTs"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_tuple_type_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTupleTypeClass"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_create_tuple_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateTupleClassFromTs"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_instance_tuple_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceTupleClassFromTs"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_with_optional_method_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWithOptionalMethodClass"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_optional_method_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWithoutOptionalMethodClass"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_class_with_optional_method)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassWithOptionalMethod"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_create_class_without_optional_method)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassWithoutOptionalMethod"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_with_optional_method_instance_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWithOptionalMethodInstanceClass"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_optional_method_instance_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWithoutOptionalMethodInstanceClass"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_optional_arg_with_all_args)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalArgWithAllArgs"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_optional_arg_with_one_args)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalArgWithOneArgs"));
}
// NOTE (issues 17772) fix spread operator
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_spread_operator_arg_with_all_args)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSpreadOperatorArgWithAllArgs"));
}
// NOTE (issues 17772) fix spread operator
TEST_F(EtsInterfaceTsToEtsTest, DISABLED_check_spread_operator_arg_with_one_args)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSpreadOperatorArgWithOneArgs"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_spread_operator_arg_with_all_args)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWithoutSpreadOperatorArgWithAllArgs"));
}

TEST_F(EtsInterfaceTsToEtsTest, check_without_spread_operator_arg_with_one_args)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWithoutSpreadOperatorArgWithOneArgs"));
}

}  // namespace ark::ets::interop::js::testing
