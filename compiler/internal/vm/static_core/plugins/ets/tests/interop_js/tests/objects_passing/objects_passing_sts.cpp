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

// Tests are disabled. When passing an object to a function imported from ETS,
// ASSERTION FAILED occurs: !klass->IsInterface()//

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropObjectPassingEts : public EtsInteropTest {};
// NOTE(andreypetukhov) enable after fixibng #18183
TEST_F(EtsInteropObjectPassingEts, DISABLED_test_passing_object)
{
    ASSERT_EQ(true, RunJsTestSuite("objects_passing/objects_passing_sts.js"));
}
// NOTE(andreypetukhov) enable after fixibng #18183
TEST_F(EtsInteropObjectPassingEts, DISABLED_test_passing_class)
{
    ASSERT_EQ(true, RunJsTestSuite("objects_passing/class_passing_sts.js"));
}

}  // namespace ark::ets::interop::js::testing