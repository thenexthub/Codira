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

#include <cstring>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include "libarkbase/utils/logger.h"
#include "libarkbase/os/stacktrace.h"
#include "libarkbase/mem/alloc_tracker.h"

namespace ark {

static constexpr size_t NUM_SKIP_FRAMES = 1;
static constexpr size_t ARENA_SIZE = 4096;
static constexpr size_t ENTRY_HDR_SIZE = sizeof(int32_t);

static const char *GetDumpFilePath()
{
#if defined(PANDA_TARGET_MOBILE)
    return "/data/local/tmp/memdump.bin";
#else
    return "memdump.bin";
#endif
}

static void Write(uint32_t val, std::ostream &out)
{
    out.write(reinterpret_cast<char *>(&val), sizeof(val));
}

static void Write(const std::string &str, std::ostream &out)
{
    Write(static_cast<uint32_t>(str.size()), out);
    out.write(str.c_str(), static_cast<std::streamsize>(str.length()));
}

static size_t CalcHash(const std::vector<uintptr_t> &st)
{
    size_t hash = 0;
    std::hash<uintptr_t> addrHash;
    for (uintptr_t addr : st) {
        hash |= addrHash(addr);
    }
    return hash;
}

// On a phone getting a stacktrace is expensive operation.
// An application doesn't launch in timeout and gets killed.
// This function is aimed to skip getting stacktraces for some allocations.
#if defined(PANDA_TARGET_MOBILE)
static bool SkipStacktrace(size_t num)
{
    static constexpr size_t FREQUENCY = 5;
    return num % FREQUENCY != 0;
}
#else
static bool SkipStacktrace([[maybe_unused]] size_t num)
{
    return false;
}
#endif

void DetailAllocTracker::TrackAlloc(void *addr, size_t size, SpaceType space)
{
    if (addr == nullptr) {
        return;
    }
    Stacktrace stacktrace = SkipStacktrace(++allocCounter_) ? Stacktrace() : GetStacktrace();
    os::memory::LockHolder lock(mutex_);

    uint32_t stacktraceId = stacktraces_.size();
    if (stacktrace.size() > NUM_SKIP_FRAMES) {
        stacktraces_.emplace_back(stacktrace.begin() + NUM_SKIP_FRAMES, stacktrace.end());
    } else {
        stacktraces_.emplace_back(stacktrace);
    }
    if (curArena_.size() < sizeof(AllocInfo)) {
        AllocArena();
    }
    auto info = new (curArena_.data()) AllocInfo(curId_++, size, static_cast<uint32_t>(space), stacktraceId);
    curArena_ = curArena_.SubSpan(sizeof(AllocInfo));
    curAllocs_.insert({addr, info});
}

void DetailAllocTracker::TrackFree(void *addr)
{
    if (addr == nullptr) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    auto it = curAllocs_.find(addr);
    ASSERT(it != curAllocs_.end());
    AllocInfo *alloc = it->second;
    curAllocs_.erase(it);
    if (curArena_.size() < sizeof(FreeInfo)) {
        AllocArena();
    }
    new (curArena_.data()) FreeInfo(alloc->GetId());
    curArena_ = curArena_.SubSpan(sizeof(FreeInfo));
}

void DetailAllocTracker::AllocArena()
{
    if (curArena_.size() >= ENTRY_HDR_SIZE) {
        *reinterpret_cast<uint32_t *>(curArena_.data()) = 0;
    }
    arenas_.emplace_back(new uint8_t[ARENA_SIZE]);
    curArena_ = Span<uint8_t>(arenas_.back().get(), arenas_.back().get() + ARENA_SIZE);
}

void DetailAllocTracker::Dump()
{
    LOG(ERROR, RUNTIME) << "DetailAllocTracker::Dump to " << GetDumpFilePath();
    std::ofstream out(GetDumpFilePath(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out) {
        LOG(ERROR, RUNTIME) << "DetailAllocTracker: Cannot open " << GetDumpFilePath()
                            << " for writing: " << strerror(errno) << "."
                            << "\nCheck the directory has write permissions or"
                            << " selinux is disabled.";
    }
    Dump(out);
    LOG(ERROR, RUNTIME) << "DetailAllocTracker: dump file has been written";
}

void DetailAllocTracker::Dump(std::ostream &out)
{
    os::memory::LockHolder lock(mutex_);

    Write(0, out);  // number of items, will be updated later
    Write(0, out);  // number of stacktraces, will be updated later

    std::map<uint32_t, uint32_t> idMap;
    uint32_t numStacks = WriteStacks(out, &idMap);

    // Write end marker to the current arena
    if (curArena_.size() >= ENTRY_HDR_SIZE) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *reinterpret_cast<uint32_t *>(curArena_.data()) = 0;
    }
    uint32_t numItems = 0;
    for (auto &arena : arenas_) {
        uint8_t *ptr = arena.get();
        size_t pos = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (pos < ARENA_SIZE && *reinterpret_cast<uint32_t *>(ptr + pos) != 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            uint32_t tag = *reinterpret_cast<uint32_t *>(ptr + pos);
            if (tag == ALLOC_TAG) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto alloc = reinterpret_cast<AllocInfo *>(ptr + pos);
                Write(alloc->GetTag(), out);
                Write(alloc->GetId(), out);
                Write(alloc->GetSize(), out);
                Write(alloc->GetSpace(), out);
                Write(idMap[alloc->GetStacktraceId()], out);
                pos += sizeof(AllocInfo);
            } else if (tag == FREE_TAG) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto info = reinterpret_cast<FreeInfo *>(ptr + pos);
                Write(info->GetTag(), out);
                Write(info->GetAllocId(), out);
                pos += sizeof(FreeInfo);
            } else {
                UNREACHABLE();
            }
            ++numItems;
        }
    }

    out.seekp(0);
    Write(numItems, out);
    Write(numStacks, out);
}

