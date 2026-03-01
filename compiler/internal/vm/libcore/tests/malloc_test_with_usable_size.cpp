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

// (b/291762537): This code uses malloc_usable_size(), and thus can't be
// built with _FORTIFY_SOURCE>=3.
#if defined(_FORTIFY_SOURCE) && _FORTIFY_SOURCE >= 3
#undef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 2
#endif

#include <gtest/gtest.h>

#include <elf.h>
#include <limits.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/auxv.h>
#include <sys/cdefs.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <tinyxml2.h>

#include <android-base/file.h>
#include <android-base/test_utils.h>

#include "utils.h"

#if defined(__BIONIC__)

#include "SignalUtils.h"

#include "platform/bionic/dlext_namespaces.h"
#include "platform/bionic/malloc.h"
#include "platform/bionic/mte.h"
#include "platform/bionic/reserved_signals.h"
#include "private/bionic_config.h"

#define HAVE_REALLOCARRAY 1

#elif defined(__GLIBC__)

#define HAVE_REALLOCARRAY __GLIBC_PREREQ(2, 26)

#elif defined(ANDROID_HOST_MUSL)

#define HAVE_REALLOCARRAY 1

#endif

TEST(malloc, malloc_std) {
  // Simple malloc test.
  void* ptr = malloc(100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));
  free(ptr);
}

TEST(malloc, calloc_std) {
  // Simple calloc test.
  size_t alloc_len = 100;
  char* ptr = (char*)calloc(1, alloc_len);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(alloc_len, malloc_usable_size(ptr));
  for (size_t i = 0; i < alloc_len; i++) {
    ASSERT_EQ(0, ptr[i]);
  }
  free(ptr);
}

TEST(malloc, memalign_multiple) {
  SKIP_WITH_HWASAN << "hwasan requires power of 2 alignment";
  // Memalign test where the alignment is any value.
  for (size_t i = 0; i <= 12; i++) {
    for (size_t alignment = 1 << i; alignment < (1U << (i + 1)); alignment++) {
      char* ptr = reinterpret_cast<char*>(memalign(alignment, 100));
      ASSERT_TRUE(ptr != nullptr) << "Failed at alignment " << alignment;
      ASSERT_LE(100U, malloc_usable_size(ptr)) << "Failed at alignment " << alignment;
      ASSERT_EQ(0U, reinterpret_cast<uintptr_t>(ptr) % ((1U << i)))
          << "Failed at alignment " << alignment;
      free(ptr);
    }
  }
}

TEST(malloc, memalign_realloc) {
  // Memalign and then realloc the pointer a couple of times.
  for (size_t alignment = 1; alignment <= 4096; alignment <<= 1) {
    char* ptr = (char*)memalign(alignment, 100);
    ASSERT_TRUE(ptr != nullptr);
    ASSERT_LE(100U, malloc_usable_size(ptr));
    ASSERT_EQ(0U, (intptr_t)ptr % alignment);
    memset(ptr, 0x23, 100);

    ptr = (char*)realloc(ptr, 200);
    ASSERT_TRUE(ptr != nullptr);
    ASSERT_LE(200U, malloc_usable_size(ptr));
    ASSERT_TRUE(ptr != nullptr);
    for (size_t i = 0; i < 100; i++) {
      ASSERT_EQ(0x23, ptr[i]);
    }
    memset(ptr, 0x45, 200);

    ptr = (char*)realloc(ptr, 300);
    ASSERT_TRUE(ptr != nullptr);
    ASSERT_LE(300U, malloc_usable_size(ptr));
    for (size_t i = 0; i < 200; i++) {
      ASSERT_EQ(0x45, ptr[i]);
    }
    memset(ptr, 0x67, 300);

    ptr = (char*)realloc(ptr, 250);
    ASSERT_TRUE(ptr != nullptr);
    ASSERT_LE(250U, malloc_usable_size(ptr));
    for (size_t i = 0; i < 250; i++) {
      ASSERT_EQ(0x67, ptr[i]);
    }
    free(ptr);
  }
}

TEST(malloc, malloc_realloc_larger) {
  // Realloc to a larger size, malloc is used for the original allocation.
  char* ptr = (char*)malloc(100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));
  memset(ptr, 67, 100);

  ptr = (char*)realloc(ptr, 200);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(200U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(67, ptr[i]);
  }
  free(ptr);
}

TEST(malloc, malloc_realloc_smaller) {
  // Realloc to a smaller size, malloc is used for the original allocation.
  char* ptr = (char*)malloc(200);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(200U, malloc_usable_size(ptr));
  memset(ptr, 67, 200);

  ptr = (char*)realloc(ptr, 100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(67, ptr[i]);
  }
  free(ptr);
}

