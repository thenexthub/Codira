/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

class ESCompatErrorTest : public EtsInteropTest {};
TEST_F(ESCompatErrorTest, compat_error)
{
    // NOTE (vpukhov): compat accessors
    ASSERT_EQ(true, RunJsTestSuite("compat_error.js"));
}

TEST_F(ESCompatErrorTest, compat_error_with_cause)
{
    // NOTE (oignatenko) uncomment code in Error_TestJSSampleWithCause after interop will be supported in this direction
    ASSERT_EQ(true, RunJsTestSuite("error_js_suites/test_cause.js"));
}

TEST_F(ESCompatErrorTest, compat_error_to_string)
{
    // NOTE (oignatenko) uncomment code in Error_TestJSSampleWithCause after interop will be supported in this direction
    ASSERT_EQ(true, RunJsTestSuite("error_js_suites/test_to_string.js"));
}

}  // namespace ark::ets::interop::js::testing
