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

class EtsGetterTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsGetterTsToEtsTest, check_getter_public_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetterPublicClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_public_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreatePublicGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_public_getter_instance_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkPublicGetterInstanceClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_union_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUnionTypeGetterClassInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_union_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateUnionTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_union_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceUnionTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_union_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUnionTypeGetterClassString"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_union_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateUnionTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_union_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceUnionTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_literal_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkLiteralTypeGetterClassInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_literal_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkLiteralTypeGetterClassString"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_literal_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateLiteralTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_literal_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateLiteralTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_literal_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceLiteralTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_literal_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceLiteralTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_tuple_type_getter_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTupleTypeGetterClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_tuple_type_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateTupleTypeGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_tuple_type_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceTupleTypeGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassString"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_bool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassBool"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_arr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassArr"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassObj"));
}

TEST_F(EtsGetterTsToEtsTest, DISABLED_check_any_type_getter_class_tuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassTuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_any_type_getter_class_union)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeGetterClassUnion"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_bool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsBool"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_arr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsArr"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsObj"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_tuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsTuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_any_type_getter_class_from_ts_union)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnyTypeGetterClassFromTsUnion"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsInt"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsString"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_bool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsBool"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_arr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsArr"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsObj"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_tuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsTuple"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_type_getter_class_from_ts_union)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyTypeGetterClassFromTsUnion"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_any_explicit_type_getter_class_from_ts_explicit)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAnyExplicitTypeGetterClassFromTsExplicit"));
}

TEST_F(EtsGetterTsToEtsTest, check_getter_subset_by_ref_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetterSubsetByRefClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_subset_by_ref_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateSubsetByRefGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_subset_by_ref_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceSubsetByRefGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_getter_subset_by_value_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetterSubsetByValueClass"));
}

TEST_F(EtsGetterTsToEtsTest, check_create_subset_by_value_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateSubsetByValueGetterClassFromTs"));
}

TEST_F(EtsGetterTsToEtsTest, check_instance_subset_by_value_getter_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceSubsetByValueGetterClassFromTs"));
}
}  // namespace ark::ets::interop::js::testing
