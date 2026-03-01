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

#include "plugins/ets/runtime/types/ets_object.h"
#include <atomic>
#include <cstdint>
#include "plugins/ets/runtime/ets_mark_word.h"
#include "plugins/ets/runtime/ets_object_state_info.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "runtime/handle_base-inl.h"

namespace ark::ets {

/* static */
EtsObject *EtsObject::Create(EtsCoroutine *etsCoroutine, EtsClass *klass)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    ASSERT(klass != nullptr);
    return static_cast<EtsObject *>(ObjectHeader::Create(etsCoroutine, klass->GetRuntimeClass()));
}

/* static */
EtsObject *EtsObject::Create(EtsClass *klass)
{
    return Create(EtsCoroutine::GetCurrent(), klass);
}

/* static */
EtsObject *EtsObject::CreateNonMovable(EtsClass *klass)
{
    ASSERT(klass != nullptr);
    return static_cast<EtsObject *>(ObjectHeader::CreateNonMovable(klass->GetRuntimeClass()));
}

uint32_t EtsObject::GetHashCode()
{
    uint32_t hash = 0;
    while (!TryGetHashCode(&hash)) {
    }
    return hash;
}

uint32_t EtsObject::GetInteropIndex() const
{
    uint32_t index = 0;
    while (!TryGetInteropIndex(&index)) {
    }
    return index;
}

void EtsObject::SetInteropIndex(uint32_t index)
{
    while (!TrySetInteropIndex(index)) {
    }
}

void EtsObject::DropInteropIndex()
{
    while (!TryDropInteropIndex()) {
    }
}

bool EtsObject::TryGetHashCode(uint32_t *hash)
{
    ASSERT(hash != nullptr);
    //  Atomic with relaxed order reason: on this level of execution where we use only atomic mark word for testing the
    //  state and its changing if it's needed. When we EtsObjectStateInfo static methods, we will reread value of mark
    //  word with acquire order to mark future relaxed read from EtsObjectStateInfo visible for us.
    EtsMarkWord currentMarkWord = AtomicGetMark(std::memory_order_relaxed);
    switch (currentMarkWord.GetState()) {
        case EtsMarkWord::STATE_HASHED: {
            *hash = currentMarkWord.GetHash();
            return true;
        }
        case EtsMarkWord::STATE_HAS_INTEROP_INDEX: {
            auto localHash = GenerateHashCode();
            auto interopHash = currentMarkWord.GetInteropIndex();
            if (!EtsObjectStateInfo::TryAddNewInfo(this, localHash, interopHash)) {
                return false;
            }
            *hash = localHash;
            return true;
        }
        case EtsMarkWord::STATE_UNLOCKED: {
            // In STATE_UNLOCKED case we should just set mark word in HASHED_STATE with new hash.
            auto newMarkWord = currentMarkWord.DecodeFromHash(GenerateHashCode());
            // Atomic with relaxed order reason: AtomicSetMark is CAS operation so if 'currentMarkWord' is not actual
            // this commend will find actual one and next load will read it. In this execution branch we have no relaxed
            // or non-atomic store, so release ordering is not needed.
            AtomicSetMark(currentMarkWord, newMarkWord, std::memory_order_relaxed);
            return false;
        }
        case EtsMarkWord::STATE_USE_INFO: {
            //  In STATE_USE_INFO case we should read hash from EtsObjectState table.
            return EtsObjectStateInfo::TryReadEtsHash(this, hash);
        }
        default: {
            LOG(FATAL, RUNTIME) << "Error on TryGetHashCode(): invalid state " << currentMarkWord.GetState();
            return true;  // go out of loop
        }
    }
}

bool EtsObject::TryGetInteropIndex(uint32_t *index) const
{
    ASSERT(index != nullptr);
    //  Atomic with relaxed order reason: on this level of execution where we use only atomic mark word for testing the
    //  state and its changing if it's needed. When we use EtsObjectStateInfo static methods, we will reread value of
    //  mark word with acquire order to mark future relaxed read from EtsObjectStateInfo visible for us.
    EtsMarkWord currentMarkWord = AtomicGetMark(std::memory_order_relaxed);
    switch (currentMarkWord.GetState()) {
        case EtsMarkWord::STATE_HAS_INTEROP_INDEX: {
            *index = currentMarkWord.GetInteropIndex();
            return true;
        }
        case EtsMarkWord::STATE_USE_INFO: {
            // In STATE_USE_INFO case we should read hash from EtsObjectState table.
            return EtsObjectStateInfo::TryReadInteropIndex(this, index);
        }
        default: {
            LOG(FATAL, RUNTIME) << "Error on TryGetInteropCode(): invalid state " << currentMarkWord.GetState();
            return true;  // go out of loop
        }
    }
}

