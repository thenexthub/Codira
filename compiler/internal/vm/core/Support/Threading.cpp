//===-- toolchain/Support/Threading.cpp- Control multithreading mode --*- C++ -*-==//
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
//
// This file defines helper functions for running LLVM in a multi-threaded
// environment.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/Threading.h"
#include "vm/core/Config/config.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/Support/Jobserver.h"

#include <cassert>
#include <optional>
#include <stdlib.h>

using namespace vm::core;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#if LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))
uint64_t toolchain::get_threadid() { return 0; }

uint32_t toolchain::get_max_thread_name_length() { return 0; }

void toolchain::set_thread_name(const Twine &Name) {}

void toolchain::get_thread_name(SmallVectorImpl<char> &Name) { Name.clear(); }

toolchain::BitVector toolchain::get_thread_affinity_mask() { return {}; }

unsigned toolchain::ThreadPoolStrategy::compute_thread_count() const {
  // When threads are disabled, ensure clients will loop at least once.
  return 1;
}

// Unknown if threading turned off
int toolchain::get_physical_cores() { return -1; }

#else

static int computeHostNumHardwareThreads();

unsigned toolchain::ThreadPoolStrategy::compute_thread_count() const {
  if (UseJobserver)
    if (auto JS = JobserverClient::getInstance())
      return JS->getNumJobs();

  int MaxThreadCount =
      UseHyperThreads ? computeHostNumHardwareThreads() : get_physical_cores();
  if (MaxThreadCount <= 0)
    MaxThreadCount = 1;
  if (ThreadsRequested == 0)
    return MaxThreadCount;
  if (!Limit)
    return ThreadsRequested;
  return std::min((unsigned)MaxThreadCount, ThreadsRequested);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Threading.inc"
#endif
#ifdef _WIN32
#include "Windows/Threading.inc"
#endif

// Must be included after Threading.inc to provide definition for toolchain::thread
// because FreeBSD's condvar.h (included by user.h) misuses the "thread"
// keyword.
#include "vm/core/Support/thread.h"

#if defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
const std::optional<unsigned> toolchain::thread::DefaultStackSize = 8 * 1024 * 1024;
#elif defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
const std::optional<unsigned> toolchain::thread::DefaultStackSize = 4 * 1024 * 1024;
#else
const std::optional<unsigned> toolchain::thread::DefaultStackSize;
#endif


#endif

std::optional<ThreadPoolStrategy>
toolchain::get_threadpool_strategy(StringRef Num, ThreadPoolStrategy Default) {
  if (Num == "all")
    return toolchain::hardware_concurrency();
  if (Num.empty())
    return Default;
  unsigned V;
  if (Num.getAsInteger(10, V))
    return std::nullopt; // malformed 'Num' value
  if (V == 0)
    return Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = toolchain::hardware_concurrency();
  S.ThreadsRequested = V;
  return S;
}
