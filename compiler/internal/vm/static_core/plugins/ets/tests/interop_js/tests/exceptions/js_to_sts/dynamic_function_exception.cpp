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

class EtsDynumicFunctionException : public EtsInteropTest {};

TEST_F(EtsDynumicFunctionException, CheckUserExceptionError)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckUserExceptionError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckNameUserExceptionError)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckNameUserExceptionError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckMessageUserExceptionError)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckMessageUserExceptionError");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CatchUserException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CatchUserException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CatchBuiltinException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CatchBuiltinException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckNameBuiltinException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckNameBuiltinException");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsDynumicFunctionException, CheckMessageBuiltinException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "CheckMessageBuiltinException");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing