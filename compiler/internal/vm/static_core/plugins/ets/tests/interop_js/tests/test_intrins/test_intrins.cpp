/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropJsIntrinsTest : public EtsInteropTest {
public:
    void SetUp() override
    {
        EtsInteropTest::SetUp();
        // Need this load, because test use propery that we set into gtest_env in call of this method.
        LoadModuleAs("test_intrins", "index");
    }
};

TEST_F(EtsInteropJsIntrinsTest, basic1)
{
    constexpr double ARG0 = 271;
    constexpr double ARG1 = 314;
    constexpr double RES = ARG0 + ARG1;
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<double>(GetPackageName(), "jsSumWrapperNumNum", ARG0, ARG1);
    ASSERT_EQ(ret, RES);
}

TEST_F(EtsInteropJsIntrinsTest, basic2)
{
    constexpr double ARG0 = 12.34;
    constexpr std::string_view ARG1 = "foo";
    constexpr std::string_view RES = "12.34foo";
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<std::string>(GetPackageName(), "jsSumWrapperNumStr", ARG0, ARG1);
    ASSERT_EQ(ret, RES);
}

TEST_F(EtsInteropJsIntrinsTest, test_convertors)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testUndefined"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNull"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBoolean"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNumber"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testString"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testObject"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBigint"));
    // NOTE(vpukhov): symbol, function, external

    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testStringOps"));
}

TEST_F(EtsInteropJsIntrinsTest, test_builtin_array_convertors)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayAny"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayBoolean"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayInt"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayNumber"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayString"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayObject"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayMultidim"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBuiltinArrayInstanceof"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInitArrayComponent"));
}

TEST_F(EtsInteropJsIntrinsTest, test_named_access)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNamedAccess"));
}

TEST_F(EtsInteropJsIntrinsTest, test_newcall)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testNewcall"));
}

TEST_F(EtsInteropJsIntrinsTest, test_value_call)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testValueCall"));
}

TEST_F(EtsInteropJsIntrinsTest, test_call_stress)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCallStress"));
}

// Remove after JSValue cast fix
TEST_F(EtsInteropJsIntrinsTest, DISABLED_test_undefined_cast_bug)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testUndefinedCastBug"));
}

TEST_F(EtsInteropJsIntrinsTest, test_lambda_proxy)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testLambdaProxy"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testLambdaProxyRecursive"));
}

TEST_F(EtsInteropJsIntrinsTest, test_exception_forwarding)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testExceptionForwardingFromjs"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testExceptionForwardingRecursive"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCoreErrorForwarding"));
}

TEST_F(EtsInteropJsIntrinsTest, test_typechecks)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testTypecheckGetProp"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testTypecheckJscall"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testTypecheckCallets"));
}

TEST_F(EtsInteropJsIntrinsTest, test_accessor_throws)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetThrows"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetThrows"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJscallResolutionThrows1"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJscallResolutionThrows2"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJscallResolutionThrows3"));
}

}  // namespace ark::ets::interop::js::testing
