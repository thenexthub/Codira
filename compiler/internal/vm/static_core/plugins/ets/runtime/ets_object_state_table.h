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

#ifndef PLUGINS_ETS_RUNTIME_ETS_OBJECT_STATE_TABLE_H
#define PLUGINS_ETS_RUNTIME_ETS_OBJECT_STATE_TABLE_H

#include <atomic>
#include <cstdint>

#include "libarkbase/os/mutex.h"

#include "runtime/include/mem/panda_containers.h"

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_mark_word.h"
#include "plugins/ets/runtime/ets_object_state_info.h"

namespace ark::ets {

class EtsObject;

/// @brief This class is used to contain and interact with information about each EtsObject that is USE_INFO state.
class EtsObjectStateTable {
    // NOTE(molotkovmikhail): this table can be implemented with lock-free guaranty to reduse overhead of lock usage.
    NO_COPY_SEMANTIC(EtsObjectStateTable);
    NO_MOVE_SEMANTIC(EtsObjectStateTable);

public:
    static constexpr EtsObjectStateInfo::Id MAX_TABLE_ID = EtsMarkWord::INFO_TABLE_POINTER_MAX_COUNT;

    explicit EtsObjectStateTable(mem::InternalAllocatorPtr allocator);
    ~EtsObjectStateTable();

    EtsObjectStateInfo *CreateInfo(EtsObject *obj);
    EtsObjectStateInfo *LookupInfo(EtsObjectStateInfo::Id id);
    void FreeInfo(EtsObjectStateInfo::Id id);

    void EnumerateObjectStates(const std::function<void(EtsObjectStateInfo *)> &cb);

    void DeflateInfo();

private:
    mem::InternalAllocatorPtr allocator_;
    os::memory::RWLock lock_;
    PandaUnorderedMap<EtsObjectStateInfo::Id, EtsObjectStateInfo *> table_;
    EtsObjectStateInfo::Id lastId_ {0};
};

}  // namespace ark::ets

#endif  // PLUGINS_ETS_RUNTIME_ETS_OBJECT_STATE_TABLE_H