/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CreateLocalScopeTest : public AniTest {};

// Define constants to replace magic numbers
constexpr ani_size SPECIFIED_CAPACITY = 60;
constexpr ani_size MAX_CAPACITY = 32000000;
constexpr ani_size MIN_CAPACITY = 1;
constexpr ani_size REF_NUM = 10;
constexpr ani_size LOOP_COUNT = 3;
constexpr std::string_view TEST_STRING = "test";

TEST_F(CreateLocalScopeTest, create_local_scope_test)
{
    // Passing 0 as capacity should return ANI_INVALID_ARGS
    ASSERT_EQ(env_->CreateLocalScope(0), ANI_INVALID_ARGS);

    // Passing SPECIFIED_CAPACITY as capacity should succeed and return ANI_OK
    ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string string = nullptr;

    // Create SPECIFIED_CAPACITY strings in the newly created local scope
    for (ani_size i = 1; i <= SPECIFIED_CAPACITY; ++i) {
        // Construct a unique stringName for each iteration
        std::string stringName = "String_NewUTF8_" + std::to_string(i) + ";";

        // Attempt to create a new UTF8 string and check the result
        ASSERT_EQ(env_->String_NewUTF8(stringName.c_str(), stringName.size(), &string), ANI_OK);
        ASSERT_NE(string, nullptr);
    }

    // Create another string to confirm that string creation still works
    ani_status res = env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &string);
    ASSERT_EQ(res, ANI_OK);

    // Destroy the local scope after string creation
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    /*
     * Attempt to create a local scope with very large capacity (MAX_CAPACITY).
     * The comment below indicates that the free size of local reference storage is
     * smaller than the requested capacity, thus should return ANI_OUT_OF_MEMORY.
     */
    // Free size of local reference storage is less than capacity: MAX_CAPACITY
    // blocks_count_: 3 need_blocks: 533334 blocks_free: 524285
    ASSERT_EQ(env_->CreateLocalScope(MAX_CAPACITY), ANI_OUT_OF_MEMORY);
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope)
{
    // Passing 0 as capacity should return ANI_INVALID_ARGS
    ASSERT_EQ(env_->CreateEscapeLocalScope(0), ANI_INVALID_ARGS);

    // Passing SPECIFIED_CAPACITY as capacity should succeed
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string string = nullptr;
    // Create multiple strings to test scope behavior
    for (ani_size i = 1; i <= SPECIFIED_CAPACITY; ++i) {
        std::string stringName = "String_NewUTF8_" + std::to_string(i) + ";";

        ASSERT_EQ(env_->String_NewUTF8(stringName.c_str(), stringName.size(), &string), ANI_OK);
        ASSERT_NE(string, nullptr);
    }

    // Create another string to confirm it still works
    ani_status res = env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &string);
    ASSERT_EQ(res, ANI_OK);

    ani_ref result;
    // Destroy the escape local scope and retrieve the final reference
    ASSERT_EQ(env_->DestroyEscapeLocalScope(string, &result), ANI_OK);

    /*
     * Trying to create an escape local scope with a large capacity
     * should fail due to insufficient memory (ANI_OUT_OF_MEMORY).
     */
    // Free size of local reference storage is less than capacity: MAX_CAPACITY
    // blocks_count_: 3 need_blocks: 533334 blocks_free: 524285
    ASSERT_EQ(env_->CreateEscapeLocalScope(MAX_CAPACITY), ANI_OUT_OF_MEMORY);
}

TEST_F(CreateLocalScopeTest, destroy_local_scope)
{
    // Create a local scope with capacity of MIN_CAPACITY
    ASSERT_EQ(env_->CreateLocalScope(MIN_CAPACITY), ANI_OK);

    ani_string string = nullptr;
    // Attempt to create a new string
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &string), ANI_OK);

    // Destroy the local scope after string creation
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope)
{
    // Passing SPECIFIED_CAPACITY as capacity should succeed
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string string = nullptr;
    // Create a string to test with
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &string), ANI_OK);

    ani_ref result;
    // Destroy the escape local scope and retrieve the final reference
    ASSERT_EQ(env_->DestroyEscapeLocalScope(string, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_null)
{
    // test for null
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_ref result;
    ASSERT_EQ(env_->DestroyEscapeLocalScope(nullRef, &result), ANI_OK);
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(result, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_undefinde)
{
    // test for undefined
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ani_ref undefined;
    ASSERT_EQ(env_->GetUndefined(&undefined), ANI_OK);
    ani_ref result;
    ASSERT_EQ(env_->DestroyEscapeLocalScope(undefined, &result), ANI_OK);
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(result, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, testHugeNrRefs)
{
    ASSERT_EQ(env_->CreateLocalScope(ani_size(std::numeric_limits<uint32_t>::max()) - 1), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->CreateLocalScope(ani_size(std::numeric_limits<uint32_t>::max()) - 0), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->CreateLocalScope(ani_size(std::numeric_limits<ani_size>::max()) - 1), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->CreateLocalScope(ani_size(std::numeric_limits<ani_size>::max()) - 0), ANI_OUT_OF_MEMORY);

    ASSERT_EQ(env_->CreateEscapeLocalScope(ani_size(std::numeric_limits<uint32_t>::max()) - 1), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->CreateEscapeLocalScope(ani_size(std::numeric_limits<uint32_t>::max()) - 0), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->CreateEscapeLocalScope(ani_size(std::numeric_limits<ani_size>::max()) - 1), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->CreateEscapeLocalScope(ani_size(std::numeric_limits<ani_size>::max()) - 0), ANI_OUT_OF_MEMORY);
}

TEST_F(CreateLocalScopeTest, create_local_scope_test1)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
        ani_string objectRef {};
        ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, create_local_scope_test2)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateLocalScope(MIN_CAPACITY), ANI_OK);
        for (ani_size j = 1; j <= SPECIFIED_CAPACITY; j++) {
            ani_string objectRef {};
            ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
        }
    }
}

