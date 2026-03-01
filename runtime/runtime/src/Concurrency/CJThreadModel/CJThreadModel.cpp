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


#include "CODEThreadModel.h"

#include <pthread.h>
#include <thread>

#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Cki.h"
#include "Exception/ExceptionCApi.h"
#include "LoaderManager.h"
#include "Mutator/MutatorManager.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif
#include "schedule.h"

namespace MapleRuntime {
extern "C" void MRT_VisitorCaller(void* argPtr, void* handle)
{
    ObjectRef& ref = reinterpret_cast<ObjectRef&>(reinterpret_cast<LWTData*>(argPtr)->obj);
    (*reinterpret_cast<RootVisitor*>(handle))(ref);
    ObjectRef& map = reinterpret_cast<ObjectRef&>(reinterpret_cast<LWTData*>(argPtr)->threadObject);
    (*reinterpret_cast<RootVisitor*>(handle))(map);
}

// External interface for adapting to concurrent tasks
extern "C" uintptr_t MRT_CreateMutator()
{
    Mutator* mutator = MutatorManager::Instance().CreateMutator();
    ThreadLocalData* threadData = reinterpret_cast<ThreadLocalData*>(MRT_GetThreadLocalData());
    MRT_PreRunManagedCode(mutator, 1, threadData); // one layer call chain
    return 0;
}

extern "C" uintptr_t MRT_TransitMutatorToExit()
{
    MutatorManager::Instance().TransitMutatorToExit();
    return 0;
}

extern "C" void MRT_DestroyMutator(void* mutator)
{
    MutatorManager::Instance().DestroyMutator(reinterpret_cast<Mutator*>(mutator));
}

extern "C" bool MRT_CheckMutatorStatus(void* mutator)
{
    Mutator *curMutator = reinterpret_cast<Mutator*>(mutator);
    auto status = curMutator->GetUnwindContext().GetUnwindContextStatus();
    if (status == UnwindContextStatus::RISKY && curMutator->InSaferegion()) {
        curMutator->SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_EXIT);

        status = curMutator->GetUnwindContext().GetUnwindContextStatus();
        if (status != UnwindContextStatus::RISKY || !curMutator->InSaferegion()) {
            curMutator->ClearSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_EXIT);
            return false;
        }

        return true;
    }
    return false;
}

static void RegisterCODEThreadHooks()
{
    (void)CODEThreadSchdHookRegister(MRT_StopGCWork, SCHD_STOP);
    (void)CODEThreadSchdHookRegister(MRT_CreateMutator, SCHD_CREATE_MUTATOR);
    (void)CODEThreadSchdHookRegister(MRT_TransitMutatorToExit, SCHD_DESTROY_MUTATOR);
    (void)CODEThreadSchdHookRegister(MRT_GetSafepointProtectedPage, SCHD_PREEMPT_REQ);
    (void)CODEThreadDestructorHookRegister(MRT_DestroyMutator);
    (void)CODEThreadGetMutatorStatusHookRegister(MRT_CheckMutatorStatus);
    LogRegister(MRT_DumpLog, ENABLE_LOG(LogType::CODETHREAD), LogFile::GetLogLevel());
}

static bool GetStackGuardFlagEnv()
{
    const char* env = std::getenv("MRT_STACK_CHECK");
    if (env != nullptr) {
        if (CString::ParseFlagFromEnv(env)) {
            return true;
        }
        LOG(RTLOG_ERROR, "unsupported MRT_STACK_CHECK. Should set variable to 1 or true or TRUE\n");
    }
    return false;
}

// ConcurrencyParam.processorNum set the processor number of scheduler, it is set as following ways:
// 1. User can set the environment variable 'codeProcessorNum' firstly.
// 2. If the variable 'codeProcessorNum' is invalid, set it by return value of hardware_concurrency().
// 3. If not, use a default value of 8 to set it finally.
void CODEThreadModel::Init(const ConcurrencyParam param, ScheduleType scheduleType)
{
    ScheduleAttr attr;
    ScheduleAttrInit(&attr);
    ScheduleAttrThstackSizeSet(&attr, param.thStackSize * KB);
    ScheduleAttrCostackSizeSet(&attr, param.coStackSize * KB);
    ScheduleAttrProcessorNumSet(&attr, param.processorNum);
    stackGuardCheck = GetStackGuardFlagEnv();
    if (stackGuardCheck) {
        ScheduleAttrStackProtectSet(&attr, true);
    }
    if (scheduleType == SCHEDULE_UI_THREAD) {
        ScheduleAttrStackGrowSet(&attr, false);
    }

    ScheduleGetTlsHookRegister((GetTlsHookFunc)MRT_GetThreadLocalData);

    // should not use system page size to calculate reserved stack size,
    // because the page size could be different in different system.
#ifdef _WIN64
    constexpr uint32_t reservedStackSize = 24 * KB;
#else
    constexpr uint32_t reservedStackSize = 8 * KB;
#endif
    CODEThreadStackReversedSet(reservedStackSize);
    scheduler = ScheduleNew(scheduleType, &attr);
    Cki::CreateCKI();

#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanInitialize();
#endif
    RegisterCODEThreadHooks();
}

void CODEThreadModel::VisitGCRoots(RootVisitor* visitorHandle)
{
    ScheduleAllCODEThreadVisit(MRT_VisitorCaller, visitorHandle);
}

// Get current mutator from tls
Mutator* ConcurrencyModel::GetMutator()
{
    if (!IsRuntimeThread()) {
        return reinterpret_cast<Mutator*>(CODEThreadGetMutator());
    } else {
        return ThreadLocal::GetMutator();
    }
}

void ConcurrencyModel::SetMutator(Mutator* mutator) { CODEThreadSetMutator(mutator); }
} // namespace MapleRuntime
