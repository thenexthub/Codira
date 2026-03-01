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
class PromiseResolverRejectTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        std::string rejected = "rejected";
        ASSERT_EQ(env_->String_NewUTF8(rejected.c_str(), rejected.size(), &errStr), ANI_OK);
        ani_ref undef {};
        ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("escompat.Error", &cls), ANI_OK);
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &ctor), ANI_OK);

        ani_object errObj {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(cls, ctor, &errObj, errStr, undef), ANI_OK);
        err = static_cast<ani_error>(errObj);

        ani_object promise {};
        ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes, readability-identifier-naming)
    ani_error err;
    ani_string errStr;
    ani_resolver resolver;
    // NOLINTEND(misc-non-private-member-variables-in-classes, readability-identifier-naming)
};

TEST_F(PromiseResolverRejectTest, wrong_env)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(nullptr, resolver, err), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, wrong_rejection)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error", "wrong reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, wrong_resolver)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, nullptr, err), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"rejection", "ani_error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"rejection", "ani_error", "wrong reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, throw_error)
{
    ThrowError();

    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, err), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(PromiseResolverRejectTest, bad_resolver)
{
    const auto fakeRef = reinterpret_cast<ani_resolver>(static_cast<long>(0x0ff0f1f));  // NOLINT(google-runtime-int)
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, fakeRef, err), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"rejection", "ani_error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, success)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, err), ANI_OK);
}

TEST_F(PromiseResolverRejectTest, string_rejection)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, reinterpret_cast<ani_error>(errStr)), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error", "wrong reference type: ani_string"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, undef_rejection)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, reinterpret_cast<ani_error>(undef)), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error", "wrong reference type: undefined"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, null_rejection)
{
    ani_ref null {};
    ASSERT_EQ(env_->GetNull(&null), ANI_OK);
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, reinterpret_cast<ani_error>(null)), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error", "wrong reference type: null"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, cls_rejection)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_promise_resolver_reject_test.A", &cls), ANI_OK);
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, reinterpret_cast<ani_error>(cls)), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error", "wrong reference type: ani_class"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, bad_rejection)
{
    const auto fakeRef = reinterpret_cast<ani_error>(static_cast<long>(0x0ff0f1f));  // NOLINT(google-runtime-int)
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, fakeRef), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver"},
        {"rejection", "ani_error", "wrong reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

TEST_F(PromiseResolverRejectTest, double_rejection)
{
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, err), ANI_OK);

    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(env_, resolver, err), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"resolver", "ani_resolver", "wrong resolver"},
        {"rejection", "ani_error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("PromiseResolver_Reject", testLines);
}

}  // namespace ark::ets::ani::verify::testing
