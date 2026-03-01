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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <atomic>

#include "assembler/assembly-parser.h"
#include "libarkfile/file.h"
#include "libarkbase/trace/trace.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/runtime.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "runtime/interpreter/runtime_interface.h"
#include "tools/sampler/aspt_converter.h"

namespace ark::tooling::sampler::test {

inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

static const char *g_profilerFilename = "profiler_result.aspt";
static const char *g_pandaFileName = "sampling_profiler_test_ark_asm.abc";
static constexpr size_t TEST_CYCLE_THRESHOLD = 100;
static const std::shared_ptr<SamplesRecord> G_SAMPLES_RECORD = std::make_shared<SamplesRecord>();

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class SamplerTest : public testing::Test {
public:
    // NOLINTNEXTLINE(readability-function-size)
    void SetUp() override
    {
        Logger::Initialize(logger::Options(""));

        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetInterpreterType("cpp");
        auto execPath = ark::os::file::File::GetExecutablePath();
        std::string pandaStdLib =
            execPath.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        options.SetBootPandaFiles({pandaStdLib});
        Runtime::Create(options);

        auto pf = panda_file::OpenPandaFileOrZip(g_pandaFileName);
        Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

        thread_ = ark::MTManagedThread::GetCurrent();
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }

    void FullfillFakeSample(SampleInfo *ps)
    {
        for (uint32_t i = 0; i < SampleInfo::StackInfo::MAX_STACK_DEPTH; ++i) {
            ps->stackInfo.managedStack[i] = {i, pfId_};
        }
        ps->threadInfo.threadId = GetThreadId();
        ps->stackInfo.managedStackSize = SampleInfo::StackInfo::MAX_STACK_DEPTH;
    }

    // Friend wrappers for accesing samplers private fields
    static os::thread::NativeHandleType ExtractListenerTid(const Sampler *sPtr)
    {
        return sPtr->listenerTid_;
    }

    static os::thread::NativeHandleType ExtractSamplerTid(const Sampler *sPtr)
    {
        return sPtr->samplerTid_;
    }

    static PandaSet<os::thread::ThreadId> ExtractManagedThreads(Sampler *sPtr)
    {
        // Sending a copy to avoid of datarace
        os::memory::LockHolder holder(sPtr->managedThreadsLock_);
        PandaSet<os::thread::ThreadId> managedThreadsCopy = sPtr->managedThreads_;
        return managedThreadsCopy;
    }

    static size_t ExtractLoadedPFSize(Sampler *sPtr)
    {
        os::memory::LockHolder holder(sPtr->loadedPfsLock_);
        return sPtr->loadedPfs_.size();
    }

    static std::array<int, 2> ExtractPipes(const Sampler *sPtr)
    {
        // Sending a copy to avoid of datarace
        return sPtr->communicator_.listenerPipe_;
    }

    static bool ExtractIsActive(const Sampler *sPtr)
    {
        // Atomic with acquire order reason: To ensure start/stop load correctly
        return sPtr->isActive_.load(std::memory_order_acquire);
    }

    uint32_t GetThreadId()
    {
        return os::thread::GetCurrentThreadId();
    }

protected:
    ark::MTManagedThread *thread_ {nullptr};
    uintptr_t pfId_ {0};
    uint32_t checksum_ {0};
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

TEST_F(SamplerTest, SamplerInitTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);

    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), false);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);

    // Second run
    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorSamplerInitTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);

    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), false);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);

    // Second run
    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

void RunManagedThread(std::atomic<bool> *syncFlag)
{
    auto *mThr = ark::MTManagedThread::Create(ark::Runtime::GetCurrent(), ark::Runtime::GetCurrent()->GetPandaVM());
    mThr->ManagedCodeBegin();

    *syncFlag = true;
    while (*syncFlag) {
        // Calling safepoint 'cause starting profiler required to stop all managed threads
        interpreter::RuntimeInterface::Safepoint();
    }

    mThr->ManagedCodeEnd();
    mThr->Destroy();
}

void RunManagedThreadAndSaveThreadId(std::atomic<bool> *syncFlag, os::thread::ThreadId *id)
{
    auto *mThr = ark::MTManagedThread::Create(ark::Runtime::GetCurrent(), ark::Runtime::GetCurrent()->GetPandaVM());
    mThr->ManagedCodeBegin();

    *id = os::thread::GetCurrentThreadId();
    *syncFlag = true;
    while (*syncFlag) {
        // Calling safepoint 'cause starting profiler required to stop all managed threads
        interpreter::RuntimeInterface::Safepoint();
    }

    mThr->ManagedCodeEnd();
    mThr->Destroy();
}

