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

#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/integrate/ets_ani_expo.h"
#include "plugins/ets/runtime/types/ets_error.h"
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

namespace ark::ets::test {

class EtsVMPostforkAotTest : public ani::testing::AniTest {
private:
    std::vector<ani_option> GetExraAniOption()
    {
        ani_option jit = {"--ext:compiler-enable-jit=false", nullptr};
        return std::vector<ani_option> {jit};
    }
};

void CompileInvalidAot(const char *abcPath, const char *anPath)
{
    const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
    ASSERT_NE(stdlib, nullptr);

    std::string aotArgs = "--load-runtimes=ets";
    aotArgs += " --boot-panda-files=" + std::string(stdlib) + ":" + std::string(stdlib);
    aotArgs += " --paoc-panda-files=" + std::string(abcPath);
    aotArgs += " --paoc-output=" + std::string(anPath);

    const char *binPath = std::getenv("OLDPWD");
    ASSERT_NE(binPath, nullptr);
    std::string arkAot = std::string(binPath) + "/bin/ark_aot";
    std::string runArkAot = arkAot + " " + aotArgs;

    // NOLINTNEXTLINE(cert-env33-c)
    [[maybe_unused]] int ret = std::system(runArkAot.c_str());
    ASSERT_EQ(ret, 0);
}

// issue 27234
TEST_F(EtsVMPostforkAotTest, DISABLED_PostforkAotFailedTest)
{
    const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
    ASSERT_NE(abcPath, nullptr);

    std::string abcPathString(abcPath);
    size_t dot = abcPathString.find_last_of('.');
    ASSERT(dot != std::string_view::npos);
    ASSERT(abcPathString.substr(dot) == ".abc" || abcPathString.substr(dot) == ".zip");

    std::string aotPath = abcPathString.substr(0, dot) + ".an";

    CompileInvalidAot(abcPath, aotPath.c_str());

    ani_option enableAn = {"--ext:--enable-an", nullptr};
    std::vector<ani_option> options;
    options.push_back(enableAn);
    pid_t pid = fork();
    ASSERT_NE(pid, -1);
    if (pid == 0) {
        ark::ets::ETSAni::Postfork(env_, options, false);
        _exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        ASSERT_TRUE(WIFSIGNALED(status));
        EXPECT_EQ(WTERMSIG(status), SIGABRT);
    }
}
}  // namespace ark::ets::test
