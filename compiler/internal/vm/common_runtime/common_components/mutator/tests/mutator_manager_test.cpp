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

#include "common_components/heap/ark_collector/ark_collector.h"
#include "common_components/tests/test_helper.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_components/common_runtime/base_runtime.cpp"

using namespace common;

namespace common::test {
class MutatorManagerTest : public BaseTestWithScope {
protected:
    void SetUp() override
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }
    void TearDown() override
    {
        BaseRuntime::GetInstance()->FiniFromDynamic();
    }
};

HWTEST_F_L0(MutatorManagerTest, BindMutatorOnly_Test1)
{
    MutatorManager *managerPtr = new MutatorManager();
    Mutator mutator;
    mutator.Init();
    managerPtr->UnbindMutatorOnly();
    bool res = managerPtr->BindMutatorOnly(&mutator);
    ASSERT_TRUE(res);
    delete managerPtr;
}

HWTEST_F_L0(MutatorManagerTest, IsRuntimeThread_Test1)
{
    ThreadLocalData* localData = ThreadLocal::GetThreadLocalData();
    localData->threadType = ThreadType::HOT_UPDATE_THREAD;
    bool ret = IsRuntimeThread();
    EXPECT_TRUE(ret == true);

    localData->threadType = ThreadType::ARK_PROCESSOR;
    ret = IsRuntimeThread();
    EXPECT_TRUE(ret == false);
}

HWTEST_F_L0(MutatorManagerTest, IsGcThread_Test1)
{
    ThreadLocalData* localData = ThreadLocal::GetThreadLocalData();
    localData->threadType = ThreadType::GC_THREAD;
    bool ret = IsGcThread();
    EXPECT_TRUE(ret == true);

    localData->threadType = ThreadType::ARK_PROCESSOR;
    ret = IsGcThread();
    EXPECT_TRUE(ret == false);
}

HWTEST_F_L0(MutatorManagerTest, BindMutator_Test1)
{
    ThreadLocalData* localData = ThreadLocal::GetThreadLocalData();
    localData->buffer = nullptr;
    MutatorManager *managerPtr = new MutatorManager();
    Mutator mutator;
    mutator.Init();
    managerPtr->BindMutator(mutator);
    EXPECT_TRUE(localData->buffer != nullptr);

    managerPtr->BindMutator(mutator);
    EXPECT_TRUE(localData->mutator != nullptr);
    delete managerPtr;
}

HWTEST_F_L0(MutatorManagerTest, CreateMutator_Test1)
{
    ThreadLocalData* localData = ThreadLocal::GetThreadLocalData();
    localData->mutator = new Mutator();
    MutatorManager *managerPtr = new MutatorManager();
    Mutator* ptr = managerPtr->CreateMutator();
    EXPECT_TRUE(ptr != nullptr);

    delete managerPtr;
    delete localData->mutator;
}

HWTEST_F_L0(MutatorManagerTest, CreateRuntimeMutato_Test1)
{
    ThreadType threadType = ThreadType::FP_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    EXPECT_TRUE(ptr != nullptr);

    threadType = ThreadType::GC_THREAD;
    ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    EXPECT_TRUE(ptr != nullptr);
}

HWTEST_F_L0(MutatorManagerTest, DestroyRuntimeMutator_Test1)
{
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    EXPECT_NE(ptr, nullptr);

    MutatorManager::Instance().DestroyRuntimeMutator(threadType);
    ptr = ThreadLocal::GetMutator();
    EXPECT_EQ(ptr, nullptr);
}

HWTEST_F_L0(MutatorManagerTest, DestroyMutator_Test1)
{
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);

    MutatorManager::Instance().DestroyMutator(ptr);
    EXPECT_TRUE(MutatorManager::Instance().TryAcquireMutatorManagementRLock());
    MutatorManager::Instance().MutatorManagementRUnlock();

    MutatorManager::Instance().MutatorManagementWLock();
    MutatorManager::Instance().DestroyMutator(ptr);
    EXPECT_FALSE(MutatorManager::Instance().TryAcquireMutatorManagementRLock());
    MutatorManager::Instance().MutatorManagementWUnlock();
}

HWTEST_F_L0(MutatorManagerTest, AcquireMutatorManagementWLockForCpuProfile_Test1)
{
    std::atomic<bool> threadStarted{false};
    std::thread testthread([&]() {
        threadStarted = true;
        MutatorManager::Instance().MutatorManagementWLock();
        MutatorManager::Instance().AcquireMutatorManagementWLockForCpuProfile();
    });
    while (!threadStarted) {}
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    MutatorManager::Instance().MutatorManagementWUnlock();
    testthread.join();
    EXPECT_FALSE(MutatorManager::Instance().TryAcquireMutatorManagementRLock());
    MutatorManager::Instance().MutatorManagementWUnlock();
}

HWTEST_F_L0(MutatorManagerTest, EnsureCpuProfileFinish_Test1)
{
    std::list<Mutator*> undoneMutators;
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    ptr->SetCpuProfileState(MutatorBase::CpuProfileState::FINISH_CPUPROFILE);
    undoneMutators.push_back(ptr);
    MutatorManager::Instance().EnsureCpuProfileFinish(undoneMutators);
    EXPECT_EQ(undoneMutators.size(), 0);
}

