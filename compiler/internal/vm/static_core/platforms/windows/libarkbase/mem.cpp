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
#include "libarkbase/macros.h"
#include "libarkbase/utils/type_helpers.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/logger.h"

#include <limits>

#include <windows.h>
#include <cerrno>
#include <io.h>

#include <sysinfoapi.h>
#include <type_traits>

#define MAP_FAILED (reinterpret_cast<void *>(-1))

namespace ark::os::mem {

static int mem_errno(const DWORD err, const int deferr)
{
    return err == 0 ? deferr : err;
}

static DWORD mem_protection_flags_for_page(const int prot)
{
    DWORD flags = 0;

    if (static_cast<unsigned>(prot) == MMAP_PROT_NONE) {
        return flags;
    }

    if ((static_cast<unsigned>(prot) & MMAP_PROT_EXEC) != 0) {
        flags = ((static_cast<unsigned>(prot) & MMAP_PROT_WRITE) != 0) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    } else {
        flags = ((static_cast<unsigned>(prot) & MMAP_PROT_WRITE) != 0) ? PAGE_READWRITE : PAGE_READONLY;
    }

    return flags;
}

static DWORD mem_protection_flags_for_file(const int prot, const uint32_t mapFlags)
{
    DWORD flags = 0;
    if (prot == MMAP_PROT_NONE) {
        return flags;
    }

    /* Notice that only single FILE_MAP_COPY flag can ensure a copy-on-write mapping which
     * MMAP_FLAG_PRIVATE needs. It can't be bitwise OR'ed with FILE_MAP_ALL_ACCESS, FILE_MAP_READ
     * or FILE_MAP_WRITE. Or else it will be converted to PAGE_READONLY or PAGE_READWRITE, and make
     * the changes synced back to the original file.
     */
    if ((mapFlags & MMAP_FLAG_PRIVATE) != 0) {
        return FILE_MAP_COPY;
    }

    if ((static_cast<unsigned>(prot) & MMAP_PROT_READ) != 0) {
        flags |= FILE_MAP_READ;
    }
    if ((static_cast<unsigned>(prot) & MMAP_PROT_WRITE) != 0) {
        flags |= FILE_MAP_WRITE;
    }
    if ((static_cast<unsigned>(prot) & MMAP_PROT_EXEC) != 0) {
        flags |= FILE_MAP_EXECUTE;
    }

    return flags;
}

static DWORD mem_select_lower_bound(off_t off)
{
    using uoff_t = std::make_unsigned_t<off_t>;
    return (sizeof(off_t) <= sizeof(DWORD)) ? static_cast<DWORD>(off)
                                            : static_cast<DWORD>(static_cast<uoff_t>(off) & 0xFFFFFFFFL);
}

static DWORD mem_select_upper_bound(off_t off)
{
    constexpr uint32_t OFFSET_DWORD = 32;
    using uoff_t = std::make_unsigned_t<off_t>;
    return (sizeof(off_t) <= sizeof(DWORD))
               ? static_cast<DWORD>(0)
               : static_cast<DWORD>((static_cast<uoff_t>(off) >> OFFSET_DWORD) & 0xFFFFFFFFL);
}

// CC-OFFNXT(G.FUN.01) solid logic
void *mmap([[maybe_unused]] void *addr, size_t len, uint32_t prot, int flags, int fildes, off_t off)
{
    errno = 0;

    // Skip unsupported combinations of flags:
    if (len == 0 || (static_cast<unsigned>(flags) & MMAP_FLAG_FIXED) != 0 || prot == MMAP_PROT_EXEC) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    HANDLE h = ((static_cast<unsigned>(flags) & MMAP_FLAG_ANONYMOUS) == 0)
                   ? reinterpret_cast<HANDLE>(_get_osfhandle(fildes))
                   : INVALID_HANDLE_VALUE;
    if ((static_cast<unsigned>(flags) & MMAP_FLAG_ANONYMOUS) == 0 && h == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return MAP_FAILED;
    }

    const auto protPage = mem_protection_flags_for_page(prot);
    const off_t maxSize = off + static_cast<off_t>(len);
    const auto maxSizeLow = mem_select_lower_bound(maxSize);
    const auto maxSizeHigh = mem_select_upper_bound(maxSize);
    HANDLE fm = CreateFileMapping(h, nullptr, protPage, maxSizeHigh, maxSizeLow, nullptr);
    if (fm == nullptr) {
        errno = mem_errno(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    const auto protFile = mem_protection_flags_for_file(prot, flags);
    const auto fileOffLow = mem_select_lower_bound(off);
    const auto fileOffHigh = mem_select_upper_bound(off);
    void *map = MapViewOfFile(fm, protFile, fileOffHigh, fileOffLow, len);
    CloseHandle(fm);
    if (map == nullptr) {
        errno = mem_errno(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    return map;
}

int munmap(void *addr, [[maybe_unused]] size_t len)
{
    if (UnmapViewOfFile(addr)) {
        return 0;
    }

    errno = mem_errno(GetLastError(), EPERM);

    return -1;
}

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
    void *result = mmap(hint, mapSize, prot, flags, file.GetFd(), mapOffset);
    if (result == MAP_FAILED) {
        return BytePtr(nullptr, 0, MmapDeleter);
    }

    return BytePtr(static_cast<std::byte *>(result) + offset, size, MmapDeleter);
}

BytePtr MapExecuted(size_t size)
{
    // By design caller should pass valid size, so don't do any additional checks except ones that
    // mmap do itself
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    void *result = mmap(nullptr, size, MMAP_PROT_EXEC | MMAP_PROT_WRITE, MMAP_FLAG_SHARED | MMAP_FLAG_ANONYMOUS, -1, 0);
    if (result == reinterpret_cast<void *>(-1)) {
        result = nullptr;
    }

    return BytePtr(static_cast<std::byte *>(result), (result == nullptr) ? 0 : size, MmapDeleter);
}

std::optional<Error> MakeMemWithProtFlag(void *mem, size_t size, int prot)
{
    PDWORD old = nullptr;
    int r = VirtualProtect(mem, size, prot, old);
    if (r != 0) {
        return Error(GetLastError());
    }
    return {};
}

std::optional<Error> MakeMemReadExec(void *mem, size_t size)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return MakeMemWithProtFlag(mem, size, MMAP_PROT_EXEC | MMAP_PROT_READ);
}

std::optional<Error> MakeMemReadWrite(void *mem, size_t size)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return MakeMemWithProtFlag(mem, size, MMAP_PROT_WRITE | MMAP_PROT_READ);
}

std::optional<Error> MakeMemReadOnly(void *mem, size_t size)
{
    return MakeMemWithProtFlag(mem, size, MMAP_PROT_READ);
}

std::optional<Error> MakeMemProtected(void *mem, size_t size)
{
    return MakeMemWithProtFlag(mem, size, MMAP_PROT_NONE);
}

uint32_t GetPageSize()
{
    constexpr size_t PAGE_SIZE = 4096;
    return PAGE_SIZE;
}

static size_t GetCacheLineSizeFromOs()
{
    // NOLINTNEXTLINE(google-runtime-int)
    size_t lineSize = 0;
    DWORD bufferSize = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer = 0;

    GetLogicalProcessorInformation(0, &bufferSize);
#ifdef ENABLE_THIS_CODE_IN_FUTURE
    if (bufferSize == 0) {
        // malloc behavior for zero bytes is implementation defined
        // So, check it here
        LOG_IF(lineSize == 0, FATAL, RUNTIME) << "Can't get cache line size from OS";
        UNREACHABLE();
    }
#endif  // ENABLE_THIS_CODE_IN_FUTURE
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(bufferSize);
    GetLogicalProcessorInformation(&buffer[0], &bufferSize);

    for (i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            lineSize = buffer[i].Cache.LineSize;
            break;
        }
    }
    LOG_IF(lineSize == 0, FATAL, RUNTIME) << "Can't get cache line size from OS";

    free(buffer);
    return lineSize;
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
    void *result =
        mmap(nullptr, size, MMAP_PROT_READ | MMAP_PROT_WRITE, MMAP_FLAG_PRIVATE | MMAP_FLAG_ANONYMOUS, -1, 0);
    if (UNLIKELY(result == MAP_FAILED)) {
        result = nullptr;
    }
    if ((result != nullptr) && forcePoison) {
        ASAN_POISON_MEMORY_REGION(result, size);
    }

    return result;
}

std::optional<Error> PartiallyUnmapRaw([[maybe_unused]] void *mem, [[maybe_unused]] size_t size)
{
    // We can't partially unmap allocated memory
    // Because UnmapViewOfFile in win32 doesn't support to unmap
    // partial of the mapped memory
    return {};
}

void *MapRWAnonymousWithAlignmentRaw(size_t size, size_t aligmentInBytes, bool forcePoison)
{
    ASSERT(aligmentInBytes != 0);
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
        auto end_part = reinterpret_cast<void *>(alignedMem + size);
        PartiallyUnmapRaw(end_part, unusedInEnd);
    }
    return reinterpret_cast<void *>(alignedMem);
}

