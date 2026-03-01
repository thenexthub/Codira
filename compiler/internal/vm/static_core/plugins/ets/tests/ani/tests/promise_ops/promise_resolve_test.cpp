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

#include "ani/ani.h"
#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class PromiseResolveTest : public AniTest {};

TEST_F(PromiseResolveTest, ResolvePromise)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);

    std::string resolved = "resolved";
    ani_string resolution = nullptr;
    ASSERT_EQ(env_->String_NewUTF8(resolved.c_str(), resolved.size(), &resolution), ANI_OK);

    ASSERT_EQ(env_->PromiseResolver_Resolve(resolver, resolution), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("promise_resolve_test", "checkResolve", promise, resolution), ANI_TRUE);
}

TEST_F(PromiseResolveTest, invalid_argument)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);

    std::string resolved = "resolved";
    ani_string resolution = nullptr;
    ASSERT_EQ(env_->String_NewUTF8(resolved.c_str(), resolved.size(), &resolution), ANI_OK);

    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(nullptr, resolver, resolution), ANI_INVALID_ARGS);
}

TEST_F(PromiseResolveTest, invalid_argument2)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);

    std::string resolved = "resolved";
    ani_string resolution = nullptr;
    ASSERT_EQ(env_->String_NewUTF8(resolved.c_str(), resolved.size(), &resolution), ANI_OK);

    ASSERT_EQ(env_->PromiseResolver_Resolve(nullptr, resolution), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
