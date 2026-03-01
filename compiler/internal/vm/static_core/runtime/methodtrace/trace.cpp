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

#include "trace.h"

#include "include/runtime_notification.h"
#include "include/runtime.h"
#include "include/method.h"
#include "include/class.h"
#include "plugins.h"
#include "runtime/include/runtime.h"

#include <string>

namespace ark {

os::memory::Mutex g_traceLock;                     // NOLINT(fuchsia-statically-constructed-objects)
Trace *volatile Trace::singletonTrace_ = nullptr;  // CC-OFF(G.FMT.15) pointer CV qualifier
bool Trace::isTracing_ = false;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
LanguageContext Trace::ctx_ = LanguageContext(nullptr);

static uint64_t SystemMicroSecond()
{
    timespec current {};
    clock_gettime(CLOCK_MONOTONIC, &current);
    // NOTE(ht): Deleting OS-specific code, move to libpandabase, issue #3210
    return static_cast<uint64_t>(current.tv_sec) * UINT64_C(1000000) +  // 1000000 - time
           static_cast<uint64_t>(current.tv_nsec) / UINT64_C(1000);     // 1000 - time
}

static uint64_t RealTimeSecond()
{
    timespec current {};
    clock_gettime(CLOCK_REALTIME, &current);
    // NOTE(ht): Deleting OS-specific code, move to libpandabase, issue #3210
    return static_cast<uint64_t>(current.tv_sec);
}

static void ThreadCpuClock(os::thread::NativeHandleType thread, clockid_t *clockId)
{
#ifdef PANDA_TARGET_MACOS
    *clockId = CLOCK_THREAD_CPUTIME_ID;
#else
    pthread_getcpuclockid(thread, clockId);
#endif
}
static uint64_t GetCpuMicroSecond()
{
    auto threadSelf = os::thread::GetNativeHandle();
    clockid_t clockId;
    ThreadCpuClock(threadSelf, &clockId);
    timespec current {};
    clock_gettime(clockId, &current);
    // NOTE(ht): Deleting OS-specific code, move to libpandabase, issue #3210
    return static_cast<uint64_t>(current.tv_sec) * UINT64_C(1000000) +  // 1000000 - time
           static_cast<uint64_t>(current.tv_nsec) / UINT64_C(1000);     // 1000 - time
}

Trace::Trace(PandaUniquePtr<ark::os::unix::file::File> traceFile, size_t bufferSize)
    : traceFile_(std::move(traceFile)),
      bufferSize_(std::max(TRACE_HEADER_REAL_LENGTH, bufferSize)),
      traceStartTime_(SystemMicroSecond()),
      baseCpuTime_(GetCpuMicroSecond()),
      bufferOffset_(0)
{
    buffer_ = MakePandaUnique<uint8_t[]>(bufferSize_);  // NOLINT(modernize-avoid-c-arrays)
    if (memset_s(buffer_.get(), bufferSize_, 0, TRACE_HEADER_LENGTH) != EOK) {
        LOG(ERROR, RUNTIME) << __func__ << " memset error";
    }
    WriteDataByte(buffer_.get(), MAGIC_VALUE, numberOf4Bytes_);
    WriteDataByte(buffer_.get() + numberOf4Bytes_, TRACE_VERSION, numberOf2Bytes_);
    WriteDataByte(buffer_.get() + numberOf4Bytes_ + numberOf2Bytes_, TRACE_HEADER_LENGTH, numberOf2Bytes_);
    WriteDataByte(buffer_.get() + numberOf8Bytes_, traceStartTime_, numberOf8Bytes_);
    WriteDataByte(buffer_.get() + numberOf8Bytes_ + numberOf8Bytes_, TRACE_ITEM_SIZE, numberOf2Bytes_);
    // Atomic with relaxed order reason: data race with buffer_offset_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    bufferOffset_.store(TRACE_HEADER_LENGTH, std::memory_order_relaxed);
}

Trace::~Trace() = default;

void Trace::StartTracing(const char *traceFilename, size_t bufferSize)
{
    os::memory::LockHolder lock(g_traceLock);
    if (singletonTrace_ != nullptr) {
        LOG(ERROR, RUNTIME) << "method tracing is running, ignoring new request";
        return;
    }

    PandaString fileName(traceFilename);
    if (fileName.empty()) {
        fileName = "method";
        fileName = fileName + ConvertToString(std::to_string(RealTimeSecond()));
        fileName += ".trace";
#ifdef PANDA_TARGET_MOBILE
        fileName = "/data/misc/trace/" + fileName;
#endif  // PANDA_TARGET_MOBILE
    }

    auto traceFile = MakePandaUnique<ark::os::unix::file::File>(
        ark::os::file::Open(fileName, ark::os::file::Mode::READWRITECREATE).GetFd());
    if (!traceFile->IsValid()) {
        LOG(ERROR, RUNTIME) << "Cannot OPEN/CREATE the trace file " << fileName;
        return;
    }

    panda_file::SourceLang lang = ark::plugins::RuntimeTypeToLang(Runtime::GetRuntimeType());
    ctx_ = Runtime::GetCurrent()->GetLanguageContext(lang);

    singletonTrace_ = ctx_.CreateTrace(std::move(traceFile), bufferSize);

    Runtime::GetCurrent()->GetNotificationManager()->AddListener(singletonTrace_,
                                                                 RuntimeNotificationManager::Event::METHOD_EVENTS);
    isTracing_ = true;
}

void Trace::StopTracing()
{
    os::memory::LockHolder lock(g_traceLock);
    if (singletonTrace_ == nullptr) {
        LOG(ERROR, RUNTIME) << "need to stop method tracing, but no method tracing is running";
    } else {
        Runtime::GetCurrent()->GetNotificationManager()->RemoveListener(
            singletonTrace_, RuntimeNotificationManager::Event::METHOD_EVENTS);
        singletonTrace_->SaveTracingData();
        isTracing_ = false;
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(singletonTrace_);
        singletonTrace_ = nullptr;
    }
}

void Trace::TriggerTracing()
{
    if (!isTracing_) {
        Trace::StartTracing("", FILE_SIZE);
    } else {
        Trace::StopTracing();
    }
}

void Trace::RecordThreadsInfo([[maybe_unused]] PandaOStringStream *os)
{
    os::memory::LockHolder lock(threadInfoLock_);
    for (const auto &info : threadInfoSet_) {
        (*os) << info;
    }
}
uint64_t Trace::GetAverageTime()
{
    uint64_t begin = GetCpuMicroSecond();
    for (uint32_t i = 0; i < loopNumber_; i++) {
        GetCpuMicroSecond();
    }
    uint64_t delay = GetCpuMicroSecond() - begin;
    return delay / divideNumber_;
}

void Trace::RecordMethodsInfo(PandaOStringStream *os, const PandaSet<Method *> &calledMethods)
{
    for (const auto &method : calledMethods) {
        (*os) << GetMethodDetailInfo(method);
    }
}

void Trace::WriteInfoToBuf(const ManagedThread *thread, Method *method, EventFlag event, uint32_t threadTime,
                           uint32_t realTime)
{
    uint32_t writeAfterOffset;
    uint32_t writeBeforeOffset;

    // Atomic with relaxed order reason: data race with buffer_offset_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    writeBeforeOffset = bufferOffset_.load(std::memory_order_relaxed);
    do {
        writeAfterOffset = writeBeforeOffset + TRACE_ITEM_SIZE;
        if (bufferSize_ < static_cast<size_t>(writeAfterOffset)) {
            overbrim_ = true;
            return;
        }
    } while (!bufferOffset_.compare_exchange_weak(writeBeforeOffset, writeAfterOffset, std::memory_order_relaxed));

    EventFlag flag = TRACE_METHOD_ENTER;
    switch (event) {
        case EventFlag::TRACE_METHOD_ENTER:
            flag = TRACE_METHOD_ENTER;
            break;
        case EventFlag::TRACE_METHOD_EXIT:
            flag = TRACE_METHOD_EXIT;
            break;
        case EventFlag::TRACE_METHOD_UNWIND:
            flag = TRACE_METHOD_UNWIND;
            break;
        default:
            LOG(ERROR, RUNTIME) << "unrecognized events" << std::endl;
    }
    uint32_t methodActionValue = EncodeMethodAndEventToId(method, flag);

    uint8_t *ptr = buffer_.get() + writeBeforeOffset;
    WriteDataByte(ptr, thread->GetId(), numberOf2Bytes_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ptr += numberOf2Bytes_;
    WriteDataByte(ptr, methodActionValue, numberOf4Bytes_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ptr += numberOf4Bytes_;
    WriteDataByte(ptr, threadTime, numberOf4Bytes_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ptr += numberOf4Bytes_;
    WriteDataByte(ptr, realTime, numberOf4Bytes_);
}

uint32_t Trace::EncodeMethodAndEventToId(Method *method, EventFlag flag)
{
    uint32_t methodActionId = (EncodeMethodToId(method) << ENCODE_EVENT_BITS) | flag;
    ASSERT(method == DecodeIdToMethod(methodActionId));
    return methodActionId;
}

uint32_t Trace::EncodeMethodToId(Method *method)
{
    os::memory::LockHolder lock(methodsLock_);
    uint32_t methodIdValue = 0;
    auto iter = methodIdPandamap_.find(method);
    if (iter != methodIdPandamap_.end()) {
        methodIdValue = iter->second;
    } else {
        methodIdValue = methodsCalledVector_.size();
        methodsCalledVector_.push_back(method);
        methodIdPandamap_.emplace(method, methodIdValue);
    }
    return methodIdValue;
}

Method *Trace::DecodeIdToMethod(uint32_t id)
{
    os::memory::LockHolder lock(methodsLock_);
    return methodsCalledVector_[id >> ENCODE_EVENT_BITS];
}

void Trace::GetUniqueMethods(size_t bufferLength, PandaSet<Method *> *calledMethodsSet)
{
    uint8_t *dataBegin = buffer_.get() + TRACE_HEADER_LENGTH;
    uint8_t *dataEnd = buffer_.get() + bufferLength;

    while (dataBegin < dataEnd) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint32_t decodedData = GetDataFromBuffer(dataBegin + numberOf2Bytes_, numberOf4Bytes_);
        Method *method = DecodeIdToMethod(decodedData);
        calledMethodsSet->insert(method);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dataBegin += TRACE_ITEM_SIZE;
    }
}

void Trace::SaveTracingData()
{
    uint64_t elapsed = SystemMicroSecond() - traceStartTime_;
    PandaOStringStream ostream;
    ostream << TRACE_STAR_CHAR << "version\n";
    ostream << TRACE_VERSION << "\n";
    ostream << "data-file-overflow=" << (overbrim_ ? "true" : "false") << "\n";
    ostream << "clock=dual\n";
    ostream << "elapsed-time-usec=" << elapsed << "\n";

    size_t bufEnd;
    // Atomic with relaxed order reason: data race with buffer_offset_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    bufEnd = bufferOffset_.load(std::memory_order_relaxed);
    size_t numRecords = (bufEnd - TRACE_HEADER_LENGTH) / TRACE_ITEM_SIZE;
    ostream << "num-method-calls=" << numRecords << "\n";
    ostream << "clock-call-overhead-nsec=" << GetAverageTime() << "\n";
    ostream << "pid=" << getpid() << "\n";
    ostream << "vm=panda\n";

    ostream << TRACE_STAR_CHAR << "threads\n";
    RecordThreadsInfo(&ostream);
    ostream << TRACE_STAR_CHAR << "methods\n";

    PandaSet<Method *> calledMethodsSet;
    GetUniqueMethods(bufEnd, &calledMethodsSet);

    RecordMethodsInfo(&ostream, calledMethodsSet);
    ostream << TRACE_STAR_CHAR << "end\n";
    PandaString methodsInfo(ostream.str());

    traceFile_->WriteAll(reinterpret_cast<const void *>(methodsInfo.c_str()), methodsInfo.length());
    traceFile_->WriteAll(buffer_.get(), bufEnd);
    traceFile_->Close();
}

void Trace::MethodEntry(ManagedThread *thread, Method *method)
{
    os::memory::LockHolder lock(threadInfoLock_);
    uint32_t threadTime = 0;
    uint32_t realTime = 0;
    GetTimes(&threadTime, &realTime);
    WriteInfoToBuf(thread, method, EventFlag::TRACE_METHOD_ENTER, threadTime, realTime);

    PandaOStringStream os;
    auto threadName = GetThreadName(thread);
    os << thread->GetId() << "\t" << threadName << "\n";
    PandaString info = os.str();
    threadInfoSet_.insert(info);
}

void Trace::MethodExit(ManagedThread *thread, Method *method)
{
    uint32_t threadTime = 0;
    uint32_t realTime = 0;
    GetTimes(&threadTime, &realTime);
    WriteInfoToBuf(thread, method, EventFlag::TRACE_METHOD_EXIT, threadTime, realTime);
}

void Trace::GetTimes(uint32_t *threadTime, uint32_t *realTime)
{
    *threadTime = GetCpuMicroSecond() - baseCpuTime_;
    *realTime = SystemMicroSecond() - traceStartTime_;
}

}  // namespace ark
