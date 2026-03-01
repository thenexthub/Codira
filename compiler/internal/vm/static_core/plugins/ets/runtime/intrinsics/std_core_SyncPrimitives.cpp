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

#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_exceptions.h"

namespace ark::ets::intrinsics {

EtsObject *EtsMutexCreate()
{
    return EtsMutex::Create(EtsCoroutine::GetCurrent());
}

void EtsMutexLock(EtsObject *mutex)
{
    ASSERT(mutex->GetClass() == PlatformTypes()->coreMutex);
    EtsMutex::FromEtsObject(mutex)->Lock();
}

void EtsMutexUnlock(EtsObject *mutex)
{
    ASSERT(mutex->GetClass() == PlatformTypes()->coreMutex);
    if (!EtsMutex::FromEtsObject(mutex)->IsHeld()) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::ILLEGAL_LOCK_STATE_ERROR,
                          "Unable to unlock Mutex: state is already unlocked");
        return;
    }
    EtsMutex::FromEtsObject(mutex)->Unlock();
}

EtsObject *EtsEventCreate()
{
    return EtsEvent::Create(EtsCoroutine::GetCurrent());
}

void EtsEventWait(EtsObject *event)
{
    ASSERT(event->GetClass() == PlatformTypes()->coreEvent);
    EtsEvent::FromEtsObject(event)->Wait();
}

void EtsEventFire(EtsObject *event)
{
    ASSERT(event->GetClass() == PlatformTypes()->coreEvent);
    EtsEvent::FromEtsObject(event)->Fire();
}

EtsObject *EtsCondVarCreate()
{
    return EtsCondVar::Create(EtsCoroutine::GetCurrent());
}

void EtsCondVarWait(EtsObject *condVar, EtsObject *mutex)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(condVar->GetClass() == PlatformTypes(coro)->coreCondVar);
    ASSERT(mutex->GetClass() == PlatformTypes(coro)->coreMutex);
    EtsHandleScope scope(coro);
    EtsHandle<EtsMutex> hMutex(coro, EtsMutex::FromEtsObject(mutex));
    EtsCondVar::FromEtsObject(condVar)->Wait(hMutex);
}

void EtsCondVarNotifyOne(EtsObject *condVar, EtsObject *mutex)
{
    ASSERT(condVar->GetClass() == PlatformTypes()->coreCondVar);
    ASSERT(mutex->GetClass() == PlatformTypes()->coreMutex);
    EtsCondVar::FromEtsObject(condVar)->NotifyOne(EtsMutex::FromEtsObject(mutex));
}

void EtsCondVarNotifyAll(EtsObject *condVar, EtsObject *mutex)
{
    ASSERT(condVar->GetClass() == PlatformTypes()->coreCondVar);
    ASSERT(mutex->GetClass() == PlatformTypes()->coreMutex);
    EtsCondVar::FromEtsObject(condVar)->NotifyAll(EtsMutex::FromEtsObject(mutex));
}

EtsObject *EtsQueueSpinlockCreate()
{
    return EtsQueueSpinlock::Create(EtsCoroutine::GetCurrent());
}

void EtsQueueSpinlockGuard(EtsObject *spinlock, EtsObject *callback)
{
    ASSERT(spinlock->GetClass() == PlatformTypes()->coreQueueSpinlock);
    auto *coro = EtsCoroutine::GetCurrent();
    EtsHandleScope scope(coro);
    EtsHandle<EtsQueueSpinlock> hSpinlock(coro, EtsQueueSpinlock::FromEtsObject(spinlock));
    EtsHandle<EtsObject> hCallback(coro, callback);
    EtsQueueSpinlock::Guard guard(hSpinlock);
    LambdaUtils::InvokeVoid(coro, hCallback.GetPtr());
}

void EtsReadLock(EtsObject *rwLock)
{
    ASSERT(rwLock->GetClass() == PlatformTypes()->coreRWLock);
    EtsRWLock::FromEtsObject(rwLock)->ReadLock();
}

void EtsReadUnlock(EtsObject *rwLock)
{
    ASSERT(rwLock->GetClass() == PlatformTypes()->coreRWLock);
    auto state = EtsRWLock::FromEtsObject(rwLock)->GetState();
    if (EtsRWLock::State::IsUnlocked(state) || EtsRWLock::State::HasWriteLock(state)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::ILLEGAL_LOCK_STATE_ERROR,
                          "Unable to unlock ReadLock: state is already unlocked or write-locked");
        return;
    }
    EtsRWLock::FromEtsObject(rwLock)->Unlock();
}

void EtsWriteLock(EtsObject *rwLock)
{
    ASSERT(rwLock->GetClass() == PlatformTypes()->coreRWLock);
    EtsRWLock::FromEtsObject(rwLock)->WriteLock();
}

void EtsWriteUnlock(EtsObject *rwLock)
{
    ASSERT(rwLock->GetClass() == PlatformTypes()->coreRWLock);
    auto state = EtsRWLock::FromEtsObject(rwLock)->GetState();
    if (EtsRWLock::State::IsUnlocked(state) || EtsRWLock::State::HasReadLock(state)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::ILLEGAL_LOCK_STATE_ERROR,
                          "Unable to unlock WriteLock: state is already unlocked or read-locked");
        return;
    }
    EtsRWLock::FromEtsObject(rwLock)->Unlock();
}

}  // namespace ark::ets::intrinsics
