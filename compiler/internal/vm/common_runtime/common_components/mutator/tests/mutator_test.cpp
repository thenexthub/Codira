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

#include "common_components/heap/heap_manager.h"
#include "common_components/mutator/mutator.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_components/tests/test_helper.h"
#include "common_interfaces/base_runtime.h"
#include "common_interfaces/objects/base_object.h"

using namespace common;

namespace common::test {
class TestMutator : public Mutator {
public:
    using Mutator::VisitRawObjects;
};
class MutatorTest : public BaseTestWithScope {
protected:
    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }
    static void TearDownTestCase() {}
};

HWTEST_F_L0(MutatorTest, GetThreadLocalData_Test1)
{
    ThreadLocalData *ret = GetThreadLocalData();
    ASSERT_TRUE(ret != nullptr);
}

HWTEST_F_L0(MutatorTest, TransitionGCPhase_Test1)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    bool ret = mutatorBase.TransitionGCPhase(false);
    EXPECT_TRUE(ret == true);

    MutatorBase::SuspensionType flag = MutatorBase::SuspensionType::SUSPENSION_FOR_GC_PHASE;
    mutatorBase.SetSuspensionFlag(flag);

    ret = mutatorBase.TransitionGCPhase(false);
    EXPECT_TRUE(ret == true);

    ret = mutatorBase.TransitionGCPhase(true);
    EXPECT_TRUE(ret == true);
    ret = mutatorBase.TransitionGCPhase(false);
    EXPECT_TRUE(ret == true);
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_Test0)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();

    MutatorBase::SuspensionType  flag = MutatorBase::SuspensionType::SUSPENSION_FOR_STW;
    mutatorBase.SetSuspensionFlag(flag);

    mutatorBase.HandleSuspensionRequest();
    EXPECT_TRUE(mutatorBase.InSaferegion() == false);

    flag = MutatorBase::SuspensionType::SUSPENSION_FOR_GC_PHASE;
    mutatorBase.SetSuspensionFlag(flag);
    mutatorBase.HandleSuspensionRequest();
    EXPECT_TRUE(mutatorBase.InSaferegion() == false);
}

HWTEST_F_L0(MutatorTest, SuspendForStw_Test0)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();

    mutatorBase.SuspendForStw();
    EXPECT_TRUE(mutatorBase.InSaferegion() == false);
}

HWTEST_F_L0(MutatorTest, VisitRawObjects_Nullptr_NoCall)
{
    TestMutator mutator;
    mutator.Init();
    mutator.PushRawObject(nullptr);

    bool called = false;
    RootVisitor func = [&called](const ObjectRef& obj) {
        called = true;
    };
    mutator.VisitRawObjects(func);
    EXPECT_FALSE(called);
}

HWTEST_F_L0(MutatorTest, VisitRawObjects_NonNull_Call)
{
    TestMutator mutator;
    mutator.Init();
    BaseObject mockObj;
    mutator.PushRawObject(&mockObj);

    bool called = false;
    RootVisitor func = [&called, &mockObj](const ObjectRef& obj) {
        called = true;
        EXPECT_EQ(obj.object, &mockObj);
    };

    mutator.VisitRawObjects(func);
    EXPECT_TRUE(called);
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_Test1)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    MutatorBase::SuspensionType flag = MutatorBase::SuspensionType::SUSPENSION_FOR_STW;
    mutatorBase.SetSuspensionFlag(flag);
    flag = MutatorBase::SuspensionType::SUSPENSION_FOR_GC_PHASE;
    mutatorBase.SetSuspensionFlag(flag);
    mutatorBase.HandleSuspensionRequest();
    EXPECT_FALSE(mutatorBase.InSaferegion());
    EXPECT_TRUE(mutatorBase.FinishedTransition());
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_Test2)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    MutatorBase::SuspensionType flag = MutatorBase::SuspensionType::SUSPENSION_FOR_CPU_PROFILE;
    mutatorBase.SetSuspensionFlag(flag);
    mutatorBase.SetCpuProfileState(MutatorBase::CpuProfileState::FINISH_CPUPROFILE);
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        mutatorBase.ClearSuspensionFlag(MutatorBase::SUSPENSION_FOR_CPU_PROFILE);
    });
    mutatorBase.HandleSuspensionRequest();
    t.join();
    EXPECT_FALSE(mutatorBase.InSaferegion());
    EXPECT_TRUE(mutatorBase.FinishedCpuProfile());
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_Test3)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    mutatorBase.SetSuspensionFlag(MutatorBase::SUSPENSION_FOR_STW);
    mutatorBase.SetSuspensionFlag(MutatorBase::SUSPENSION_FOR_CPU_PROFILE);
    mutatorBase.SetCpuProfileState(MutatorBase::CpuProfileState::FINISH_CPUPROFILE);
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        mutatorBase.ClearSuspensionFlag(MutatorBase::SUSPENSION_FOR_CPU_PROFILE);
    });
    mutatorBase.HandleSuspensionRequest();
    t.join();
    EXPECT_FALSE(mutatorBase.InSaferegion());
    EXPECT_TRUE(mutatorBase.FinishedCpuProfile());
}

