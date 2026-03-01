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

class PromiseRejectTest : public AniTest {};

TEST_F(PromiseRejectTest, ResolvePromise)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);

    ani_class errorClass {};
    ASSERT_EQ(env_->FindClass("escompat.Error", &errorClass), ANI_OK);

    ani_method constructor {};
    ASSERT_EQ(env_->Class_FindMethod(errorClass, "<ctor>", "C{std.core.String}:", &constructor), ANI_OK);

    std::string rejected = "rejected";
    ani_string rejection {};
    ASSERT_EQ(env_->String_NewUTF8(rejected.c_str(), rejected.size(), &rejection), ANI_OK);

    ani_object errorObject {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(errorClass, constructor, &errorObject, rejection), ANI_OK);

    auto err = static_cast<ani_error>(errorObject);

    ASSERT_EQ(env_->PromiseResolver_Reject(resolver, err), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("promise_reject_test", "checkReject", promise, err), ANI_TRUE);
}

TEST_F(PromiseRejectTest, InvalidArgument1)
{
    ani_resolver resolver {};
    ani_error error {};
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(nullptr, resolver, error), ANI_INVALID_ARGS);
}

TEST_F(PromiseRejectTest, InvalidArgument2)
{
    ani_error error {};
    ASSERT_EQ(env_->PromiseResolver_Reject(nullptr, error), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
