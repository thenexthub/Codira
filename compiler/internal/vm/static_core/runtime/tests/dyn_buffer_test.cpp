/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "runtime/include/runtime.h"
#include "runtime/regexp/ecmascript/mem/dyn_chunk.h"

namespace ark::mem::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark::coretypes;

class DynBufferTest : public testing::Test {
public:
    DynBufferTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~DynBufferTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(DynBufferTest);
    NO_MOVE_SEMANTIC(DynBufferTest);

private:
    ark::MTManagedThread *thread_ {};
};

TEST_F(DynBufferTest, EmitAndGet)
{
    DynChunk dynChunk = DynChunk();
    // NOLINTNEXTLINE(readability-magic-numbers)
    dynChunk.EmitChar(65U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    dynChunk.EmitU16(66U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    dynChunk.EmitU32(67U);
    ASSERT_EQ(dynChunk.GetSize(), 7U);
    ASSERT_EQ(dynChunk.GetAllocatedSize(), DynChunk::ALLOCATE_MIN_SIZE);
    ASSERT_EQ(dynChunk.GetError(), false);
    dynChunk.Insert(1, 1);
    uint32_t val1 = dynChunk.GetU8(0U);
    uint32_t val2 = dynChunk.GetU16(2U);
    uint32_t val3 = dynChunk.GetU32(4U);
    ASSERT_EQ(val1, 65U);
    ASSERT_EQ(val2, 66U);
    ASSERT_EQ(val3, 67U);
}

TEST_F(DynBufferTest, EmitSelfAndGet)
{
    DynChunk dynChunk = DynChunk();
    // NOLINTNEXTLINE(readability-magic-numbers)
    dynChunk.EmitChar(65U);
    dynChunk.EmitSelf(0, 1);
    ASSERT_EQ(dynChunk.GetSize(), 2U);
    ASSERT_EQ(dynChunk.GetAllocatedSize(), DynChunk::ALLOCATE_MIN_SIZE);
    ASSERT_EQ(dynChunk.GetError(), false);
    uint32_t val1 = dynChunk.GetU8(0);
    uint32_t val2 = dynChunk.GetU8(1);
    ASSERT_EQ(val1, 65U);
    ASSERT_EQ(val2, 65U);
}

TEST_F(DynBufferTest, EmitStrAndGet)
{
    DynChunk dynChunk = DynChunk();
    dynChunk.EmitStr("abc");
    ASSERT_EQ(dynChunk.GetSize(), 4U);
    ASSERT_EQ(dynChunk.GetAllocatedSize(), DynChunk::ALLOCATE_MIN_SIZE);
    ASSERT_EQ(dynChunk.GetError(), false);
    uint32_t val1 = dynChunk.GetU8(0U);
    uint32_t val2 = dynChunk.GetU8(1U);
    uint32_t val3 = dynChunk.GetU8(2U);
    uint32_t val4 = dynChunk.GetU8(3U);
    ASSERT_EQ(val1, 97U);
    ASSERT_EQ(val2, 98U);
    ASSERT_EQ(val3, 99U);
    ASSERT_EQ(val4, 0);
}
}  // namespace ark::mem::test
