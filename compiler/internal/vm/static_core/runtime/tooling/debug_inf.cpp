/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "include/tooling/debug_inf.h"

//
// Debuge interface for native tools(perf, libunwind).
//

namespace ark::tooling {
#ifdef __cplusplus
extern "C" {
#endif

enum CodeAction {
    CODE_NOACTION = 0,
    CODE_ADDED,
    CODE_REMOVE,
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct PCodeItem {
    std::atomic<PCodeItem *> next;
    PCodeItem *prev;
    const uint8_t *codeBase;
    uint64_t codeSize;
    uint64_t timestamp;
};

struct PCodeMetaInfo {
    uint32_t version = 1;
    uint32_t action = CODE_NOACTION;
    PCodeItem *releventItem = nullptr;
    std::atomic<PCodeItem *> head {nullptr};

    // Panda-specific fields
    uint32_t flags = 0;
    uint32_t sizeMetaInfo = sizeof(PCodeMetaInfo);
    uint32_t sizeCodeitem = sizeof(PCodeItem);
    std::atomic_uint32_t updateLock {0};
    uint64_t timestamp = 1;
};

// perf currently use g_jitDebugDescriptor and g_dexDebugDescriptor
// to find the jit code item and dexfiles.
// for using the variable interface, we doesn't change the name in panda
// NOLINTNEXTLINE(readability-identifier-naming, fuchsia-statically-constructed-objects)
PCodeMetaInfo g_jitDebugDescriptor;
// NOLINTNEXTLINE(readability-identifier-naming, fuchsia-statically-constructed-objects)
PCodeMetaInfo g_dexDebugDescriptor;

#ifdef __cplusplus
}  // extern "C"
#endif

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::map<const std::string, PCodeItem *> DebugInf::aexItemMap_;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
ark::os::memory::Mutex DebugInf::jitItemLock_;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
ark::os::memory::Mutex DebugInf::aexItemLock_;

void DebugInf::AddCodeMetaInfo(const panda_file::File *file)
{
    ark::os::memory::LockHolder lock(aexItemLock_);
    ASSERT(file != nullptr);
    auto it = aexItemMap_.find(file->GetFilename());
    if (it != aexItemMap_.end()) {
        return;
    }

    PCodeItem *item = AddCodeMetaInfoImpl(&g_dexDebugDescriptor, {file->GetBase(), file->GetHeader()->fileSize});
    aexItemMap_.emplace(file->GetFilename(), item);
}

void DebugInf::DelCodeMetaInfo(const panda_file::File *file)
{
    ark::os::memory::LockHolder lock(aexItemLock_);
    ASSERT(file != nullptr);
    auto it = aexItemMap_.find(file->GetFilename());
    if (it == aexItemMap_.end()) {
        return;
    }

    DelCodeMetaInfoImpl(&g_dexDebugDescriptor, file);
}

void DebugInf::Lock(PCodeMetaInfo *mi)
{
    // Atomic with relaxed order reason: data race with update_lock_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    mi->updateLock.fetch_add(1, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
}

void DebugInf::UnLock(PCodeMetaInfo *mi)
{
    std::atomic_thread_fence(std::memory_order_release);
    // Atomic with relaxed order reason: data race with update_lock_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    mi->updateLock.fetch_add(1, std::memory_order_relaxed);
}

PCodeItem *DebugInf::AddCodeMetaInfoImpl(PCodeMetaInfo *metaInfo, [[maybe_unused]] Span<const uint8_t> inss)
{
    uint64_t timestamp = std::max(metaInfo->timestamp + 1, ark::time::GetCurrentTimeInNanos());

    // Atomic with relaxed order reason: data race with metaInfo with no synchronization or ordering constraints imposed
    // on other reads or writes
    auto *head = metaInfo->head.load(std::memory_order_relaxed);

    auto *codeItem = new PCodeItem;
    codeItem->codeBase = inss.begin();
    codeItem->codeSize = inss.Size();
    codeItem->prev = nullptr;
    // Atomic with relaxed order reason: data race with code_item with no synchronization or ordering constraints
    // imposed on other reads or writes
    codeItem->next.store(head, std::memory_order_relaxed);
    codeItem->timestamp = timestamp;

    // lock
    Lock(metaInfo);
    if (head != nullptr) {
        head->prev = codeItem;
    }

    // Atomic with relaxed order reason: data race with metaInfo with no synchronization or ordering constraints imposed
    // on other reads or writes
    metaInfo->head.store(codeItem, std::memory_order_relaxed);
    metaInfo->releventItem = codeItem;
    metaInfo->action = CODE_ADDED;

    // unlock
    UnLock(metaInfo);

    return codeItem;
}

void DebugInf::DelCodeMetaInfoImpl(PCodeMetaInfo *metaInfo, const panda_file::File *file)
{
    PCodeItem *codeItem = aexItemMap_[file->GetFilename()];
    ASSERT(codeItem != nullptr);
    uint64_t timestamp = std::max(metaInfo->timestamp + 1, ark::time::GetCurrentTimeInNanos());
    // lock
    Lock(metaInfo);

    // Atomic with relaxed order reason: data race with code_item with no synchronization or ordering constraints
    // imposed on other reads or writes
    auto next = codeItem->next.load(std::memory_order_relaxed);
    if (codeItem->prev != nullptr) {
        // Atomic with relaxed order reason: data race with code_item with no synchronization or ordering constraints
        // imposed on other reads or writes
        codeItem->prev->next.store(next, std::memory_order_relaxed);
    } else {
        // Atomic with relaxed order reason: data race with metaInfo with no synchronization or ordering constraints
        // imposed on other reads or writes
        metaInfo->head.store(next, std::memory_order_relaxed);
    }

    if (next != nullptr) {
        next->prev = codeItem->prev;
    }

    metaInfo->releventItem = codeItem;
    metaInfo->action = CODE_REMOVE;
    metaInfo->timestamp = timestamp;

    // unlock
    UnLock(metaInfo);
}
}  // namespace ark::tooling
