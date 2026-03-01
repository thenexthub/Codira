/*
 * Copyright (C) 2025 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <gtest/gtest.h>

#include <err.h>

#include <android-base/file.h>
#include <android-base/silent_death_test.h>
#include <android-base/test_utils.h>

using err_DeathTest = SilentDeathTest;

static std::string name() {
  return android::base::Basename(android::base::GetExecutablePath());
}

TEST(err_DeathTest, err) {
  ASSERT_EXIT({ errno = 0; err(22, "x%c", 'y'); },
              ::testing::ExitedWithCode(22), name() + ": xy: Success\n");
}

TEST(err_DeathTest, err_errno) {
  ASSERT_EXIT({ errno = EINVAL; err(22, "x%c", 'y'); },
              ::testing::ExitedWithCode(22), name() + ": xy: Invalid argument\n");
}

TEST(err_DeathTest, err_null_fmt) {
  ASSERT_EXIT({ errno = EINVAL; err(22, nullptr); },
              ::testing::ExitedWithCode(22), name() + ": Invalid argument\n");
}

TEST(err, errx) {
  ASSERT_EXIT({ errno = 0; errx(22, "x%c", 'y'); },
              ::testing::ExitedWithCode(22), name() + ": xy\n");
}

TEST(err, errx_null_format) {
  ASSERT_EXIT({ errno = EINVAL; errx(22, nullptr); },
              ::testing::ExitedWithCode(22), name() + ": \n");
}

TEST(err, warn) {
  CapturedStderr cap;
  errno = 0;
  warn("x%c", 'y');
  ASSERT_EQ(cap.str(), name() + ": xy: Success\n");
}

TEST(err, warn_errno) {
  CapturedStderr cap;
  errno = EINVAL;
  warn("x%c", 'y');
  ASSERT_EQ(cap.str(), name() + ": xy: Invalid argument\n");
}

TEST(err, warnx) {
  CapturedStderr cap;
  errno = EINVAL;
  warnx("x%c", 'y');
  ASSERT_EQ(cap.str(), name() + ": xy\n");
}
