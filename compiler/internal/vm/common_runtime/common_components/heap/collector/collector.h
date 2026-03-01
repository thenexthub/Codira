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

#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <set>
#include <vector>

#include "common_components/common/base_object.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/heap/collector/gc_stats.h"
#include "common_interfaces/thread/mutator_base.h"
#include "common_interfaces/base/runtime_param.h"

namespace common {
enum CollectorType {
    NO_COLLECTOR = 0,   // No Collector
    PROXY_COLLECTOR,           // Proxy of Collector
    COPY_COLLECTOR,            // Regional-Copying GC
    SMOOTH_COLLECTOR,               // wgc
    COLLECTOR_TYPE_COUNT,
};

// Central garbage identification algorithm.
class Collector {
public:
    Collector();
    virtual ~Collector() = default;

    static const char* GetGCPhaseName(GCPhase phase);

    // Initializer and finalizer.
    virtual void Init(const RuntimeParam& param) = 0;
    virtual void Fini() {}
    const char* GetCollectorName() const;

    // This pure virtual function implements the trigger of GC.
    // reason: Reason for GC.
    // async:  Trigger from unsafe context, e.g., holding a lock, in the middle of an allocation.
    //         In order to prevent deadlocks, async trigger only add one async gc task and will not block.
    void RequestGC(GCReason reason, bool async, GCType gcType);

    virtual GCPhase GetGCPhase() const { return gcPhase_.load(std::memory_order_acquire); }

    virtual void SetGCPhase(const GCPhase phase) { gcPhase_.store(phase, std::memory_order_release); }

    virtual void FixObjectRefFields(BaseObject*) const {}

    virtual void RunGarbageCollection(uint64_t, GCReason, GCType) = 0;

    virtual GCStats& GetGCStats()
    {
        LOG_COMMON(FATAL) << "Unresolved fatal";
        UNREACHABLE_CC();
    }

    virtual BaseObject* ForwardObject(BaseObject*) = 0;

    virtual bool ShouldIgnoreRequest(GCRequest& quest) = 0;
    virtual bool IsFromObject(BaseObject*) const = 0;
    virtual bool IsUnmovableFromObject(BaseObject*) const = 0;
    virtual BaseObject* FindToVersion(BaseObject* obj) const = 0;

    virtual bool TryUpdateRefField(BaseObject*, RefField<>&, BaseObject*&) const = 0;
    virtual bool TryForwardRefField(BaseObject*, RefField<>&, BaseObject*&) const = 0;
    virtual bool TryUntagRefField(BaseObject*, RefField<>&, BaseObject*&) const = 0;
    virtual RefField<> GetAndTryTagRefField(BaseObject*) const = 0;

    virtual bool IsOldPointer(RefField<>&) const = 0;
    virtual bool IsCurrentPointer(RefField<>&) const = 0;
    virtual void AddRawPointerObject(BaseObject*) = 0;
    virtual void RemoveRawPointerObject(BaseObject*) = 0;

    BaseObject* FindLatestVersion(BaseObject* obj) const
    {
        if (obj == nullptr) { return nullptr; }

        auto to = FindToVersion(obj);
        if (to != nullptr) {
            return to;
        }
        return obj;
    };

protected:
    virtual void RequestGCInternal(GCReason, bool, GCType)
    {
        LOG_COMMON(FATAL) << "Unresolved fatal";
        UNREACHABLE_CC();
    }

    CollectorType collectorType_ = CollectorType::NO_COLLECTOR;
    std::atomic<GCPhase> gcPhase_ = { GCPhase::GC_PHASE_IDLE };
};
} // namespace common

#endif  // COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_H