void RunNativeThread(std::atomic<bool> *syncFlag)
{
    auto *mThr = ark::MTManagedThread::Create(ark::Runtime::GetCurrent(), ark::Runtime::GetCurrent()->GetPandaVM());

    *syncFlag = true;
    while (*syncFlag) {
    }

    mThr->Destroy();
}

// Testing notification thread started/finished
TEST_F(SamplerTest, SamplerEventThreadNotificationTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_FALSE(ExtractManagedThreads(sp).empty());
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    std::atomic<bool> syncFlag1 = false;
    std::atomic<bool> syncFlag2 = false;
    std::atomic<bool> syncFlag3 = false;
    std::thread managedThread1(RunManagedThread, &syncFlag1);
    std::thread managedThread2(RunManagedThread, &syncFlag2);
    std::thread managedThread3(RunManagedThread, &syncFlag3);

    while (!syncFlag1 || !syncFlag2 || !syncFlag3) {
        ;
    }
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4UL);

    syncFlag1 = false;
    syncFlag2 = false;
    syncFlag3 = false;
    managedThread1.join();
    managedThread2.join();
    managedThread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorSamplerEventThreadNotificationTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_FALSE(ExtractManagedThreads(sp).empty());
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    std::atomic<bool> syncFlag1 = false;
    std::atomic<bool> syncFlag2 = false;
    std::atomic<bool> syncFlag3 = false;
    std::thread managedThread1(RunManagedThread, &syncFlag1);
    std::thread managedThread2(RunManagedThread, &syncFlag2);
    std::thread managedThread3(RunManagedThread, &syncFlag3);

    while (!syncFlag1 || !syncFlag2 || !syncFlag3) {
        ;
    }
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4UL);

    syncFlag1 = false;
    syncFlag2 = false;
    syncFlag3 = false;
    managedThread1.join();
    managedThread2.join();
    managedThread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing notification thread started/finished
TEST_F(SamplerTest, SamplerCheckThreadIdTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    std::atomic<bool> syncFlag1 = false;
    os::thread::ThreadId mtId = 0;
    std::thread managedThread1(RunManagedThreadAndSaveThreadId, &syncFlag1, &mtId);

    while (!syncFlag1) {
        ;
    }
    // only one additional managed thread must be running
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 2UL);
    bool isPassed = false;

    for (const auto &elem : ExtractManagedThreads(sp)) {
        if (elem == mtId) {
            isPassed = true;
            break;
        }
    }
    ASSERT_TRUE(isPassed);

    syncFlag1 = false;
    managedThread1.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorSamplerCheckThreadIdTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    std::atomic<bool> syncFlag1 = false;
    os::thread::ThreadId mtId = 0;
    std::thread managedThread1(RunManagedThreadAndSaveThreadId, &syncFlag1, &mtId);

    while (!syncFlag1) {
        ;
    }
    // only one additional managed thread must be running
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 2UL);
    bool isPassed = false;

    for (const auto &elem : ExtractManagedThreads(sp)) {
        if (elem == mtId) {
            isPassed = true;
            break;
        }
    }
    ASSERT_TRUE(isPassed);

    syncFlag1 = false;
    managedThread1.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing thread collection
TEST_F(SamplerTest, SamplerCollectThreadTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    std::atomic<bool> syncFlag1 = false;
    std::atomic<bool> syncFlag2 = false;
    std::atomic<bool> syncFlag3 = false;
    std::thread managedThread1(RunManagedThread, &syncFlag1);
    std::thread managedThread2(RunManagedThread, &syncFlag2);
    std::thread managedThread3(RunManagedThread, &syncFlag3);

    while (!syncFlag1 || !syncFlag2 || !syncFlag3) {
        ;
    }

    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4UL);

    syncFlag1 = false;
    syncFlag2 = false;
    syncFlag3 = false;
    managedThread1.join();
    managedThread2.join();
    managedThread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorSamplerCollectThreadTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    std::atomic<bool> syncFlag1 = false;
    std::atomic<bool> syncFlag2 = false;
    std::atomic<bool> syncFlag3 = false;
    std::thread managedThread1(RunManagedThread, &syncFlag1);
    std::thread managedThread2(RunManagedThread, &syncFlag2);
    std::thread managedThread3(RunManagedThread, &syncFlag3);

    while (!syncFlag1 || !syncFlag2 || !syncFlag3) {
        ;
    }

    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4UL);

    syncFlag1 = false;
    syncFlag2 = false;
    syncFlag3 = false;
    managedThread1.join();
    managedThread2.join();
    managedThread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing native thread collection
