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

#include <cstdint>
#include <limits>
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/tests/native/native_test_helper.h"
#include "plugins/ets/runtime/libani_helpers/external_array_buffer.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)

namespace ark::ets::test {

static constexpr int TWO = 2;
static constexpr int THREE = 3;
static constexpr int EIGHT = 8;
static constexpr int TWENTY_FOUR = 24;
static constexpr int HUNDRED_TWENTY_THREE = 123;
static constexpr int TWO_HUNDRED_THIRTY_FOUR = 234;
static constexpr int THREE_HUNDRED_FOURTY_FIVE = 345;
static constexpr const char *CHECK_ARRAY_BUFFER_FUNCTION = "checkCreatedArrayBuffer";
static constexpr const char *IS_DETACHED = "isDetached";

static void TestFinalizer(void *data, void *expectedData)
{
    ASSERT_EQ(data, expectedData);
    delete[] reinterpret_cast<int64_t *>(data);
}

class EtsNativeInterfaceArrayBufferTest : public EtsNapiTestBaseClass {};

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateEmpty)
{
    ani_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(0, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    CheckUnhandledError(env_);

    void *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, &resultData, &resultLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, 0);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateWithLength)
{
    ani_arraybuffer arrayBuffer;
    int64_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(data, nullptr);
    CheckUnhandledError(env_);

    data[0] = HUNDRED_TWENTY_THREE;
    data[1] = TWO_HUNDRED_THIRTY_FOUR;
    data[TWO] = THREE_HUNDRED_FOURTY_FIVE;

    int64_t *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&resultData), &resultLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, TWENTY_FOUR);
    ASSERT_EQ(resultData[0], HUNDRED_TWENTY_THREE);
    ASSERT_EQ(resultData[1], TWO_HUNDRED_THIRTY_FOUR);
    ASSERT_EQ(resultData[TWO], THREE_HUNDRED_FOURTY_FIVE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateExternalEmpty)
{
    ani_arraybuffer arrayBuffer;
    void *data = new int64_t[0];
    auto status = CreateFinalizableArrayBuffer(env_, data, 0, TestFinalizer, data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    CheckUnhandledError(env_);

    void *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, &resultData, &resultLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, 0);

    ani_boolean bufferIsExpected = ANI_FALSE;
    CallEtsFunction(&bufferIsExpected, "NativeTest", CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, 0);
    ASSERT_EQ(bufferIsExpected, ANI_TRUE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferCreateExternalWithLength)
{
    ani_arraybuffer arrayBuffer;
    auto *data = new int64_t[THREE];
    data[0] = HUNDRED_TWENTY_THREE;
    data[1] = TWO_HUNDRED_THIRTY_FOUR;
    data[TWO] = THREE_HUNDRED_FOURTY_FIVE;

    auto status = CreateFinalizableArrayBuffer(env_, data, THREE, TestFinalizer, data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    CheckUnhandledError(env_);

    int64_t *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&resultData), &resultLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, THREE);
    ASSERT_EQ(resultData[0], HUNDRED_TWENTY_THREE);
    ASSERT_EQ(resultData[1], TWO_HUNDRED_THIRTY_FOUR);
    ASSERT_EQ(resultData[TWO], THREE_HUNDRED_FOURTY_FIVE);

    ani_boolean bufferIsExpected = ANI_FALSE;
    CallEtsFunction(&bufferIsExpected, "NativeTest", CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, THREE);
    ASSERT_EQ(bufferIsExpected, ANI_TRUE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferDetachNonDetachable)
{
    ani_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(TWENTY_FOUR, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    CheckUnhandledError(env_);

    ani_boolean result = ANI_FALSE;
    auto resultStatus = IsDetachedArrayBuffer(env_, arrayBuffer, &result);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    auto resultstatus1 = DetachArrayBuffer(env_, arrayBuffer);
    ASSERT_EQ(resultstatus1, ANI_INVALID_ARGS);

    auto resultstatus2 = IsDetachedArrayBuffer(env_, arrayBuffer, &result);
    ASSERT_EQ(resultstatus2, ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ani_boolean isDetached = ANI_FALSE;
    CallEtsFunction(&isDetached, "NativeTest", IS_DETACHED, arrayBuffer);
    ASSERT_EQ(isDetached, ANI_FALSE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferDetachDetachable)
{
    ani_arraybuffer arrayBuffer;
    // Buffer will be manually detached later in the test, so allocate buffer as `unique_ptr`
    auto data = std::make_unique<int64_t[]>(THREE);  // NOLINT(modernize-avoid-c-arrays)

    auto status = CreateFinalizableArrayBuffer(env_, data.get(), THREE, TestFinalizer, data.get(), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    CheckUnhandledError(env_);

    ani_boolean bufferIsExpected = ANI_FALSE;
    CallEtsFunction(&bufferIsExpected, "NativeTest", CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, THREE);
    ASSERT_EQ(bufferIsExpected, ANI_TRUE);

    ani_boolean result = false;
    auto resultStatus = IsDetachedArrayBuffer(env_, arrayBuffer, &result);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    auto resultstatus1 = DetachArrayBuffer(env_, arrayBuffer);
    ASSERT_EQ(resultstatus1, ANI_OK);

    auto resultstatus2 = IsDetachedArrayBuffer(env_, arrayBuffer, &result);
    ASSERT_EQ(resultstatus2, ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_boolean isDetached = ANI_FALSE;
    CallEtsFunction(&isDetached, "NativeTest", IS_DETACHED, arrayBuffer);
    ASSERT_EQ(isDetached, ANI_TRUE);

    auto resultstatus3 = DetachArrayBuffer(env_, arrayBuffer);
    ASSERT_EQ(resultstatus3, ANI_INVALID_ARGS);

    CallEtsFunction(&isDetached, "NativeTest", IS_DETACHED, arrayBuffer);
    ASSERT_EQ(isDetached, ANI_TRUE);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferInvalidArgs)
{
    ani_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(-1, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer1;
    void *data1 = nullptr;
    size_t oversize = static_cast<size_t>(std::numeric_limits<int>::max()) + 1U;
    auto status1 = env_->CreateArrayBuffer(oversize, &data1, &arrayBuffer1);
    ASSERT_EQ(status1, ANI_INVALID_ARGS);

    void *data2 = nullptr;
    auto status2 = env_->CreateArrayBuffer(TWENTY_FOUR, &data2, nullptr);
    ASSERT_EQ(status2, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer3;
    auto status3 = env_->CreateArrayBuffer(TWENTY_FOUR, nullptr, &arrayBuffer3);
    ASSERT_EQ(status3, ANI_INVALID_ARGS);

    auto externalData = std::make_unique<int64_t[]>(THREE);  // NOLINT(modernize-avoid-c-arrays)

    auto status4 = CreateFinalizableArrayBuffer(env_, externalData.get(), THREE, TestFinalizer, nullptr, nullptr);
    ASSERT_EQ(status4, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer5;
    auto status5 = CreateFinalizableArrayBuffer(env_, externalData.get(), -1, TestFinalizer, nullptr, &arrayBuffer5);
    ASSERT_EQ(status5, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer6;
    auto status6 = CreateFinalizableArrayBuffer(env_, nullptr, THREE, TestFinalizer, nullptr, &arrayBuffer6);
    ASSERT_EQ(status6, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer7;
    auto status7 = CreateFinalizableArrayBuffer(env_, externalData.get(), THREE, nullptr, nullptr, &arrayBuffer7);
    ASSERT_EQ(status7, ANI_INVALID_ARGS);

    auto status8 = CreateFinalizableArrayBuffer(env_, nullptr, 0, nullptr, nullptr, nullptr);
    ASSERT_EQ(status8, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer9;
    void *data9;
    ASSERT_EQ(env_->CreateArrayBuffer(EIGHT, &data9, &arrayBuffer9), ANI_OK);
    size_t length9;
    auto status9 = env_->ArrayBuffer_GetInfo(arrayBuffer9, nullptr, &length9);
    ASSERT_EQ(status9, ANI_INVALID_ARGS);

    CheckUnhandledError(env_);
}

TEST_F(EtsNativeInterfaceArrayBufferTest, ArrayBufferInvalidArgs2)
{
    ani_arraybuffer arrayBuffer10;
    void *data10;
    ASSERT_EQ(env_->CreateArrayBuffer(EIGHT, &data10, &arrayBuffer10), ANI_OK);
    void *resultData10;
    auto status10 = env_->ArrayBuffer_GetInfo(arrayBuffer10, &resultData10, nullptr);
    ASSERT_EQ(status10, ANI_INVALID_ARGS);

    size_t length11;
    void *resultData11;
    auto status11 = env_->ArrayBuffer_GetInfo(nullptr, &resultData11, &length11);
    ASSERT_EQ(status11, ANI_INVALID_ARGS);

    ani_boolean result12;
    auto status12 = IsDetachedArrayBuffer(env_, nullptr, &result12);
    ASSERT_EQ(status12, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer13;
    void *data13;
    ASSERT_EQ(env_->CreateArrayBuffer(EIGHT, &data13, &arrayBuffer13), ANI_OK);
    auto status13 = IsDetachedArrayBuffer(env_, arrayBuffer13, nullptr);
    ASSERT_EQ(status13, ANI_INVALID_ARGS);

    auto status14 = DetachArrayBuffer(env_, nullptr);
    ASSERT_EQ(status14, ANI_INVALID_ARGS);

    CheckUnhandledError(env_);
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
