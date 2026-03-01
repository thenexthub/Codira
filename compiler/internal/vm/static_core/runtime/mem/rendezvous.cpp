/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "runtime/mem/rendezvous.h"

#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark {

Rendezvous::Rendezvous(PandaVM *pandaVm) : mutatorLock_(pandaVm->GetMutatorLock()) {}

void Rendezvous::SafepointBegin()
{
    ASSERT(!mutatorLock_->HasLock());
    LOG(DEBUG, GC) << "Rendezvous: SafepointBegin";
    // Suspend all threads
    Thread::GetCurrent()->GetVM()->GetThreadManager()->SuspendAllThreads();
    // Acquire write MutatorLock
    mutatorLock_->WriteLock();
}

void Rendezvous::SafepointEnd()
{
    ASSERT(mutatorLock_->HasLock());
    LOG(DEBUG, GC) << "Rendezvous: SafepointEnd";
    // Release write MutatorLock
    mutatorLock_->Unlock();
    // Resume all threads
    Thread::GetCurrent()->GetVM()->GetThreadManager()->ResumeAllThreads();
    LOG(DEBUG, GC) << "Rendezvous: SafepointEnd exit";
}

ScopedSuspendAllThreadsRunning::ScopedSuspendAllThreadsRunning(Rendezvous *rendezvous) : rendezvous_(rendezvous)
{
    ASSERT(rendezvous_ != nullptr);
    ASSERT(rendezvous_->GetMutatorLock()->HasLock());
    rendezvous_->GetMutatorLock()->Unlock();
    rendezvous_->SafepointBegin();
}

ScopedSuspendAllThreadsRunning::~ScopedSuspendAllThreadsRunning()
{
    rendezvous_->SafepointEnd();
    rendezvous_->GetMutatorLock()->ReadLock();
}

}  // namespace ark
