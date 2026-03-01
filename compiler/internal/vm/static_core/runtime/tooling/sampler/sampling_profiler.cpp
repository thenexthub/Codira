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

#include <sys/syscall.h>
#include <atomic>

#include "coroutines/coroutine_stats.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/thread.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "runtime/include/managed_thread.h"
#include "runtime/thread_manager.h"
#include "runtime/tooling/sampler/stack_walker_base.h"
#include "runtime/tooling/pt_thread_info.h"
#include "runtime/signal_handler.h"
#include "runtime/coroutines/coroutine.h"

namespace ark::tooling::sampler {

static std::atomic<int> g_sCurrentHandlersCounter = 0;

/* static */
Sampler *Sampler::instance_ = nullptr;

static std::atomic<size_t> g_sLostSamples = 0;
static std::atomic<size_t> g_sLostSegvSamples = 0;
static std::atomic<size_t> g_sLostInvalidSamples = 0;
static std::atomic<size_t> g_sLostNotFindSamples = 0;
static std::atomic<size_t> g_sTotalSamples = 0;

class ScopedThreadSampling {
public:
    explicit ScopedThreadSampling(ThreadSamplingInfo *samplingInfo) : samplingInfo_(samplingInfo)
    {
        ASSERT(samplingInfo_ != nullptr);
        ASSERT(samplingInfo_->IsThreadSampling() == false);
        samplingInfo_->SetThreadSampling(true);
    }

    ~ScopedThreadSampling()
    {
        ASSERT(samplingInfo_->IsThreadSampling() == true);
        samplingInfo_->SetThreadSampling(false);
    }

private:
    ThreadSamplingInfo *samplingInfo_;

    NO_COPY_SEMANTIC(ScopedThreadSampling);
    NO_MOVE_SEMANTIC(ScopedThreadSampling);
};

class ScopedHandlersCounting {
public:
    explicit ScopedHandlersCounting()
    {
        ++g_sCurrentHandlersCounter;
    }

    ~ScopedHandlersCounting()
    {
        --g_sCurrentHandlersCounter;
    }