TEST_F(SamplerTest, SamplerCollectNativeThreadTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    std::atomic<bool> syncFlag1 = false;
    std::atomic<bool> syncFlag2 = false;
    std::atomic<bool> syncFlag3 = false;
    std::thread managedThread1(RunManagedThread, &syncFlag1);
    std::thread nativeThread2(RunNativeThread, &syncFlag2);

    while (!syncFlag1 || !syncFlag2) {
        ;
    }

    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    // two additional threads must be running - a managed and a native one
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 3UL);
    std::thread nativeThread3(RunNativeThread, &syncFlag3);
    while (!syncFlag3) {
        ;
    }

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4UL);

    syncFlag1 = false;
    syncFlag2 = false;
    syncFlag3 = false;
    managedThread1.join();
    nativeThread2.join();
    nativeThread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorSamplerCollectNativeThreadTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    std::atomic<bool> syncFlag1 = false;
    std::atomic<bool> syncFlag2 = false;
    std::atomic<bool> syncFlag3 = false;
    std::thread managedThread1(RunManagedThread, &syncFlag1);
    std::thread nativeThread2(RunNativeThread, &syncFlag2);

    while (!syncFlag1 || !syncFlag2) {
        ;
    }

    ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
    ASSERT_NE(ExtractListenerTid(sp), 0);
    ASSERT_NE(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), true);

    // two additional threads must be running - a managed and a native one
    ASSERT_EQ(ExtractManagedThreads(sp).size(), 3UL);
    std::thread nativeThread3(RunNativeThread, &syncFlag3);
    while (!syncFlag3) {
        ;
    }

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 4UL);

    syncFlag1 = false;
    syncFlag2 = false;
    syncFlag3 = false;
    managedThread1.join();
    nativeThread2.join();
    nativeThread3.join();

    ASSERT_EQ(ExtractManagedThreads(sp).size(), 1);

    sp->Stop();
    ASSERT_EQ(ExtractListenerTid(sp), 0);
    ASSERT_EQ(ExtractSamplerTid(sp), 0);
    ASSERT_EQ(ExtractIsActive(sp), false);
    Sampler::Destroy(sp);
}

// Testing pipes
TEST_F(SamplerTest, SamplerPipesTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename));

    ASSERT_NE(ExtractPipes(sp)[ThreadCommunicator::PIPE_READ_ID], 0);
    ASSERT_NE(ExtractPipes(sp)[ThreadCommunicator::PIPE_WRITE_ID], 0);

    sp->Stop();
    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorSamplerPipesTest)
{
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD));

    ASSERT_NE(ExtractPipes(sp)[ThreadCommunicator::PIPE_READ_ID], 0);
    ASSERT_NE(ExtractPipes(sp)[ThreadCommunicator::PIPE_WRITE_ID], 0);

    sp->Stop();
    Sampler::Destroy(sp);
}

// Stress testing restart
TEST_F(SamplerTest, ProfilerRestartStressTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD / 10;
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    for (uint32_t i = 0; i < CURRENT_TEST_THRESHOLD; i++) {
        ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(g_profilerFilename)), true);
        sp->Stop();
    }

    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, InspectorProfilerRestartStressTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD / 10;
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);

    for (uint32_t i = 0; i < CURRENT_TEST_THRESHOLD; i++) {
        ASSERT_EQ(sp->Start(std::make_unique<InspectorStreamWriter>(G_SAMPLES_RECORD)), true);
        sp->Stop();
    }

    Sampler::Destroy(sp);
}

TEST_F(SamplerTest, ThreadCommunicatorTest)
{
    ThreadCommunicator communicator;

    SampleInfo sampleInput;
    SampleInfo sampleOutput;
    FullfillFakeSample(&sampleInput);
    ASSERT_TRUE(communicator.Init());
    ASSERT_TRUE(communicator.SendSample(sampleInput));
    ASSERT_TRUE(communicator.ReadSample(&sampleOutput));
    ASSERT_EQ(sampleOutput, sampleInput);
}

