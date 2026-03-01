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

#include <ctime>

#include "gtest/gtest.h"
#include "runtime/include/flattened_string_cache.h"

namespace ark::coretypes::test {
class FlattenedStringCacheTest : public testing::Test {
public:
    FlattenedStringCacheTest()
    {
        Runtime::Create(options_);
    }

    ~FlattenedStringCacheTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(FlattenedStringCacheTest);
    NO_MOVE_SEMANTIC(FlattenedStringCacheTest);

    void SetUp() override
    {
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        thread_->ManagedCodeEnd();
    }

    static coretypes::String *Cast(uintptr_t addr)
    {
        ASSERT(IsAligned(addr, DEFAULT_ALIGNMENT_IN_BYTES));
        return reinterpret_cast<coretypes::String *>(addr);
    }

private:
    ark::MTManagedThread *thread_ {};
    RuntimeOptions options_;
};

TEST_F(FlattenedStringCacheTest, TestUpdate)
{
    auto *cache = FlattenedStringCache::Create(PandaVM::GetCurrent());
    const uintptr_t keyStrAddr1 = 0x11223E00;
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr1)), nullptr);

    const uintptr_t cachedStrAddr1 = 0x11223200;
    FlattenedStringCache::Update(cache, Cast(keyStrAddr1), Cast(cachedStrAddr1));
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr1)), Cast(cachedStrAddr1));

    const uintptr_t keyStrAddr2 = 0x11223FC0;
    const uintptr_t cachedStrAddr2 = 0x11223210;
    FlattenedStringCache::Update(cache, Cast(keyStrAddr2), Cast(cachedStrAddr2));
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr1)), Cast(cachedStrAddr1));
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr2)), Cast(cachedStrAddr2));

    const uintptr_t keyStrAddr3 = 0x11224000;
    const uintptr_t cachedStrAddr3 = 0x11223220;
    FlattenedStringCache::Update(cache, Cast(keyStrAddr3), Cast(cachedStrAddr3));
    // first entry was overwritten
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr1)), nullptr);
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr2)), Cast(cachedStrAddr2));
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr3)), Cast(cachedStrAddr3));
}

TEST_F(FlattenedStringCacheTest, TestNullCache)
{
    auto cache = nullptr;
    const uintptr_t keyStrAddr = 0x11223E00;
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr)), nullptr);
    const uintptr_t cachedStrAddr = 0x11223200;
    FlattenedStringCache::Update(cache, Cast(keyStrAddr), Cast(cachedStrAddr));
    ASSERT_EQ(FlattenedStringCache::Get(cache, Cast(keyStrAddr)), nullptr);
}
}  // namespace ark::coretypes::test
