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

#include "libpandabase/utils/arch.h"
#include "libpandabase/utils/regmask.h"

namespace panda::test {

HWTEST(ArchTest, GetCallerRegsMaskTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(GetCallerRegsMask(Arch::AARCH32, false), ArchTraits<Arch::AARCH32>::CALLER_REG_MASK);
    ASSERT_EQ(GetCallerRegsMask(Arch::AARCH64, false), ArchTraits<Arch::AARCH64>::CALLER_REG_MASK);
    ASSERT_EQ(GetCallerRegsMask(Arch::X86, false), ArchTraits<Arch::X86>::CALLER_REG_MASK);
    ASSERT_EQ(GetCallerRegsMask(Arch::X86_64, false), ArchTraits<Arch::X86_64>::CALLER_REG_MASK);
}

HWTEST(ArchTest, GetCalleeRegsMaskTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(GetCalleeRegsMask(Arch::AARCH32, false), ArchTraits<Arch::AARCH32>::CALLEE_REG_MASK);
    ASSERT_EQ(GetCalleeRegsMask(Arch::AARCH64, false), ArchTraits<Arch::AARCH64>::CALLEE_REG_MASK);
    ASSERT_EQ(GetCalleeRegsMask(Arch::X86, false), ArchTraits<Arch::X86>::CALLEE_REG_MASK);
    ASSERT_EQ(GetCalleeRegsMask(Arch::X86_64, false), ArchTraits<Arch::X86_64>::CALLEE_REG_MASK);
}

HWTEST(ArchTest, GetCalleeRegTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(GetFirstCalleeReg(Arch::X86_64, true), 1U);
    ASSERT_EQ(GetLastCalleeReg(Arch::X86_64, true), 0U);
}

}  // namespace panda::test
