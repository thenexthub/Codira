/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "ani_gtest.h"
#include "plugins/ets/runtime/libani_helpers/external_array_buffer.h"

namespace ark::ets::ani_helpers::testing {

class ExternalArrayBufferTest : public ark::ets::ani::testing::AniTest {
public:
    void CheckArrayBuffer(ani_arraybuffer arrayBuffer, uint8_t *externalData, int size)
    {
        uint8_t *data = nullptr;
        size_t length = 0;
        auto status = env_->ArrayBuffer_GetInfo(arrayBuffer, reinterpret_cast<void **>(&data), &length);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(length, size);
        ASSERT_EQ(data, externalData);
    }
};

void TestFinalizer([[maybe_unused]] void *data, [[maybe_unused]] void *finalizerHint) {}

TEST_F(ExternalArrayBufferTest, CreateExternalArrayBuffer)
{
    const int size = 3u;
    auto data = std::make_unique<uint8_t[]>(size);
    data[0u] = 1u;
    data[1u] = 2u;
    data[2u] = 3u;

    ani_arraybuffer arrayBuffer;
    auto status = CreateExternalArrayBuffer(env_, data.get(), size, &arrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    CheckArrayBuffer(arrayBuffer, data.get(), size);

    ani_arraybuffer finalizableArrayBuffer;
    status = CreateFinalizableArrayBuffer(env_, data.get(), size, TestFinalizer, nullptr, &finalizableArrayBuffer);
    ASSERT_EQ(status, ANI_OK);
    CheckArrayBuffer(finalizableArrayBuffer, data.get(), size);
}

TEST_F(ExternalArrayBufferTest, CreateExternalArrayBufferInvalid)
{
    const int size = 3u;
    const int offset = 1u;
    auto data = std::make_unique<uint8_t[]>(size + offset);
    uint8_t *unalignedPtr = data.get() + offset;
    unalignedPtr[0u] = 1u;
    unalignedPtr[1u] = 2u;
    unalignedPtr[2u] = 3u;

    ani_arraybuffer arrayBuffer;
    auto status = CreateExternalArrayBuffer(env_, unalignedPtr, size, &arrayBuffer);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
    status = CreateFinalizableArrayBuffer(env_, unalignedPtr, size, TestFinalizer, nullptr, &arrayBuffer);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani_helpers::testing
