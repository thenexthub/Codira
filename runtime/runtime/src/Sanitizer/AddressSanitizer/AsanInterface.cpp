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


#include "AsanInterface.h"

#include <cerrno>
#include <sys/mman.h>
#include <tuple>
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_vm.h>
#endif

#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Base/MemUtils.h"
#include "Common/TypeDef.h"
#include "Sanitizer/ArrayReferenceCounter.h"
#include "Sanitizer/SanitizerCompilerCalls.h"
#include "Sanitizer/SanitizerMacros.h"
#include "Sanitizer/SanitizerSymbols.h"
#include "schedule.h"

namespace MapleRuntime {
namespace Sanitizer {
using namespace std;
using FakeStackHandle = void*;

static ArrayReferenceCounter<bool>* g_counter = nullptr;
static intptr_t g_aliasOffset = 0;

// stores thread fake stack
// thread local ref: runtime/src/Mutator/ThreadLocal.cpp
#if defined(_WIN64)
static thread_local FakeStackHandle g_fakeStack;
#elif !defined(__clang__) && defined(__aarch64__)
__asm__(".section .maplert_tls, \"waT\"");
static __attribute__((section(".maplert_tls#"))) thread_local FakeStackHandle g_fakeStack;
#elif defined(__APPLE__)
static thread_local FakeStackHandle g_fakeStack;
#else
// g++ generates "awT" and "%progbits" for __attribute((section)), which causes "changed section
// attributes" warning (or error) at assembling stage. "#" suffix (depends on target architecture)
// could comment out the attribute g++ generates to suppress the "changed section attributes" warning.
__asm__(".section .maplert_tls, \"waT\"");
static __attribute__((section(".maplert_tls"))) thread_local FakeStackHandle g_fakeStack;
#endif

static inline void* GetMemoryAlias(void* addr)
{
    return reinterpret_cast<void*>(reinterpret_cast<intptr_t>(addr) + g_aliasOffset);
}

static inline void* GetMemoryFromAlias(void* alias)
{
    return reinterpret_cast<void*>(reinterpret_cast<intptr_t>(alias) - g_aliasOffset);
}

static void* CreateHeapMemoryAlias(void* addr, size_t size)
{
    auto baseAddr = reinterpret_cast<uintptr_t>(addr);
    uintptr_t baseEnd = baseAddr + reinterpret_cast<uintptr_t>(size);
    void* alias = nullptr;
#ifdef __APPLE__
    vm_prot_t cur;
    vm_prot_t max;
    kern_return_t ret = mach_vm_remap(
        // dest
        mach_task_self(), reinterpret_cast<mach_vm_address_t*>(&alias),
        reinterpret_cast<mach_vm_size_t>(size),
        // addr mask for alignment
        0,
        // VM_FLAGS_ANYWHERE: map to anywhere where is available
        // VM_FLAGS_RETURN_DATA_ADDR: make alias return the real address
        VM_FLAGS_ANYWHERE | VM_FLAGS_RETURN_DATA_ADDR,
        // source
        mach_task_self(), reinterpret_cast<mach_vm_address_t>(addr),
        // whether to make a copy-on-write (true) mapping or a shared (false) mapping.
        FALSE,
        // r/w/x permission, make it default
        &cur, &max,
        // inherit from source
        VM_INHERIT_DEFAULT);
    CHECK_DETAIL(ret == KERN_SUCCESS, "Create heap memory alias from [0x%lx, 0x%lx] failed (%d): %s",
                 baseAddr, baseEnd, ret, mach_error_string(ret));
#else
    // old size == 0 means we don't actually unmap the old addr, just create an alias for it
    alias = mremap(addr, 0, size, MREMAP_MAYMOVE, 0);
    CHECK_DETAIL(alias != MAP_FAILED, "Create heap memory alias from [0x%lx, 0x%lx] failed (%d): %s",
                 baseAddr, baseEnd, errno, std::strerror(errno));
#endif
    DLOG(SANITIZER, "Create heap memory alias at range (0x%lx, 0x%lx) for heap(0x%lx, 0x%lx)",
         reinterpret_cast<uintptr_t>(alias), reinterpret_cast<uintptr_t>(alias) + reinterpret_cast<uintptr_t>(size),
         baseAddr, baseEnd);
    return alias;
}

void OnHeapAllocated(void* addr, size_t size)
{
    // 1. create heap memory alias to avoid FP
    auto alias = CreateHeapMemoryAlias(addr, size);
    g_aliasOffset = reinterpret_cast<intptr_t>(alias) - reinterpret_cast<intptr_t>(addr);

    // 2. allocate reference counter for array
    g_counter = new ArrayReferenceCounter<bool>(alias, size);
    CHECK_DETAIL(g_counter != nullptr, "Array Reference Counter allocation failed.");

    // 3. all checks are performed on alias
    DLOG(SANITIZER, "register memory alias to asan runtime");
    REAL(__asan_poison_memory_region)(alias, size);
    REAL(__lsan_register_root_region)(alias, size);
}

static void DeleteHeapMemoryAlias(void* addr, size_t size)
{
    // mac platform remap don't have unmap interface for mach vm
#ifndef __APPLE__
    void* alias = GetMemoryAlias(addr);
    CHECK_DETAIL(munmap(alias, size) == 0, "Delete heap memory alias failed: %s", std::strerror(errno));
#endif
    return;
}

void OnHeapDeallocated(void* addr, size_t size)
{
    // 3. unpoison alias before lsan check, otherwise it will be an FP
    auto alias = GetMemoryAlias(addr);
    REAL(__asan_unpoison_memory_region)(alias, size);
    REAL(__lsan_do_leak_check)();

    // 2. check and de-allocate counter
    if (!g_counter->Empty()) {
        auto logger = [](uintptr_t addr, pair<int64_t, bool> data) {
            Logger::GetLogger().FormatLog(RTLOG_FAIL, true,
                                          "Unreleased array: 0x%lx (real addr: %p) with refcount %ld", addr,
                                          GetMemoryFromAlias(reinterpret_cast<void*>(addr)), data.first);
        };
        g_counter->DiagnoseUnreleased(logger);
        delete g_counter;
        g_counter = nullptr;
        DeleteHeapMemoryAlias(addr, size);

        Logger::GetLogger().FormatLog(RTLOG_FATAL, true, "Detect un-released array");
        BUILTIN_UNREACHABLE();
    }
    delete g_counter;
    g_counter = nullptr;

    // 1. remove heap memory alias
    DeleteHeapMemoryAlias(addr, size);
}

void OnHeapMadvise(void* addr, size_t size)
{
    auto alias = GetMemoryAlias(addr);
    if (!g_counter->AddressInRange(reinterpret_cast<uintptr_t>(alias))) {
        return;
    }

    // we have to clear out on addr, memset on alias will trigger asan check failed
    MapleRuntime::MemorySet(reinterpret_cast<uintptr_t>(addr), size, 0, size);
    (void)madvise(alias, size, MADV_DONTNEED);
}

void HandleNoReturn(uint64_t from, uint64_t to)
{
    DLOG(SANITIZER, "Handle no return stack range (0x%lx, 0x%lx)", from, to);
    if (from >= to) {
        return;
    }

    // from is old rsp, to is new rsp, to should greater than from
    uint64_t size = to - from;
    REAL(__asan_unpoison_memory_region)(reinterpret_cast<void*>(from), size);
}

void* ArrayAcquireMemoryRegion(ArrayRef, void* addr, size_t size)
{
    auto alias = GetMemoryAlias(addr);
    g_counter->Lock(reinterpret_cast<uintptr_t>(alias));
    int64_t count = g_counter->CounterAddOrInsert(reinterpret_cast<uintptr_t>(alias), false).first;
    if (count == 1) {
        REAL(__asan_unpoison_memory_region)(alias, size);
    }
    g_counter->Unlock(reinterpret_cast<uintptr_t>(alias));
    DLOG(SANITIZER, "acquire array(%p), alias[%p], refcount[%ld]", addr, alias, count);
    return alias;
}

void* ArrayReleaseMemoryRegion(ArrayRef, void* alias, size_t size)
{
    void* addr = GetMemoryFromAlias(alias);
    g_counter->Lock(reinterpret_cast<uintptr_t>(alias));
    int64_t count = g_counter->CounterSubOrDelete(reinterpret_cast<uintptr_t>(alias)).first;
    if (count == 0) {
        REAL(__asan_poison_memory_region)(alias, size);
    }
    g_counter->Unlock(reinterpret_cast<uintptr_t>(alias));
    DLOG(SANITIZER, "release array(%p), alias[%p], refcount[%ld]", addr, alias, count);
    return addr;
}

void AsanStartSwitchThreadContext(void* oldThread, void* newThread)
{
    void* fakeStack = nullptr;
    auto t = reinterpret_cast<struct CODEThread*>(newThread);
    void* bottom = CODEThreadStackAddrGetByCODEThrd(t);
    uintptr_t size = CODEThreadStackSizeGetByCODEThrd(t);
    REAL(__sanitizer_start_switch_fiber)(&fakeStack, bottom, size);
    CODEThreadSetSanitizerContext(oldThread, fakeStack);
}

void AsanEndSwitchThreadContext(void* newThread)
{
    // just apply the new stack to asan thread
    // we don't need to write back from asan thread since we already have it,
    // therefore we just pass nullptr here
    REAL(__sanitizer_finish_switch_fiber)(CODEThreadGetSanitizerContext(newThread), nullptr, nullptr);
}

void AsanEnterCODEThread(void* thread)
{
    void* addr = CODEThreadStackAddrGet();
    size_t size = CODEThreadStackSizeGet();
    void* threadFakeStack = nullptr;
    REAL(__sanitizer_start_switch_fiber)(&threadFakeStack, addr, size);
    // don't need pthread stack, we can get it from pthread_attr_getstack at exit
    REAL(__sanitizer_finish_switch_fiber)(CODEThreadGetSanitizerContext(CODEThreadGetHandle()), nullptr, nullptr);
    g_fakeStack = threadFakeStack;
}

void AsanExitCODEThread(void* thread)
{
    void* addr = nullptr;
    size_t size = 0;
    pthread_attr_t attr;
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_getstack(&attr, &addr, &size);
    (void)pthread_attr_destroy(&attr);
    // passing nullptr indicates the fiber will die, delete the fake stack
    REAL(__sanitizer_start_switch_fiber)(nullptr, addr, size);
    REAL(__sanitizer_finish_switch_fiber)(g_fakeStack, nullptr, nullptr);
    g_fakeStack = nullptr;
}

void AsanRead(volatile const void* addr, uintptr_t size)
{
    REAL(__asan_loadN)(addr, size);
}

void AsanWrite(volatile const void* addr, uintptr_t size)
{
    REAL(__asan_storeN)(addr, size);
}

extern "C" {
MRT_EXPORT void CODE_MCC_AsanRead(volatile const void* addr, uintptr_t size)
{
    REAL(__asan_loadN)(addr, size);
}

MRT_EXPORT void CODE_MCC_AsanWrite(volatile const void* addr, uintptr_t size)
{
    REAL(__asan_storeN)(addr, size);
}

MRT_EXPORT void CODE_MCC_AsanHandleNoReturn(const void* rsp)
{
    void* stackAddr = CODEThreadStackAddrGet();
    if (stackAddr == nullptr) {
        REAL(__asan_handle_no_return)(rsp);
    } else {
        Sanitizer::HandleNoReturn(reinterpret_cast<uint64_t>(stackAddr), reinterpret_cast<uint64_t>(rsp));
    }
}
}
} // namespace Sanitizer
} // namespace MapleRuntime
