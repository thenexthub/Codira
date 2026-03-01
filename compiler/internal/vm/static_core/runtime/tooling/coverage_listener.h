/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef COVERAGELISTENER_H_
#define COVERAGELISTENER_H_

#include <atomic>
#include <cstdint>
#include <chrono>
#include <optional>
#include <queue>
#include <string_view>
#include <thread>

#include "include/runtime_notification.h"
#include "libarkbase/os/mutex.h"

namespace std {
template <>
struct hash<std::pair<ark::panda_file::File::EntityId, uint32_t>> {
    size_t operator()(const std::pair<ark::panda_file::File::EntityId, uint32_t> &key) const
    {
        size_t hash1 = hash<ark::panda_file::File::EntityId>()(key.first);
        size_t hash2 = hash<uint32_t>()(key.second);
        return hash1 ^ (hash2 << 1);  // NOLINT(hicpp-signed-bitwise)
    }
};
}  // namespace std

namespace ark::tooling {
using BytecodeCountMap = std::unordered_map<std::pair<panda_file::File::EntityId, uint32_t>, uint32_t>;
class BytecodeCounter;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class CoverageListener final : public RuntimeListener {
public:
    explicit CoverageListener(const std::string &fileName);
    ~CoverageListener();  // NOLINT(modernize-use-override)

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override;

    void StopCoverage();

private:
    void ClearInfoFile();
    void StopDumpThread();

    void DumpThreadWorker();
    void DumpCoverageInfoToFile(BytecodeCountMap &bytecodeCountMap);

    PandaUniquePtr<BytecodeCounter> delegate_;

    std::string fileName_;
    std::ofstream outFile;  // NOLINT(readability-identifier-naming)
    std::thread dumpThread_;
    std::atomic<bool> stopThread_ = false;
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class BytecodeCounter final : public RuntimeListener {
public:
    static constexpr uint32_t BYTE_CODE_DATA_BUFFER_SIZE = 20000;
    static constexpr uint32_t MAX_DUMP_INTERVAL_MS = 1000;
    static constexpr uint32_t QUEUE_WAIT_TIMEOUT_MS = 500;

    explicit BytecodeCounter(uint32_t bufferSize = BYTE_CODE_DATA_BUFFER_SIZE);
    ~BytecodeCounter() override;

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override;

    void StopCoverage();
    std::optional<std::queue<BytecodeCountMap>> GetQueue();

private:
    NO_THREAD_SAFETY_ANALYSIS void TriggerDump();

    os::memory::Mutex mapMutex_;
    os::memory::Mutex queueMutex_;
    os::memory::ConditionVariable queueConditionalVar_;

    uint32_t bufferSize_;
    BytecodeCountMap bytecodeCountMap_ GUARDED_BY(mapMutex_);
    std::queue<BytecodeCountMap> dumpQueue_ GUARDED_BY(queueMutex_);
    std::atomic<std::chrono::system_clock::time_point> lastDumpTime_;
};

}  // namespace ark::tooling
#endif  // COVERAGELISTENER_H_
