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

namespace ark::ets::ani::testing {

class PromiseNewTest : public AniTest {};

TEST_F(PromiseNewTest, ResolvePromise)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);
    ASSERT_NE(promise, nullptr);
}

TEST_F(PromiseNewTest, invalid_argument)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->c_api->Promise_New(nullptr, &resolver, &promise), ANI_INVALID_ARGS);
}

TEST_F(PromiseNewTest, invalid_argument2)
{
    ani_object promise {};

    ASSERT_EQ(env_->Promise_New(nullptr, &promise), ANI_INVALID_ARGS);
}

TEST_F(PromiseNewTest, invalid_argument3)
{
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, nullptr), ANI_INVALID_ARGS);
}

TEST_F(PromiseNewTest, continuous_calls)
{
    ani_object promise {};
    ani_resolver resolver {};
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);
        ASSERT_NE(promise, nullptr);
    }
}
}  // namespace ark::ets::ani::testing