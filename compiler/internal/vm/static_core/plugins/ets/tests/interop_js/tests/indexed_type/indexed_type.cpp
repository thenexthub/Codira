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

class EtsIndexedType : public EtsInteropTest {};

TEST_F(EtsIndexedType, getArrValueByIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeArrValueByIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "changeArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkLengthArr)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkLengthArr");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkAllArrValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkAllArrValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getCustomArrValueByIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getCustomArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeCustomArrValueByIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "changeCustomArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkLengthCustomArr)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkLengthCustomArr");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkAllCustomArrValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkAllCustomArrValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getRecordValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getRecordValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeRecordValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "changeRecordValue");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_getRecordValueIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getRecordValueIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_changeRecordValueIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "changrRecordValueIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_checkRecordValueLength)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkRecordValueLength");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_checkRecordValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkRecordValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getTypedArrayValueByIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getTypedArrayValueByIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18131
TEST_F(EtsIndexedType, DISABLED_changeTypedArrayValueByIndex)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "changeTypedArrayValueByIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18131
TEST_F(EtsIndexedType, DISABLED_checkLengthTypedArray)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkLengthTypedArray");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18131
TEST_F(EtsIndexedType, DISABLED_checkAllTypedArrayValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "checkAllTypedArrayValue");
    ASSERT_EQ(ret, true);
}
// NOTE enable after fixing #22422
TEST_F(EtsIndexedType, DISABLED_getValueFromProxyRecord)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getValueFromProxyRecord");
    ASSERT_EQ(ret, true);
}
// NOTE enable after fixing #22422
TEST_F(EtsIndexedType, DISABLED_changeValueFromProxyRecord)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "changeValueFromProxyRecord");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18238
TEST_F(EtsIndexedType, DISABLED_getValueFromProxyByKeyRecord)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "getValueFromProxyByKeyRecord");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
