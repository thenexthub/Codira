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

class EtsInteropInstanceOf : public EtsInteropTest {};

/*
 * =============================================
 * === Tests <object type>_instanceof_object ===
 * =============================================
 */
TEST_F(EtsInteropInstanceOf, Test_object_instanceof_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestObjectInstanceofObject"));
}

TEST_F(EtsInteropInstanceOf, Test_etstype_instanceof_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestEtstypeInstanceofObject"));
}

TEST_F(EtsInteropInstanceOf, Test_jsvalue_instanceof_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestJsvalueInstanceofObject"));
}

TEST_F(EtsInteropInstanceOf, Test_dynvalue_instanceof_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestDynvalueInstanceofObject"));
}

TEST_F(EtsInteropInstanceOf, Test_dyndecl_instanceof_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestDyndeclInstanceofObject"));
}

/*
 * ==============================================
 * === Tests <object type>_instanceof_etstype ===
 * ==============================================
 */
TEST_F(EtsInteropInstanceOf, Test_object_instanceof_etstype)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestObjectInstanceofEtstype"));
}

TEST_F(EtsInteropInstanceOf, Test_etstype_instanceof_etstype)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestEtstypeInstanceofEtstype"));
}

TEST_F(EtsInteropInstanceOf, Test_jsvalue_instanceof_etstype)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestJsvalueInstanceofEtstype"));
}

TEST_F(EtsInteropInstanceOf, Test_dynvalue_instanceof_etstype)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestDynvalueInstanceofEtstype"));
}

// When the SPEC determines whether 1.1 can inherit 1.2, then decides whether to enable.
TEST_F(EtsInteropInstanceOf, DISABLED_Test_dyndecl_instanceof_etstype)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestDyndeclInstanceofEtstype"));
}

/*
 * ==============================================
 * === Tests <object type>_instanceof_dyndecl ===
 * ==============================================
 */
TEST_F(EtsInteropInstanceOf, Test_object_instanceof_dyndecl)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestObjectInstanceofDyndecl"));
}

TEST_F(EtsInteropInstanceOf, Test_etstype_instanceof_dyndecl)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestEtstypeInstanceofDyndecl"));
}

TEST_F(EtsInteropInstanceOf, Test_jsvalue_instanceof_dyndecl)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestJsvalueInstanceofDyndecl"));
}

TEST_F(EtsInteropInstanceOf, Test_dynvalue_instanceof_dyndecl)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestDynvalueInstanceofDyndecl"));
}

TEST_F(EtsInteropInstanceOf, Test_dyndecl_instanceof_dyndecl)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "TestDyndeclInstanceofDyndecl"));
}

}  // namespace ark::ets::interop::js::testing
