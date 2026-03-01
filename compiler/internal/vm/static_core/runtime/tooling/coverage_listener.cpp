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

#include "coverage_listener.h"

namespace ark::tooling {

// NOLINTNEXTLINE(modernize-pass-by-value)
CoverageListener::CoverageListener(const std::string &fileName) : fileName_(fileName)
{
    delegate_ = MakePandaUnique<BytecodeCounter>();
    ClearInfoFile();
    outFile.open(fileName_, std::ofstream::app);
    dumpThread_ = std::thread(&CoverageListener::DumpThreadWorker, this);
}

CoverageListener::~CoverageListener()
{
    StopDumpThread();
    outFile.close();
}

void CoverageListener::BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset)
{
    delegate_->BytecodePcChanged(thread, method, bcOffset);
}

void CoverageListener::StopCoverage()
{
    delegate_->StopCoverage();
}

void CoverageListener::ClearInfoFile()
{
    std::ofstream ofs(fileName_, std::ofstream::trunc);
    if (!ofs.is_open()) {
        LOG(ERROR, RUNTIME) << "Fail to open coverage file: " << fileName_;
    } else {
        LOG(INFO, RUNTIME) << "Coverage file cleared: " << fileName_;
    }
    ofs.close();
}

void CoverageListener::StopDumpThread()
{
    if (dumpThread_.joinable()) {
        // Atomic with release order reason: ensure prior writes are visible to the acquiring thread.
        stopThread_.store(true, std::memory_order_release);
        dumpThread_.join();
    }
}

void CoverageListener::DumpThreadWorker()
{
    auto dumpQueue = [this]() {
        auto localQueueOpt = delegate_->GetQueue();
        if (!localQueueOpt.has_value()) {
            return;
        }
        std::queue<BytecodeCountMap> &localQueue = localQueueOpt.value();
        while (!localQueue.empty()) {
            auto tempMap = std::move(localQueue.front());
            localQueue.pop();
            DumpCoverageInfoToFile(tempMap);
        }
    };

    // Atomic with acquire order reason: ensure we see the latest store from the releasing thread.
    while (!stopThread_.load(std::memory_order_acquire)) {
        dumpQueue();
    }
    dumpQueue();
    LOG(INFO, RUNTIME) << "Dump thread stopped";
}

void CoverageListener::DumpCoverageInfoToFile(BytecodeCountMap &bytecodeCountMap)
{
    if (bytecodeCountMap.empty()) {
        return;
    }

    if (!outFile.is_open()) {
        LOG(ERROR, RUNTIME) << "outFile.is_open() fail";
        return;
    }

    for (const auto &entry : bytecodeCountMap) {
        const auto &key = entry.first;
        uint32_t value = entry.second;
        outFile << key.first << "," << key.second << "," << value << std::endl;
    }

    LOG(INFO, RUNTIME) << "DumpCoverageInfoToFile success, dumped " << bytecodeCountMap.size() << " records";
}

BytecodeCounter::BytecodeCounter(uint32_t bufferSize) : bufferSize_(bufferSize)
{
    // Atomic with release order reason: ensure prior writes are visible to the acquiring thread.
    lastDumpTime_.store(std::chrono::system_clock::now(), std::memory_order_release);
}

BytecodeCounter::~BytecodeCounter() = default;

void BytecodeCounter::BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset)
{
    (void)thread;
    if (method == nullptr) {
        return;
    }

    {
        os::memory::LockHolder countMapLock(mapMutex_);
        bytecodeCountMap_[std::make_pair(method->GetFileId(), bcOffset)]++;
        if (bytecodeCountMap_.size() < bufferSize_) {
            return;
        }
    }
    TriggerDump();
}

void BytecodeCounter::StopCoverage()
{
    TriggerDump();
}

std::optional<std::queue<BytecodeCountMap>> BytecodeCounter::GetQueue()
{
    // Atomic with acquire order reason: ensure we see the latest store from the releasing thread.
    auto timeSinceLastDump = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - lastDumpTime_.load(std::memory_order_acquire));
    if (timeSinceLastDump.count() > MAX_DUMP_INTERVAL_MS) {
        LOG(INFO, RUNTIME) << "timeSinceLastDump > 1000, trigger dump";
        TriggerDump();
        os::memory::LockHolder lock(queueMutex_);
        return std::exchange(dumpQueue_, {});
    }
    os::memory::LockHolder lock(queueMutex_);
    if (dumpQueue_.empty()) {
        if (queueConditionalVar_.TimedWait(&queueMutex_, QUEUE_WAIT_TIMEOUT_MS)) {
            LOG(INFO, RUNTIME) << "TimedWait timeout, return empty queue";
            return std::nullopt;
        }
    }
    return std::exchange(dumpQueue_, {});
}

void BytecodeCounter::TriggerDump()
{
    // Atomic with release order reason: ensure prior writes are visible to the acquiring thread.
    lastDumpTime_.store(std::chrono::system_clock::now(), std::memory_order_release);
    bool dumpQueueNotEmpty = false;
    {
        os::memory::ScopedLock lock(mapMutex_, queueMutex_);
        if (!bytecodeCountMap_.empty()) {
            dumpQueue_.push(std::move(bytecodeCountMap_));
            dumpQueueNotEmpty = true;
        }
    }

    if (dumpQueueNotEmpty) {
        queueConditionalVar_.Signal();
    }
}
}  // namespace ark::tooling
