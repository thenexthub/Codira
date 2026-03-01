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
#include <string>
#include "operand_types_print.h"

namespace ark::test {

TEST(typetests, test1)
{
    std::string_view descriptor1;
    EXPECT_DEATH({ ark::pandasm::Type type1 = ark::pandasm::Type::FromDescriptor(descriptor1); }, ".*");

    std::string_view descriptor2 = "X";
    EXPECT_DEATH({ ark::pandasm::Type::FromDescriptor(descriptor2); }, ".*");

    std::string_view descriptor3 = "Z";
    std::string descriptor3Expect = "u1";
    ark::pandasm::Type type3 = ark::pandasm::Type::FromDescriptor(descriptor3);
    ASSERT_EQ(type3.GetComponentName(), descriptor3Expect);

    std::string_view descriptor4 = "Lobj.Obj;";
    std::string descriptor4Expect = "obj.Obj";
    ark::pandasm::Type type4 = ark::pandasm::Type::FromDescriptor(descriptor4);
    ASSERT_EQ(type4.GetComponentName(), descriptor4Expect);

    std::string_view descriptor5 = "[Lobj.Obj;";
    std::string descriptor5Expect = "obj.Obj";
    ark::pandasm::Type type5 = ark::pandasm::Type::FromDescriptor(descriptor5);
    ASSERT_EQ(type5.GetComponentName(), descriptor5Expect);
    ASSERT_EQ(type5.GetRank(), 1);

    std::string_view descriptor6 = "[[[Lobj.Obj;";
    std::string descriptor6Expect = "obj.Obj";
    ark::pandasm::Type type6 = ark::pandasm::Type::FromDescriptor(descriptor6);
    ASSERT_EQ(type6.GetComponentName(), descriptor6Expect);
    ASSERT_EQ(type6.GetRank(), 3);
}

}  // namespace ark::test
