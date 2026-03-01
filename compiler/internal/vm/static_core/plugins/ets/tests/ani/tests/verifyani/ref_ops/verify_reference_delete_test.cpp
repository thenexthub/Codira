/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

// NOLINTBEGIN(hicpp-signed-bitwise, google-runtime-int)
namespace ark::ets::ani::verify::testing {

class ReferenceDeleteTest : public VerifyAniTest {};

TEST_F(ReferenceDeleteTest, wrong_env)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ASSERT_EQ(env_->c_api->Reference_Delete(nullptr, ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"lref", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);

    ASSERT_EQ(env_->c_api->Reference_Delete(env_, ref), ANI_OK);
}

TEST_F(ReferenceDeleteTest, wrong_lref_null)
{
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);
}

TEST_F(ReferenceDeleteTest, wrong_lref_from_other_scope)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ASSERT_EQ(env_->c_api->Reference_Delete(env_, ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "a local reference can only be deleted in the scope where it was created"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, ref), ANI_OK);
}

TEST_F(ReferenceDeleteTest, wrong_ref_global)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ani_ref gref {};
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ASSERT_EQ(env_->c_api->Reference_Delete(env_, gref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);

    ASSERT_EQ(env_->GlobalReference_Delete(gref), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(ReferenceDeleteTest, wrong_ref_stack)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    struct Native {
        static ani_long Fn(ani_env *env, ani_ref o)
        {
            // Try to delete local ref
            ani_status status = env->Reference_Delete(o);
            return static_cast<ani_long>(status);
        }
    };
    std::array functions {
        ani_native_function {"foo", nullptr, reinterpret_cast<void *>(&Native::Fn)},
    };

    // Bind native method.
    ani_module m {};
    ASSERT_EQ(env_->FindModule("verify_reference_delete_test", &m), ANI_OK);
    ASSERT_EQ(env_->Module_BindNativeFunctions(m, functions.data(), functions.size()), ANI_OK);

    // Call native method.
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(m, "foo", "C{std.core.Object}:l", &fn), ANI_OK);
    ani_long ret {};
    // NOTE(ypigunova): remove cast when #26993 will be solved
    ani_ref realRef = reinterpret_cast<VRef *>(ref)->GetRef();

    ASSERT_EQ(env_->Function_Call_Long(fn, &ret, realRef), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(ret, static_cast<long>(ANI_ERROR));

    // Check the error that occurs when deleting a stack reference.
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "a local reference can only be deleted in the scope where it was created"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);

    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(ReferenceDeleteTest, wrong_lref_junk_like_stack_ref)
{
    const long lRefX = 0xfff0;
    auto junkStackRef = reinterpret_cast<ani_ref>(lRefX | long(ark::mem::Reference::Reference::ObjectType::STACK));
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, junkStackRef), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);
}

TEST_F(ReferenceDeleteTest, wrong_lref_junk_like_local_ref)
{
    const long lRefX = 0xfff0;
    auto junkLocalRef = reinterpret_cast<ani_ref>(lRefX | long(ark::mem::Reference::Reference::ObjectType::LOCAL));
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, junkLocalRef), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);
}

TEST_F(ReferenceDeleteTest, wrong_lref_junk_like_global_ref)
{
    const long lRefX = 0xfff0;
    auto junkGlobalRef = reinterpret_cast<ani_ref>(lRefX | long(ark::mem::Reference::Reference::ObjectType::GLOBAL));
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, junkGlobalRef), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);
}

TEST_F(ReferenceDeleteTest, wrong_lref_junk_like_weak_ref)
{
    const long lRefX = 0xfff0;
    auto junkWeakRef = reinterpret_cast<ani_ref>(lRefX | long(ark::mem::Reference::Reference::ObjectType::WEAK));
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, junkWeakRef), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);
}

TEST_F(ReferenceDeleteTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Reference_Delete(nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"lref", "ani_ref", "it is not local reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_Delete", testLines);
}

TEST_F(ReferenceDeleteTest, success)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, ref), ANI_OK);
}

TEST_F(ReferenceDeleteTest, success_with_pending_error)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ThrowError();

    ASSERT_EQ(env_->c_api->Reference_Delete(env_, ref), ANI_OK);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
// NOLINTEND(hicpp-signed-bitwise, google-runtime-int)
