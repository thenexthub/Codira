/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class PromiseResolverResolveTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        std::string resolved = "resolved";
        ASSERT_EQ(env_->String_NewUTF8(resolved.c_str(), resolved.size(), &resolution), ANI_OK);

        ani_object promise {};
        ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes, readability-identifier-naming)
    ani_string resolution;
    ani_resolver resolver;
    // NOLINTEND(misc-non-private-member-variables-in-classes, readability-identifier-naming)
};

TEST_F(PromiseResolverResolveTest, wrong_env)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(nullptr, resolver, resolution), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"resolver", "ani_resolver"},
        {"resolution", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);
}

TEST_F(PromiseResolverResolveTest, wrong_resolution)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, resolver, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"resolution", "ani_ref", "wrong reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);
}

TEST_F(PromiseResolverResolveTest, wrong_resolver)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, nullptr, resolution), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"resolution", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);
}

TEST_F(PromiseResolverResolveTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"resolution", "ani_ref", "wrong reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);
}

TEST_F(PromiseResolverResolveTest, throw_error)
{
    ThrowError();

    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, resolver, resolution), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"resolver", "ani_resolver"},
        {"resolution", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(PromiseResolverResolveTest, bad_resolver)
{
    const auto fakeRef = reinterpret_cast<ani_resolver>(static_cast<long>(0x0ff0f1f));  // NOLINT(google-runtime-int)
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, fakeRef, resolution), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"resolution", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);
}

TEST_F(PromiseResolverResolveTest, success)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, resolver, resolution), ANI_OK);
}

TEST_F(PromiseResolverResolveTest, double_rejection)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, resolver, resolution), ANI_OK);

    ASSERT_EQ(env_->c_api->PromiseResolver_Resolve(env_, resolver, resolution), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"resolution", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Resolve", testLines);
}

}  // namespace ark::ets::ani::verify::testing
