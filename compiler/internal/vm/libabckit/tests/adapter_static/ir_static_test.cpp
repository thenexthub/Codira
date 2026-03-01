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
#include <iostream>
#include "libabckit/src/adapter_static/ir_static.h"

namespace libabckit::test::adapter_static {
class IrStaticTest : public ::testing::Test {};

TEST_F(IrStaticTest, IrStaticTestInvalid)
{
    EXPECT_EQ(GfindOrCreateConstantI64Static(nullptr, 0), nullptr);
    EXPECT_EQ(GfindOrCreateConstantI32Static(nullptr, 0), nullptr);
    EXPECT_EQ(GfindOrCreateConstantU64Static(nullptr, 0), nullptr);
    EXPECT_EQ(GfindOrCreateConstantF64Static(nullptr, 0), nullptr);
}
}  // namespace libabckit::test::adapter_static