TEST_F(CreateLocalScopeTest, create_local_scope_test3)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
        ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, create_local_scope_test4)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
        ani_string objectRef {};
        ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
        ASSERT_EQ(env_->Reference_Delete(objectRef), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, create_local_scope_test5)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, destroy_local_scope_test2)
{
    ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(CreateLocalScopeTest, destroy_local_scope_test3)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);
        ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope_test1)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope_test2)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(MIN_CAPACITY), ANI_OK);

    for (ani_size i = 1; i <= SPECIFIED_CAPACITY; i++) {
        ani_string objectRef {};
        ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope_test3)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(objectRef, &result), ANI_OK);
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope_test4)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(objectRef), ANI_OK);
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope_test5)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_test1)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(objectRef, &result), ANI_OK);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_test2)
{
    for (ani_size i = 1; i <= LOOP_COUNT; i++) {
        ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
        ani_string objectRef {};
        ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);

        ani_ref result {};
        ASSERT_EQ(env_->DestroyEscapeLocalScope(objectRef, &result), ANI_OK);
    }
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_test3)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string objectRefA {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRefA), ANI_OK);

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("create_local_scope.Operations", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(cls, &result), ANI_OK);

    ani_static_method method {};
    auto classA = reinterpret_cast<ani_class>(result);
    ASSERT_EQ(env_->Class_FindStaticMethod(classA, "or", "zz:z", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(classA, method, &res, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, scope_test1)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);
    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(objectRef, &result), ANI_OK);

    ani_size size = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(static_cast<ani_string>(result), &size), ANI_OK);
    ASSERT_EQ(size, TEST_STRING.size());
}

static void CheckReferenceEquality(ani_env *env, ani_ref lhs, ani_ref rhs)
{
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env->Reference_Equals(lhs, rhs, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    isEquals = ANI_FALSE;
    ASSERT_EQ(env->Reference_StrictEquals(lhs, rhs, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, scope_test2)
{
    ani_string stringRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &stringRef), ANI_OK);

    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);

    auto identityRef = CallEtsFunction<ani_ref>("create_local_scope", "identity", stringRef);
    CheckReferenceEquality(env_, stringRef, identityRef);

    ani_ref escapedRef {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(identityRef, &escapedRef), ANI_OK);

    CheckReferenceEquality(env_, stringRef, escapedRef);
}

TEST_F(CreateLocalScopeTest, scope_test4)
{
    ASSERT_EQ(env_->CreateLocalScope(REF_NUM), ANI_OK);

    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);

    ani_string objectRefA {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRefA), ANI_OK);

    ani_string objectRefB {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRefB), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(objectRefA, &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {};
    ani_size actualSize = 0U;
    ani_status status = env_->String_GetUTF8SubString(reinterpret_cast<ani_string>(result), 0U, TEST_STRING.size(),
                                                      utfBuffer, bufferSize, &actualSize);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(actualSize, TEST_STRING.size());
    ASSERT_STREQ(utfBuffer, TEST_STRING.data());

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(CreateLocalScopeTest, scope_test5)
{
    ASSERT_EQ(env_->CreateLocalScope(REF_NUM), ANI_OK);
    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("create_local_scope.Student", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &ctor), ANI_OK);
    ASSERT_NE(ctor, nullptr);

    ani_object objectA {};
    const ani_int age = 12;
    ASSERT_EQ(env_->Object_New(cls, ctor, &objectA, age), ANI_OK);
    ASSERT_NE(objectA, nullptr);

    ani_string str {};
    std::string name = "Tom";
    ASSERT_EQ(env_->String_NewUTF8(name.data(), name.size(), &str), ANI_OK);
    ASSERT_NE(str, nullptr);
    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(objectA, "name", str), ANI_OK);

    ani_ref nameValue {};
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(objectA, "getName", nullptr, &nameValue), ANI_OK);
    ASSERT_NE(nameValue, nullptr);

    ani_ref escapedValue {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(nameValue, &escapedValue), ANI_OK);
    ASSERT_NE(escapedValue, nullptr);

    std::string resString {};
    GetStdString(env_, reinterpret_cast<ani_string>(escapedValue), resString);
    ASSERT_STREQ(resString.data(), name.c_str());

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_test4)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);

    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(nullRef, &result), ANI_OK);

    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(result, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_test5)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);

    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(undefinedRef, &result), ANI_OK);

    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(result, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, create_local_scope_invalid_env)
{
    ASSERT_EQ(env_->c_api->CreateLocalScope(nullptr, SPECIFIED_CAPACITY), ANI_INVALID_ARGS);
}

TEST_F(CreateLocalScopeTest, destroy_local_scope_invalid_env)
{
    ASSERT_EQ(env_->c_api->DestroyLocalScope(nullptr), ANI_INVALID_ARGS);
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope_invalid_env)
{
    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(nullptr, SPECIFIED_CAPACITY), ANI_INVALID_ARGS);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_invalid_env)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);
    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
    ani_ref result {};
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(nullptr, objectRef, &result), ANI_INVALID_ARGS);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_invalid_result)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(REF_NUM), ANI_OK);
    ani_string objectRef {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &objectRef), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(objectRef, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
