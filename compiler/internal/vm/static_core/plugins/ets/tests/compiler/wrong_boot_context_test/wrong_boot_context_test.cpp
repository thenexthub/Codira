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
#include "compiler_gtest.h"
#include "gtest/gtest.h"
#include "libarkbase/os/file.h"
#include "libarkbase/test_utilities.h"

namespace ark::ets::compiler::testing {

class WrongBootContext : public CompilerTest {
public:
    WrongBootContext()
    {
        std::string execPath = ark::os::file::File::GetExecutablePath().Value();
        auto pos = execPath.rfind('/');
        paocPath_ = execPath.substr(0, pos) + "/../bin/ark_aot ";
    }

protected:
    std::string paocPath_;                               // NOLINT(misc-non-private-member-variables-in-classes)
    std::string paocOut_ {" --paoc-output=tmp123.an "};  // NOLINT(misc-non-private-member-variables-in-classes)
    std::string paocRt_ {" --load-runtimes=ets "};       // NOLINT(misc-non-private-member-variables-in-classes)
};

DEATH_TEST_F(WrongBootContext, catch_abort)
{
    std::string paocCommand = paocPath_ + paocRt_ + bootFileString_ + paocOut_ + " --paoc-panda-files " + abcFile_;

    // NOLINTNEXTLINE(cert-env33-c)
    auto ret = std::system(paocCommand.c_str());

    ASSERT_TRUE(ret == 0);
};

}  // namespace ark::ets::compiler::testing
