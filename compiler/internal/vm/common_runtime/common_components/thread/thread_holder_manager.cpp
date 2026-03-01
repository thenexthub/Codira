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

#include "common_interfaces/thread/thread_holder_manager.h"

#include <algorithm>

#include "common_components/mutator/mutator_manager.h"
#include "common_components/log/log.h"
#include "common_interfaces/thread/thread_holder-inl.h"

namespace common {
void ThreadHolderManager::RegisterThreadHolder([[maybe_unused]] ThreadHolder *holder)
{
    Mutator *mutator = static_cast<Mutator *>(holder->GetMutator());

    auto& mutator_manager = MutatorManager::Instance();
    mutator_manager.MutatorManagementRLock();

    {
        std::lock_guard<std::mutex> guard(mutator_manager.allMutatorListLock_);
        mutator_manager.allMutatorList_.push_back(mutator);
    }
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());

    mutator_manager.MutatorManagementRUnlock();
}

void ThreadHolderManager::BindMutator(ThreadHolder *holder)
{
    Mutator *mutator = static_cast<Mutator *>(holder->GetMutator());

    auto& mutator_manager = MutatorManager::Instance();
    mutator_manager.MutatorManagementRLock();

    mutator_manager.BindMutator(*mutator);

    mutator_manager.MutatorManagementRUnlock();
}

void ThreadHolderManager::UnbindMutator(ThreadHolder *holder)
{
    Mutator *mutator = static_cast<Mutator *>(holder->GetMutator());

    auto& mutator_manager = MutatorManager::Instance();
    mutator_manager.MutatorManagementRLock();

    mutator_manager.UnbindMutator(*mutator);

    mutator_manager.MutatorManagementRUnlock();
}

// Note: currently only called when thread is to be destroyed.
void ThreadHolderManager::UnregisterThreadHolder(ThreadHolder *holder)
{
    Mutator *mutator = static_cast<Mutator *>(holder->GetMutator());

    auto& mutator_manager = MutatorManager::Instance();
    mutator_manager.MutatorManagementRLock();

    {
        std::lock_guard<std::mutex> guard(mutator_manager.allMutatorListLock_);
        auto& list = mutator_manager.allMutatorList_;
        auto it = std::find(list.begin(), list.end(), mutator);
        if (it != list.end()) {
            list.erase(it);
        }
    }
    mutator->ResetMutator();
    mutator_manager.MutatorManagementRUnlock();
}

void ThreadHolderManager::SuspendAll(ThreadHolder *current)
{
    DCHECK_CC(current != nullptr);
    DCHECK_CC(!current->IsInRunningState());
    SuspendAllImpl(current);
}

void ThreadHolderManager::ResumeAll(ThreadHolder *current)
{
    DCHECK_CC(current != nullptr);
    DCHECK_CC(!current->IsInRunningState());
    ResumeAllImpl(current);
}

void ThreadHolderManager::IterateAll(CommonRootVisitor visitor)
{
    MutatorManager::Instance().VisitAllMutators([&visitor] (Mutator &mutator) {
        mutator.GetThreadHolder()->VisitAllThreads(visitor);
    });
}

void ThreadHolderManager::SuspendAllImpl([[maybe_unused]] ThreadHolder *current)
{
    MutatorManager::Instance().StopTheWorld(false, GCPhase::GC_PHASE_IDLE);
}

void ThreadHolderManager::ResumeAllImpl(ThreadHolder *current)
{
    MutatorManager::Instance().StartTheWorld();
}
}  // namespace common
