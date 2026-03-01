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

#include <atomic>

#include "plugins/ets/runtime/ets_object_state_info.h"
#include "plugins/ets/runtime/ets_mark_word.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

bool EtsObjectStateInfo::DeflateInternal()
{
    auto interopIndex = GetInteropIndex();
    if (interopIndex != INVALID_INTEROP_INDEX) {
        LOG(DEBUG, RUNTIME) << "Info: " << this << " can not be deflated: contains valid interop index";
        return false;
    }
    auto markWord = obj_->AtomicGetMark(std::memory_order_acquire);
    ASSERT(markWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    // Info state means that we will have a ets hash so it should be used in new mark word
    auto hash = GetEtsHash();
    // We create new mark word,
    auto newMarkWord = markWord.DecodeFromHash(hash);
    ASSERT(newMarkWord.GetHash() == hash);
    // Atomic with relaxed order reason: ordering constraints are not required
    // CC-OFFNXT(G.CTL.03) false positive
    while (!obj_->AtomicSetMark<false>(markWord, newMarkWord, std::memory_order_relaxed)) {
    }
    return true;
}

/*static*/
bool EtsObjectStateInfo::TryAddNewInfo(EtsObject *obj, uint32_t hash, uint32_t index)
{
    // Atomic with acquire order reason: ordering constraints for correctness of future non-atomic and relaxed loadings.
    auto markWord = obj->AtomicGetMark(std::memory_order_acquire);
    // First we check if state of mark word is still MarkWord::STATE_HASHED.
    auto state = markWord.GetState();
    if (state != EtsMarkWord::STATE_HASHED && state != EtsMarkWord::STATE_HAS_INTEROP_INDEX) {
        // Currect state is not STATE_HASHED so we can not add info for this object.
        return false;
    }
    // Creating of new info and writing `hash` and `index` in it.
    auto *co = EtsCoroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *table = co->GetPandaVM()->GetEtsObjectStateTable();
    ASSERT(table != nullptr);
    auto *info = table->CreateInfo(obj);
    LOG_IF(info == nullptr, FATAL, RUNTIME) << "Couldn't create new info. Out of memory?";
    info->SetEtsHash(hash);
    info->SetInteropIndex(index);
    // Finally we should try to exchange mark word with new one with MONITOR state.
    // Creating of new local mark word
    auto newMarkWord = markWord.DecodeFromInfo(info->GetId());
    ASSERT(newMarkWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    ASSERT(newMarkWord.GetInfoId() == info->GetId());
    // Atomic with release order reason: ordering constraints for correctness of past non-atomic and relaxed storings.
    if (!obj->AtomicSetMark(markWord, newMarkWord, std::memory_order_release)) {
        // Other thread have changed object mark word
        table->FreeInfo(info->GetId());
        return false;
    }
    return true;
}

/*static*/
bool EtsObjectStateInfo::TryReadEtsHash(const EtsObject *obj, uint32_t *hash)
{
    // Atomic with acquire order reason: ordering constraints for correctness of future non-atomic and relaxed loadings.
    auto markWord = obj->AtomicGetMark(std::memory_order_acquire);
    // First we check if state of mark word is still EtsMarkWord::STATE_USE_INFO.
    // This method can be called only if state of the object is EtsMarkWord::STATE_USE_INFO.
    // The state can be changed only in GC.
    ASSERT(markWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    // We need to get EtsObjectStateInfo from table
    auto *co = EtsCoroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *table = co->GetPandaVM()->GetEtsObjectStateTable();
    auto id = markWord.GetInfoId();
    auto *info = table->LookupInfo(id);
    LOG_IF(info == nullptr || info->GetEtsObject() != obj, FATAL, RUNTIME)
        << "TryReadEtsHash: info was concurrently deleted from table";
    // we can read hash and return it.
    *hash = info->GetEtsHash();
    return true;
}

/*static*/
bool EtsObjectStateInfo::TryReadInteropIndex(const EtsObject *obj, uint32_t *index)
{
    // Atomic with acquire order reason: ordering constraints for correctness of future non-atomic and relaxed loadings.
    auto markWord = obj->AtomicGetMark(std::memory_order_acquire);
    // This method can be called only if state of the object is EtsMarkWord::STATE_USE_INFO.
    // The state can be changed only in GC.
    ASSERT(markWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    // We need to get EtsObjectStateInfo from table
    auto *co = EtsCoroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *table = co->GetPandaVM()->GetEtsObjectStateTable();
    auto id = markWord.GetInfoId();
    auto *info = table->LookupInfo(id);
    LOG_IF(info == nullptr || info->GetEtsObject() != obj, FATAL, RUNTIME)
        << "TryReadInteropIndex: info was concurrently deleted from table";
    // we can read hash, check if it's valid and return it.
    auto currentIndex = info->GetInteropIndex();
    LOG_IF(currentIndex == INVALID_INTEROP_INDEX, FATAL, RUNTIME) << "Try read invalid interop index";
    *index = currentIndex;
    return true;
}

/*static*/
bool EtsObjectStateInfo::TryDropInteropIndex(EtsObject *obj)
{
    // Atomic with acquire order reason: ordering constraints for correctness of future non-atomic and relaxed loadings.
    auto markWord = obj->AtomicGetMark(std::memory_order_acquire);
    // This method can be called only if state of the object is EtsMarkWord::STATE_USE_INFO.
    // The state can be changed only in GC.
    ASSERT(markWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    auto *co = EtsCoroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *table = co->GetPandaVM()->GetEtsObjectStateTable();
    ASSERT(table != nullptr);
    auto id = markWord.GetInfoId();
    auto *info = table->LookupInfo(id);
    LOG_IF(info == nullptr || info->GetEtsObject() != obj, FATAL, RUNTIME)
        << "TryDropInteropIndex: info was concurrently deleted from table";
    // Droping here means that we set INVALID_INDEX
    info->SetInteropIndex(EtsObjectStateInfo::INVALID_INTEROP_INDEX);
    return true;
}

/*static*/
bool EtsObjectStateInfo::TryResetInteropIndex(EtsObject *obj, uint32_t index)
{
    auto markWord = obj->AtomicGetMark(std::memory_order_acquire);
    // This method can be called only if state of the object is EtsMarkWord::STATE_USE_INFO.
    // The state can be changed only in GC.
    ASSERT(markWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    // We need to find current interop index
    auto *co = EtsCoroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *table = co->GetPandaVM()->GetEtsObjectStateTable();
    auto id = markWord.GetInfoId();
    auto *info = table->LookupInfo(id);
    LOG_IF(info == nullptr || info->GetEtsObject() != obj, FATAL, RUNTIME)
        << "TryResetInteropIndex: info was concurrently deleted from table";
    info->SetInteropIndex(index);
    return true;
}

bool EtsObjectStateInfo::TryCheckIfInteropIndexIsValid(const EtsObject *obj, bool *isValid)
{
    auto markWord = obj->AtomicGetMark(std::memory_order_acquire);
    // This method can be called only if state of the object is EtsMarkWord::STATE_USE_INFO.
    // The state can be changed only in GC.
    ASSERT(markWord.GetState() == EtsMarkWord::STATE_USE_INFO);
    // We need to find current interop index
    auto *co = EtsCoroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *table = co->GetPandaVM()->GetEtsObjectStateTable();
    auto id = markWord.GetInfoId();
    auto *info = table->LookupInfo(id);
    if (info == nullptr || info->GetEtsObject() != obj) {
        LOG(DEBUG, RUNTIME) << "TryCheckIfInteropIndexIsValid: info was concurrently deleted from table";
        return true;
    }
    auto currentIndex = info->GetInteropIndex();
    *isValid = currentIndex != INVALID_INTEROP_INDEX;
    return true;
}

}  // namespace ark::ets
