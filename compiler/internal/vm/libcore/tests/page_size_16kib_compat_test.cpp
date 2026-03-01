/*
 * Copyright (C) 2024 The Android Open Source Project
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

#if __has_include (<android/dlext_private.h>)
#define IS_ANDROID_DL
#endif

#include "page_size_compat_helpers.h"

#include <android-base/parsebool.h>
#include <android-base/properties.h>
#include <android-base/silent_death_test.h>

using PageSize16KiBCompatTest_DeathTest = SilentDeathTest;
using ::android::base::ParseBool;
using ::android::base::ParseBoolResult;

#if defined(IS_ANDROID_DL)
#include <android/dlext_private.h>
#endif

static inline std::string CompatMode() {
  return android::base::GetProperty("bionic.linker.16kb.app_compat.enabled", "false");
}

static inline bool CompatModeDisabled(const std::string& compat_mode) {
  return ParseBool(compat_mode) == ParseBoolResult::kFalse;
}

static inline bool CompatModeEnabled(const std::string& compat_mode) {
  return ParseBool(compat_mode) == ParseBoolResult::kTrue;
}

static inline bool CompatModeFatal(const std::string& compat_mode) {
  return compat_mode == "fatal";
}

static inline std::string TestLibPath() {
  return GetTestLibRoot() + "/libtest_elf_max_page_size_4kib.so";
}

TEST(PageSize16KiBCompatTest, ElfAlignment4KiB_LoadElf) {
  if (getpagesize() != 0x4000) {
    GTEST_SKIP() << "This test is only applicable to 16kB page-size devices";
  }

  std::string compat_mode = CompatMode();
  if (CompatModeFatal(compat_mode)) {
    GTEST_SKIP() << "This test is only applicable if dlopen() errors are not fatal";
  }

  std::string lib = TestLibPath();
  void* handle = nullptr;

  OpenTestLibrary(lib, CompatModeDisabled(compat_mode), &handle);

  if (CompatModeEnabled(compat_mode)) CallTestFunction(handle);
}

TEST(PageSize16KiBCompatTest, ElfAlignment4KiB_NonAdajacentWritableSegments_LoadElf) {
  if (getpagesize() != 0x4000) {
    GTEST_SKIP() << "This test is only applicable to 16kB page-size devices";
  }

  std::string lib = GetPrebuiltElfDir() + "/libtest_invalid-rw_rx_rw_load_segments.so";
  std::string compat_mode = CompatMode();
  void* handle = nullptr;

  OpenTestLibrary(lib, CompatModeDisabled(compat_mode), &handle);
}

TEST(PageSize16KiBCompatTest,
     ElfAlignment4KiB_ExecutalbeSegmentsSeparatedByWritableSegment_LoadElf) {
  if (getpagesize() != 0x4000) {
    GTEST_SKIP() << "This test is only applicable to 16kB page-size devices";
  }

  std::string lib = GetPrebuiltElfDir() + "/libtest_invalid-rw_rx_rw_load_segments.so";
  std::string compat_mode = CompatMode();
  void* handle = nullptr;

  OpenTestLibrary(lib, CompatModeDisabled(compat_mode), &handle);
}

TEST(PageSize16KiBCompatTest, ElfAlignment4KiB_LoadElf_perAppOption) {
  if (getpagesize() != 0x4000) {
    GTEST_SKIP() << "This test is only applicable to 16kB page-size devices";
  }

  std::string compat_mode = CompatMode();
  if (CompatModeFatal(compat_mode)) {
    GTEST_SKIP() << "This test is only applicable if dlopen() errors are not fatal";
  }

#if defined(IS_ANDROID_DL)
  android_set_16kb_appcompat_mode(true);
#endif

  std::string lib = TestLibPath();
  void* handle = nullptr;

  OpenTestLibrary(lib, false /*should_fail*/, &handle);
  CallTestFunction(handle);

#if defined(IS_ANDROID_DL)
  android_set_16kb_appcompat_mode(false);
#endif
}

static void FatalDlError() {
  std::string lib = TestLibPath();
  void* handle = nullptr;

  OpenTestLibrary(lib, true /*should_fail*/, &handle);
}

TEST(PageSize16KiBCompatTest_DeathTest, AppDlopenErrIsFatal) {
  if (getpagesize() != 0x4000) {
    GTEST_SKIP() << "This test is only applicable to 16kB page-size devices";
  }

  std::string compat_mode = CompatMode();
  if (!CompatModeFatal(compat_mode)) {
    GTEST_SKIP() << "This test is only applicable if dlopen() errors are fatal";
  }

  ASSERT_DEATH(FatalDlError(),
               ".*program alignment (.*) cannot be smaller than system page size.*");
}
