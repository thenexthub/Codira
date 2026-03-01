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

#include "libarkbase/os/mem.h"
#include "libarkbase/cpu_features.h"
#include "libarkbase/utils/type_helpers.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/tsan_interface.h"

#include <limits>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include <type_traits>

#if defined(__GLIBC__) || defined(PANDA_TARGET_MOBILE)
#include <malloc.h>
#endif

namespace ark::os::mem {

void MmapDeleter(std::byte *ptr, size_t size) noexcept
{
    if (ptr != nullptr) {
        munmap(ptr, size);
    }
}

// CC-OFFNXT(G.FUN.01) solid logic
BytePtr MapFile(file::File file, uint32_t prot, uint32_t flags, size_t size, size_t fileOffset, void *hint)
{
    size_t mapOffset = RoundDown(fileOffset, GetPageSize());
    size_t offset = fileOffset - mapOffset;
    size_t mapSize = size + offset;
    void *result =
        mmap(hint, mapSize, static_cast<int>(prot), static_cast<int>(flags), file.GetFd(), static_cast<int>(mapOffset));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    if (result == MAP_FAILED) {
        return BytePtr(nullptr, 0, MmapDeleter);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return BytePtr(static_cast<std::byte *>(result) + offset, size, offset, MmapDeleter);
}

BytePtr MapExecuted(size_t size)
{
    // By design caller should pass valid size, so don't do any additional checks except ones that
    // mmap do itself
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    void *result = mmap(nullptr, size, PROT_EXEC | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        result = nullptr;
    }

    return BytePtr(static_cast<std::byte *>(result), (result == nullptr) ? 0 : size, MmapDeleter);
}

std::optional<Error> MakeMemWithProtFlag(void *mem, size_t size, int prot)
{
    int r = mprotect(mem, size, prot);
    if (r != 0) {
        return Error(errno);
    }
    return {};
}

std::optional<Error> MakeMemReadExec(void *mem, size_t size)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return MakeMemWithProtFlag(mem, size, PROT_EXEC | PROT_READ);
}

std::optional<Error> MakeMemReadWrite(void *mem, size_t size)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return MakeMemWithProtFlag(mem, size, PROT_WRITE | PROT_READ);
}

std::optional<Error> MakeMemReadOnly(void *mem, size_t size)
{
    return MakeMemWithProtFlag(mem, size, PROT_READ);
}

std::optional<Error> MakeMemProtected(void *mem, size_t size)
{
    return MakeMemWithProtFlag(mem, size, PROT_NONE);
}

uintptr_t AlignDownToPageSize(uintptr_t addr)
{
    const auto sysPageSize = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    addr &= ~(sysPageSize - 1);
    return addr;
}

void *AlignedAlloc(size_t alignmentInBytes, size_t size)
{
    size_t alignedSize = (size + alignmentInBytes - 1) & ~(alignmentInBytes - 1);
#if defined PANDA_TARGET_MOBILE || defined PANDA_TARGET_MACOS
    void *ret = nullptr;
    int r = posix_memalign(reinterpret_cast<void **>(&ret), alignmentInBytes, alignedSize);
    if (r != 0) {
        std::cerr << "posix_memalign failed, code: " << r << std::endl;
        ASSERT(0);
    }
#else
    auto ret = aligned_alloc(alignmentInBytes, alignedSize);
#endif
    ASSERT(reinterpret_cast<uintptr_t>(ret) == (reinterpret_cast<uintptr_t>(ret) & ~(alignmentInBytes - 1)));
    return ret;
}

void AlignedFree(void *mem)
{
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    std::free(mem);
}

static uint32_t GetPageSizeFromOs()
{
    // NOLINTNEXTLINE(google-runtime-int)
    long sz = sysconf(_SC_PAGESIZE);
    LOG_IF(sz == -1, FATAL, RUNTIME) << "Can't get page size from OS";
    return static_cast<uint32_t>(sz);
}

uint32_t GetPageSize()
{
    // NOLINTNEXTLINE(google-runtime-int)
    static uint32_t sz = GetPageSizeFromOs();
    return sz;
}