TEST(malloc, malloc_multiple_realloc) {
  // Multiple reallocs, malloc is used for the original allocation.
  char* ptr = (char*)malloc(200);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(200U, malloc_usable_size(ptr));
  memset(ptr, 0x23, 200);

  ptr = (char*)realloc(ptr, 100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(0x23, ptr[i]);
  }

  ptr = (char*)realloc(ptr, 50);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(50U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 50; i++) {
    ASSERT_EQ(0x23, ptr[i]);
  }

  ptr = (char*)realloc(ptr, 150);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(150U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 50; i++) {
    ASSERT_EQ(0x23, ptr[i]);
  }
  memset(ptr, 0x23, 150);

  ptr = (char*)realloc(ptr, 425);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(425U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 150; i++) {
    ASSERT_EQ(0x23, ptr[i]);
  }
  free(ptr);
}

TEST(malloc, calloc_realloc_larger) {
  // Realloc to a larger size, calloc is used for the original allocation.
  char* ptr = (char*)calloc(1, 100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));

  ptr = (char*)realloc(ptr, 200);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(200U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(0, ptr[i]);
  }
  free(ptr);
}

TEST(malloc, calloc_realloc_smaller) {
  // Realloc to a smaller size, calloc is used for the original allocation.
  char* ptr = (char*)calloc(1, 200);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(200U, malloc_usable_size(ptr));

  ptr = (char*)realloc(ptr, 100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(0, ptr[i]);
  }
  free(ptr);
}

