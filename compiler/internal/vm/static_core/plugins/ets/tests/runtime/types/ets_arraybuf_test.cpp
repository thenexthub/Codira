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
#include "ets_coroutine.h"
#include "ets_mirror_class_test_base.h"

#include "types/ets_class.h"
#include "types/ets_arraybuffer.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsArrayBufferTest : public testing::Test {
public:
    EtsArrayBufferTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("g1-gc");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
    }

    ~EtsArrayBufferTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsArrayBufferTest);
    NO_MOVE_SEMANTIC(EtsArrayBufferTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    const EtsPlatformTypes *GetPlatformTypes()
    {
        return PlatformTypes(vm_);
    }

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, managedData_, "data"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, weakRef_, "weakRef"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, nativeData_, "dataAddress"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, byteLength_, "_byteLength"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, isResizable_, "isResizable")};
    }

private:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsArrayBufferTest, MemoryLayout)
{
    EtsClass *klass = GetPlatformTypes()->coreArrayBuffer;
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

/**
 * @brief Creates an ArrayBuffer in movable space
 * and reallocates it to non-movable space to guarantee a valid data pointer.
 */
TEST_F(EtsArrayBufferTest, ReallocateToNonMovable)
{
    static constexpr int expectedLen = 24;

    auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsEscompatArrayBuffer> handle(coro, EtsEscompatArrayBuffer::Create(coro, expectedLen));
    ASSERT_NE(handle.GetPtr(), nullptr);

    ASSERT_FALSE(EtsEscompatArrayBuffer::IsNonMovableArray(coro, handle.GetPtr()));
    ASSERT_EQ(handle->GetByteLength(), expectedLen);

    auto *data = handle->GetData<uint8_t *>();
    ASSERT_NE(data, nullptr);

    data[0U] = 1U;
    data[1U] = 2U;
    data[2U] = 3U;

    EtsEscompatArrayBuffer::ReallocateNonMovableArray(coro, handle.GetPtr(), expectedLen);

    ASSERT_TRUE(EtsEscompatArrayBuffer::IsNonMovableArray(coro, handle.GetPtr()));
    ASSERT_EQ(handle->GetByteLength(), expectedLen);

    // Validate ArrayBuffer after data reallocation.
    auto *newData = handle->GetData<uint8_t *>();
    ASSERT_NE(newData, nullptr);

    handle->Set(3U, 4U);

    ASSERT_EQ(newData[0U], 1U);
    ASSERT_EQ(newData[1U], 2U);
    ASSERT_EQ(newData[2U], 3U);
    ASSERT_EQ(newData[3U], 4U);
}

}  // namespace ark::ets::test