static void CommunicatorStressWritterThread(const ThreadCommunicator *com, const SampleInfo &sample,
                                            uint32_t messagesAmount)
{
    for (uint32_t i = 0; i < messagesAmount; ++i) {
        // If the sample write failed we retrying to send it
        if (!com->SendSample(sample)) {
            std::cerr << "Failed to send a sample" << std::endl;
            Runtime::Abort();
        }
    }
}

static constexpr uint32_t MESSAGES_AMOUNT = TEST_CYCLE_THRESHOLD * 100;

TEST_F(SamplerTest, ThreadCommunicatorMultithreadTest)
{
    ThreadCommunicator communicator;
    SampleInfo sampleOutput;
    SampleInfo sampleInput;
    FullfillFakeSample(&sampleInput);
    ASSERT_TRUE(communicator.Init());

    std::thread sender(CommunicatorStressWritterThread, &communicator, sampleInput, MESSAGES_AMOUNT);
    for (uint32_t i = 0; i < MESSAGES_AMOUNT; ++i) {
        // If the sample write failed we retrying to send it
        if (!communicator.ReadSample(&sampleOutput)) {
            std::cerr << "Failed to read a sample" << std::endl;
            Runtime::Abort();
        }
        ASSERT_EQ(sampleOutput, sampleInput);
    }
    sender.join();
}

// Testing reader and writer by writing and reading from .aspt one sample
TEST_F(SamplerTest, StreamWriterReaderTest)
{
    const char *streamTestFilename = "stream_writer_reader_test.aspt";
    SampleInfo sampleOutput;
    SampleInfo sampleInput;

    {
        FileStreamWriter writer(streamTestFilename);
        FullfillFakeSample(&sampleInput);

        writer.WriteSample(sampleInput);
    }

    SampleReader reader(streamTestFilename);
    ASSERT_TRUE(reader.GetNextSample(&sampleOutput));
    ASSERT_EQ(sampleOutput, sampleInput);
    ASSERT_FALSE(reader.GetNextSample(&sampleOutput));
    ASSERT_FALSE(reader.GetNextModule(nullptr));
}

TEST_F(SamplerTest, InspectorStreamWriterReaderTest)
{
    const std::shared_ptr<SamplesRecord> samplesRecord = std::make_shared<SamplesRecord>();
    SampleInfo sampleInput;

    {
        InspectorStreamWriter writer(samplesRecord);
        FullfillFakeSample(&sampleInput);

        writer.WriteSample(sampleInput);
    }

    auto allThreadsProfileInfos = samplesRecord->GetAllThreadsProfileInfos();
    ASSERT_NE(allThreadsProfileInfos, nullptr);
    ASSERT_EQ(allThreadsProfileInfos->size(), 1);
    ASSERT_NE((*allThreadsProfileInfos)[0]->nodeCount, 0);
    ASSERT_EQ((*allThreadsProfileInfos)[0]->startTime, 0);
    ASSERT_EQ((*allThreadsProfileInfos)[0]->stopTime, 0);
    ASSERT_EQ((*allThreadsProfileInfos)[0]->tid, GetThreadId());
}

// Testing reader and writer by writing and reading from .aspt lots of samples
TEST_F(SamplerTest, StreamWriterReaderLotsSamplesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *streamTestFilename = "stream_writer_reader_test_lots_samples.aspt";
    SampleInfo sampleOutput;
    SampleInfo sampleInput;

    {
        FileStreamWriter writer(streamTestFilename);
        FullfillFakeSample(&sampleInput);

        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteSample(sampleInput);
        }
    }

    SampleReader reader(streamTestFilename);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextSample(&sampleOutput));
        ASSERT_EQ(sampleOutput, sampleInput);
    }
    ASSERT_FALSE(reader.GetNextSample(&sampleOutput));
    ASSERT_FALSE(reader.GetNextModule(nullptr));
}

