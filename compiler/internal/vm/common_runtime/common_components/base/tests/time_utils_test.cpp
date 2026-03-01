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
#include <regex>
#include <string>

#include "common_components/base/time_utils.h"
#include "common_components/tests/test_helper.h"
#include "common_components/base/c_string.h"

using namespace common;
using namespace common::TimeUtil;
namespace common::test {

class TimeUtilsTest : public common::test::BaseTestWithScope {
};

HWTEST_F_L0(TimeUtilsTest, GetDigitDate)
{
#ifndef NDEBUG
    CString result = GetDigitDate();
    EXPECT_FALSE(result.IsEmpty());
    
    // 验证返回的格式是否符合yyyymmdd_hhmmss
    std::string dateStr = result.Str();
    std::regex pattern("\\d{8}_\\d{6}");
    EXPECT_TRUE(std::regex_match(dateStr, pattern));
#endif
}

HWTEST_F_L0(TimeUtilsTest, GetTimestamp) {
    CString result = GetTimestamp();
    EXPECT_FALSE(result.IsEmpty());
    
    // 验证返回的格式是否符合yyyy-mm-dd hh:mm:ss.ffffff
    std::string timestamp = result.Str();
    std::regex pattern("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{6}");
    EXPECT_TRUE(std::regex_match(timestamp, pattern));
}

HWTEST_F_L0(TimeUtilsTest, NanoSeconds)
{
    uint64_t nano1 = NanoSeconds();
    
    // 等待一小段时间
    SleepForNano(1000);

    uint64_t nano2 = NanoSeconds();
    
    // 验证时间是否递增
    EXPECT_TRUE(nano2 > nano1);
}

HWTEST_F_L0(TimeUtilsTest, MilliSeconds)
{
    uint64_t milli1 = MilliSeconds();

    // 等待一小段时间
    SleepForNano(1000);

    uint64_t milli2 = MilliSeconds();
    
    // 验证时间是否递增
    EXPECT_TRUE(milli2 >= milli1);
}

HWTEST_F_L0(TimeUtilsTest, MicroSeconds)
{
    uint64_t micro1 = MicroSeconds();
    
    // 等待一小段时间
    SleepForNano(1000);
    
    uint64_t micro2 = MicroSeconds();
    
    // 验证时间是否递增
    EXPECT_TRUE(micro2 > micro1);
}
} // namespace common::test