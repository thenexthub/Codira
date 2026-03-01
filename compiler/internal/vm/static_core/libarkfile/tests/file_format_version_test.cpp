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

#include <libarkfile/include/file_format_version.h>

#include <gtest/gtest.h>

namespace ark::panda_file::test {
TEST(File, TestGetVersion)
{
    std::string versionstr;
    for (size_t i = 0; i < File::VERSION_SIZE; i++) {
        versionstr += std::to_string(VERSION[i]);
        if (i == (File::VERSION_SIZE - 1)) {
            break;
        }
        versionstr += ".";
    }
    EXPECT_EQ(GetVersion(VERSION), versionstr);
}

TEST(File, GetMinVersion)
{
    std::string versionstr;
    for (size_t i = 0; i < File::VERSION_SIZE; i++) {
        versionstr += std::to_string(MIN_VERSION[i]);
        if (i == (File::VERSION_SIZE - 1)) {
            break;
        }
        versionstr += ".";
    }
    EXPECT_EQ(GetVersion(MIN_VERSION), versionstr);
}

}  // namespace ark::panda_file::test