TEST_F(SamplerTest, InspectorStreamWriterReaderLotsSamplesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const std::shared_ptr<SamplesRecord> samplesRecord = std::make_shared<SamplesRecord>();
    SampleInfo sampleInput;

    {
        InspectorStreamWriter writer(samplesRecord);
        FullfillFakeSample(&sampleInput);

        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteSample(sampleInput);
        }
    }

    auto allThreadsProfileInfos = samplesRecord->GetAllThreadsProfileInfos();
    ASSERT_NE(allThreadsProfileInfos, nullptr);
    ASSERT_EQ(allThreadsProfileInfos->size(), 1);
    ASSERT_NE((*allThreadsProfileInfos)[0]->nodeCount, 0);
    ASSERT_EQ((*allThreadsProfileInfos)[0]->startTime, 0);
    ASSERT_EQ((*allThreadsProfileInfos)[0]->stopTime, 0);
    ASSERT_EQ((*allThreadsProfileInfos)[0]->tid, GetThreadId());
    ASSERT_EQ((*allThreadsProfileInfos)[0]->timeDeltas.size(), CURRENT_TEST_THRESHOLD);
}

// Testing reader and writer by writing and reading from .aspt one module
TEST_F(SamplerTest, ModuleWriterReaderTest)
{
    const char *streamTestFilename = "stream_module_test_filename.aspt";
    FileInfo moduleInput = {pfId_, checksum_, "~/folder/folder/lib/panda_file.pa"};
    FileInfo moduleOutput = {};

    {
        FileStreamWriter writer(streamTestFilename);
        writer.WriteModule(moduleInput);
    }

    SampleReader reader(streamTestFilename);
    ASSERT_TRUE(reader.GetNextModule(&moduleOutput));
    ASSERT_EQ(moduleOutput, moduleInput);
    ASSERT_FALSE(reader.GetNextModule(&moduleOutput));
    ASSERT_FALSE(reader.GetNextSample(nullptr));
}

// Testing reader and writer by writing and reading from .aspt lots of modules
TEST_F(SamplerTest, ModuleWriterReaderLotsModulesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *streamTestFilename = "stream_lots_modules_test_filename.aspt";
    FileInfo moduleInput = {pfId_, checksum_, "~/folder/folder/lib/panda_file.pa"};
    FileInfo moduleOutput = {};

    {
        FileStreamWriter writer(streamTestFilename);
        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteModule(moduleInput);
        }
    }

    SampleReader reader(streamTestFilename);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextModule(&moduleOutput));
        ASSERT_EQ(moduleOutput, moduleInput);
    }
    ASSERT_FALSE(reader.GetNextModule(&moduleOutput));
    ASSERT_FALSE(reader.GetNextSample(nullptr));
}

// Testing reader and writer by writing and reading from .aspt lots of modules
TEST_F(SamplerTest, WriterReaderLotsRowsModulesAndSamplesTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *streamTestFilename = "stream_lots_modules_and_samples_test_filename.aspt";
    FileInfo moduleInput = {pfId_, checksum_, "~/folder/folder/lib/panda_file.pa"};
    FileInfo moduleOutput = {};
    SampleInfo sampleOutput;
    SampleInfo sampleInput;

    {
        FileStreamWriter writer(streamTestFilename);
        FullfillFakeSample(&sampleInput);
        for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
            writer.WriteModule(moduleInput);
            writer.WriteSample(sampleInput);
        }
    }

    SampleReader reader(streamTestFilename);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextModule(&moduleOutput));
        ASSERT_EQ(moduleOutput, moduleInput);
    }

    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        ASSERT_TRUE(reader.GetNextSample(&sampleOutput));
        ASSERT_EQ(sampleOutput, sampleInput);
    }

    ASSERT_FALSE(reader.GetNextModule(&moduleOutput));
    ASSERT_FALSE(reader.GetNextSample(&sampleOutput));
}

// Send sample to listener and check it inside the file
TEST_F(SamplerTest, ListenerWriteFakeSampleTest)
{
    const char *streamTestFilename = "listener_write_fake_sample_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(streamTestFilename)), true);

    SampleInfo sampleOutput;
    SampleInfo sampleInput;
    FullfillFakeSample(&sampleInput);
    sp->GetCommunicator().SendSample(sampleInput);
    sp->Stop();

    bool status = true;
    bool isPassed = false;
    SampleReader reader(streamTestFilename);
    while (status) {
        status = reader.GetNextSample(&sampleOutput);
        if (sampleOutput == sampleInput) {
            isPassed = true;
            break;
        }
    }

    ASSERT_TRUE(isPassed);

    Sampler::Destroy(sp);
}