    NO_COPY_SEMANTIC(ScopedHandlersCounting);
    NO_MOVE_SEMANTIC(ScopedHandlersCounting);
};

/* static */
Sampler *Sampler::Create()
{
    /*
     * Sampler can be created only once and managed by one thread
     * Runtime::Tools owns it ptr after it's created
     */
    ASSERT(instance_ == nullptr);
    instance_ = new Sampler;

    /**
     * As soon as the sampler is created, we subscribe to the events
     * This is done so that start and stop do not depend on the runtime
     * Internal issue #13780
     */
    ASSERT(Runtime::GetCurrent() != nullptr);

    Runtime::GetCurrent()->GetNotificationManager()->AddListener(instance_,
                                                                 RuntimeNotificationManager::Event::THREAD_EVENTS);
    Runtime::GetCurrent()->GetNotificationManager()->AddListener(instance_,
                                                                 RuntimeNotificationManager::Event::LOAD_MODULE);
    /**
     * Collect threads and modules which were created before sampler start
     * If we collect them before add listeners then new thread can be created (or new module can be loaded)
     * so we will lose this thread (or module)
     */
    instance_->CollectThreads();
    instance_->CollectModules();

    return Sampler::instance_;
}

static void LogProfilerStats()
{
    LOG(INFO, PROFILER) << "Total samples: " << g_sTotalSamples;
    LOG(INFO, PROFILER) << "Lost samples: " << g_sLostSamples;
    LOG(INFO, PROFILER) << "Lost samples(Invalid method ptr): " << g_sLostInvalidSamples;
    LOG(INFO, PROFILER) << "Lost samples(Invalid pf ptr): " << g_sLostNotFindSamples;
    LOG(INFO, PROFILER) << "Lost samples(SIGSEGV occured): " << g_sLostSegvSamples;
}

/* static */
void Sampler::Destroy(Sampler *sampler)
{
    ASSERT(instance_ != nullptr);
    ASSERT(instance_ == sampler);

    // Atomic with acquire order reason: To ensure start/stop load correctly
    ASSERT(!sampler->isActive_.load(std::memory_order_acquire));

    Runtime::GetCurrent()->GetNotificationManager()->RemoveListener(instance_,
                                                                    RuntimeNotificationManager::Event::THREAD_EVENTS);
    Runtime::GetCurrent()->GetNotificationManager()->RemoveListener(instance_,
                                                                    RuntimeNotificationManager::Event::LOAD_MODULE);

    instance_->ClearManagedThreadSet();
    instance_->ClearLoadedPfs();

    delete sampler;
    instance_ = nullptr;

    LOG(INFO, PROFILER) << "Sampling profiler destroyed";
    LogProfilerStats();
}

Sampler::Sampler() : runtime_(Runtime::GetCurrent()), sampleInterval_(DEFAULT_SAMPLE_INTERVAL_US)
{
    ASSERT_NATIVE_CODE();
}

void Sampler::AddThreadHandle(ManagedThread *thread)
{
    os::memory::LockHolder holder(managedThreadsLock_);
    managedThreads_.insert(thread->GetId());
}

void Sampler::EraseThreadHandle(ManagedThread *thread)
{
    os::memory::LockHolder holder(managedThreadsLock_);
    managedThreads_.erase(thread->GetId());
}

void Sampler::ThreadStart(ManagedThread *managedThread)
{
    AddThreadHandle(managedThread);
}

void Sampler::ThreadEnd(ManagedThread *managedThread)
{
    EraseThreadHandle(managedThread);
}

void Sampler::LoadModule(std::string_view name)
{
    auto callback = [this, name](const panda_file::File &pf) {
        if (pf.GetFilename() == name) {
            auto ptrId = reinterpret_cast<uintptr_t>(&pf);
            FileInfo pfModule;
            pfModule.ptr = ptrId;
            pfModule.pathname = pf.GetFullFileName();
            pfModule.checksum = pf.GetHeader()->checksum;
            if (!loadedPfsQueue_.FindValue(ptrId)) {
                loadedPfsQueue_.Push(pfModule);
            }
            os::memory::LockHolder holder(loadedPfsLock_);
            this->loadedPfs_.push_back(pfModule);
            return false;
        }
        return true;
    };
    runtime_->GetClassLinker()->EnumeratePandaFiles(callback, false);
}

bool Sampler::Start(std::unique_ptr<StreamWriter> &&writer)
{
    // Atomic with acquire order reason: To ensure start/stop load correctly
    if (isActive_.load(std::memory_order_acquire)) {
        LOG(ERROR, PROFILER) << "Attemp to start sampling profiler while it's already started";
        return false;
    }

    if (UNLIKELY(!communicator_.Init())) {
        LOG(ERROR, PROFILER) << "Failed to create pipes for sampling listener. Profiler cannot be started";
        return false;
    }

    // Atomic with release order reason: To ensure start store correctly
    isActive_.store(true, std::memory_order_release);

    listenerThread_ = std::make_unique<std::thread>(&Sampler::ListenerThreadEntry, this, std::move(writer));
    listenerTid_ = listenerThread_->native_handle();

    // All prepairing actions should be done before this thread is started
    samplerThread_ = std::make_unique<std::thread>(&Sampler::SamplerThreadEntry, this);
    samplerTid_ = samplerThread_->native_handle();
    LOG(INFO, PROFILER) << "Sampling profiler started";
    return true;
}

bool Sampler::Stop()
{
    // Atomic with acquire order reason: To ensure start/stop load correctly
    if (!isActive_.load(std::memory_order_acquire)) {
        LOG(ERROR, PROFILER) << "Attemp to stop sampling profiler, but it was not started";
        return false;
    }
    if (!samplerThread_->joinable()) {
        LOG(FATAL, PROFILER) << "Sampling profiler thread unexpectedly disappeared";
        UNREACHABLE();
    }
    if (!listenerThread_->joinable()) {
        LOG(FATAL, PROFILER) << "Listener profiler thread unexpectedly disappeared";
        UNREACHABLE();
    }

    // Atomic with release order reason: To ensure stop store correctly
    isActive_.store(false, std::memory_order_release);
    samplerThread_->join();
    listenerThread_->join();

    // After threads are stopped we can clear all sampler info
    samplerThread_.reset();
    listenerThread_.reset();
    samplerTid_ = 0;
    listenerTid_ = 0;

    LOG(INFO, PROFILER) << "Sampling profiler stopped";
    LogProfilerStats();
    return true;
}

void Sampler::WriteLoadedPandaFiles(StreamWriter *writerPtr)
{
    os::memory::LockHolder holder(loadedPfsLock_);
    if (LIKELY(loadedPfs_.empty())) {
        return;
    }
    for (const auto &module : loadedPfs_) {
        if (!writerPtr->IsModuleWritten(module)) {
            writerPtr->WriteModule(module);
        }
    }
    loadedPfs_.clear();
}

void Sampler::CollectThreads()
{
    auto tManager = runtime_->GetPandaVM()->GetThreadManager();
    if (UNLIKELY(tManager == nullptr)) {
        // NOTE(m.strizhak): make it for languages without thread_manager
        LOG(FATAL, PROFILER) << "Thread manager is nullptr";
        UNREACHABLE();
    }

    tManager->EnumerateThreads(
        [this](ManagedThread *thread) {
            AddThreadHandle(thread);
            return true;
        },
        static_cast<unsigned int>(EnumerationFlag::ALL), static_cast<unsigned int>(EnumerationFlag::VM_THREAD));
}

void Sampler::CollectModules()
{
    auto callback = [this](const panda_file::File &pf) {
        auto ptrId = reinterpret_cast<uintptr_t>(&pf);
        FileInfo pfModule;

        pfModule.ptr = ptrId;
        pfModule.pathname = pf.GetFullFileName();
        pfModule.checksum = pf.GetHeader()->checksum;

        if (!loadedPfsQueue_.FindValue(ptrId)) {
            loadedPfsQueue_.Push(pfModule);
        }

        os::memory::LockHolder holder(loadedPfsLock_);
        this->loadedPfs_.push_back(pfModule);

        return true;
    };
    runtime_->GetClassLinker()->EnumeratePandaFiles(callback, false);
}

static SampleInfo::ThreadStatus GetThreadStatus(ManagedThread *mthread)
{
    ASSERT(mthread != nullptr);

    auto threadStatus = mthread->GetStatus();
    if (threadStatus == ThreadStatus::RUNNING) {
        return SampleInfo::ThreadStatus::RUNNING;
    }

    bool isCoroutineRunning = false;
    if (Coroutine::ThreadIsCoroutine(mthread)) {
        isCoroutineRunning = Coroutine::CastFromThread(mthread)->GetCoroutineStatus() == Coroutine::Status::RUNNING;
    }
    if (threadStatus == ThreadStatus::NATIVE && isCoroutineRunning) {
        return SampleInfo::ThreadStatus::RUNNING;
    }

    return SampleInfo::ThreadStatus::SUSPENDED;
}

struct SamplerFrameInfo {
    Frame *frame;
    bool isCompiled;
};

/**
 * @brief Collects samples from boundary frames.
 * @returns true if bypass frame was found, false otherwise.
 */
static bool CollectBoundaryFrames(SamplerFrameInfo &frameInfo, SampleInfo &sample, size_t &stackCounter)
{
    ASSERT(frameInfo.frame != nullptr);

    bool isFrameBoundary = true;
    while (isFrameBoundary) {
        auto *prevFrame = frameInfo.frame->GetPrevFrame();
        const auto *method = frameInfo.frame->GetMethod();
        if (StackWalkerBase::IsMethodInI2CFrame(method)) {
            sample.stackInfo.managedStack[stackCounter].pandaFilePtr = helpers::ToUnderlying(FrameKind::BRIDGE);
            sample.stackInfo.managedStack[stackCounter].fileId = helpers::ToUnderlying(FrameKind::BRIDGE);
            ++stackCounter;

            frameInfo.frame = prevFrame;
            frameInfo.isCompiled = false;
        } else if (StackWalkerBase::IsMethodInC2IFrame(method)) {
            sample.stackInfo.managedStack[stackCounter].pandaFilePtr = helpers::ToUnderlying(FrameKind::BRIDGE);
            sample.stackInfo.managedStack[stackCounter].fileId = helpers::ToUnderlying(FrameKind::BRIDGE);
            ++stackCounter;

            frameInfo.frame = prevFrame;
            frameInfo.isCompiled = true;
        } else if (StackWalkerBase::IsMethodInBPFrame(method)) {
            g_sLostSamples++;
            return true;
        } else {
            isFrameBoundary = false;
        }
    }
    return false;
}

static void ProcessCompiledTopFrame(SamplerFrameInfo &frameInfo, SampleInfo &sample, size_t &stackCounter,
                                    void *signalContextPtr)
{
    CFrame cframe(frameInfo.frame);
    if (cframe.IsNative()) {
        return;
    }

    auto signalContext = SignalContext(signalContextPtr);
    auto fp = signalContext.GetFP();
    if (fp == nullptr) {
        sample.stackInfo.managedStack[stackCounter].pandaFilePtr = helpers::ToUnderlying(FrameKind::BRIDGE);
        sample.stackInfo.managedStack[stackCounter].fileId = helpers::ToUnderlying(FrameKind::BRIDGE);
        ++stackCounter;

        // fp is not set yet, so cframe not finished, currently in bridge, previous frame iframe
        frameInfo.isCompiled = false;
        return;
    }

    auto pc = signalContext.GetPC();
    bool pcInCompiledCode = InAllocatedCodeRange(pc);
    if (pcInCompiledCode) {
        // Currently in compiled method so get it from fp
        frameInfo.frame = reinterpret_cast<Frame *>(fp);
    } else {
        const LockFreeQueue &pfsQueue = Sampler::GetSampleQueuePF();
        auto pfId = reinterpret_cast<uintptr_t>(frameInfo.frame->GetMethod()->GetPandaFile());
        if (pfsQueue.FindValue(pfId)) {
            sample.stackInfo.managedStack[stackCounter].pandaFilePtr = helpers::ToUnderlying(FrameKind::BRIDGE);
            sample.stackInfo.managedStack[stackCounter].fileId = helpers::ToUnderlying(FrameKind::BRIDGE);
            ++stackCounter;

            // pc not in jitted code, so fp is not up-to-date, currently not in cfame
            frameInfo.isCompiled = false;
        }
    }
}

/**
 * @brief Walk stack frames and collect samples.
 * @returns true if invalid frame was encountered, false otherwise.
 */
static bool CollectFrames(SamplerFrameInfo &frameInfo, SampleInfo &sample, size_t &stackCounter)
{
    const LockFreeQueue &pfsQueue = Sampler::GetSampleQueuePF();
    auto stackWalker = StackWalkerBase(frameInfo.frame, frameInfo.isCompiled);
    while (stackWalker.HasFrame()) {
        auto *method = stackWalker.GetMethod();
        if (method == nullptr || IsInvalidPointer(reinterpret_cast<uintptr_t>(method))) {
            g_sLostSamples++;
            g_sLostInvalidSamples++;
            return true;
        }

        auto *pf = method->GetPandaFile();
        auto pfId = reinterpret_cast<uintptr_t>(pf);
        if (!pfsQueue.FindValue(pfId)) {
            g_sLostSamples++;
            g_sLostNotFindSamples++;
            return true;
        }

        sample.stackInfo.managedStack[stackCounter].pandaFilePtr = pfId;
        sample.stackInfo.managedStack[stackCounter].fileId = method->GetFileId().GetOffset();
        sample.stackInfo.managedStack[stackCounter].bcOffset = stackWalker.GetBytecodePc();
        ++stackCounter;
        stackWalker.NextFrame();

        if (stackCounter == SampleInfo::StackInfo::MAX_STACK_DEPTH) {
            // According to the limitations we should drop all frames that is higher than MAX_STACK_DEPTH
            break;
        }
    }
    sample.timeStamp = Sampler::GetMicrosecondsTimeStamp();
    return false;
}

void SigProfSamplingProfilerHandler([[maybe_unused]] int signum, [[maybe_unused]] siginfo_t *siginfo,
                                    [[maybe_unused]] void *ptr)
{
    if (g_sCurrentHandlersCounter == 0) {
        // Sampling ended if S_CURRENT_HANDLERS_COUNTER is 0. Thread started executing handler for signal
        // that was sent before end, so thread is late now and we should return from handler
        return;
    }
    auto scopedHandlersCounting = ScopedHandlersCounting();

    Coroutine *coro = Coroutine::GetCurrent();
    if (coro != nullptr && coro->GetCoroutineStatus() != Coroutine::Status::RUNNING) {
        return;
    }

    ManagedThread *mthread = coro == nullptr ? ManagedThread::GetCurrent() : coro;
    ASSERT(mthread != nullptr);

    // Checking that code is being executed
    auto *framePtr = reinterpret_cast<CFrame::SlotType *>(mthread->GetCurrentFrame());
    if (framePtr == nullptr) {
        return;
    }

    g_sTotalSamples++;

    // Note that optimized variables may end up with incorrect value as a consequence of a longjmp() operation
    // - see "local variable clobbering and setjmp".
    // Variables below are not volatile because they are not used after longjmp() is done.
    SamplerFrameInfo frameInfo {mthread->GetCurrentFrame(), mthread->IsCurrentFrameCompiled()};

    SampleInfo sample {};
    // `mthread` is passed as non-const argument into `GetThreadStatus`, so call it before `setjmp`
    // in order to bypass "variable might be clobbered by ‘longjmp’" compiler warning.
    sample.threadInfo.threadStatus = GetThreadStatus(mthread);
    sample.threadInfo.threadId = coro == nullptr ? os::thread::GetCurrentThreadId() : coro->GetCoroutineId();
    size_t stackCounter = 0;

    ScopedThreadSampling scopedThreadSampling(mthread->GetPtThreadInfo()->GetSamplingInfo());

    auto &sigSegvJmpBuf = mthread->GetPtThreadInfo()->GetSamplingInfo()->GetSigSegvJmpEnv();
    // NOLINTNEXTLINE(cert-err52-cpp)
    if (setjmp(sigSegvJmpBuf) != 0) {
        // This code executed after longjmp()
        // In case of SIGSEGV we lose the sample
        g_sLostSamples++;
        g_sLostSegvSamples++;
        return;
    }

    if (StackWalkerBase::IsMethodInBoundaryFrame(frameInfo.frame->GetMethod())) {
        auto foundBypassFrame = CollectBoundaryFrames(frameInfo, sample, stackCounter);
        if (foundBypassFrame) {
            return;
        }
    } else if (frameInfo.isCompiled) {
        ProcessCompiledTopFrame(frameInfo, sample, stackCounter, ptr);
    }

    auto lostSample = CollectFrames(frameInfo, sample, stackCounter);
    if (lostSample) {
        return;
    }

    if (stackCounter == 0) {
        return;
    }
    sample.stackInfo.managedStackSize = stackCounter;

    const ThreadCommunicator &communicator = Sampler::GetSampleCommunicator();
    communicator.SendSample(sample);
}

void Sampler::SamplerThreadEntry()
{
    struct sigaction action {};
    action.sa_sigaction = &SigProfSamplingProfilerHandler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    // Clear signal set
    sigemptyset(&action.sa_mask);
    // Ignore incoming sigprof if handler isn't completed
    sigaddset(&action.sa_mask, SIGPROF);

    struct sigaction oldAction {};

    if (sigaction(SIGPROF, &action, &oldAction) == -1) {
        LOG(FATAL, PROFILER) << "Sigaction failed, can't start profiling";
        UNREACHABLE();
    }

    // We keep handler assigned to SigProfSamplingProfilerHandler after sampling end because
    // otherwice deadlock can happen if signal will be slow and reach thread after handler resignation
    if (oldAction.sa_sigaction != nullptr && oldAction.sa_sigaction != SigProfSamplingProfilerHandler) {
        LOG(FATAL, PROFILER) << "SIGPROF signal handler was overriden in sampling profiler";
        UNREACHABLE();
    }
    ++g_sCurrentHandlersCounter;

    auto pid = getpid();
    // Atomic with acquire order reason: To ensure start/stop load correctly
    while (isActive_.load(std::memory_order_acquire)) {
        {
            os::memory::LockHolder holder(managedThreadsLock_);
            for (const auto &threadId : managedThreads_) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                if (syscall(SYS_tgkill, pid, threadId, SIGPROF) != 0) {
                    LOG(DEBUG, PROFILER) << "Can't send signal to thread";
                }
            }
        }
        os::thread::NativeSleepUS(sampleInterval_);
    }

