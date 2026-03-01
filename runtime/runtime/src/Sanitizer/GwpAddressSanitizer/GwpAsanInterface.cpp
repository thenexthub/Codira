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

#include "GwpAsanInterface.h"

#include <climits>
#include <random>
#include <set>

#include "Base/Log.h"
#include "Base/SpinLock.h"
#include "Heap/Allocator/Allocator.h"
#include "ObjectModel/MArray.inline.h"
#include "Sanitizer/SanitizerCompilerCalls.h"
#include "securec.h"

namespace MapleRuntime {
namespace Sanitizer {
using namespace std;

static bool g_gwpEnabled = false;

// sampling config
static long int g_samplingRate = 5000;
static atomic_uint g_counter = { 0 };

// canary logger, <addr, size> pair
static std::map<void*, uint64_t>* g_canary;
static SpinLock g_lock;

static void PrintGwpAsanHelpMessage()
{
    PRINT_INFO("Available flags for cangjie GWP-ASan:\n");
    PRINT_INFO("\tcodeEnableGwpAsan\n");
    PRINT_INFO("\t\t- If true, enable GWP-ASan for detect array overflow and acquire leak. (Current Value: %s)\n",
               g_gwpEnabled ? "true" : "false");

    PRINT_INFO("\tcodeGwpAsanSampleRate\n");
    PRINT_INFO(
        "\t\t- The probability (1 / SampleRate) that an array which "
        "is acquired by using std.core.acquireArrayRawData is selected for GWP-ASan sampling.\n"
        "\t\t  Default is 5000. Sample rates up to (2^31 - 1) are supported. (Current Value: %lu)\n",
        g_samplingRate);

    PRINT_INFO("\tcodeGwpAsanHelp\n");
    PRINT_INFO("\t\t- Print the flag descriptions. (Current Value: true)\n");
}

void SetupGwpAsanAsNeeded()
{
    auto enabled = std::getenv("codeEnableGwpAsan");
    if (enabled != nullptr) {
        g_gwpEnabled = CString::ParseFlagFromEnv(enabled);
    }

    auto sampling = std::getenv("codeGwpAsanSampleRate");
    if (sampling != nullptr) {
        char* pEnd{};
        auto val = std::strtol(sampling, &pEnd, 10);
        // not a negative number, full number, overflow
        if (sampling[0] == '-' || *pEnd != '\0' || val <= 0 || val > INT_MAX || errno == ERANGE) {
            LOG(RTLOG_FATAL, "Unsupported codeGwpAsanSampleRate parameter. Valid sample rates range is (0, 2^31 - 1].\n");
            BUILTIN_UNREACHABLE();
        }
        g_samplingRate = static_cast<int>(val);
    }

    // to show the configured value correctly, this must be the last one to do
    auto help = std::getenv("codeGwpAsanHelp");
    if (help != nullptr && CString::ParseFlagFromEnv(help)) {
        PrintGwpAsanHelpMessage();
    }
}

void OnHeapAllocated(void*, size_t)
{
    if (UNLIKELY(g_gwpEnabled)) {
        g_canary = new (std::nothrow) std::map<void*, uint64_t>();
        CHECK_DETAIL(g_canary != nullptr, "gwpasan metadata allocation failed.");
    }
}

void OnHeapDeallocated(void*, size_t)
{
    if (LIKELY(!g_gwpEnabled)) {
        return;
    }

    if (!g_canary->empty()) {
        for (auto array : *g_canary) {
            Logger::GetLogger().FormatLog(RTLOG_FAIL, true, "Unreleased array: %p", array.first);
        }
        delete g_canary;
        g_canary = nullptr;

        Logger::GetLogger().FormatLog(RTLOG_FATAL, true, "Detect un-released array");
        BUILTIN_UNREACHABLE();
    }

    delete g_canary;
    g_canary = nullptr;
}

static void CheckCanary(void* addr, size_t size, uint64_t expect)
{
    auto remainSize = size - AlignDown(size, Allocator::ALLOC_ALIGN);
    uint8_t padSize = remainSize == 0 ? 0 : static_cast<uint8_t>(Allocator::ALLOC_ALIGN - remainSize);
    DLOG(SANITIZER, "gwpasan check array(%p): head canary: 0x%lx, tail canary: 0x%01x", addr, size, padSize);

    // check head canary
    if (size != expect) {
        LOG(RTLOG_FAIL, "Gwp-Asan sanity check failed on raw array addr %p", addr);
        LOG(RTLOG_FAIL, "Head canary (array[-1]) mismatch: expect: 0x%lx, actual: 0x%lx", expect, size);
        LOG(RTLOG_FATAL, "Gwp-Asan Aborted.");
        BUILTIN_UNREACHABLE();
    }

    // array is aligned, no space for tail canary
    if (remainSize == 0) {
        return;
    }

    // array is not aligned, check tail canary
    uint8_t* padding = reinterpret_cast<uint8_t*>(addr) + size;
    for (size_t i = 0; i < padSize; i++) {
        if (UNLIKELY(padding[i] != padSize)) {
            LOG(RTLOG_FAIL, "Gwp-Asan sanity check failed on raw array addr %p", addr);
            LOG(RTLOG_FAIL, "Tail canary (array[size+%d]) mismatch: expect: 0x%01x, actual: 0x%01x",
                i, padSize, padding[i]);
            LOG(RTLOG_FATAL, "Gwp-Asan Aborted.");
            BUILTIN_UNREACHABLE();
        }
    }
}

void* ArrayAcquireMemoryRegion(ArrayRef, void* addr, size_t size)
{
    if (LIKELY(!g_gwpEnabled)) {
        return addr;
    }

    // sample only on rate match
    if (LIKELY(g_counter.fetch_add(1) % g_samplingRate != 0)) {
        return addr;
    }

    g_lock.Lock();
    // if found, we just check out canary, rather than generate again
    auto canary = g_canary->find(addr);
    if (canary != g_canary->end()) {
        CheckCanary(addr, size, canary->second);
        g_lock.Unlock();
        return addr;
    }

    // store canary
    (void)g_canary->emplace(addr, size);
    LOG(RTLOG_INFO, "Gwp-Asan acquires array [%p]. Current sampled array count: %zu", addr, g_canary->size());

    // array is aligned, no for tail canary
    auto remainSize = size - AlignDown(size, Allocator::ALLOC_ALIGN);
    if (remainSize == 0) {
        g_lock.Unlock();
        DLOG(SANITIZER, "gwpasan acquire array(%p): head canary: 0x%lx, tail canary: 0x0", addr, size);
        return addr;
    }

    // array is not aligned, generate a tail canary
    auto padSize = static_cast<uint8_t>(Allocator::ALLOC_ALIGN - remainSize);
    CHECK_DETAIL(memset_s(reinterpret_cast<uint8_t*>(addr) + AlignDown(size, Allocator::ALLOC_ALIGN) + remainSize,
        padSize, padSize, padSize) == EOK, "array padding memset failed.");
    g_lock.Unlock();
    DLOG(SANITIZER, "gwpasan acquire array(%p): head canary: 0x%lx, tail canary: 0x%01x", addr, size, padSize);
    return addr;
}

void* ArrayReleaseMemoryRegion(ArrayRef, void* addr, size_t size)
{
    if (LIKELY(!g_gwpEnabled)) {
        return addr;
    }

    g_lock.Lock();
    auto canary = g_canary->find(addr);
    if (LIKELY(canary == g_canary->end())) {
        g_lock.Unlock();
        return addr;
    }

    CheckCanary(addr, size, canary->second);
    g_canary->erase(canary);
    LOG(RTLOG_INFO, "Gwp-Asan releases array [%p]. Current sampled array count: %zu", addr, g_canary->size());
    g_lock.Unlock();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    auto remainSize = size - AlignDown(size, Allocator::ALLOC_ALIGN);
    uint8_t padSize = remainSize == 0 ? 0 : static_cast<uint8_t>(Allocator::ALLOC_ALIGN - remainSize);
    DLOG(SANITIZER, "gwpasan release array(%p): head canary: 0x%lx, tail canary: 0x%01x", addr, size, padSize);
#endif
    return addr;
}
} // namespace Sanitizer
} // namespace MapleRuntime
