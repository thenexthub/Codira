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

#ifndef PLUGINS_ETS_RUNTIME_ETS_OBJECT_STATE_INFO_H
#define PLUGINS_ETS_RUNTIME_ETS_OBJECT_STATE_INFO_H

#include <atomic>
#include <cstdint>
#include <limits>
#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"

namespace ark::ets {

class EtsObject;
class EtsCoroutine;

class EtsObjectStateInfo {
public:
    using Id = ObjectPointerType;
    static constexpr uint32_t INVALID_INTEROP_INDEX = std::numeric_limits<uint32_t>::max();

    EtsObjectStateInfo(EtsObject *obj, Id id) : obj_(obj), id_(id) {}
    ~EtsObjectStateInfo() = default;

    NO_COPY_SEMANTIC(EtsObjectStateInfo);
    NO_MOVE_SEMANTIC(EtsObjectStateInfo);

    EtsObject *GetEtsObject() const
    {
        return obj_;
    }

    void SetEtsObject(EtsObject *obj)
    {
        obj_ = obj;
    }

    Id GetId() const
    {
        return id_;
    }

    void SetEtsHash(uint32_t hash)
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        etsHash_.store(hash, std::memory_order_relaxed);
    }
    uint32_t GetEtsHash() const
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        return etsHash_.load(std::memory_order_relaxed);
    }

    void SetInteropIndex(uint32_t index)
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        interopIndex_.store(index, std::memory_order_relaxed);
    }
    uint32_t GetInteropIndex() const
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        return interopIndex_.load(std::memory_order_relaxed);
    }

    bool DeflateInternal();

    static bool TryAddNewInfo(EtsObject *obj, uint32_t hash, uint32_t index);
    static bool TryReadEtsHash(const EtsObject *obj, uint32_t *hash);
    static bool TryReadInteropIndex(const EtsObject *obj, uint32_t *index);
    static bool TryDropInteropIndex(EtsObject *obj);
    static bool TryResetInteropIndex(EtsObject *obj, uint32_t index);
    static bool TryCheckIfInteropIndexIsValid(const EtsObject *obj, bool *isValid);

private:
    std::atomic_uint32_t etsHash_ {0};
    std::atomic_uint32_t interopIndex_ {0};
    EtsObject *obj_ {nullptr};
    Id id_ {0};
};

}  // namespace ark::ets

#endif  // PLUGINS_ETS_RUNTIME_ETS_OBJECT_STATE_INFO_H