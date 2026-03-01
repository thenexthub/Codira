//===-- SupportTest.cpp ---------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "lldb/Host/posix/Support.h"
#include "llvm/Support/Threading.h"
#include "gtest/gtest.h"

#if defined(_AIX)
#include "lldb/Host/aix/Support.h"
#elif defined(__linux__)
#include "lldb/Host/linux/Support.h"
#endif

using namespace lldb_private;

#ifndef __APPLE__
TEST(Support, getProcFile_Pid) {
  auto BufferOrError = getProcFile(getpid(), "status");
  ASSERT_TRUE(BufferOrError);
  ASSERT_TRUE(*BufferOrError);
}
#endif // #ifndef __APPLE__

#if (defined(_AIX) || defined(__linux__)) && defined(LLVM_ENABLE_THREADING)
TEST(Support, getProcFile_Tid) {
  auto BufferOrError = getProcFile(getpid(), llvm::get_threadid(),
#ifdef _AIX
                                   "lwpstatus"
#else
                                   "status"
#endif
  );
  ASSERT_TRUE(BufferOrError);
  ASSERT_TRUE(*BufferOrError);
}
#endif // #if (defined(_AIX) || defined(__linux__)) &&
       // defined(LLVM_ENABLE_THREADING)
