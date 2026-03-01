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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_INFO_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_INFO_H

#include <array>
#include <cstring>
#include <securec.h>
#include <string>

#include "libarkbase/macros.h"
#include "libarkbase/os/thread.h"

namespace ark::tooling::sampler {

// Saving one sample info
struct SampleInfo {
    enum class ThreadStatus : uint32_t { UNDECLARED = 0, RUNNING = 1, SUSPENDED = 2 };

    struct ManagedStackFrameId {
        uintptr_t fileId {0};
        uintptr_t pandaFilePtr {0};
        uint32_t bcOffset {0};
    };

    struct StackInfo {
        static constexpr size_t MAX_STACK_DEPTH = 100;
        uintptr_t managedStackSize {0};
        // We can't use dynamic memory 'cause of usage inside the signal handler
        std::array<ManagedStackFrameId, MAX_STACK_DEPTH> managedStack;
    };

    struct ThreadInfo {
        // Id of the thread from which sample was obtained
        uint32_t threadId {0};
        ThreadStatus threadStatus {ThreadStatus::UNDECLARED};
    };
    uint64_t timeStamp {0};
    ThreadInfo threadInfo {};
    StackInfo stackInfo {};
};

// Saving one module info (panda file, .so)
struct FileInfo {
    uintptr_t ptr {0};
    uint32_t checksum {0};
    std::string pathname;
};

enum class FrameKind : uintptr_t { BRIDGE = 1 };

bool operator==(const SampleInfo &lhs, const SampleInfo &rhs);
bool operator!=(const SampleInfo &lhs, const SampleInfo &rhs);
bool operator==(const FileInfo &lhs, const FileInfo &rhs);
bool operator!=(const FileInfo &lhs, const FileInfo &rhs);
bool operator==(const SampleInfo::ManagedStackFrameId &lhs, const SampleInfo::ManagedStackFrameId &rhs);
bool operator!=(const SampleInfo::ManagedStackFrameId &lhs, const SampleInfo::ManagedStackFrameId &rhs);
bool operator<(const SampleInfo::ManagedStackFrameId &lhs, const SampleInfo::ManagedStackFrameId &rhs);
bool operator==(const SampleInfo::StackInfo &lhs, const SampleInfo::StackInfo &rhs);
bool operator!=(const SampleInfo::StackInfo &lhs, const SampleInfo::StackInfo &rhs);
bool operator==(const SampleInfo::ThreadInfo &lhs, const SampleInfo::ThreadInfo &rhs);
bool operator!=(const SampleInfo::ThreadInfo &lhs, const SampleInfo::ThreadInfo &rhs);

inline uintptr_t ReadUintptrTBitMisaligned(const void *ptr)
{
    /*
     * Pointer might be misaligned
     * To avoid of UB we should read misaligned adresses with memcpy
     */
    uintptr_t value = 0;
    [[maybe_unused]] int r = memcpy_s(&value, sizeof(value), ptr, sizeof(value));
    ASSERT(r == 0);
    return value;
}

inline uint32_t ReadUint32TBitMisaligned(const void *ptr)
{
    uint32_t value = 0;
    [[maybe_unused]] int r = memcpy_s(&value, sizeof(value), ptr, sizeof(value));
    ASSERT(r == 0);
    return value;
}

inline bool operator==(const SampleInfo::ManagedStackFrameId &lhs, const SampleInfo::ManagedStackFrameId &rhs)
{
    return lhs.fileId == rhs.fileId && lhs.pandaFilePtr == rhs.pandaFilePtr;
}

inline bool operator!=(const SampleInfo::ManagedStackFrameId &lhs, const SampleInfo::ManagedStackFrameId &rhs)
{
    return !(lhs == rhs);
}
inline bool operator<(const SampleInfo::ManagedStackFrameId &lhs, const SampleInfo::ManagedStackFrameId &rhs)
{
    return std::tie(lhs.fileId, lhs.pandaFilePtr) < std::tie(rhs.fileId, rhs.pandaFilePtr);
}

inline bool operator==(const FileInfo &lhs, const FileInfo &rhs)
{
    return lhs.ptr == rhs.ptr && lhs.pathname == rhs.pathname;
}

inline bool operator!=(const FileInfo &lhs, const FileInfo &rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const SampleInfo::StackInfo &lhs, const SampleInfo::StackInfo &rhs)
{
    if (lhs.managedStackSize != rhs.managedStackSize) {
        return false;
    }
    for (uint32_t i = 0; i < lhs.managedStackSize; ++i) {
        if (lhs.managedStack[i] != rhs.managedStack[i]) {
            return false;
        }
    }
    return true;
}

inline bool operator!=(const SampleInfo::StackInfo &lhs, const SampleInfo::StackInfo &rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const SampleInfo::ThreadInfo &lhs, const SampleInfo::ThreadInfo &rhs)
{
    return lhs.threadId == rhs.threadId && lhs.threadStatus == rhs.threadStatus;
}

inline bool operator!=(const SampleInfo::ThreadInfo &lhs, const SampleInfo::ThreadInfo &rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const SampleInfo &lhs, const SampleInfo &rhs)
{
    if (lhs.threadInfo != rhs.threadInfo) {
        return false;
    }
    if (lhs.stackInfo != rhs.stackInfo) {
        return false;
    }
    return true;
}

inline bool operator!=(const SampleInfo &lhs, const SampleInfo &rhs)
{
    return !(lhs == rhs);
}

}  // namespace ark::tooling::sampler

// Definind std::hash for SampleInfo to use it as an unordered_map key
namespace std {

template <>
struct hash<ark::tooling::sampler::SampleInfo> {
    std::size_t operator()(const ark::tooling::sampler::SampleInfo &s) const
    {
        auto stackInfo = s.stackInfo;
        ASSERT(stackInfo.managedStackSize <= ark::tooling::sampler::SampleInfo::StackInfo::MAX_STACK_DEPTH);
        size_t summ = 0;
        for (size_t i = 0; i < stackInfo.managedStackSize; ++i) {
            summ += stackInfo.managedStack[i].pandaFilePtr ^ stackInfo.managedStack[i].fileId;
        }
        constexpr uint32_t THREAD_STATUS_SHIFT = 20;
        return std::hash<size_t>()(
            (summ ^ stackInfo.managedStackSize) +
            (s.threadInfo.threadId ^ (static_cast<uint32_t>(s.threadInfo.threadStatus) << THREAD_STATUS_SHIFT)));
    }
};

// Definind std::hash for FileInfo to use it as an unordered_set key
template <>
struct hash<ark::tooling::sampler::FileInfo> {
    size_t operator()(const ark::tooling::sampler::FileInfo &m) const
    {
        size_t h1 = std::hash<uintptr_t> {}(m.ptr);
        size_t h2 = std::hash<uint32_t> {}(m.checksum);
        size_t h3 = std::hash<std::string> {}(m.pathname);
        return h1 ^ (h2 << 1U) ^ (h3 << 2U);
    }
};

}  // namespace std

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_INFO_H
