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

#include <cstdlib>
#include <gtest/gtest.h>

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_error.h"
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

#include "runtime/include/class.h"
#include "runtime/include/object_header.h"

namespace ark::ets::test {

class EntrypointTest : public ani::testing::AniTest {};
TEST_F(EntrypointTest, EntrypointReturnTest)
{
    auto runtime = Runtime::GetCurrent();
    ASSERT_NE(runtime, nullptr);

    auto pandafile = std::getenv("ANI_GTEST_ABC_PATH");
    ASSERT_NE(pandafile, nullptr);

    auto result = runtime->ExecutePandaFile(pandafile, "entrypoint_return_test.ETSGLOBAL::main", {});
    ASSERT_FALSE(result);
    auto returnError = result.Error();
    ASSERT_EQ(returnError, Runtime::Error::INVALID_ENTRY_POINT);
}

}  // namespace ark::ets::test
