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

#include "common_components/tests/test_helper.h"
#include "common_components/platform/map.h"

#include <sys/mman.h>
#include <unistd.h>

class PageProtectTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F_L0(PageProtectTest, TestPageProtect)
{
    size_t pageSize = getpagesize();
    void* mem = mmap(nullptr, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT_NE(mem, MAP_FAILED);

    bool success = common::PageProtect(mem, pageSize, PROT_READ);
    EXPECT_TRUE(success);

    munmap(mem, pageSize);
}

HWTEST_F_L0(PageProtectTest, TestNullptrMemory)
{
    bool success = common::PageProtect(nullptr, getpagesize(), -1);
    EXPECT_FALSE(success);
}