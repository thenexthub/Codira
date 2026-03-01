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

#include <ctime>

#include "gtest/gtest.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_base-inl.h"

namespace ark::concurrency::test {

class MonitorTest : public testing::Test {
public:
    MonitorTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        // NOLINTNEXTLINE(readability-magic-numbers)
        seed_ = 0xDEADBEEF;
#endif
        srand(seed_);
        // We need to create a runtime instance to be able to create strings.
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options_);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~MonitorTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
        // Logger::Destroy();
    }

    NO_COPY_SEMANTIC(MonitorTest);
    NO_MOVE_SEMANTIC(MonitorTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_ {};

private:
    unsigned seed_ {};
    RuntimeOptions options_;
};

TEST_F(MonitorTest, MonitorEnterTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
}

TEST_F(MonitorTest, MonitorDoubleEnterTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
}

TEST_F(MonitorTest, MonitorDoubleObjectTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header1 = ObjectHeader::Create(cls);
    auto header2 = ObjectHeader::Create(cls);
    Monitor::MonitorEnter(header1);
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
    Monitor::MonitorEnter(header2);
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    Monitor::MonitorExit(header1);
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    Monitor::MonitorExit(header2);
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
}

TEST_F(MonitorTest, HeavyMonitorEnterTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    auto thread = MTManagedThread::GetCurrent();
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    ASSERT_TRUE(Monitor::Inflate(header, thread));
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header);
    // We unlock the monitor, but keep the pointer to it
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_FALSE(Monitor::HoldsLock(header));
}

TEST_F(MonitorTest, HeavyMonitorDeflateTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    auto thread = MTManagedThread::GetCurrent();
    ASSERT_TRUE(Monitor::Inflate(header, thread));
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(Monitor::Deflate(header));
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
}

TEST_F(MonitorTest, HeavyMonitorDoubleEnterTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    auto thread = MTManagedThread::GetCurrent();
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    ASSERT_TRUE(Monitor::Inflate(header, thread));
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_FALSE(Monitor::HoldsLock(header));
}

TEST_F(MonitorTest, HeavyMonitorDoubleObjectTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header1 = ObjectHeader::Create(cls);
    auto header2 = ObjectHeader::Create(cls);
    auto thread = MTManagedThread::GetCurrent();
    ASSERT_TRUE(Monitor::Inflate(header1, thread));
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
    ASSERT_TRUE(Monitor::Inflate(header2, thread));
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header1);
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header2);
    ASSERT_TRUE(header1->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header2->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_FALSE(Monitor::HoldsLock(header1));
    ASSERT_FALSE(Monitor::HoldsLock(header2));
}

TEST_F(MonitorTest, MonitorDoubleObjectHoldsLockTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header1 = ObjectHeader::Create(cls);
    auto header2 = ObjectHeader::Create(cls);
    ASSERT_FALSE(Monitor::HoldsLock(header1));
    ASSERT_FALSE(Monitor::HoldsLock(header2));
    Monitor::MonitorEnter(header1);
    ASSERT_TRUE(Monitor::HoldsLock(header1));
    ASSERT_FALSE(Monitor::HoldsLock(header2));
    Monitor::MonitorEnter(header2);
    ASSERT_TRUE(Monitor::HoldsLock(header1));
    ASSERT_TRUE(Monitor::HoldsLock(header2));
    Monitor::MonitorExit(header1);
    ASSERT_FALSE(Monitor::HoldsLock(header1));
    ASSERT_TRUE(Monitor::HoldsLock(header2));
    Monitor::MonitorExit(header2);
    ASSERT_FALSE(Monitor::HoldsLock(header1));
    ASSERT_FALSE(Monitor::HoldsLock(header2));
}

TEST_F(MonitorTest, MonitorGenerateHashAndEnterTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    auto hash = header->GetHashCode<PandaAssemblyLanguageConfig::MT_MODE>();
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(header);
    // We unlock the monitor, but keep the pointer to it
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header->GetHashCode<PandaAssemblyLanguageConfig::MT_MODE>() == hash);
    ASSERT_FALSE(Monitor::HoldsLock(header));
}

TEST_F(MonitorTest, MonitorEnterAndGenerateHashTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    auto hash = header->GetHashCode<PandaAssemblyLanguageConfig::MT_MODE>();
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header->GetHashCode<PandaAssemblyLanguageConfig::MT_MODE>() == hash);
    Monitor::MonitorExit(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_TRUE(header->GetHashCode<PandaAssemblyLanguageConfig::MT_MODE>() == hash);
    ASSERT_FALSE(Monitor::HoldsLock(header));
}

TEST_F(MonitorTest, HeavyMonitorGcTest)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto thread = MTManagedThread::GetCurrent();
    auto header = ObjectHeader::Create(cls);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> objHandle(thread, header);
    Monitor::MonitorEnter(objHandle.GetPtr());
    ASSERT_TRUE(objHandle->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    ASSERT_TRUE(Monitor::Inflate(objHandle.GetPtr(), thread));
    ASSERT_TRUE(objHandle->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    ASSERT_TRUE(objHandle->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    Monitor::MonitorExit(objHandle.GetPtr());
    ASSERT_TRUE(objHandle->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    ASSERT_TRUE(objHandle->AtomicGetMark().GetState() == MarkWord::STATE_UNLOCKED);
    ASSERT_FALSE(Monitor::HoldsLock(objHandle.GetPtr()));
}

TEST_F(MonitorTest, MonitorTestLightLockOverflow)
{
    if constexpr (std::is_same_v<MemoryModelConfig::Size, uint64_t>) {
        GTEST_SKIP() << "Value MarkWord::LIGHT_LOCK_LOCK_MAX_COUNT too high for iterating";
    }

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *cls = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    auto header = ObjectHeader::Create(cls);
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_LIGHT_LOCKED);
    // Set lock count to MAX-1
    {
        MarkWord mark = header->AtomicGetMark();
        MarkWord newMark = mark.DecodeFromLightLock(mark.GetThreadId(), MarkWord::LIGHT_LOCK_LOCK_MAX_COUNT - 1);
        ASSERT_TRUE(header->AtomicSetMark(mark, newMark));
    }
    Monitor::MonitorEnter(header);
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    // Unlock all recursive locks
    for (uint64_t cnt = 0; cnt < MarkWord::LIGHT_LOCK_LOCK_MAX_COUNT; cnt++) {
        Monitor::MonitorExit(header);
    }
    ASSERT_TRUE(header->AtomicGetMark().GetState() == MarkWord::STATE_HEAVY_LOCKED);
    ASSERT_FALSE(Monitor::HoldsLock(header));
}

}  // namespace ark::concurrency::test