TEST(malloc, calloc_multiple_realloc) {
  // Multiple reallocs, calloc is used for the original allocation.
  char* ptr = (char*)calloc(1, 200);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(200U, malloc_usable_size(ptr));

  ptr = (char*)realloc(ptr, 100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(100U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 100; i++) {
    ASSERT_EQ(0, ptr[i]);
  }

  ptr = (char*)realloc(ptr, 50);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(50U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 50; i++) {
    ASSERT_EQ(0, ptr[i]);
  }

  ptr = (char*)realloc(ptr, 150);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(150U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 50; i++) {
    ASSERT_EQ(0, ptr[i]);
  }
  memset(ptr, 0, 150);

  ptr = (char*)realloc(ptr, 425);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_LE(425U, malloc_usable_size(ptr));
  for (size_t i = 0; i < 150; i++) {
    ASSERT_EQ(0, ptr[i]);
  }
  free(ptr);
}

#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
extern "C" void* pvalloc(size_t);
#endif

TEST(malloc, pvalloc_std) {
#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
  size_t pagesize = sysconf(_SC_PAGESIZE);
  void* ptr = pvalloc(100);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_TRUE((reinterpret_cast<uintptr_t>(ptr) & (pagesize - 1)) == 0);
  ASSERT_LE(pagesize, malloc_usable_size(ptr));
  free(ptr);
#else
  GTEST_SKIP() << "pvalloc not supported.";
#endif
}

TEST(malloc, calloc_usable_size) {
  for (size_t size = 1; size <= 2048; size++) {
    void* pointer = malloc(size);
    ASSERT_TRUE(pointer != nullptr);
    memset(pointer, 0xeb, malloc_usable_size(pointer));
    free(pointer);

    // We should get a previous pointer that has been set to non-zero.
    // If calloc does not zero out all of the data, this will fail.
    uint8_t* zero_mem = reinterpret_cast<uint8_t*>(calloc(1, size));
    ASSERT_TRUE(pointer != nullptr);
    size_t usable_size = malloc_usable_size(zero_mem);
    for (size_t i = 0; i < usable_size; i++) {
      ASSERT_EQ(0, zero_mem[i]) << "Failed at allocation size " << size << " at byte " << i;
    }
    free(zero_mem);
  }
}

TEST(malloc, reallocarray) {
#if HAVE_REALLOCARRAY
  void* p = reallocarray(nullptr, 2, 32);
  ASSERT_TRUE(p != nullptr);
  ASSERT_GE(malloc_usable_size(p), 64U);
#else
  GTEST_SKIP() << "reallocarray not available";
#endif
}

TEST(malloc, mallinfo) {
#if defined(__BIONIC__) || defined(ANDROID_HOST_MUSL)
  SKIP_WITH_HWASAN << "hwasan does not implement mallinfo";
  static size_t sizes[] = {8, 32, 128, 4096, 32768, 131072, 1024000, 10240000, 20480000, 300000000};

  static constexpr size_t kMaxAllocs = 50;

  for (size_t size : sizes) {
    // If some of these allocations are stuck in a thread cache, then keep
    // looping until we make an allocation that changes the total size of the
    // memory allocated.
    // jemalloc implementations counts the thread cache allocations against
    // total memory allocated.
    void* ptrs[kMaxAllocs] = {};
    bool pass = false;
    for (size_t i = 0; i < kMaxAllocs; i++) {
      size_t allocated = mallinfo().uordblks;
      ptrs[i] = malloc(size);
      ASSERT_TRUE(ptrs[i] != nullptr);
      size_t new_allocated = mallinfo().uordblks;
      if (allocated != new_allocated) {
        size_t usable_size = malloc_usable_size(ptrs[i]);
        // Only check if the total got bigger by at least allocation size.
        // Sometimes the mallinfo numbers can go backwards due to compaction
        // and/or freeing of cached data.
        if (new_allocated >= allocated + usable_size) {
          pass = true;
          break;
        }
      }
    }
    for (void* ptr : ptrs) {
      free(ptr);
    }
    ASSERT_TRUE(pass) << "For size " << size << " allocated bytes did not increase after "
                      << kMaxAllocs << " allocations.";
  }
#else
  GTEST_SKIP() << "glibc is broken";
#endif
}

TEST(malloc, mallinfo2) {
#if defined(__BIONIC__) || defined(ANDROID_HOST_MUSL)
  SKIP_WITH_HWASAN << "hwasan does not implement mallinfo2";
  static size_t sizes[] = {8, 32, 128, 4096, 32768, 131072, 1024000, 10240000, 20480000, 300000000};

  static constexpr size_t kMaxAllocs = 50;

  for (size_t size : sizes) {
    // If some of these allocations are stuck in a thread cache, then keep
    // looping until we make an allocation that changes the total size of the
    // memory allocated.
    // jemalloc implementations counts the thread cache allocations against
    // total memory allocated.
    void* ptrs[kMaxAllocs] = {};
    bool pass = false;
    for (size_t i = 0; i < kMaxAllocs; i++) {
      struct mallinfo info = mallinfo();
      struct mallinfo2 info2 = mallinfo2();
      // Verify that mallinfo and mallinfo2 are exactly the same.
      ASSERT_EQ(static_cast<size_t>(info.arena), info2.arena);
      ASSERT_EQ(static_cast<size_t>(info.ordblks), info2.ordblks);
      ASSERT_EQ(static_cast<size_t>(info.smblks), info2.smblks);
      ASSERT_EQ(static_cast<size_t>(info.hblks), info2.hblks);
      ASSERT_EQ(static_cast<size_t>(info.hblkhd), info2.hblkhd);
      ASSERT_EQ(static_cast<size_t>(info.usmblks), info2.usmblks);
      ASSERT_EQ(static_cast<size_t>(info.fsmblks), info2.fsmblks);
      ASSERT_EQ(static_cast<size_t>(info.uordblks), info2.uordblks);
      ASSERT_EQ(static_cast<size_t>(info.fordblks), info2.fordblks);
      ASSERT_EQ(static_cast<size_t>(info.keepcost), info2.keepcost);

      size_t allocated = info2.uordblks;
      ptrs[i] = malloc(size);
      ASSERT_TRUE(ptrs[i] != nullptr);

      info = mallinfo();
      info2 = mallinfo2();
      // Verify that mallinfo and mallinfo2 are exactly the same.
      ASSERT_EQ(static_cast<size_t>(info.arena), info2.arena);
      ASSERT_EQ(static_cast<size_t>(info.ordblks), info2.ordblks);
      ASSERT_EQ(static_cast<size_t>(info.smblks), info2.smblks);
      ASSERT_EQ(static_cast<size_t>(info.hblks), info2.hblks);
      ASSERT_EQ(static_cast<size_t>(info.hblkhd), info2.hblkhd);
      ASSERT_EQ(static_cast<size_t>(info.usmblks), info2.usmblks);
      ASSERT_EQ(static_cast<size_t>(info.fsmblks), info2.fsmblks);
      ASSERT_EQ(static_cast<size_t>(info.uordblks), info2.uordblks);
      ASSERT_EQ(static_cast<size_t>(info.fordblks), info2.fordblks);
      ASSERT_EQ(static_cast<size_t>(info.keepcost), info2.keepcost);

      size_t new_allocated = info2.uordblks;
      if (allocated != new_allocated) {
        size_t usable_size = malloc_usable_size(ptrs[i]);
        // Only check if the total got bigger by at least allocation size.
        // Sometimes the mallinfo2 numbers can go backwards due to compaction
        // and/or freeing of cached data.
        if (new_allocated >= allocated + usable_size) {
          pass = true;
          break;
        }
      }
    }
    for (void* ptr : ptrs) {
      free(ptr);
    }
    ASSERT_TRUE(pass) << "For size " << size << " allocated bytes did not increase after "
                      << kMaxAllocs << " allocations.";
  }
#else
  GTEST_SKIP() << "glibc is broken";
#endif
}

// Jemalloc doesn't pass this test right now, so leave it as disabled.
TEST(malloc, DISABLED_alloc_after_fork) {
  // Both of these need to be a power of 2.
  static constexpr size_t kMinAllocationSize = 8;
  static constexpr size_t kMaxAllocationSize = 2097152;

  static constexpr size_t kNumAllocatingThreads = 5;
  static constexpr size_t kNumForkLoops = 100;

  std::atomic_bool stop;

  // Create threads that simply allocate and free different sizes.
  std::vector<std::thread*> threads;
  for (size_t i = 0; i < kNumAllocatingThreads; i++) {
    std::thread* t = new std::thread([&stop] {
      while (!stop) {
        for (size_t size = kMinAllocationSize; size <= kMaxAllocationSize; size <<= 1) {
          void* ptr;
          android::base::DoNotOptimize(ptr = malloc(size));
          free(ptr);
        }
      }
    });
    threads.push_back(t);
  }

  // Create a thread to fork and allocate.
  for (size_t i = 0; i < kNumForkLoops; i++) {
    pid_t pid;
    if ((pid = fork()) == 0) {
      for (size_t size = kMinAllocationSize; size <= kMaxAllocationSize; size <<= 1) {
        void* ptr;
        android::base::DoNotOptimize(ptr = malloc(size));
        ASSERT_TRUE(ptr != nullptr);
        // Make sure we can touch all of the allocation.
        memset(ptr, 0x1, size);
        ASSERT_LE(size, malloc_usable_size(ptr));
        free(ptr);
      }
      _exit(10);
    }
    ASSERT_NE(-1, pid);
    AssertChildExited(pid, 10);
  }

  stop = true;
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }
}

