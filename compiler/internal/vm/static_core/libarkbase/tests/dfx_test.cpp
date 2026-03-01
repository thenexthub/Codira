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

#include "libarkbase/os/thread.h"
#include "libarkbase/utils/dfx.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/string_helpers.h"

#include <gtest/gtest.h>

namespace ark::test {

void MapDfxOption(std::map<DfxOptionHandler::DfxOption, uint8_t> &optionMap, DfxOptionHandler::DfxOption option)
{
    switch (option) {
#ifdef PANDA_TARGET_UNIX
        case DfxOptionHandler::COMPILER_NULLCHECK:
            optionMap[DfxOptionHandler::COMPILER_NULLCHECK] = 1;
            break;
        case DfxOptionHandler::SIGNAL_CATCHER:
            optionMap[DfxOptionHandler::SIGNAL_CATCHER] = 1;
            break;
        case DfxOptionHandler::SIGNAL_HANDLER:
            optionMap[DfxOptionHandler::SIGNAL_HANDLER] = 1;
            break;
        case DfxOptionHandler::ARK_SIGQUIT:
            optionMap[DfxOptionHandler::ARK_SIGQUIT] = 1;
            break;
        case DfxOptionHandler::ARK_SIGUSR1:
            optionMap[DfxOptionHandler::ARK_SIGUSR1] = 1;
            break;
        case DfxOptionHandler::ARK_SIGUSR2:
            optionMap[DfxOptionHandler::ARK_SIGUSR2] = 1;
            break;
        case DfxOptionHandler::MOBILE_LOG:
            optionMap[DfxOptionHandler::MOBILE_LOG] = 1;
            break;
#endif  // PANDA_TARGET_UNIX
        case DfxOptionHandler::REFERENCE_DUMP:
            optionMap[DfxOptionHandler::REFERENCE_DUMP] = 1;
            break;
        case DfxOptionHandler::DFXLOG:
            optionMap[DfxOptionHandler::DFXLOG] = 0;
            break;
        default:
            break;
    }
}

TEST(DfxController, Initialization)
{
    if (DfxController::IsInitialized()) {
        DfxController::Destroy();
    }
    EXPECT_FALSE(DfxController::IsInitialized());

    DfxController::Initialize();
    EXPECT_TRUE(DfxController::IsInitialized());

    DfxController::Destroy();
    EXPECT_FALSE(DfxController::IsInitialized());

    std::map<DfxOptionHandler::DfxOption, uint8_t> optionMap;
    for (auto option = DfxOptionHandler::DfxOption(0); option < DfxOptionHandler::END_FLAG;
         option = DfxOptionHandler::DfxOption(option + 1)) {
        MapDfxOption(optionMap, option);
    }

    DfxController::Initialize(optionMap);
    EXPECT_TRUE(DfxController::IsInitialized());

    DfxController::Destroy();
    EXPECT_FALSE(DfxController::IsInitialized());
}

TEST(DfxController, TestResetOptionValueFromString)
{
    if (DfxController::IsInitialized()) {
        DfxController::Destroy();
    }
    EXPECT_FALSE(DfxController::IsInitialized());

    DfxController::Initialize();
    EXPECT_TRUE(DfxController::IsInitialized());

    DfxController::ResetOptionValueFromString("dfx-log:1");
    EXPECT_EQ(DfxController::GetOptionValue(DfxOptionHandler::DFXLOG), 1U);

    DfxController::Destroy();
    EXPECT_FALSE(DfxController::IsInitialized());
}

TEST(DfxController, TestPrintDfxOptionValues)
{
    if (DfxController::IsInitialized()) {
        DfxController::Destroy();
    }
    EXPECT_FALSE(DfxController::IsInitialized());

    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::DFX));

    DfxController::Initialize();
    EXPECT_TRUE(DfxController::IsInitialized());

    testing::internal::CaptureStderr();

    DfxController::PrintDfxOptionValues();

    std::string err = testing::internal::GetCapturedStderr();
    uint32_t tid = os::thread::GetCurrentThreadId();
#ifdef PANDA_TARGET_UNIX
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string res = helpers::string::Format(
        "[TID %06x] E/dfx: DFX option: compiler-nullcheck, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: signal-catcher, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: signal-handler, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: sigquit, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: sigusr1, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: sigusr2, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: mobile-log, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: hung-update, option values: 0\n"
        "[TID %06x] E/dfx: DFX option: reference-dump, option values: 1\n"
        "[TID %06x] E/dfx: DFX option: dfx-log, option values: 0\n",
        tid, tid, tid, tid, tid, tid, tid, tid, tid, tid, tid);
#else
    std::string res = helpers::string::Format("[TID %06x] E/dfx: DFX option: dfx-log, option values: 0\n", tid, tid);
#endif
    EXPECT_EQ(err, res);

    Logger::Destroy();
    EXPECT_FALSE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::DFX));

    DfxController::Destroy();
    EXPECT_FALSE(DfxController::IsInitialized());
}

TEST(DfxController, TestSetDefaultOption)
{
    if (DfxController::IsInitialized()) {
        DfxController::Destroy();
    }
    EXPECT_FALSE(DfxController::IsInitialized());

    DfxController::Initialize();
    DfxController::Initialize();
    ASSERT_EQ(DfxController::GetOptionValue(DfxOptionHandler::COMPILER_NULLCHECK), 1);
}
}  // namespace ark::test
