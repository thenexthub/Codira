/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "libpandabase/utils/bit_field.h"
#include "libpandabase/utils/cframe_layout.h"

namespace panda::test {

HWTEST(CFrameLayoutTest, AlignSpillCountTest, testing::ext::TestSize.Level0)
{
    CFrameLayout cfl1(Arch::AARCH64, 4U);
    ASSERT_EQ(cfl1.GetSpillsCount(), 5U);

    CFrameLayout cfl2(Arch::X86_64, 0U);
    ASSERT_EQ(cfl2.GetSpillsCount(), 0U);

    CFrameLayout cfl3(Arch::AARCH32, 2U);
    ASSERT_EQ(cfl3.GetSpillsCount(), 7U);

    CFrameLayout cfl4(Arch::AARCH32, 1U);
    ASSERT_EQ(cfl4.GetSpillsCount(), 5U);

    CFrameLayout cfl5(Arch::X86, 5U);
    ASSERT_EQ(cfl5.GetSpillsCount(), 5U);
}

}  // namespace panda::test