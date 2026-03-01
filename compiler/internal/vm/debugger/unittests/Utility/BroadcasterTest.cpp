//===-- BroadcasterTest.cpp -----------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "lldb/Utility/Broadcaster.h"
#include "lldb/Utility/Event.h"
#include "lldb/Utility/Listener.h"
#include "lldb/Utility/Predicate.h"

#include <thread>

using namespace lldb;
using namespace lldb_private;

TEST(BroadcasterTest, BroadcastEvent) {
  EventSP event_sp;
  Broadcaster broadcaster(nullptr, "test-broadcaster");
  std::chrono::seconds timeout(0);

  // Create a listener, sign it up, make sure it receives an event.
  ListenerSP listener1_sp = Listener::MakeListener("test-listener1");
  const uint32_t event_mask1 = 1;
  EXPECT_EQ(event_mask1,
            listener1_sp->StartListeningForEvents(&broadcaster, event_mask1));
  broadcaster.BroadcastEvent(event_mask1, nullptr);
  EXPECT_TRUE(listener1_sp->GetEvent(event_sp, timeout));
  EXPECT_EQ(event_mask1, event_sp->GetType());

  {
    // Add one more listener, make sure it works as well.
    ListenerSP listener2_sp = Listener::MakeListener("test-listener2");
    const uint32_t event_mask2 = 1;
    EXPECT_EQ(event_mask2, listener2_sp->StartListeningForEvents(
                               &broadcaster, event_mask1 | event_mask2));
    broadcaster.BroadcastEvent(event_mask2, nullptr);
    EXPECT_TRUE(listener2_sp->GetEvent(event_sp, timeout));
    EXPECT_EQ(event_mask2, event_sp->GetType());

    // Both listeners should get this event.
    broadcaster.BroadcastEvent(event_mask1, nullptr);
    EXPECT_TRUE(listener1_sp->GetEvent(event_sp, timeout));
    EXPECT_EQ(event_mask1, event_sp->GetType());
    EXPECT_TRUE(listener2_sp->GetEvent(event_sp, timeout));
    EXPECT_EQ(event_mask2, event_sp->GetType());
  }

  // Now again only one listener should be active.
  broadcaster.BroadcastEvent(event_mask1, nullptr);
  EXPECT_TRUE(listener1_sp->GetEvent(event_sp, timeout));
  EXPECT_EQ(event_mask1, event_sp->GetType());
}

TEST(BroadcasterTest, EventTypeHasListeners) {
  EventSP event_sp;
  Broadcaster broadcaster(nullptr, "test-broadcaster");

  const uint32_t event_mask = 1;
  EXPECT_FALSE(broadcaster.EventTypeHasListeners(event_mask));

  {
    ListenerSP listener_sp = Listener::MakeListener("test-listener");
    EXPECT_EQ(event_mask,
              listener_sp->StartListeningForEvents(&broadcaster, event_mask));
    EXPECT_TRUE(broadcaster.EventTypeHasListeners(event_mask));
  }

  EXPECT_FALSE(broadcaster.EventTypeHasListeners(event_mask));
}
