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

#include "common_components/heap/collector/gc_request.h"
#include "common_components/heap/collector/gc_stats.h"
#include "common_components/base/time_utils.h"

#include "common_components/tests/test_helper.h"

using namespace common;

namespace common {
class GCRequestTest : public common::test::BaseTestWithScope {
};

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Heu_Test1) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCStats::SetPrevGCFinishTime(now);
    GCRequest req = { GC_REASON_HEU, "heuristic", false, true, 0, 0 };
    req.SetMinInterval(now + 1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_TRUE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Heu_Test2) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCStats::SetPrevGCFinishTime(1000);
    GCRequest req = { GC_REASON_HEU, "heuristic", false, true, 0, 0 };
    req.SetMinInterval(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Young_Test1) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCStats::SetPrevGCFinishTime(now);
    GCRequest req = { GC_REASON_YOUNG, "young", false, true, 0, 0 };
    req.SetMinInterval(now + 1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_TRUE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Young_Test2) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCStats::SetPrevGCFinishTime(1000);
    GCRequest req = { GC_REASON_YOUNG, "young", false, true, 0, 0 };
    req.SetMinInterval(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Native_Test1) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCStats::SetPrevGCFinishTime(now);
    GCRequest req = { GC_REASON_NATIVE, "native", false, true, 0, 0 };
    req.SetMinInterval(now + 1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_TRUE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Native_Test2) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCStats::SetPrevGCFinishTime(1000);
    GCRequest req = { GC_REASON_NATIVE, "native", false, true, 0, 0 };
    req.SetMinInterval(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Oom_Test1) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCRequest req = { GC_REASON_OOM, "oom", false, true, 0, 0 };
    req.SetMinInterval(now + 1000);
    req.SetPrevRequestTime(now);

    bool result = req.ShouldBeIgnored();
    EXPECT_TRUE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Oom_Test2) {
    GCRequest req = { GC_REASON_OOM, "oom", false, true, 0, 0 };
    req.SetMinInterval(0);
    req.SetPrevRequestTime(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Oom_Test3) {
    GCRequest req = { GC_REASON_OOM, "oom", false, true, 0, 0 };
    req.SetMinInterval(1000);
    req.SetPrevRequestTime(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Force_Test1) {
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    GCRequest req = { GC_REASON_FORCE, "force", false, true, 0, 0 };
    req.SetMinInterval(now + 1000);
    req.SetPrevRequestTime(now);

    bool result = req.ShouldBeIgnored();
    EXPECT_TRUE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Force_Test2) {
    GCRequest req = { GC_REASON_FORCE, "force", false, true, 0, 0 };
    req.SetMinInterval(0);
    req.SetPrevRequestTime(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_Force_Test3) {
    GCRequest req = { GC_REASON_FORCE, "force", false, true, 0, 0 };
    req.SetMinInterval(1000);
    req.SetPrevRequestTime(1000);

    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRequestTest, ShouldBeIgnored_User_Test1) {
    GCRequest req = { GC_REASON_USER, "user", false, true, 0, 0 };
    bool result = req.ShouldBeIgnored();
    EXPECT_FALSE(result);
}
} // namespace common::test