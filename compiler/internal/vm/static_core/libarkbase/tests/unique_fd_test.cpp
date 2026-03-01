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

#include "libarkbase/os/unique_fd.h"

#include <gtest/gtest.h>
#include <utility>
#include <unistd.h>

namespace ark::os::unique_fd {

enum TestValue { DEFAULT_VALUE = -1L, STDIN_VALUE, STDOUT_VALUE, STDERR_VALUE };

struct DuplicateFD {
    // CHECKER_IGNORE_NEXTLINE(AF0005)
    // NOLINTNEXTLINE(android-cloexec-dup)
    int stdinValue = ::dup(STDIN_VALUE);
    // CHECKER_IGNORE_NEXTLINE(AF0005)
    // NOLINTNEXTLINE(android-cloexec-dup)
    int stdoutValue = ::dup(STDOUT_VALUE);
    // CHECKER_IGNORE_NEXTLINE(AF0005)
    // NOLINTNEXTLINE(android-cloexec-dup)
    int stferrValue = ::dup(STDERR_VALUE);
};

TEST(UniqueFd, Construct)
{
    DuplicateFD dupDf;
    auto fdA = UniqueFd();
    auto fdB = UniqueFd(dupDf.stdinValue);
    auto fdC = UniqueFd(dupDf.stdoutValue);
    auto fdD = UniqueFd(dupDf.stferrValue);

    EXPECT_EQ(fdA.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdB.Get(), dupDf.stdinValue);
    EXPECT_EQ(fdC.Get(), dupDf.stdoutValue);
    EXPECT_EQ(fdD.Get(), dupDf.stferrValue);

    auto fdE = std::move(fdA);
    auto fdF = std::move(fdB);
    auto fdG = std::move(fdC);
    auto fdH = std::move(fdD);

    // NOLINTBEGIN(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    EXPECT_EQ(fdA.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdB.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdC.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdD.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdE.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdF.Get(), dupDf.stdinValue);
    EXPECT_EQ(fdG.Get(), dupDf.stdoutValue);
    EXPECT_EQ(fdH.Get(), dupDf.stferrValue);
}

TEST(UniqueFd, Equal)
{
    DuplicateFD dupDf;
    auto fdA = UniqueFd();
    auto fdB = UniqueFd(dupDf.stdinValue);
    auto fdC = UniqueFd(dupDf.stdoutValue);
    auto fdD = UniqueFd(dupDf.stferrValue);

    auto fdE = UniqueFd();
    auto fdF = UniqueFd();
    auto fdG = UniqueFd();
    auto fdH = UniqueFd();
    fdE = std::move(fdA);
    fdF = std::move(fdB);
    fdG = std::move(fdC);
    fdH = std::move(fdD);

    EXPECT_EQ(fdA.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdB.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdC.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdD.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdE.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdF.Get(), dupDf.stdinValue);
    EXPECT_EQ(fdG.Get(), dupDf.stdoutValue);
    EXPECT_EQ(fdH.Get(), dupDf.stferrValue);
    // NOLINTEND(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
}

TEST(UniqueFd, Release)
{
    DuplicateFD dupDf;
    auto fdA = UniqueFd();
    auto fdB = UniqueFd(dupDf.stdinValue);
    auto fdC = UniqueFd(dupDf.stdoutValue);
    auto fdD = UniqueFd(dupDf.stferrValue);

    auto numA = fdA.Release();
    auto numB = fdB.Release();
    auto numC = fdC.Release();
    auto numD = fdD.Release();

    EXPECT_EQ(fdA.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdB.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdC.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdD.Get(), DEFAULT_VALUE);
    EXPECT_EQ(numA, DEFAULT_VALUE);
    EXPECT_EQ(numB, dupDf.stdinValue);
    EXPECT_EQ(numC, dupDf.stdoutValue);
    EXPECT_EQ(numD, dupDf.stferrValue);
}

TEST(UniqueFd, Reset)
{
    DuplicateFD dupDf;

    auto numA = DEFAULT_VALUE;
    auto numB = dupDf.stdinValue;
    auto numC = dupDf.stdoutValue;
    auto numD = dupDf.stferrValue;

    auto fdA = UniqueFd();
    auto fdB = UniqueFd();
    auto fdC = UniqueFd();
    auto fdD = UniqueFd();

    fdA.Reset(numA);
    fdB.Reset(numB);
    fdC.Reset(numC);
    fdD.Reset(numD);

    EXPECT_EQ(fdA.Get(), DEFAULT_VALUE);
    EXPECT_EQ(fdB.Get(), dupDf.stdinValue);
    EXPECT_EQ(fdC.Get(), dupDf.stdoutValue);
    EXPECT_EQ(fdD.Get(), dupDf.stferrValue);
}

}  // namespace ark::os::unique_fd
