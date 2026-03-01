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

#ifndef PANDA_RUNTIME_COROUTINES_AFFINITY_MASK_H
#define PANDA_RUNTIME_COROUTINES_AFFINITY_MASK_H

#include "libarkbase/globals.h"

#include "runtime/coroutines/coroutine_worker.h"

namespace ark {

class AffinityMask final {
public:
    // maximum workers count should conform to number of bits in the affinity mask
    static constexpr size_t MAX_WORKERS_COUNT = 128;
    using Storage = std::bitset<MAX_WORKERS_COUNT>;
    using CoroutineWorkerGroupId = Storage;

    AffinityMask() = default;
    ~AffinityMask() = default;

    DEFAULT_MOVE_SEMANTIC(AffinityMask);
    DEFAULT_COPY_SEMANTIC(AffinityMask);

    size_t NumAllowedWorkers() const
    {
        return data_.count();
    }

    bool IsWorkerAllowed(CoroutineWorker::Id workerId) const
    {
        return data_.test(workerId);
    }

    AffinityMask &SetWorkerAllowed(CoroutineWorker::Id workerId)
    {
        data_.set(workerId);
        return *this;
    }

    AffinityMask &SetWorkerNotAllowed(CoroutineWorker::Id workerId)
    {
        data_.reset(workerId);
        return *this;
    }

    bool operator==(const AffinityMask &rhs) const
    {
        return this->data_ == rhs.data_;
    }

    // NOTE: constexpr
    static AffinityMask Full()
    {
        return AffinityMask(Storage().set());
    }

    // NOTE: constexpr
    static AffinityMask Empty()
    {
        return AffinityMask(Storage().reset());
    }

    static AffinityMask FromGroupId(const CoroutineWorkerGroupId &groupId)
    {
        auto mask = AffinityMask::Empty();
        mask.data_ = groupId;
        return mask;
    }

private:
    // NOTE: constexpr
    explicit AffinityMask(Storage data) : data_(data) {}

    // every bit means that the corresponding worker is allowed
    Storage data_;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_AFFINITY_MASK_H */