void VerifyAllocationsAreZero(std::function<void*(size_t)> alloc_func, std::string function_name,
                              std::vector<size_t>& test_sizes, size_t max_allocations) {
  // Vector of zero'd data used for comparisons. Make it twice the largest size.
  std::vector<char> zero(test_sizes.back() * 2, 0);

  SCOPED_TRACE(testing::Message() << function_name << " failed to zero memory");

  for (size_t test_size : test_sizes) {
    std::vector<void*> ptrs(max_allocations);
    for (size_t i = 0; i < ptrs.size(); i++) {
      SCOPED_TRACE(testing::Message() << "size " << test_size << " at iteration " << i);
      ptrs[i] = alloc_func(test_size);
      ASSERT_TRUE(ptrs[i] != nullptr);
      size_t alloc_size = malloc_usable_size(ptrs[i]);
      ASSERT_LE(alloc_size, zero.size());
      ASSERT_EQ(0, memcmp(ptrs[i], zero.data(), alloc_size));

      // Set the memory to non-zero to make sure if the pointer
      // is reused it's still zero.
      memset(ptrs[i], 0xab, alloc_size);
    }
    // Free the pointers.
    for (size_t i = 0; i < ptrs.size(); i++) {
      free(ptrs[i]);
    }
    for (size_t i = 0; i < ptrs.size(); i++) {
      SCOPED_TRACE(testing::Message() << "size " << test_size << " at iteration " << i);
      ptrs[i] = malloc(test_size);
      ASSERT_TRUE(ptrs[i] != nullptr);
      size_t alloc_size = malloc_usable_size(ptrs[i]);
      ASSERT_LE(alloc_size, zero.size());
      ASSERT_EQ(0, memcmp(ptrs[i], zero.data(), alloc_size));
    }
    // Free all of the pointers later to maximize the chance of reusing from
    // the first loop.
    for (size_t i = 0; i < ptrs.size(); i++) {
      free(ptrs[i]);
    }
  }
}

