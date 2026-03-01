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

class EtsInteropClassOperationsTest : public EtsInteropTest {
public:
    void SetUp() override
    {
        EtsInteropTest::SetUp();
        LoadModuleAs("module", "module");
        if (std::getenv("PACKAGE_NAME") != nullptr) {
            packageName_ = std::string(std::getenv("PACKAGE_NAME"));
        } else {
            std::cerr << "PACKAGE_NAME is not set" << std::endl;
            std::abort();
        }
    }
};

TEST_F(EtsInteropClassOperationsTest, TestJSCallEmpty)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallEmpty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSONStringifyESObjectInstance)
{
    auto success = CallEtsFunction<bool>(GetPackageName(), "testJSONStringifyESObjectInstance");
    ASSERT_TRUE(success.value());
}

TEST_F(EtsInteropClassOperationsTest, DISABLED_TestJSONStringifyESObjectArray)
{
    auto success = CallEtsFunction<bool>(GetPackageName(), "testJSONStringifyESObjectArray");
    ASSERT_TRUE(success.value());
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewEmpty)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewEmpty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticEmpty)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallStaticMethodEmpty");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallObject)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallObject");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewObject)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewObject");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewObjectSetProperty)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewSetPropertyObject");
    ASSERT_EQ(ret, 0);
}

// returns nan
TEST_F(EtsInteropClassOperationsTest, TestJSCallMethodObject)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallMethodObject");
    ASSERT_EQ(ret, 0);
}

// simplification of previous failure
TEST_F(EtsInteropClassOperationsTest, TestJSCallMethodObjectSimple)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallMethodSimple");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, DISABLED_TestJSCallString)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewString)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestjsnewSetPropertyString)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewSetPropertyString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticString)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallStaticMethodString");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallArray)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSNewArray)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestjsnewSetPropertyArray)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jsnewSetPropertyArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestJSCallStaticArray)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "jscallStaticMethodArray");
    ASSERT_EQ(ret, 0);
}

TEST_F(EtsInteropClassOperationsTest, TestNamespace)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "testNamespace");
    ASSERT_EQ(ret, 0);
}

}  // namespace ark::ets::interop::js::testing
