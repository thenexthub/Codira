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

#include "code_allocator.h"
#include "libarkbase/mem/base_mem_stats.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/trace/trace.h"

#include <securec.h>
#include <cstring>

namespace ark {

const Alignment CodeAllocator::PAGE_LOG_ALIGN = GetLogAlignment(os::mem::GetPageSize());

CodeAllocator::CodeAllocator(BaseMemStats *memStats)
    : arenaAllocator_([&]() {
          trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);
          // Do not set up mem_stats in internal arena allocator, because we will manage memstats here.
          return ArenaAllocator(SpaceType::SPACE_TYPE_CODE, nullptr);
      }()),
      memStats_(memStats)
{
    ASSERT(LOG_ALIGN_MIN <= PAGE_LOG_ALIGN && PAGE_LOG_ALIGN <= LOG_ALIGN_MAX);
}

CodeAllocator::~CodeAllocator()
{
    codeRangeStart_ = nullptr;
    codeRangeEnd_ = nullptr;
}

void *CodeAllocator::AllocateCode(size_t size, const void *codeBuff)
{
    trace::ScopedTrace scopedTrace("Allocate Code");
    void *codePtr = arenaAllocator_.Alloc(size, PAGE_LOG_ALIGN);
    if (UNLIKELY(codePtr == nullptr || memcpy_s(codePtr, size, codeBuff, size) != EOK)) {
        return nullptr;
    }
    ProtectCode(os::mem::MapRange<std::byte>(static_cast<std::byte *>(codePtr), size));
    memStats_->RecordAllocateRaw(size, SpaceType::SPACE_TYPE_CODE);
    CodeRangeUpdate(codePtr, size);
    return codePtr;
}

os::mem::MapRange<std::byte> CodeAllocator::AllocateCodeUnprotected(size_t size)
{
    trace::ScopedTrace scopedTrace("Allocate Code");
    void *codePtr = arenaAllocator_.Alloc(size, PAGE_LOG_ALIGN);
    if (UNLIKELY(codePtr == nullptr)) {
        return os::mem::MapRange<std::byte>(nullptr, 0);
    }
    memStats_->RecordAllocateRaw(size, SpaceType::SPACE_TYPE_CODE);
    CodeRangeUpdate(codePtr, size);
    return os::mem::MapRange<std::byte>(static_cast<std::byte *>(codePtr), size);
}

/* static */
void CodeAllocator::ProtectCode(os::mem::MapRange<std::byte> memRange)
{
    memRange.MakeReadExec();
}

bool CodeAllocator::InAllocatedCodeRange(const void *pc)
{
    os::memory::ReadLockHolder rlock(codeRangeLock_);
    return (pc >= codeRangeStart_) && (pc <= codeRangeEnd_);
}

void CodeAllocator::CodeRangeUpdate(void *ptr, size_t size)
{
    os::memory::WriteLockHolder rwlock(codeRangeLock_);
    if (ptr < codeRangeStart_ || codeRangeStart_ == nullptr) {
        codeRangeStart_ = ptr;
    }
    void *bufferEnd = ToVoidPtr(ToUintPtr(ptr) + size);
    if (bufferEnd > codeRangeEnd_ || codeRangeEnd_ == nullptr) {
        codeRangeEnd_ = bufferEnd;
    }
}

}  // namespace ark