bool EtsObject::TrySetInteropIndex(uint32_t index)
{
    //  Atomic with relaxed order reason: on this level of execution where we use only atomic mark word for testing the
    //  state and its changing if it's needed. When we use EtsObjectStateInfo static methods, we will reread value of
    //  mark word with acquire order to mark future relaxed read from EtsObjectStateInfo visible for us.
    EtsMarkWord currentMarkWord = AtomicGetMark(std::memory_order_relaxed);
    switch (currentMarkWord.GetState()) {
        case EtsMarkWord::STATE_UNLOCKED:
            [[fallthrough]];
        case EtsMarkWord::STATE_HAS_INTEROP_INDEX: {
            auto newMarkWord = currentMarkWord.DecodeFromInteropIndex(index);
            // Atomic with relaxed order reason: AtomicSetMark is CAS operation so if 'currentMarkWord' is not actual
            // this commend will find actual one and next load will read it. In this execution branch we have no relaxed
            // or non-atomic store, so release ordering is not needed.
            return AtomicSetMark(currentMarkWord, newMarkWord, std::memory_order_relaxed);
        }
        case EtsMarkWord::STATE_HASHED: {
            auto hash = currentMarkWord.GetHash();
            return EtsObjectStateInfo::TryAddNewInfo(this, hash, index);
        }
        case EtsMarkWord::STATE_USE_INFO: {
            // we should check if interop index is invalid. If it's true, we can set new one, otherwise we should fail.
            return EtsObjectStateInfo::TryResetInteropIndex(this, index);
        }
        default: {
            LOG(FATAL, RUNTIME) << "Error on TrySetInteropCode(): invalid state " << currentMarkWord.GetState();
            return true;  // go out of loop
        }
    }
}

bool EtsObject::TryDropInteropIndex()
{
    //  Atomic with relaxed order reason: on this level of execution where we use only atomic mark word for testing the
    //  state and its changing if it's needed. When we EtsObjectStateInfo static methods, we will reread value of mark
    //  word with acquire order to mark future relaxed read from EtsObjectStateInfo visible for us.
    EtsMarkWord currentMarkWord = AtomicGetMark(std::memory_order_relaxed);
    switch (currentMarkWord.GetState()) {
        case EtsMarkWord::STATE_HAS_INTEROP_INDEX: {
            auto newMarkWord = currentMarkWord.DecodeFromUnlocked();
            return AtomicSetMark(currentMarkWord, newMarkWord, std::memory_order_relaxed);
        }
        case EtsMarkWord::STATE_USE_INFO: {
            return EtsObjectStateInfo::TryDropInteropIndex(this);
        }
        default: {
            LOG(FATAL, RUNTIME) << "Error on TryDropInteropCode(): invalid state " << currentMarkWord.GetState();
            return true;  // go out of loop
        }
    }
}

bool EtsObject::TryCheckIfHasInteropIndex(bool *hasInteropIndex) const
{
    //  Atomic with relaxed order reason: on this level of execution where we use only atomic mark word for testing the
    //  state and its changing if it's needed. When we EtsObjectStateInfo static methods, we will reread value of mark
    //  word with acquire order to mark future relaxed read from EtsObjectStateInfo visible for us.
    EtsMarkWord currentMarkWord = AtomicGetMark(std::memory_order_relaxed);
    switch (currentMarkWord.GetState()) {
        case EtsMarkWord::STATE_USE_INFO: {
            return EtsObjectStateInfo::TryCheckIfInteropIndexIsValid(this, hasInteropIndex);
        }
        case EtsMarkWord::STATE_HAS_INTEROP_INDEX: {
            *hasInteropIndex = true;
            return true;
        }
        case EtsMarkWord::STATE_HASHED: {
            [[fallthrough]];
        }
        case EtsMarkWord::UNUSED_STATE_STATE_LIGHT_LOCKED: {
            [[fallthrough]];
        }
        case EtsMarkWord::STATE_UNLOCKED: {
            *hasInteropIndex = false;
            return true;
        }
        default:
            LOG(FATAL, RUNTIME) << "Invalid state " << currentMarkWord.GetState();
            return true;
    }
}

/*static*/
uint32_t EtsObject::GenerateHashCode()
{
    auto hash = ObjectHeader::GenerateHashCode();
    return hash >> EtsMarkWord::INTEROP_INDEX_FLAG_SIZE;
}

}  // namespace ark::ets