HWTEST_F_L0(MutatorTest, HandleGCPhase_SatbNodeNotNull)
{
    Mutator* mutator = MutatorManager::Instance().CreateRuntimeMutator(ThreadType::ARK_PROCESSOR);
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *mockObj = reinterpret_cast<BaseObject*>(addr);
    new (mockObj) BaseObject();
    mutator->RememberObjectInSatbBuffer(mockObj);
    EXPECT_NE(mutator->GetSatbBufferNode(), nullptr);
    MutatorBase* base = static_cast<MutatorBase*>(mutator->GetMutatorBasePtr());
    base->TransitionToGCPhaseExclusive(GCPhase::GC_PHASE_REMARK_SATB);
    EXPECT_EQ(mutator->GetSatbBufferNode(), nullptr);
    MutatorManager::Instance().DestroyRuntimeMutator(ThreadType::ARK_PROCESSOR);
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_LeaveSaferegion1)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    mutatorBase.IncObserver();
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        mutatorBase.DecObserver();
    });
    mutatorBase.HandleSuspensionRequest();
    t.join();
    EXPECT_FALSE(mutatorBase.InSaferegion());
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_LeaveSaferegion2)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    MutatorBase::SuspensionType flag = MutatorBase::SuspensionType::SUSPENSION_FOR_CPU_PROFILE;
    mutatorBase.SetSuspensionFlag(flag);
    mutatorBase.SetCpuProfileState(MutatorBase::CpuProfileState::FINISH_CPUPROFILE);
    mutatorBase.IncObserver();
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        mutatorBase.ClearSuspensionFlag(MutatorBase::SUSPENSION_FOR_CPU_PROFILE);
        mutatorBase.DecObserver();
    });
    mutatorBase.HandleSuspensionRequest();
    t.join();
    EXPECT_FALSE(mutatorBase.InSaferegion());
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_Test4)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    mutatorBase.SetSuspensionFlag(MutatorBase::SUSPENSION_FOR_STW);
    EXPECT_NO_FATAL_FAILURE(mutatorBase.HandleSuspensionRequest());
}

HWTEST_F_L0(MutatorTest, HandleSuspensionRequest_Test5)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();
    mutatorBase.SetSuspensionFlag(MutatorBase::SUSPENSION_FOR_GC_PHASE);
    EXPECT_NO_FATAL_FAILURE(mutatorBase.HandleSuspensionRequest());
}

HWTEST_F_L0(MutatorTest, TransitionToGCPhaseExclusive_TestEnum)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();

    GCPhase phase = GCPhase::GC_PHASE_ENUM;
    EXPECT_NO_FATAL_FAILURE(mutatorBase.TransitionToGCPhaseExclusive(phase));
}

HWTEST_F_L0(MutatorTest, TransitionToGCPhaseExclusive_TestPrecopy)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();

    GCPhase phase = GCPhase::GC_PHASE_PRECOPY;
    EXPECT_NO_FATAL_FAILURE(mutatorBase.TransitionToGCPhaseExclusive(phase));
}

HWTEST_F_L0(MutatorTest, TransitionToGCPhaseExclusive_TestIdle)
{
    MutatorBase mutatorBase;
    mutatorBase.Init();

    GCPhase phase = GCPhase::GC_PHASE_IDLE;
    EXPECT_NO_FATAL_FAILURE(mutatorBase.TransitionToGCPhaseExclusive(phase));
}
}  // namespace common::test
