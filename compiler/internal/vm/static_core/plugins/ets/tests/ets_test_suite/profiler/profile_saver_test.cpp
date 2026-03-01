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

#include <gtest/gtest.h>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "ets_vm.h"
#include "include/runtime.h"
#include "include/runtime_options.h"
#include "libarkbase/test_utilities.h"
#include "jit/libprofile/aot_profiling_data.h"
#include "jit/profiling_loader.h"
#include "jit/profiling_saver.h"
#include "libarkbase/os/filesystem.h"

namespace ark::test {
class ProfileSaverTest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetLoadRuntimes({"ets"});
        options.SetProfilesaverSleepingTimeMs(0U);
        options.SetCompilerProfilingThreshold(0U);
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "profile_saver_test.abc"});
        options_ = std::move(options);
    }

    void SetProfilesaverSleepingTimeMs(uint32_t value)
    {
        options_.SetProfilesaverSleepingTimeMs(value);
    }

    void SetWorkersType(const std::string &value)
    {
        options_.SetWorkersType(value);
    }

    void SetIncrementalProfilesaverEnabled(bool value)
    {
        options_.SetIncrementalProfilesaverEnabled(value);
    }

    void CreateRuntime()
    {
        bool success = Runtime::Create(options_);
        ASSERT_TRUE(success) << "Cannot create Runtime";
    }

    void RunPandafile()
    {
        const std::string mainFunc = "profile_saver_test.ETSGLOBAL::main";
        const std::string fileName = "profile_saver_test.abc";
        auto res = Runtime::GetCurrent()->ExecutePandaFile(fileName.c_str(), mainFunc.c_str(), {});
        ASSERT_TRUE(res);
    }

    void InitPGOFilePath(const std::string &pgoFilePath)
    {
        options_.SetProfileOutput(pgoFilePath);
        if (std::filesystem::exists(pgoFilePath)) {
            std::filesystem::remove(pgoFilePath);
        }
    }

    void CheckLoadProfile()
    {
        ProfilingLoader loader;
        const std::string &pgoFilePath = options_.GetProfileOutput();
        auto profileCtxOrError = loader.LoadProfile(PandaString(pgoFilePath));
        if (!profileCtxOrError) {
            std::cerr << profileCtxOrError.Error();
        }
        auto classCtxStr = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetBootClassContext() + ":" +
                           Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetAppClassContext();
        ASSERT_TRUE(profileCtxOrError.HasValue());
        ASSERT_EQ(*profileCtxOrError, PandaString(classCtxStr));
    }

    void CheckLoadProfileFail()
    {
        ProfilingLoader loader;
        const std::string &pgoFilePath = options_.GetProfileOutput();
        auto profileCtxOrError = loader.LoadProfile(PandaString(pgoFilePath));
        ASSERT_FALSE(profileCtxOrError.HasValue());
    }

    void DestroyRuntime()
    {
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }

private:
    RuntimeOptions options_;
};

TEST_F(ProfileSaverTest, ProfileSaverWorkerTest)
{
    InitPGOFilePath("profile_saver_worker_test.ap");
    SetIncrementalProfilesaverEnabled(true);
    CreateRuntime();
    RunPandafile();
    EXPECT_TRUE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saverWorker = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    saverWorker->WaitForFinishSaverTask();
    ASSERT_TRUE(saverWorker->TryAddTask());
    saverWorker->WaitForFinishSaverTask();
    CheckLoadProfile();
    DestroyRuntime();
}

TEST_F(ProfileSaverTest, ProfilingSaverAddFailOfTimeTest)
{
    InitPGOFilePath("profiling_saver_add_fail_of_time_test.ap");
    const uint32_t largeSleepingTime = 10000000;
    SetProfilesaverSleepingTimeMs(largeSleepingTime);
    SetIncrementalProfilesaverEnabled(true);
    CreateRuntime();
    RunPandafile();
    EXPECT_TRUE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saver = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    EXPECT_NE(saver, nullptr);
    EXPECT_FALSE(saver->TryAddTask());
    CheckLoadProfileFail();
    DestroyRuntime();
}

TEST_F(ProfileSaverTest, ProfilingSaverDisabledWithOption)
{
    InitPGOFilePath("profiling_saver_disabled_with_option.ap");
    SetIncrementalProfilesaverEnabled(false);
    CreateRuntime();
    RunPandafile();
    EXPECT_FALSE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saver = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    EXPECT_EQ(saver, nullptr);
    CheckLoadProfileFail();
    DestroyRuntime();
}

TEST_F(ProfileSaverTest, ProfilingSaverDisabledWithWorkerType)
{
    InitPGOFilePath("profiling_saver_disabled_with_worker_type.ap");
    SetWorkersType("threadpool");
    SetIncrementalProfilesaverEnabled(true);
    CreateRuntime();
    RunPandafile();
    EXPECT_TRUE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saver = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    EXPECT_EQ(saver, nullptr);
    CheckLoadProfileFail();
    DestroyRuntime();
}
}  // namespace ark::test
