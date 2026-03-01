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

#include "ani_gtest.h"

// NOLINTBEGIN(modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FindNamespaceCombineScenesTest : public AniTest {};

TEST_F(FindNamespaceCombineScenesTest, find_namespace_combine_scenes_001)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("@abcModule.find_namespace_combine_scenes_test.spaceB", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "ii:i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ani_value args[2U];
    const ani_int value1 = 5;
    const ani_int value2 = 6;
    args[0].i = value1;
    args[1].i = value2;
    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, value1 + value2);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(modernize-avoid-c-arrays)