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

#include <thread>
#include <sstream>
#include <gtest/gtest.h>
#include "libarkbase/mem/alloc_tracker.h"

namespace ark {

struct Header {
    uint32_t numItems = 0;
    uint32_t numStacktraces = 0;
};

struct AllocInfo {
    uint32_t tag = 0;
    uint32_t id = 0;
    uint32_t size = 0;
    uint32_t space = 0;
    uint32_t stacktraceId = 0;
};

struct FreeInfo {
    uint32_t tag = 0;
    uint32_t allocId = 0;
};

static void SkipString(std::istream &in)
{
    uint32_t len = 0;
    in.read(reinterpret_cast<char *>(&len), sizeof(len));
    if (!in) {
        return;
    }
    in.seekg(len, std::ios_base::cur);
}

TEST(DetailAllocTrackerTest, NoAllocs)
{
    DetailAllocTracker tracker;
    std::stringstream out;
    tracker.Dump(out);
    out.seekg(0U);

    Header hdr;
    out.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(0U, hdr.numItems);
    ASSERT_EQ(0U, hdr.numStacktraces);
}

TEST(DetailAllocTrackerTest, OneAlloc)
{
    DetailAllocTracker tracker;
    std::stringstream out;
    // NOLINTNEXTLINE(readability-magic-numbers)
    tracker.TrackAlloc(reinterpret_cast<void *>(0x15U), 20U, SpaceType::SPACE_TYPE_INTERNAL);
    tracker.Dump(out);
    out.seekg(0U);

    Header hdr;
    out.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(1U, hdr.numItems);
    ASSERT_EQ(1U, hdr.numStacktraces);

    // skip stacktrace
    SkipString(out);
    ASSERT_FALSE(out.eof());
    AllocInfo info;
    out.read(reinterpret_cast<char *>(&info), sizeof(info));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(DetailAllocTracker::ALLOC_TAG, info.tag);
    ASSERT_EQ(0U, info.id);
    ASSERT_EQ(20U, info.size);
    ASSERT_EQ(static_cast<uint32_t>(SpaceType::SPACE_TYPE_INTERNAL), info.space);
    ASSERT_EQ(0U, info.stacktraceId);
}

TEST(DetailAllocTrackerTest, AllocAndFree)
{
    DetailAllocTracker tracker;
    std::stringstream out;
    // NOLINTNEXTLINE(readability-magic-numbers)
    tracker.TrackAlloc(reinterpret_cast<void *>(0x15U), 20U, SpaceType::SPACE_TYPE_INTERNAL);
    // NOLINTNEXTLINE(readability-magic-numbers)
    tracker.TrackFree(reinterpret_cast<void *>(0x15U));
    tracker.Dump(out);
    out.seekg(0U);

    Header hdr;
    out.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(2U, hdr.numItems);
    ASSERT_EQ(1U, hdr.numStacktraces);

    // skip stacktrace
    SkipString(out);
    ASSERT_FALSE(out.eof());
    AllocInfo alloc;
    FreeInfo free;
    out.read(reinterpret_cast<char *>(&alloc), sizeof(alloc));
    out.read(reinterpret_cast<char *>(&free), sizeof(free));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(DetailAllocTracker::ALLOC_TAG, alloc.tag);
    ASSERT_EQ(0U, alloc.id);
    ASSERT_EQ(20U, alloc.size);
    ASSERT_EQ(static_cast<uint32_t>(SpaceType::SPACE_TYPE_INTERNAL), alloc.space);
    ASSERT_EQ(0U, alloc.stacktraceId);
    ASSERT_EQ(DetailAllocTracker::FREE_TAG, free.tag);
    ASSERT_EQ(0U, free.allocId);
}

TEST(DetailAllocTrackerTest, MultithreadedAlloc)
{
    static constexpr size_t NUM_THREADS = 10;
    static constexpr size_t NUM_ITERS = 100;

    DetailAllocTracker tracker;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(
            [&tracker](size_t threadNum) {
                for (size_t iter = 0; iter < NUM_ITERS; ++iter) {
                    auto addr = reinterpret_cast<void *>(threadNum * NUM_THREADS + iter + 1U);
                    // NOLINTNEXTLINE(readability-magic-numbers)
                    tracker.TrackAlloc(addr, 10U, SpaceType::SPACE_TYPE_INTERNAL);
                }
            },
            i);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::stringstream out;
    tracker.Dump(out);
    out.seekg(0U);

    Header hdr;
    out.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(NUM_THREADS * NUM_ITERS, hdr.numItems);
    ASSERT_EQ(1U, hdr.numStacktraces);
}

TEST(DetailAllocTrackerTest, NullptrAlloc)
{
    DetailAllocTracker tracker;
    // NOLINTNEXTLINE(readability-magic-numbers)
    tracker.TrackAlloc(nullptr, 20U, SpaceType::SPACE_TYPE_INTERNAL);
    tracker.TrackFree(nullptr);
    std::stringstream out;
    tracker.Dump(out);
    out.seekg(0U);

    Header hdr;
    out.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    ASSERT_FALSE(out.eof());
    ASSERT_EQ(0U, hdr.numItems);
    ASSERT_EQ(0U, hdr.numStacktraces);
}

}  // namespace ark