    // Sending last sample on finish to avoid of deadlock in listener
    SampleInfo lastSample;
    lastSample.stackInfo.managedStackSize = 0;
    communicator_.SendSample(lastSample);

    --g_sCurrentHandlersCounter;

    const unsigned int timeToSleepMs = 100;
    do {
        os::thread::NativeSleep(timeToSleepMs);
    } while (g_sCurrentHandlersCounter != 0);
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void Sampler::ListenerThreadEntry(std::unique_ptr<StreamWriter> writerPtr)
{
    // Writing panda files that were loaded before sampler was created
    WriteLoadedPandaFiles(writerPtr.get());

    SampleInfo bufferSample;
    // Atomic with acquire order reason: To ensure start/stop load correctly
    while (isActive_.load(std::memory_order_acquire)) {
        WriteLoadedPandaFiles(writerPtr.get());
        communicator_.ReadSample(&bufferSample);
        if (LIKELY(bufferSample.stackInfo.managedStackSize != 0)) {
            writerPtr->WriteSample(bufferSample);
        }
    }
    // Writing all remaining samples
    while (!communicator_.IsPipeEmpty()) {
        WriteLoadedPandaFiles(writerPtr.get());
        communicator_.ReadSample(&bufferSample);
        if (LIKELY(bufferSample.stackInfo.managedStackSize != 0)) {
            writerPtr->WriteSample(bufferSample);
        }
    }
}

uint64_t Sampler::GetMicrosecondsTimeStamp()
{
    static constexpr int USEC_PER_SEC = 1000 * 1000;
    static constexpr int NSEC_PER_USEC = 1000;
    struct timespec time {};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * USEC_PER_SEC + time.tv_nsec / NSEC_PER_USEC;
}

}  // namespace ark::tooling::sampler
