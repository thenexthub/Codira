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

#include <link.h>
#include <dlfcn.h>
#include <cstdlib>

#include "mem_hooks.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/logger.h"

namespace ark::os::unix::mem_hooks {

size_t PandaHooks::allocViaStandard_ = 0;
bool PandaHooks::isActive_ = false;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
PandaHooks::AddrRange PandaHooks::ignoreCodeRange_;
void *(*PandaHooks::realMalloc_)(size_t) = nullptr;
void *(*PandaHooks::realMemalign_)(size_t, size_t) = nullptr;
void (*PandaHooks::realFree_)(void *) = nullptr;

int FindLibDwarfCodeRegion(dl_phdr_info *info, [[maybe_unused]] size_t size, void *data)
{
    auto arange = reinterpret_cast<PandaHooks::AddrRange *>(data);
    if (std::string_view(info->dlpi_name).find("libdwarf.so") != std::string_view::npos) {
        Span<const ElfW(Phdr)> phdrList(info->dlpi_phdr, info->dlpi_phnum);
        for (ElfW(Phdr) phdr : phdrList) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_X) != 0) {
                *arange = PandaHooks::AddrRange(info->dlpi_addr + phdr.p_vaddr, phdr.p_memsz);
                return 1;
            }
        }
    }
    return 0;
}

/* static */
void PandaHooks::Initialize()
{
    // libdwarf allocates a lot of memory using malloc internally.
    // Since this library is used only for debug purpose don't consider
    // malloc calls from this library.
    dl_iterate_phdr(FindLibDwarfCodeRegion, &PandaHooks::ignoreCodeRange_);
}

/* static */
void PandaHooks::SaveRealFunctions()
{
    realMalloc_ = reinterpret_cast<decltype(realMalloc_)>(
        dlsym(RTLD_NEXT, "malloc"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    // NOTE(audovichenko): #26275
    if (realMalloc_ == nullptr) {
        realMalloc_ = reinterpret_cast<decltype(realMalloc_)>(
            dlsym(RTLD_DEFAULT, "malloc"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    }
    realMemalign_ = reinterpret_cast<decltype(realMemalign_)>(
        dlsym(RTLD_NEXT, "memalign"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    // NOTE(audovichenko): #26275
    if (realMemalign_ == nullptr) {
        realMemalign_ = reinterpret_cast<decltype(realMemalign_)>(
            dlsym(RTLD_DEFAULT, "memalign"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    }
    realFree_ = reinterpret_cast<decltype(realFree_)>(
        dlsym(RTLD_NEXT, "free"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    // NOTE(audovichenko): #26275
    if (realFree_ == nullptr) {
        realFree_ = reinterpret_cast<decltype(realFree_)>(
            dlsym(RTLD_DEFAULT, "free"));  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    }
}

/* static */
bool PandaHooks::ShouldCountAllocation(const void *caller)
{
    return !ignoreCodeRange_.Contains(ToUintPtr(caller));
}

/* static */
void *PandaHooks::MallocHook(size_t size, const void *caller)
{
    if (ShouldCountAllocation(caller)) {
        allocViaStandard_ += size;
    }
    // tracking internal allocator is implemented by malloc, we would fail here with this option
#ifndef TRACK_INTERNAL_ALLOCATIONS
    if (allocViaStandard_ > MAX_ALLOC_VIA_STANDARD) {
        std::cerr << "Too many usage of standard allocations" << std::endl;
        abort();  // CC-OFF(G.STD.16) fatal error
    }
#endif                                 // TRACK_INTERNAL_ALLOCATIONS
    void *result = realMalloc_(size);  // NOLINT(cppcoreguidelines-no-malloc)
    if (UNLIKELY(result == nullptr)) {
        std::cerr << "Malloc error" << std::endl;
        abort();  // CC-OFF(G.STD.16) fatal error
    }
    return result;
}

/* static */
void *PandaHooks::MemalignHook(size_t alignment, size_t size, const void *caller)
{
    if (ShouldCountAllocation(caller)) {
        allocViaStandard_ += size;
    }
    // tracking internal allocator is implemented by malloc, we would fail here with this option
#ifndef TRACK_INTERNAL_ALLOCATIONS
    if (allocViaStandard_ > MAX_ALLOC_VIA_STANDARD) {
        std::cerr << "Too many usage of standard allocations" << std::endl;
        abort();  // CC-OFF(G.STD.16) fatal error
    }
#endif  // TRACK_INTERNAL_ALLOCATIONS
    void *result = realMemalign_(alignment, size);
    if (UNLIKELY(result == nullptr)) {
        std::cerr << "Align error" << std::endl;
        abort();  // CC-OFF(G.STD.16) fatal error
    }
    return result;
}

/* static */
void PandaHooks::FreeHook(void *ptr, [[maybe_unused]] const void *caller)
{
    realFree_(ptr);  // NOLINT(cppcoreguidelines-no-malloc)
}

/* static */
void PandaHooks::Enable()
{
#ifdef PANDA_USE_MEMORY_HOOKS
    isActive_ = true;
#endif  // PANDA_USE_MEMORY_HOOKS
}

/* static */
void PandaHooks::Disable()
{
#ifdef PANDA_USE_MEMORY_HOOKS
    isActive_ = false;
#endif  // PANDA_USE_MEMORY_HOOKS
}

}  // namespace ark::os::unix::mem_hooks

#ifdef PANDA_USE_MEMORY_HOOKS

// NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
void *malloc(size_t size) noexcept
{
    if (ark::os::mem_hooks::PandaHooks::realMalloc_ == nullptr) {
        ark::os::unix::mem_hooks::PandaHooks::SaveRealFunctions();
    }
    if (ark::os::mem_hooks::PandaHooks::IsActive()) {
        void *caller = __builtin_return_address(0);
        return ark::os::mem_hooks::PandaHooks::MallocHook(size, caller);
    }
    return ark::os::mem_hooks::PandaHooks::realMalloc_(size);
}

// NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
void *memalign(size_t alignment, size_t size) noexcept
{
    if (ark::os::mem_hooks::PandaHooks::realMemalign_ == nullptr) {
        ark::os::unix::mem_hooks::PandaHooks::SaveRealFunctions();
    }
    if (ark::os::mem_hooks::PandaHooks::IsActive()) {
        void *caller = __builtin_return_address(0);
        return ark::os::mem_hooks::PandaHooks::MemalignHook(alignment, size, caller);
    }
    return ark::os::mem_hooks::PandaHooks::realMemalign_(alignment, size);
}

// NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
void free(void *ptr) noexcept
{
    if (ark::os::mem_hooks::PandaHooks::realFree_ == nullptr) {
        ark::os::unix::mem_hooks::PandaHooks::SaveRealFunctions();
    }
    if (ark::os::mem_hooks::PandaHooks::IsActive()) {
        void *caller = __builtin_return_address(0);
        return ark::os::mem_hooks::PandaHooks::FreeHook(ptr, caller);
    }
    return ark::os::mem_hooks::PandaHooks::realFree_(ptr);
}

#endif  // PANDA_USE_MEMORY_HOOKS
