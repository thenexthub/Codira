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

// NOLINTBEGIN

#include "common.h"

#include "libarkbase/utils/arch.h"

#include <gtest/gtest.h>

namespace ark::cli::test {

std::string glob_argv0 = "";

class PandaOptionsTest : public PandaTest {
public:
    PandaOptionsTest()
    {
        SetEnv(glob_argv0);
        dir_name_ = GetBinPath();
    }

public:
    inline virtual std::string GetDirName() const final
    {
        return dir_name_;
    }

private:
    std::string dir_name_;
};

// NOTE(mgonopolskiy): add tests for all possible cases
TEST_F(PandaOptionsTest, PandaFiles)
{
    // Test basic functionality only in host mode.
    // NOTE(mgonopolskiy): add support for all modes
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }

    {
        auto boot = GetDirName() + Separator() + "boot.abc";
        // clang-format off
        boot += ":" + GetDirName() + Separator() + ".." + Separator() + ".." + Separator() + ".."
                    + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        // clang-format on

        auto pfile = GetDirName() + Separator() + "pfile.abc";
        auto app = GetDirName() + Separator() + "app.abc";
        std::string_view main_class = "Test";
        std::string_view main_function = "main";
        std::string entrypoint = std::string(main_class) + "::" + std::string(main_function);

        int64_t exp_res = 9;
        auto res = RunPandaFile("--boot-panda-files", boot.data(), "--panda-files", pfile.data(), app.data(),
                                entrypoint.data());
        ASSERT_TRUE(res) << "Exec failed with the error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), exp_res) << "Panda must fail";
    }
}

}  // namespace ark::cli::test

int main(int argc, char **argv)
{
    ark::cli::test::glob_argv0 = argv[0];
    // CHECKER_IGNORE_NEXTLINE(AF0010)
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND