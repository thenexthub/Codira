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

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
namespace ark::ets::ani::testing {

class ArrayBufferAccessorsTest : public AniTest {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr int TWENTY_FOUR = 24;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr int TWENTY_FIVE = 25;

    ani_arraybuffer CreateArrayBufferFromManaged()
    {
        auto res = CallEtsFunction<ani_ref>("accessors_test", "createArrayBuffer", TWENTY_FOUR);
        return static_cast<ani_arraybuffer>(res);
    }

    /**
     * @brief Checks that changes in natively-acquired array are visible by managed `at` method
     * @param arrayBuffer non-detached instance of ArrayBuffer
     */
    void ArrayBufferAtTest(ani_arraybuffer arrayBuffer)
    {
        int8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ANI_OK);

        const auto checkAtMethod = [arrayBuffer, data, env = env_](ani_int index, ani_byte expectedType) {
            data[index] = expectedType;
            ani_byte value = 0;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ASSERT_EQ(env->Object_CallMethodByName_Byte(arrayBuffer, "at", "i:b", &value, index), ANI_OK);
            ani_boolean hasError = ANI_FALSE;
            ASSERT_EQ(env->ExistUnhandledError(&hasError), ANI_OK);
            ASSERT_EQ(hasError, ANI_FALSE);
            ASSERT_EQ(value, expectedType);
        };

        for (size_t i = 0; i < length; ++i) {
            checkAtMethod(i, static_cast<ani_byte>(i));
        }
    }

    /**
     * @brief Checks that changes through managed `set` method are visible in natively-acquired array
     * @param arrayBuffer non-detached instance of ArrayBuffer
     */
    void ArrayBufferSetTest(ani_arraybuffer arrayBuffer)
    {
        int8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(length, TWENTY_FOUR);

        const auto checkSetMethod = [arrayBuffer, data, env = env_](ani_int index, ani_byte expectedType) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ASSERT_EQ(env->Object_CallMethodByName_Void(arrayBuffer, "set", "ib:", index, expectedType), ANI_OK);
            ani_boolean hasError = ANI_FALSE;
            ASSERT_EQ(env->ExistUnhandledError(&hasError), ANI_OK);
            ASSERT_EQ(hasError, ANI_FALSE);
            ASSERT_EQ(data[index], expectedType);
        };

        for (size_t i = 0; i < length; ++i) {
            checkSetMethod(i, static_cast<ani_byte>(i));
        }
    }

    /**
     * @brief Checks that `at` with invalid indexes results in managed error being thrown
     * @param arrayBuffer non-detached instance of ArrayBuffer, must be with length less or equal to 24
     */
    void ArrayBufferAtInvalidIndexTest(ani_arraybuffer arrayBuffer)
    {
        const auto checkAtMethod = [arrayBuffer, env = env_](ani_int index) {
            ani_byte value = 0;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ASSERT_EQ(env->Object_CallMethodByName_Byte(arrayBuffer, "at", "i:b", &value, index), ANI_PENDING_ERROR);
            ani_boolean hasError = ANI_FALSE;
            ASSERT_EQ(env->ExistUnhandledError(&hasError), ANI_OK);
            ASSERT_EQ(hasError, ANI_TRUE);
            ASSERT_EQ(env->ResetError(), ANI_OK);
        };

        checkAtMethod(-1);
        checkAtMethod(TWENTY_FOUR);
        checkAtMethod(TWENTY_FIVE);
    }

    /**
     * @brief Checks that `set` with invalid indexes results in managed error being thrown
     * @param arrayBuffer non-detached instance of ArrayBuffer, must be with length less or equal to 24
     */
    void ArrayBufferSetInvalidIndexTest(ani_arraybuffer arrayBuffer)
    {
        const auto checkSetMethod = [arrayBuffer, env = env_](ani_int index) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ASSERT_EQ(env->Object_CallMethodByName_Void(arrayBuffer, "set", "ib:", index, 0), ANI_PENDING_ERROR);
            ani_boolean hasError = ANI_FALSE;
            ASSERT_EQ(env->ExistUnhandledError(&hasError), ANI_OK);
            ASSERT_EQ(hasError, ANI_TRUE);
            ASSERT_EQ(env->ResetError(), ANI_OK);
        };

        checkSetMethod(-1);
        checkSetMethod(TWENTY_FOUR);
        checkSetMethod(TWENTY_FIVE);
    }
};

/// @brief Creates ArrayBuffer via ANI and tests its `at` method
TEST_F(ArrayBufferAccessorsTest, ArrayBufferGet)
{
    ani_arraybuffer arrayBuffer {};
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer via ANI and tests its `set` method
TEST_F(ArrayBufferAccessorsTest, ArrayBufferSet)
{
    ani_arraybuffer arrayBuffer {};
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferSetTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer in managed code and tests its `at` method
TEST_F(ArrayBufferAccessorsTest, ArrayBufferManagedGet)
{
    auto arrayBuffer = CreateArrayBufferFromManaged();
    ArrayBufferAtTest(arrayBuffer);
}

/// @brief Creates ArrayBuffer in managed code and tests its `set` method
TEST_F(ArrayBufferAccessorsTest, ArrayBufferManagedSet)
{
    auto arrayBuffer = CreateArrayBufferFromManaged();
    ArrayBufferSetTest(arrayBuffer);
}

TEST_F(ArrayBufferAccessorsTest, ArrayBufferGetInvalidArgs)
{
    ani_arraybuffer arrayBuffer {};
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferAtInvalidIndexTest(arrayBuffer);
}

TEST_F(ArrayBufferAccessorsTest, ArrayBufferSetInvalidArgs)
{
    ani_arraybuffer arrayBuffer {};
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    ArrayBufferSetInvalidIndexTest(arrayBuffer);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