// Verify that small and medium allocations are always zero.
// @CddTest = 9.7/C-4-1
TEST(malloc, zeroed_allocations_small_medium_sizes) {
#if !defined(__BIONIC__)
  GTEST_SKIP() << "Only valid on bionic";
#endif
  SKIP_WITH_HWASAN << "Only test system allocator, not hwasan allocator.";

  if (IsLowRamDevice()) {
    GTEST_SKIP() << "Skipped on low memory devices.";
  }

  constexpr size_t kMaxAllocations = 1024;
  std::vector<size_t> test_sizes = {16, 48, 128, 1024, 4096, 65536};
  VerifyAllocationsAreZero([](size_t size) -> void* { return malloc(size); }, "malloc", test_sizes,
                           kMaxAllocations);

  VerifyAllocationsAreZero([](size_t size) -> void* { return memalign(64, size); }, "memalign",
                           test_sizes, kMaxAllocations);

  VerifyAllocationsAreZero(
      [](size_t size) -> void* {
        void* ptr;
        if (posix_memalign(&ptr, 64, size) == 0) {
          return ptr;
        }
        return nullptr;
      },
      "posix_memalign", test_sizes, kMaxAllocations);
}

// Verify that large allocations are always zero.
// @CddTest = 9.7/C-4-1
TEST(malloc, zeroed_allocations_large_sizes) {
#if !defined(__BIONIC__)
  GTEST_SKIP() << "Only valid on bionic";
#endif
  SKIP_WITH_HWASAN << "Only test system allocator, not hwasan allocator.";

  if (IsLowRamDevice()) {
    GTEST_SKIP() << "Skipped on low memory devices.";
  }

  constexpr size_t kMaxAllocations = 20;
  std::vector<size_t> test_sizes = {1000000, 2000000, 3000000, 4000000};
  VerifyAllocationsAreZero([](size_t size) -> void* { return malloc(size); }, "malloc", test_sizes,
                           kMaxAllocations);

  VerifyAllocationsAreZero([](size_t size) -> void* { return memalign(64, size); }, "memalign",
                           test_sizes, kMaxAllocations);

  VerifyAllocationsAreZero(
      [](size_t size) -> void* {
        void* ptr;
        if (posix_memalign(&ptr, 64, size) == 0) {
          return ptr;
        }
        return nullptr;
      },
      "posix_memalign", test_sizes, kMaxAllocations);
}

// Verify that reallocs are zeroed when expanded.
// @CddTest = 9.7/C-4-1
TEST(malloc, zeroed_allocations_realloc) {
#if !defined(__BIONIC__)
  GTEST_SKIP() << "Only valid on bionic";
#endif
  SKIP_WITH_HWASAN << "Only test system allocator, not hwasan allocator.";

  if (IsLowRamDevice()) {
    GTEST_SKIP() << "Skipped on low memory devices.";
  }

  // Vector of zero'd data used for comparisons.
  constexpr size_t kMaxMemorySize = 131072;
  std::vector<char> zero(kMaxMemorySize, 0);

  constexpr size_t kMaxAllocations = 1024;
  std::vector<size_t> test_sizes = {16, 48, 128, 1024, 4096, 65536};
  // Do a number of allocations and set them to non-zero.
  for (size_t test_size : test_sizes) {
    std::vector<void*> ptrs(kMaxAllocations);
    for (size_t i = 0; i < kMaxAllocations; i++) {
      ptrs[i] = malloc(test_size);
      ASSERT_TRUE(ptrs[i] != nullptr);

      // Set the memory to non-zero to make sure if the pointer
      // is reused it's still zero.
      memset(ptrs[i], 0xab, malloc_usable_size(ptrs[i]));
    }
    // Free the pointers.
    for (size_t i = 0; i < kMaxAllocations; i++) {
      free(ptrs[i]);
    }
  }

  // Do the reallocs to a larger size and verify the rest of the allocation
  // is zero.
  constexpr size_t kInitialSize = 8;
  for (size_t test_size : test_sizes) {
    std::vector<void*> ptrs(kMaxAllocations);
    for (size_t i = 0; i < kMaxAllocations; i++) {
      ptrs[i] = malloc(kInitialSize);
      ASSERT_TRUE(ptrs[i] != nullptr);
      size_t orig_alloc_size = malloc_usable_size(ptrs[i]);

      ptrs[i] = realloc(ptrs[i], test_size);
      ASSERT_TRUE(ptrs[i] != nullptr);
      size_t new_alloc_size = malloc_usable_size(ptrs[i]);
      char* ptr = reinterpret_cast<char*>(ptrs[i]);
      ASSERT_EQ(0, memcmp(&ptr[orig_alloc_size], zero.data(), new_alloc_size - orig_alloc_size))
          << "realloc from " << kInitialSize << " to size " << test_size << " at iteration " << i;
    }
    for (size_t i = 0; i < kMaxAllocations; i++) {
      free(ptrs[i]);
    }
  }
}
