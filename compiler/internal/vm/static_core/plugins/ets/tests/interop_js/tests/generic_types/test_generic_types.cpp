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

class GenericTypesTest : public EtsInteropTest {};

TEST_F(GenericTypesTest, check_compat_objects)
{
    ASSERT_TRUE(RunJsTestSuite("check_compat_objects.js"));
}

TEST_F(GenericTypesTest, check_js_objects)
{
    ASSERT_TRUE(RunJsTestSuite("check_js_objects.js"));
}

TEST_F(GenericTypesTest, check_ets_objects)
{
    ASSERT_TRUE(RunJsTestSuite("check_ets_objects.js"));
}

TEST_F(GenericTypesTest, check_js_primitives)
{
    ASSERT_TRUE(RunJsTestSuite("check_js_primitives.js"));
}

}  // namespace ark::ets::interop::js::testing
