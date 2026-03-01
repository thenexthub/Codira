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

class EtsStaticFunctionException : public EtsInteropTest {};

TEST_F(EtsStaticFunctionException, CheckCustomException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckCustomException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckNameCustomException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckNameCustomException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckMessageCustomException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckMessageCustomException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckBuiltinExceptionInStatic)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckBuiltinExceptionInStatic");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckNameBuiltinExceptionInStatic)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckNameBuiltinExceptionInStatic");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckMessageBuiltinExceptionInStatic)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckMessageBuiltinExceptionInStatic");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, ThrowCustomError)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "ThrowCustomError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, CheckThrowNewError)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckThrowNewError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsStaticFunctionException, throwTSCustomExceptionWithImportType)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "throwTSCustomExceptionWithImportType");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing