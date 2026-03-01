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

class PromiseNewTest : public VerifyAniTest {};

TEST_F(PromiseNewTest, wrong_env)
{
    ani_object promise {};
    ani_resolver resolver {};
    ASSERT_EQ(env_->c_api->Promise_New(nullptr, &resolver, &promise), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result_resolver", "ani_resolver *"},
        {"result_promise", "ani_object *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Promise_New", testLines);
}

TEST_F(PromiseNewTest, wrong_promise)
{
    ani_resolver resolver {};
    ASSERT_EQ(env_->c_api->Promise_New(env_, &resolver, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"result_resolver", "ani_resolver *"},
        {"result_promise", "ani_object *", "wrong pointer for storing 'ani_object'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Promise_New", testLines);
}

TEST_F(PromiseNewTest, wrong_resolver)
{
    ani_object promise {};
    ASSERT_EQ(env_->c_api->Promise_New(env_, nullptr, &promise), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"result_resolver", "ani_resolver *", "wrong pointer for storing 'ani_resolver'"},
        {"result_promise", "ani_object *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Promise_New", testLines);
}

TEST_F(PromiseNewTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Promise_New(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result_resolver", "ani_resolver *", "wrong pointer for storing 'ani_resolver'"},
        {"result_promise", "ani_object *", "wrong pointer for storing 'ani_object'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Promise_New", testLines);
}

TEST_F(PromiseNewTest, throw_error)
{
    ThrowError();

    ani_object promise {};
    ani_resolver resolver {};
    ASSERT_EQ(env_->c_api->Promise_New(env_, &resolver, &promise), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"result_resolver", "ani_resolver *"},
        {"result_promise", "ani_object *"},
    };
    ASSERT_EQ(env_->ResetError(), ANI_OK);

    ASSERT_ERROR_ANI_ARGS_MSG("Promise_New", testLines);
}

TEST_F(PromiseNewTest, success)
{
    ani_object promise {};
    ani_resolver resolver {};
    ASSERT_EQ(env_->c_api->Promise_New(env_, &resolver, &promise), ANI_OK);
    ASSERT_NE(promise, nullptr);
    ASSERT_NE(resolver, nullptr);
}

}  // namespace ark::ets::ani::verify::testing
