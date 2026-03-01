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

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ani_status ANI_CLIB_FindClass(ani_env *env, const char *classDescriptor, ani_class *result);

namespace ark::ets::ani::testing {

class FindClassTest : public AniTest {};

TEST_F(FindClassTest, has_class)
{
    ani_class cls {};
    // NOLINTNEXTLINE(readability-identifier-naming)
    auto status = ANI_CLIB_FindClass(env_, "ani_ctest.Point", &cls);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(cls, nullptr);
}

}  // namespace ark::ets::ani::testing