HWTEST_F_L0(MutatorManagerTest, EnsureCpuProfileFinish_Test2)
{
    std::list<Mutator*> Mutators;
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    ptr->SetCpuProfileState(MutatorBase::CpuProfileState::NO_CPUPROFILE);
    ptr->SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);
    Mutators.push_back(ptr);
    MutatorManager::Instance().EnsureCpuProfileFinish(Mutators);
    EXPECT_EQ(Mutators.size(), 0);
}

HWTEST_F_L0(MutatorManagerTest, EnsureCpuProfileFinish_Test3)
{
    std::list<Mutator*> Mutators;
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    std::thread testthread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        ptr->SetCpuProfileState(MutatorBase::CpuProfileState::NO_CPUPROFILE);
    });
    ptr->SetCpuProfileState(MutatorBase::CpuProfileState::IN_CPUPROFILING);
    ptr->SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);
    Mutators.push_back(ptr);
    MutatorManager::Instance().EnsureCpuProfileFinish(Mutators);
    testthread.join();
    EXPECT_EQ(Mutators.size(), 0);
}

HWTEST_F_L0(MutatorManagerTest, EnsureCpuProfileFinish_Test4)
{
    std::list<Mutator*> Mutators;
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    std::thread testthread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        ptr->SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);
        ptr->SetCpuProfileState(MutatorBase::CpuProfileState::NO_CPUPROFILE);
    });
    ptr->SetCpuProfileState(MutatorBase::CpuProfileState::IN_CPUPROFILING);
    ptr->SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_FALSE);
    Mutators.push_back(ptr);
    MutatorManager::Instance().EnsureCpuProfileFinish(Mutators);
    testthread.join();
    EXPECT_EQ(Mutators.size(), 0);
}

HWTEST_F_L0(MutatorManagerTest, EnsureCpuProfileFinish_Test5)
{
    std::list<Mutator*> Mutators;
    ThreadType threadType = ThreadType::GC_THREAD;
    Mutator* ptr = MutatorManager::Instance().CreateRuntimeMutator(threadType);
    std::thread testthread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        ptr->SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);

    });
    ptr->SetCpuProfileState(MutatorBase::CpuProfileState::NO_CPUPROFILE);
    ptr->SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_FALSE);
    Mutators.push_back(ptr);
    MutatorManager::Instance().EnsureCpuProfileFinish(Mutators);
    testthread.join();
    EXPECT_EQ(Mutators.size(), 0);
}

HWTEST_F_L0(MutatorManagerTest, EnsurePhaseTransition_Test1)
{
    std::list<Mutator*> mutators;
    MutatorManager *managerPtr = new MutatorManager();
    Mutator mutator1;
    mutator1.Init();
    mutator1.SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);
    managerPtr->BindMutator(mutator1);

    Mutator mutator2;
    mutator2.Init();
    mutator2.SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);
    managerPtr->BindMutator(mutator2);
    
    mutators.push_back(&mutator1);
    EXPECT_EQ(mutators.size(), 1);
    mutators.push_back(&mutator2);
    EXPECT_EQ(mutators.size(), 2);

    GCPhase targetPhase = GCPhase::GC_PHASE_IDLE;
    managerPtr->EnsurePhaseTransition(targetPhase, mutators);
    EXPECT_EQ(mutators.size(), 0);
    delete managerPtr;
}

HWTEST_F_L0(MutatorManagerTest, EnsurePhaseTransition_Test2)
{
    std::list<Mutator*> mutators;
    MutatorManager *managerPtr = new MutatorManager();
    Mutator mutator1;
    mutator1.Init();
    mutator1.SetInSaferegion(MutatorBase::SaferegionState::SAFE_REGION_TRUE);
    managerPtr->BindMutator(mutator1);

    Mutator mutator2;
    mutator2.Init();
    mutator2.SetMutatorPhase(GCPhase::GC_PHASE_IDLE);
    EXPECT_EQ(mutator2.GetMutatorPhase(), GCPhase::GC_PHASE_IDLE);
    MutatorBase* base = static_cast<MutatorBase*>(mutator2.GetMutatorBasePtr());
    base->Init();
    MutatorBase::SuspensionType flag = MutatorBase::SuspensionType::SUSPENSION_FOR_STW;
    base->SetSuspensionFlag(flag);
    flag = MutatorBase::SuspensionType::SUSPENSION_FOR_GC_PHASE;
    base->SetSuspensionFlag(flag);
    base->HandleSuspensionRequest();
    EXPECT_TRUE(mutator2.FinishedTransition());
    managerPtr->BindMutator(mutator2);
    
    mutators.push_back(&mutator1);
    EXPECT_EQ(mutators.size(), 1);
    mutators.push_back(&mutator2);
    EXPECT_EQ(mutators.size(), 2);

    GCPhase targetPhase = GCPhase::GC_PHASE_IDLE;
    managerPtr->EnsurePhaseTransition(targetPhase, mutators);
    EXPECT_EQ(mutators.size(), 0);
    delete managerPtr;
}
}  // namespace common::test