void DetailAllocTracker::DumpMemLeaks(std::ostream &out)
{
    static constexpr size_t MAX_ENTRIES_TO_REPORT = 10;

    os::memory::LockHolder lock(mutex_);
    size_t num = 0;
    out << "found " << curAllocs_.size() << " leaks\n";
    for (auto &entry : curAllocs_) {
        out << "Allocation of " << entry.second->GetSize() << " is allocated at\n";
        uint32_t stacktraceId = entry.second->GetStacktraceId();
        auto it = stacktraces_.begin();
        std::advance(it, stacktraceId);
        PrintStack(*it, out);
        if (++num > MAX_ENTRIES_TO_REPORT) {
            break;
        }
    }
}

uint32_t DetailAllocTracker::WriteStacks(std::ostream &out, std::map<uint32_t, uint32_t> *idMap)
{
    class Key {
    public:
        explicit Key(const Stacktrace *stacktrace) : stacktrace_(stacktrace), hash_(CalcHash(*stacktrace)) {}

        DEFAULT_COPY_SEMANTIC(Key);
        DEFAULT_NOEXCEPT_MOVE_SEMANTIC(Key);

        ~Key() = default;

        bool operator==(const Key &k) const
        {
            return *stacktrace_ == *k.stacktrace_;
        }

        size_t GetHash() const
        {
            return hash_;
        }

    private:
        const Stacktrace *stacktrace_ = nullptr;
        size_t hash_ = 0U;
    };

    struct KeyHash {
        size_t operator()(const Key &k) const
        {
            return k.GetHash();
        }
    };

    std::unordered_map<Key, uint32_t, KeyHash> allocStacks;
    uint32_t stacktraceId = 0;
    uint32_t deduplicatedId = 0;
    for (Stacktrace &stacktrace : stacktraces_) {
        Key akey(&stacktrace);
        auto res = allocStacks.insert({akey, deduplicatedId});
        if (res.second) {
            std::stringstream str;
            PrintStack(stacktrace, str);
            Write(str.str(), out);
            idMap->insert({stacktraceId, deduplicatedId});
            ++deduplicatedId;
        } else {
            uint32_t id = res.first->second;
            idMap->insert({stacktraceId, id});
        }
        ++stacktraceId;
    }
    return deduplicatedId;
}

}  // namespace ark
