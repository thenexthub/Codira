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

class EtsInteropJsJSValue : public EtsInteropTest {};

TEST_F(EtsInteropJsJSValue, double2jsvalue)
{
    constexpr double TEST_VALUE = 3.6;

    // Call ets method
    auto ret = CallEtsFunction<napi_value>(GetPackageName(), "double2jsvalue", TEST_VALUE);
    ASSERT_TRUE(ret.has_value());

    // Get double from napi_value
    double v;
    napi_status status = napi_get_value_double(GetJsEnv(), ret.value(), &v);
    ASSERT_EQ(status, napi_ok);

    // Check result
    ASSERT_EQ(v, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, jsvalue2double)
{
    constexpr double TEST_VALUE = 53.23;

    // Create napi_value from double
    napi_value jsValue;
    ASSERT_EQ(napi_ok, napi_create_double(GetJsEnv(), TEST_VALUE, &jsValue));

    // Call ets method
    auto ret = CallEtsFunction<double>(GetPackageName(), "jsvalue2double", jsValue);

    // Check result
    ASSERT_EQ(ret, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, get_property_from_jsvalue)
{
    napi_env env = GetJsEnv();

    constexpr double TEST_VALUE = 67.78;
    // Create js object:
    //  {
    //      .prop = 67.78
    //  }
    napi_value jsObj;
    napi_value jsValue;
    ASSERT_EQ(napi_ok, napi_create_object(env, &jsObj));
    ASSERT_EQ(napi_ok, napi_create_double(env, TEST_VALUE, &jsValue));
    ASSERT_EQ(napi_ok, napi_set_named_property(env, jsObj, "prop", jsValue));

    // Call ets method
    auto ret = CallEtsFunction<double>(GetPackageName(), "getPropertyFromJsvalue", jsObj);

    // Check result
    ASSERT_EQ(ret, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, get_property_from_jsvalue2)
{
    napi_env env = GetJsEnv();

    constexpr double TEST_VALUE = 674.6;
    // Create js object:
    //  {
    //      .prop_1 = {
    //          .prop_2 = 674.6
    //      }
    //  }
    napi_value jsObj {};
    napi_value jsProp1 {};
    napi_value jsProp2 {};
    ASSERT_EQ(napi_ok, napi_create_object(env, &jsObj));
    ASSERT_EQ(napi_ok, napi_create_object(env, &jsProp1));
    ASSERT_EQ(napi_ok, napi_create_double(env, TEST_VALUE, &jsProp2));
    ASSERT_EQ(napi_ok, napi_set_named_property(env, jsObj, "prop_1", jsProp1));
    ASSERT_EQ(napi_ok, napi_set_named_property(env, jsProp1, "prop_2", jsProp2));

    // Call ets method
    auto ret = CallEtsFunction<double>(GetPackageName(), "getPropertyFromJsvalue2", jsObj);

    // Check result
    ASSERT_EQ(ret, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, set_property_to_jsvalue)
{
    napi_env env = GetJsEnv();

    constexpr double TEST_VALUE = 54.064;
    // Create js object:
    //  {
    //  }
    napi_value jsObj {};
    ASSERT_EQ(napi_ok, napi_create_object(env, &jsObj));

    // Call ets method
    auto ret = CallEtsFunction<napi_value>(GetPackageName(), "setPropertyToJsvalue", jsObj, TEST_VALUE);
    ASSERT_TRUE(ret.has_value());

    // Return js object:
    //  {
    //      .prop = 54.064
    //  }
    napi_value jsProp {};
    double val {};
    ASSERT_EQ(napi_ok, napi_get_named_property(env, jsObj, "prop", &jsProp));
    ASSERT_EQ(napi_ok, napi_get_value_double(env, jsProp, &val));

    // Check result
    ASSERT_EQ(val, TEST_VALUE);
}

}  // namespace ark::ets::interop::js::testing
