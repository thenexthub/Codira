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

#include "gtest/gtest.h"
#include "os/thread.h"

namespace panda::test {

class UnixThreadTest : public testing::Test {};

HWTEST_F(UnixThreadTest, ThreadGetStackInfoTest, testing::ext::TestSize.Level0)
{
    void *stack_base = nullptr;
    size_t guard_size;
    size_t stack_size;
    int error = os::thread::ThreadGetStackInfo(os::thread::GetNativeHandle(), &stack_base, &stack_size, &guard_size);
    ASSERT_EQ(error, 0);
}

}  // namespace panda
