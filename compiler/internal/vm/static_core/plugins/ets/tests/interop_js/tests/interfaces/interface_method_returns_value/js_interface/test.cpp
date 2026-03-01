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
#include <iostream>
#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropInterfaceReturnsValuesArkToJs : public EtsInteropTest {};

/* test set employing interface imported from JS as is */
// #21832: returned undefined previously, now leads to NPE
TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, DISABLED_test_interface_returns_any_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_string_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnString");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_bigint_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnBigint");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_boolean_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnBoolean");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_integer_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnInteger");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_negative_integer_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnNegativeInteger");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_infinity_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnInfinity");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_negative_infinity_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnNegativeInfinity");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_NaN_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnNaN");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_enum_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnEnum");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_undefined_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnUndefined");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_null_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnNull");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropInterfaceReturnsValuesArkToJs, test_interface_returns_function_type_imported)
{
    [[maybe_unused]] auto ret = CallEtsFunction<bool>(GetPackageName(), "type_imported__returnFunction");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
