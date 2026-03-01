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

#include <gtest/gtest.h>

#include "runtime/coroutines/coroutine.h"
#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/event_loop_module.h"

namespace ark::ets::interop::js::testing {

class EventLoopCallbackPosterTest : public EtsInteropTest {
public:
    static std::string GetFirstWord()
    {
        return "First";
    }

    static std::string GetSecondWord()
    {
        return "Second";
    }

    static std::string GetAllWordInCorrectOrder()
    {
        return GetFirstWord() + GetSecondWord();
    }
};

TEST_F(EventLoopCallbackPosterTest, ManyPostsTest)
{
    std::string messageCollector;  // it's empty string
    uv_loop_t *loop = EventLoop::GetEventLoop();
    EventLoopCallbackPosterFactoryImpl factory;
    {
        auto poster = factory.CreatePoster();
        poster->Post([&messageCollector] { messageCollector += GetFirstWord(); });
        poster->Post([&messageCollector] { messageCollector += GetSecondWord(); });
        // after 2 posts poster will be deleted with final post of deleter
    }
    ASSERT_EQ(messageCollector, "");
    // run loop only once to avoid deadlock due to mainPoster in runtime
    uv_run(loop, uv_run_mode::UV_RUN_ONCE);
    ASSERT_EQ(messageCollector, GetAllWordInCorrectOrder());
}

TEST_F(EventLoopCallbackPosterTest, DeletingInPostTest)
{
    uv_loop_t *loop = EventLoop::GetEventLoop();
    EventLoopCallbackPosterFactoryImpl factory;
    auto poster = factory.CreatePoster();
    // run loop only once to avoid deadlock due to mainPoster in runtime
    poster->Post([&poster] { poster = nullptr; });
    ASSERT_NE(poster.get(), nullptr);
    uv_run(loop, uv_run_mode::UV_RUN_ONCE);
    ASSERT_EQ(poster.get(), nullptr);
}

}  // namespace ark::ets::interop::js::testing