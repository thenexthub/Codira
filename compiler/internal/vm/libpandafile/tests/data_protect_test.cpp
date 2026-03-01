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

#include "data_protect.h"

#include <string>
#include <gtest/gtest.h>

namespace panda::panda_file::test {

HWTEST(DataProtect, PacString, testing::ext::TestSize.Level0)
{
    StringPacProtect pstr("Hello World!");
    ASSERT_STREQ(pstr.GetOriginString().c_str(), "Hello World!");
    pstr.Append('a');
    pstr.Append('a');
    pstr.Append('a');
    pstr.Append('a');
    ASSERT_STREQ(pstr.GetOriginString().c_str(), "Hello World!aaaa");
    pstr.Append(" QWERT");
    ASSERT_STREQ(pstr.GetOriginString().c_str(), "Hello World!aaaa QWERT");
    pstr.Append('1');
    ASSERT_STREQ(pstr.GetOriginString().c_str(), "Hello World!aaaa QWERT1");
    ASSERT_TRUE(pstr.CompareStringWithPacedString("Hello World!aaaa QWERT1"));
}

HWTEST(DataProtect, PacBool, testing::ext::TestSize.Level0)
{
    BoolPacProtect flag(true);
    ASSERT_TRUE(flag.GetBool());
    flag.Update(false);
    ASSERT_TRUE(!flag.GetBool());
}

}  // namespace panda::panda_file::test
