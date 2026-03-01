/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

namespace ark::ets::ani::testing {

class EnsureEnoughReferencesTest : public AniTest {};

// Define constants to replace magic numbers
constexpr ani_size SPECIFIED_CAPACITY = 60;
constexpr ani_size MAX_CAPACITY = 32000000;
constexpr ani_size MIN_CAPACITY = 1;
constexpr ani_size CAPACITY = 10;

TEST_F(EnsureEnoughReferencesTest, ensure_enough_references_test)
{
    // Ensures SPECIFIED_CAPACITY local references are available
    ASSERT_EQ(env_->EnsureEnoughReferences(SPECIFIED_CAPACITY), ANI_OK);
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

    // Destroy the local scope after string creation
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    // Ensures MIN_CAPACITY local references are available
    ASSERT_EQ(env_->EnsureEnoughReferences(MIN_CAPACITY), ANI_OK);
}

TEST_F(EnsureEnoughReferencesTest, ensure_enough_references_invalid_args_test)
{
    // Passing 0 as capacity should return ANI_INVALID_ARGS
    ASSERT_EQ(env_->EnsureEnoughReferences(0), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->EnsureEnoughReferences(MAX_CAPACITY), ANI_OUT_OF_MEMORY);
    /*
     * Attempt to create a local scope with very large capacity (MAX_CAPACITY).
     * The comment below indicates that the free size of local reference storage is
     * smaller than the requested capacity, thus should return ANI_OUT_OF_MEMORY.
     */
    // Free size of local reference storage is less than capacity: MAX_CAPACITY
    // blocks_count_: 1 need_blocks: 533334 blocks_free: 524287
    ASSERT_EQ(env_->CreateLocalScope(MAX_CAPACITY), ANI_OUT_OF_MEMORY);
}

TEST_F(EnsureEnoughReferencesTest, ensure_references_test)
{
    ASSERT_EQ(env_->EnsureEnoughReferences(CAPACITY), ANI_OK);

    for (ani_size i = 0; i <= CAPACITY; i++) {
        std::string stringName = "String_NewUTF8_" + std::to_string(i) + ";";

        ani_ref objectRef {};
        ASSERT_EQ(
            env_->String_NewUTF8(stringName.c_str(), stringName.size(), reinterpret_cast<ani_string *>(&objectRef)),
            ANI_OK);
    }
}

TEST_F(EnsureEnoughReferencesTest, testHugeNrRefs)
{
    ASSERT_EQ(env_->EnsureEnoughReferences(ani_size(std::numeric_limits<uint32_t>::max()) - 1), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->EnsureEnoughReferences(ani_size(std::numeric_limits<uint32_t>::max()) - 0), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->EnsureEnoughReferences(ani_size(std::numeric_limits<ani_size>::max()) - 1), ANI_OUT_OF_MEMORY);
    ASSERT_EQ(env_->EnsureEnoughReferences(ani_size(std::numeric_limits<ani_size>::max()) - 0), ANI_OUT_OF_MEMORY);
}

TEST_F(EnsureEnoughReferencesTest, ensure_enough_references_invalid_env)
{
    ASSERT_EQ(env_->c_api->EnsureEnoughReferences(nullptr, MIN_CAPACITY), ANI_INVALID_ARGS);
}

TEST_F(EnsureEnoughReferencesTest, ensure_enough_references_pending_error)
{
    ThrowError();
    ASSERT_EQ(env_->EnsureEnoughReferences(SPECIFIED_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::testing
