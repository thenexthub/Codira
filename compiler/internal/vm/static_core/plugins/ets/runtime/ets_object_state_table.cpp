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

#include "plugins/ets/runtime/ets_object_state_table.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"

namespace ark::ets {

EtsObjectStateTable::EtsObjectStateTable(mem::InternalAllocatorPtr allocator)
    : allocator_(allocator), table_(allocator->Adapter())
{
}

EtsObjectStateTable::~EtsObjectStateTable()
{
    for ([[maybe_unused]] auto &[id, info] : table_) {
        if (info != nullptr) {
            allocator_->Delete(info);
        }
    }
}

EtsObjectStateInfo *EtsObjectStateTable::CreateInfo(EtsObject *obj)
{
    os::memory::WriteLockHolder wlh(lock_);
    for (EtsObjectStateInfo::Id i = 0; i < MAX_TABLE_ID; ++i) {
        lastId_ = (lastId_ + 1) % MAX_TABLE_ID;
        if (table_.count(lastId_) == 0) {
            auto *info = allocator_->New<EtsObjectStateInfo>(obj, lastId_);
            if (info == nullptr) {
                return nullptr;
            }
            table_[lastId_] = info;
            return info;
        }
    }
    return nullptr;
}

EtsObjectStateInfo *EtsObjectStateTable::LookupInfo(EtsObjectStateInfo::Id id)
{
    os::memory::ReadLockHolder rlh(lock_);
    auto it = table_.find(id);
    if (it != table_.end()) {
        return it->second;
    }
    return nullptr;
}

void EtsObjectStateTable::EnumerateObjectStates(const std::function<void(EtsObjectStateInfo *)> &cb)
{
    os::memory::WriteLockHolder wlh(lock_);
    for (auto &objectState : table_) {
        cb(objectState.second);
    }
}

void EtsObjectStateTable::FreeInfo(EtsObjectStateInfo::Id id)
{
    os::memory::WriteLockHolder wlh(lock_);
    auto it = table_.find(id);
    if (it != table_.end()) {
        auto *info = it->second;
        table_.erase(id);
        allocator_->Delete(info);
    }
}

void EtsObjectStateTable::DeflateInfo()
{
    os::memory::WriteLockHolder wlh(lock_);
    for (auto infoI = table_.begin(); infoI != table_.end();) {
        auto info = infoI->second;
        if (info->DeflateInternal()) {
            infoI = table_.erase(infoI);
            allocator_->Delete(info);
        } else {
            infoI++;
        }
    }
}

}  // namespace ark::ets
