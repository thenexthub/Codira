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

class EtsSetterTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsSetterTsToEtsTest, check_abstract_class_setter)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAbstractClassSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_union_setter)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUnionSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_interface_setter)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInterfaceSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_user_class_setter)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUserClassSetter"));
}

TEST_F(EtsSetterTsToEtsTest, check_extends_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendsClass"));
}

TEST_F(EtsSetterTsToEtsTest, check_extends_class_with_value)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendsClassWithValue"));
}
// NOTE (#24570): fix interop tests
TEST_F(EtsSetterTsToEtsTest, DISABLED_check_tuple_type_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTupleTypeObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_any_type_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterAnyTypeObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_union_type_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterUnionTypeObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_interface_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterInterfaceObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_user_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterUserObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_subset_ref_set_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterSubsetRefSetObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, DISABLED_check_setter_subset_value_set_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterSubsetValueSetObjectFormTs"));
}

TEST_F(EtsSetterTsToEtsTest, check_setter_abstract_class_object_form_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetterAbstractClassObjectFormTs"));
}

}  // namespace ark::ets::interop::js::testing