static size_t GetCacheLineSizeFromOs()
{
#if !defined(__MUSL__)
#ifdef __linux__
    // NOLINTNEXTLINE(google-runtime-int)
    long sz = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
#else
    long sz = 0;
    size_t length = sizeof(sz);
    sysctlbyname("hw.cachelinesize", &sz, &length, NULL, 0);
#endif
    LOG_IF(sz <= 0, FATAL, RUNTIME) << "Can't get cache line size from OS";
    return static_cast<uint32_t>(sz);
#else
    return ark::CACHE_LINE_SIZE;
#endif
}

size_t GetCacheLineSize()
{
    // NOLINTNEXTLINE(google-runtime-int)
    static size_t sz = GetCacheLineSizeFromOs();
    return sz;
}

void *MapRWAnonymousRaw(size_t size, bool forcePoison)
{
    ASSERT(size % GetPageSize() == 0);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    void *result = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        result = nullptr;
    }
    if ((result != nullptr) && forcePoison) {
        ASAN_POISON_MEMORY_REGION(result, size);
    }

    return result;
}

std::optional<Error> PartiallyUnmapRaw(void *mem, size_t size)
{
    // We can partially unmap memory on Unix systems via common unmap
    return UnmapRaw(mem, size);
}

void *MapRWAnonymousWithAlignmentRaw(size_t size, size_t aligmentInBytes, bool forcePoison)
{
    ASSERT(aligmentInBytes % GetPageSize() == 0);
    if (size == 0) {
        return nullptr;
    }
    void *result = MapRWAnonymousRaw(size + aligmentInBytes, forcePoison);
    if (result == nullptr) {
        return result;
    }
    auto allocatedMem = reinterpret_cast<uintptr_t>(result);
    uintptr_t alignedMem =
        (allocatedMem & ~(aligmentInBytes - 1U)) + ((allocatedMem % aligmentInBytes) != 0U ? aligmentInBytes : 0U);
    ASSERT(alignedMem >= allocatedMem);
    size_t unusedInStart = alignedMem - allocatedMem;
    ASSERT(unusedInStart <= aligmentInBytes);
    size_t unusedInEnd = aligmentInBytes - unusedInStart;
    if (unusedInStart != 0) {
        PartiallyUnmapRaw(result, unusedInStart);
    }
    if (unusedInEnd != 0) {
        auto endPart = reinterpret_cast<void *>(alignedMem + size);
        PartiallyUnmapRaw(endPart, unusedInEnd);
    }
    return reinterpret_cast<void *>(alignedMem);
}

