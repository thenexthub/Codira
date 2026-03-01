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

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,
// readability-magic-numbers,cppcoreguidelines-pro-type-vararg)
namespace ark::ets::ani::testing {

class ArrayBufferGetInfoTest : public AniTest {
public:
    static constexpr const ani_size EXPECTED_SIZE = 10U;
    static constexpr const ani_size ZERO = 0U;
    static constexpr const ani_size ONE = 1U;
    static constexpr const ani_size TWO = 2U;
    static constexpr const ani_size THREE = 3U;

public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->GetUndefined(&undefinedRef_), ANI_OK);
    }

    ani_ref GetUndefined()
    {
        return undefinedRef_;
    }

private:
    ani_ref undefinedRef_ {};
};

TEST_F(ArrayBufferGetInfoTest, InvalidArgs)
{
    // Create a valid array buffer first
    ani_arraybuffer arrayBuffer;
    void *arrayBufferData = nullptr;
    auto status = env_->CreateArrayBuffer(EXPECTED_SIZE, &arrayBufferData, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);

    // Test with null array buffer
    void *data = nullptr;
    ani_size length = 0;
    auto resultStatus = env_->ArrayBuffer_GetInfo(nullptr, &data, &length);
    ASSERT_EQ(resultStatus, ANI_INVALID_ARGS);

    // Test with null data pointer
    resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, nullptr, &length);
    ASSERT_EQ(resultStatus, ANI_INVALID_ARGS);

    // Test with null length pointer
    resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, &data, nullptr);
    ASSERT_EQ(resultStatus, ANI_INVALID_ARGS);
}

TEST_F(ArrayBufferGetInfoTest, GetInfoFromRegularBuffer)
{
    // Create a regular array buffer
    ani_arraybuffer arrayBuffer;
    void *arrayBufferData = nullptr;
    auto status = env_->CreateArrayBuffer(EXPECTED_SIZE, &arrayBufferData, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);

    // Get info from the buffer
    void *data = nullptr;
    ani_size length = 0;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, &data, &length);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, EXPECTED_SIZE);

    // Verify we can write to and read from the buffer
    auto byteData = static_cast<uint8_t *>(data);
    byteData[0U] = ONE;
    byteData[EXPECTED_SIZE - 1U] = TWO;
    ASSERT_EQ(byteData[0U], ONE);
    ASSERT_EQ(byteData[EXPECTED_SIZE - 1U], TWO);
}

TEST_F(ArrayBufferGetInfoTest, GetInfoFromEmptyBuffer)
{
    // Create an empty regular array buffer
    ani_arraybuffer emptyRegularBuffer;
    void *emptyRegularBufferData = nullptr;
    auto status = env_->CreateArrayBuffer(ZERO, &emptyRegularBufferData, &emptyRegularBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(emptyRegularBuffer, nullptr);

    // Get info from the empty regular buffer
    void *regularData = nullptr;
    ani_size regularLength = 0;
    auto resultStatus = env_->ArrayBuffer_GetInfo(emptyRegularBuffer, &regularData, &regularLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_NE(regularData, nullptr);  // Should still return a valid pointer
    ASSERT_EQ(regularLength, ZERO);
}

TEST_F(ArrayBufferGetInfoTest, GetInfoFromManagedBuffer)
{
    auto array = static_cast<ani_arraybuffer>(
        CallEtsFunction<ani_ref>("arraybuffer_get_info_test", "GetArrayBuffer", GetUndefined()));
    ASSERT_NE(array, nullptr);

    void *data = nullptr;
    ani_size length = 0;
    auto resultStatus = env_->ArrayBuffer_GetInfo(array, &data, &length);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, EXPECTED_SIZE);

    // Verify data values
    auto byteData = static_cast<uint8_t *>(data);
    ASSERT_EQ(byteData[0U], ONE);
    ASSERT_EQ(byteData[1U], TWO);
    ASSERT_EQ(byteData[2U], THREE);
}

TEST_F(ArrayBufferGetInfoTest, GetInfoFromResizableBuffer)
{
    ani_class boxedIntClass {};
    ASSERT_EQ(env_->FindClass("std.core.Int", &boxedIntClass), ANI_OK);
    ani_ref maxByteLength {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(boxedIntClass, "valueOf", "i:C{std.core.Int}", &maxByteLength,
                                                     static_cast<ani_int>(EXPECTED_SIZE)),
              ANI_OK);

    auto array = static_cast<ani_arraybuffer>(
        CallEtsFunction<ani_ref>("arraybuffer_get_info_test", "GetArrayBuffer", maxByteLength));
    ASSERT_NE(array, nullptr);

    void *data = nullptr;
    ani_size length = 0;
    auto resultStatus = env_->ArrayBuffer_GetInfo(array, &data, &length);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(length, EXPECTED_SIZE);

    // Verify data values
    auto byteData = static_cast<uint8_t *>(data);
    ASSERT_EQ(byteData[0U], ONE);
    ASSERT_EQ(byteData[1U], TWO);
    ASSERT_EQ(byteData[2U], THREE);

    ani_class arrayBufferClass {};
    ASSERT_EQ(env_->FindClass("std.core.ArrayBuffer", &arrayBufferClass), ANI_OK);
    ani_method resizableGetter {};
    ASSERT_EQ(env_->Class_FindGetter(arrayBufferClass, "resizable", &resizableGetter), ANI_OK);
    ani_boolean isResizable = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(array, resizableGetter, &isResizable), ANI_OK);
    ASSERT_EQ(isResizable, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,
// readability-magic-numbers,cppcoreguidelines-pro-type-vararg)
