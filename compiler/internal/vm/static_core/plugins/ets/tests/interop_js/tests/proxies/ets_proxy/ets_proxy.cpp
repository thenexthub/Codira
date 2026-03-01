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

class JSExtendEtsTest : public EtsInteropTest {};

TEST_F(JSExtendEtsTest, access_primitives)
{
    ASSERT_EQ(true, RunJsTestSuite("check_access_primitives.js"));
}

TEST_F(JSExtendEtsTest, access_references)
{
    ASSERT_EQ(true, RunJsTestSuite("check_access_references.js"));
}

TEST_F(JSExtendEtsTest, proxy_objects)
{
    ASSERT_EQ(true, RunJsTestSuite("check_proxy_objects.js"));
}

TEST_F(JSExtendEtsTest, inheritance)
{
    ASSERT_EQ(true, RunJsTestSuite("check_inheritance.js"));
}

}  // namespace ark::ets::interop::js::testing