uintptr_t AlignDownToPageSize(uintptr_t addr)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    const size_t SYS_PAGE_SIZE = sysInfo.dwPageSize;
    addr &= ~(SYS_PAGE_SIZE - 1);
    return addr;
}

void *AlignedAlloc(size_t alignmentInBytes, size_t size)
{
    size_t alignedSize = (size + alignmentInBytes - 1) & ~(alignmentInBytes - 1);
    // aligned_alloc is not supported on MingW. instead we need to call _aligned_malloc.
    auto ret = _aligned_malloc(alignedSize, alignmentInBytes);
    // _aligned_malloc returns aligned pointer so just add assertion, no need to do runtime checks
    ASSERT_PRINT(reinterpret_cast<uintptr_t>(ret) == (reinterpret_cast<uintptr_t>(ret) & ~(alignmentInBytes - 1)),
                 "Address is not aligned");
    return ret;
}

void AlignedFree(void *mem)
{
    _aligned_free(mem);
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

std::optional<Error> TagAnonymousMemory([[maybe_unused]] const void *mem, [[maybe_unused]] size_t size,
                                        [[maybe_unused]] const char *tag)
{
    return {};
}

size_t GetNativeBytesFromMallinfo()
{
    return DEFAULT_NATIVE_BYTES_FROM_MALLINFO;
}

}  // namespace ark::os::mem
