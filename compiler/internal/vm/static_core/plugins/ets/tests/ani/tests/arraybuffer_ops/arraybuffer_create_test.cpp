/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"

#include "runtime/include/thread_scopes.h"
#include "runtime/include/gc_task.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_coroutine.h"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
namespace ark::ets::ani::testing {

class ArrayBufferCreateTest : public AniTest {
public:
    static constexpr const ani_size EXPECTED_SIZE = 10U;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *CHECK_ARRAY_BUFFER_FUNCTION = "checkCreatedArrayBuffer";
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *MODULE_NAME = "arraybuffer_create_test";
    static constexpr ani_int LOOP_COUNT = 3;
    static constexpr ani_int TEST_VALUE_5 = 5U;
};

TEST_F(ArrayBufferCreateTest, InvalidArgs)
{
    ani_arraybuffer arrayBuffer;
    void *data = nullptr;
    const int64_t invalidSize = -1;
    auto status = env_->CreateArrayBuffer(invalidSize, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer1;
    void *data1 = nullptr;
    const int64_t oversize = static_cast<int64_t>(std::numeric_limits<int>::max()) + 1;
    auto status1 = env_->CreateArrayBuffer(oversize, &data1, &arrayBuffer1);
    ASSERT_EQ(status1, ANI_INVALID_ARGS);

    void *data2 = nullptr;
    auto status2 = env_->CreateArrayBuffer(EXPECTED_SIZE, &data2, nullptr);
    ASSERT_EQ(status2, ANI_INVALID_ARGS);

    ani_arraybuffer arrayBuffer3;
    auto status3 = env_->CreateArrayBuffer(EXPECTED_SIZE, nullptr, &arrayBuffer3);
    ASSERT_EQ(status3, ANI_INVALID_ARGS);
}

TEST_F(ArrayBufferCreateTest, CreateEmpty)
{
    ani_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(0, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    ani_boolean hasError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_FALSE);

    void *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, &resultData, &resultLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, 0);

    auto bufferIsCorrect = CallEtsFunction<ani_boolean>(MODULE_NAME, CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, 0);
    ASSERT_EQ(bufferIsCorrect, ANI_TRUE);
}

TEST_F(ArrayBufferCreateTest, CreateWithLength)
{
    ani_arraybuffer arrayBuffer;
    int8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(EXPECTED_SIZE, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(data, nullptr);
    ani_boolean hasError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_FALSE);

    data[0U] = 1U;
    data[1U] = 2U;
    data[2U] = 3U;

    int8_t *resultData;
    size_t resultLength;
    auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&resultData), &resultLength);
    ASSERT_EQ(resultStatus, ANI_OK);
    ASSERT_EQ(resultData, data);
    ASSERT_EQ(resultLength, EXPECTED_SIZE);
    ASSERT_EQ(resultData[0U], 1U);
    ASSERT_EQ(resultData[1U], 2U);
    ASSERT_EQ(resultData[2U], 3U);

    auto bufferIsCorrect =
        CallEtsFunction<ani_boolean>(MODULE_NAME, CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, EXPECTED_SIZE);
    ASSERT_EQ(bufferIsCorrect, ANI_TRUE);
}

TEST_F(ArrayBufferCreateTest, CreateForManaged)
{
    ani_arraybuffer arrayBuffer;
    void *data = nullptr;
    auto status = env_->CreateArrayBuffer(EXPECTED_SIZE, &data, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(data, nullptr);

    auto byteData = static_cast<uint8_t *>(data);
    byteData[0U] = 1U;
    byteData[1U] = 2U;
    byteData[2U] = 3U;

    auto ok = CallEtsFunction<ani_boolean>(MODULE_NAME, "CheckArrayBuffer", arrayBuffer);
    ASSERT_EQ(ok, ANI_TRUE);

    auto bufferIsCorrect =
        CallEtsFunction<ani_boolean>(MODULE_NAME, CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, EXPECTED_SIZE);
    ASSERT_EQ(bufferIsCorrect, ANI_TRUE);
}

TEST_F(ArrayBufferCreateTest, TestGC)
{
    ani_arraybuffer arrayBuffer;
    uint8_t *data = nullptr;
    auto status = env_->CreateArrayBuffer(EXPECTED_SIZE, reinterpret_cast<void **>(&data), &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);

    data[0U] = 1U;
    data[1U] = 2U;
    data[2U] = 3U;

    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *vm = coro->GetPandaVM();
        ScopedManagedCodeThread managedScope(coro);
        vm->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));
    }

    uint8_t *data2;
    ani_size length;
    auto resultStatus2 = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data2), &length);
    ASSERT_EQ(resultStatus2, ANI_OK);
    ASSERT_EQ(data2, data);
    ASSERT_EQ(length, EXPECTED_SIZE);
    ASSERT_EQ(data2[0U], 1U);
    ASSERT_EQ(data2[1U], 2U);
    ASSERT_EQ(data2[2U], 3U);

    auto bufferIsCorrect =
        CallEtsFunction<ani_boolean>(MODULE_NAME, CHECK_ARRAY_BUFFER_FUNCTION, arrayBuffer, EXPECTED_SIZE);
    ASSERT_EQ(bufferIsCorrect, ANI_TRUE);
}

TEST_F(ArrayBufferCreateTest, TestLoopGetData)
{
    ani_arraybuffer arrayBuffer = nullptr;
    uint8_t *data = nullptr;
    uint8_t *data2 = nullptr;
    for (int i = 0; i < LOOP_COUNT; i++) {
        auto status = env_->CreateArrayBuffer(EXPECTED_SIZE, reinterpret_cast<void **>(&data), &arrayBuffer);
        ASSERT_EQ(status, ANI_OK);
        data[0U] = 10U;
        data[1U] = 11U;
        ani_size length = 0;
        auto resultStatus = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data2), &length);
        ASSERT_EQ(resultStatus, ANI_OK);
        ASSERT_EQ(data2, data);
        ASSERT_EQ(length, EXPECTED_SIZE);
        ASSERT_EQ(data2[0U], 10U);
        ASSERT_EQ(data2[1U], 11U);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