// Send lots of sample to listener and check it inside the file
TEST_F(SamplerTest, ListenerWriteLotsFakeSampleTest)
{
    constexpr size_t CURRENT_TEST_THRESHOLD = TEST_CYCLE_THRESHOLD * 100;
    const char *streamTestFilename = "listener_write_lots_fake_sample_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(streamTestFilename)), true);

    SampleInfo sampleOutput;
    SampleInfo sampleInput;
    size_t sentSamplesCounter = 0;
    FullfillFakeSample(&sampleInput);
    for (size_t i = 0; i < CURRENT_TEST_THRESHOLD; ++i) {
        if (sp->GetCommunicator().SendSample(sampleInput)) {
            ++sentSamplesCounter;
        }
    }
    sp->Stop();

    bool status = true;
    size_t amountOfSamples = 0;
    SampleReader reader(streamTestFilename);
    while (status) {
        if (sampleOutput == sampleInput) {
            ++amountOfSamples;
        }
        status = reader.GetNextSample(&sampleOutput);
    }

    ASSERT_EQ(amountOfSamples, sentSamplesCounter);

    Sampler::Destroy(sp);
}

// Checking that sampler collect panda files correctly
TEST_F(SamplerTest, CollectPandaFilesTest)
{
    const char *streamTestFilename = "collect_panda_file_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(streamTestFilename)), true);
    sp->Stop();

    FileInfo moduleInfo;
    SampleReader reader(streamTestFilename);
    bool status = false;
    while (reader.GetNextModule(&moduleInfo)) {
        auto pfPtr = reinterpret_cast<panda_file::File *>(moduleInfo.ptr);
        ASSERT_EQ(pfPtr->GetFullFileName(), moduleInfo.pathname);
        status = true;
    }
    ASSERT_TRUE(status);
    Sampler::Destroy(sp);
}

// Checking that sampler collect panda files correctly
TEST_F(SamplerTest, WriteModuleEventTest)
{
    const char *streamTestFilename = "collect_panda_file_test.aspt";
    auto *sp = Sampler::Create();
    ASSERT_NE(sp, nullptr);
    ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(streamTestFilename)), true);

    auto execPath = ark::os::file::File::GetExecutablePath();
    std::string pandafile =
        execPath.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";

    auto pf = panda_file::OpenPandaFileOrZip(pandafile);
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));
    // NOTE:
    // It's necessary to add assert that only one module is loaded
    // But simply add assert on loaded size is UB, because such assert may fail if the WriteLoadedPandaFiles
    // managed to dump the loaded module into the trace before the assert call occurs in this test

    sp->Stop();

    FileInfo moduleInfo;
    SampleReader reader(streamTestFilename);
    bool status = false;
    while (reader.GetNextModule(&moduleInfo)) {
        auto pfPtr = reinterpret_cast<panda_file::File *>(moduleInfo.ptr);
        ASSERT_EQ(pfPtr->GetFullFileName(), moduleInfo.pathname);
        status = true;
    }
    ASSERT_TRUE(status);

    Sampler::Destroy(sp);
}

// Sampling big pandasm program and convert it
TEST_F(SamplerTest, ProfilerSamplerSignalHandlerTest)
{
    const char *streamTestFilename = "sampler_signal_handler_test.aspt";
    const char *resultTestFilename = "sampler_signal_handler_test.csv";
    size_t sampleCounter = 0;

    {
        auto *sp = Sampler::Create();
        ASSERT_NE(sp, nullptr);
        ASSERT_EQ(sp->Start(std::make_unique<FileStreamWriter>(streamTestFilename)), true);

        {
            ASSERT_TRUE(Runtime::GetCurrent()->Execute("_GLOBAL::main", {}));
        }
        sp->Stop();

        SampleInfo sample;
        SampleReader reader(streamTestFilename);
        bool isFind = false;

        while (reader.GetNextSample(&sample)) {
            ++sampleCounter;
            if (sample.stackInfo.managedStackSize == 2U) {
                isFind = true;
                continue;
            }
            ASSERT_NE(sample.stackInfo.managedStackSize, 0);
            ASSERT_EQ(GetThreadId(), sample.threadInfo.threadId);
        }

        ASSERT_EQ(isFind, true);

        Sampler::Destroy(sp);
    }

    // Checking converter
    {
        AsptConverter conv(streamTestFilename);
        ASSERT_EQ(conv.CollectTracesStats(), sampleCounter);
        ASSERT_TRUE(conv.CollectModules());
        ASSERT_TRUE(conv.DumpResolvedTracesAsCSV(resultTestFilename));
    }
}

}  // namespace ark::tooling::sampler::test
