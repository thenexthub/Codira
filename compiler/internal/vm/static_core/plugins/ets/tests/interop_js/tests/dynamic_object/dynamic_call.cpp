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

class DynamicFunctionalCall : public EtsInteropTest {
public:
    const int jsReturnValue_ = 123;
};

TEST_F(DynamicFunctionalCall, RegularFunc)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "regularFunc");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, DynamicField)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "dynamicField");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, MethodCall)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "methodCall");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, MultipleCall)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "multipleCall");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, Lambda)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "lambda");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, SimpleArray)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "simpleArr");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, MultiDimArray)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "multidimArray");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

TEST_F(DynamicFunctionalCall, LambdaArray)
{
    auto ret = CallEtsFunction<int>(GetPackageName(), "lambdaArrCall");
    ASSERT_EQ(ret.value(), jsReturnValue_);
}

}  // namespace ark::ets::interop::js::testing
