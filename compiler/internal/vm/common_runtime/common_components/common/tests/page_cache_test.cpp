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

#include "common_components/common/page_cache.h"
#include "common_components/tests/test_helper.h"

using namespace common;

class PageCacheTestSimple : public common::test::BaseTestWithScope {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(PageCacheTestSimple, NewSpan_AllocatesSingleSpan, testing::ext::TestSize.Level0) {
    Span* span = PageCache::GetInstance()->NewSpan(1);
    ASSERT_NE(span, nullptr);
    EXPECT_EQ(span->pageNum, 1U);
}

HWTEST_F(PageCacheTestSimple, MapObjectToSpan_ReturnsCorrectSpan, testing::ext::TestSize.Level0) {
    Span* span = PageCache::GetInstance()->NewSpan(1);
    void* obj = reinterpret_cast<void*>(span->pageId << PAGE_SHIFT);

    Span* result = PageCache::GetInstance()->MapObjectToSpan(obj);
    EXPECT_EQ(result, span);
}

HWTEST_F(PageCacheTestSimple, ReleaseSpanToPageCache_CanMerge, testing::ext::TestSize.Level0) {
    Span* span1 = PageCache::GetInstance()->NewSpan(1);
    Span* span2 = PageCache::GetInstance()->NewSpan(1);

    span1->pageNum = 1;
    span1->pageId = 100;
    span2->pageNum = 1;
    span2->pageId = 101;

    PageCache::GetInstance()->ReleaseSpanToPageCache(span1);
    PageCache::GetInstance()->ReleaseSpanToPageCache(span2);

    Span* mergedSpan = PageCache::GetInstance()->NewSpan(2);
    EXPECT_NE(mergedSpan, nullptr);
    EXPECT_EQ(mergedSpan->pageNum, 2U);
}

HWTEST_F(PageCacheTestSimple, NewSpan_SplitFromLargerSpan, testing::ext::TestSize.Level0) {
    Span* bigSpan = PageCache::GetInstance()->NewSpan(5);
    ASSERT_NE(bigSpan, nullptr);
    EXPECT_EQ(bigSpan->pageNum, 5U);

    Span* smallSpan = PageCache::GetInstance()->NewSpan(2);
    ASSERT_NE(smallSpan, nullptr);
    EXPECT_EQ(smallSpan->pageNum, 2U);

    Span* remainingSpan = PageCache::GetInstance()->NewSpan(3);
    ASSERT_NE(remainingSpan, nullptr);
    EXPECT_EQ(remainingSpan->pageNum, 3U);
}

HWTEST_F(PageCacheTestSimple, ReleaseSpanToPageCache_MergeForward, testing::ext::TestSize.Level0) {
    Span* span1 = PageCache::GetInstance()->NewSpan(1);
    Span* span2 = PageCache::GetInstance()->NewSpan(1);

    span1->pageNum = 1;
    span1->pageId = 100;
    span2->pageNum = 1;
    span2->pageId = 101;

    PageCache::GetInstance()->ReleaseSpanToPageCache(span1);
    PageCache::GetInstance()->ReleaseSpanToPageCache(span2);

    Span* mergedSpan = PageCache::GetInstance()->NewSpan(2);
    EXPECT_NE(mergedSpan, nullptr);
    EXPECT_EQ(mergedSpan->pageNum, 2U);
    EXPECT_EQ(mergedSpan->pageId, 100U);
}