void *MapRWAnonymousInFirst4GB(void *minMem, size_t size, [[maybe_unused]] size_t iterativeStep)
{
    ASSERT(ToUintPtr(minMem) % GetPageSize() == 0);
    ASSERT(size % GetPageSize() == 0);
    ASSERT(iterativeStep % GetPageSize() == 0);
#ifdef PANDA_TARGET_32
    void *resultAddr = mmap(minMem, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (resultAddr == MAP_FAILED) {
        return nullptr;
    }
#else
    if (ToUintPtr(minMem) >= HIGH_BOUND_32BIT_ADDRESS) {
        return nullptr;
    }
    if (ToUintPtr(minMem) + size > HIGH_BOUND_32BIT_ADDRESS) {
        return nullptr;
    }
    uintptr_t requestedAddr = ToUintPtr(minMem);
    for (; requestedAddr + size <= HIGH_BOUND_32BIT_ADDRESS; requestedAddr += iterativeStep) {
        void *mmapAddr =
            mmap(ToVoidPtr(requestedAddr), size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mmapAddr == MAP_FAILED) {
            continue;
        }
        if (mmapAddr == ToVoidPtr(requestedAddr)) {
            break;
        }
        if (munmap(mmapAddr, size) != 0) {
            return nullptr;
        }
    }
    if (requestedAddr + size > HIGH_BOUND_32BIT_ADDRESS) {
        return nullptr;
    }
    void *resultAddr = ToVoidPtr(requestedAddr);
#endif  // PANDA_TARGET_64
    ASAN_POISON_MEMORY_REGION(resultAddr, size);
    return resultAddr;
}

void *MapRWAnonymousFixedRaw(void *mem, size_t size, bool forcePoison)
{
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON) || defined(USE_THREAD_SANITIZER)
    // If this assert fails, please decrease the size of the memory for you program
    // or don't run it with ASAN or TSAN.
    LOG_IF((reinterpret_cast<uintptr_t>(mem) <= MMAP_FIXED_MAGIC_ADDR_FOR_SANITIZERS) &&
               ((reinterpret_cast<uintptr_t>(mem) + size) >= MMAP_FIXED_MAGIC_ADDR_FOR_SANITIZERS),
           FATAL, RUNTIME)
        << "Unable to mmap mem [" << reinterpret_cast<uintptr_t>(mem) << "] because of ASAN or TSAN";
#endif
    ASSERT(size % GetPageSize() == 0);
    void *result =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        mmap(mem, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (result == MAP_FAILED) {
        result = nullptr;
    }
    if ((result != nullptr) && forcePoison) {
        // If you have such an error here:
        // ==4120==AddressSanitizer CHECK failed:
        // ../../../../src/libsanitizer/asan/asan_mapping.h:303 "((AddrIsInMem(p))) != (0)" (0x0, 0x0)
        // Look at the big comment at the start of the method.
        ASAN_POISON_MEMORY_REGION(result, size);
    }

    return result;
}

std::optional<Error> UnmapRaw(void *mem, size_t size)
{
    ASAN_UNPOISON_MEMORY_REGION(mem, size);
    int res = munmap(mem, size);
    if (UNLIKELY(res == -1)) {
        return Error(errno);
    }

    return {};
}

#ifdef PANDA_TARGET_OHOS
#include <sys/prctl.h>

#ifndef PR_SET_VMA
constexpr int PR_SET_VMA = 0x53564d41;
#endif

#ifndef PR_SET_VMA_ANON_NAME
constexpr unsigned long PR_SET_VMA_ANON_NAME = 0;
#endif
#endif  // PANDA_TARGET_OHOS

std::optional<Error> TagAnonymousMemory([[maybe_unused]] const void *mem, [[maybe_unused]] size_t size,
                                        [[maybe_unused]] const char *tag)
{
#ifdef PANDA_TARGET_OHOS
    ASSERT(size % GetPageSize() == 0);
    ASSERT(reinterpret_cast<uintptr_t>(mem) % GetPageSize() == 0);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    int res = prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME,
                    // NOLINTNEXTLINE(google-runtime-int)
                    static_cast<unsigned long>(ToUintPtr(mem)), size,
                    // NOLINTNEXTLINE(google-runtime-int)
                    static_cast<unsigned long>(ToUintPtr(tag)));
    if (UNLIKELY(res == -1)) {
        return Error(errno);
    }
#endif  // PANDA_TARGET_OHOS
    return {};
}

size_t GetNativeBytesFromMallinfo()
{
    size_t mallinfoBytes;
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON)
    mallinfoBytes = DEFAULT_NATIVE_BYTES_FROM_MALLINFO;
    LOG(INFO, RUNTIME) << "Get native bytes from mallinfo with ASAN or TSAN. Return default value";
#else
#if defined(__GLIBC__) || defined(PANDA_TARGET_MOBILE)

    // For GLIBC, uordblks is total size of space which is allocated by malloc
    // For mobile libc, uordblks is total size of space which is allocated by malloc or mmap called by malloc for
    // non-small allocations
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 33
    struct mallinfo2 info = mallinfo2();
    mallinfoBytes = info.uordblks;
#else
    struct mallinfo info = mallinfo();
    mallinfoBytes = static_cast<unsigned int>(info.uordblks);
#endif  // __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 33

#if defined(__GLIBC__)

    // For GLIBC, hblkhd is total size of space which is allocated by mmap called by malloc for non-small allocations
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 33
    mallinfoBytes += info.hblkhd;
#else
    mallinfoBytes += static_cast<unsigned int>(info.hblkhd);
#endif  // __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 33

#endif  // __GLIBC__
#else
    mallinfoBytes = DEFAULT_NATIVE_BYTES_FROM_MALLINFO;
    LOG(INFO, RUNTIME) << "Get native bytes from mallinfo without GLIBC or mobile libc. Return default value";
#endif  // __GLIBC__ || PANDA_TARGET_MOBILE
#endif  // PANDA_ASAN_ON || PANDA_TSAN_ON
    // For ASAN or TSAN, return default value. For GLIBC, return uordblks + hblkhd. For mobile libc, return uordblks.
    // For other, return default value.
    return mallinfoBytes;
}

}  // namespace ark::os::mem
