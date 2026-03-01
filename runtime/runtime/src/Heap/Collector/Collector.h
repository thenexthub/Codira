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


#ifndef MRT_COLLECTOR_H
#define MRT_COLLECTOR_H

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <set>
#include <vector>

#include "GcRequest.h"
#include "GcStats.h"

namespace MapleRuntime {
// GCPhase describes phases for stw/concurrent gc.
enum GCPhase : uint8_t {
    GC_PHASE_UNDEF = 0,
    GC_PHASE_IDLE = 1,
    GC_PHASE_FINISH = 2,
    GC_PHASE_RECLAIM_SATB_NODE = 3,
    GC_PHASE_INIT = 8,

    // only gc phase after GC_PHASE_INIT ( enum value > GC_PHASE_INIT) needs barrier.
    GC_PHASE_ENUM = 9,
    GC_PHASE_TRACE = 10,
    GC_PHASE_CLEAR_SATB_BUFFER = 11,
    GC_PHASE_POST_TRACE = 12,
    GC_PHASE_PREFORWARD = 13,
    GC_PHASE_FORWARD = 14,
};

enum CollectorType {
    NO_COLLECTOR = 0, // No Collector
    PROXY_COLLECTOR,  // Proxy of Collector
    COPY_COLLECTOR,   // Regional-Copying GC
    SMOOTH_COLLECTOR, // wgc
    COLLECTOR_TYPE_COUNT,
};

// Central garbage identification algorithm.
class Collector {
public:
    Collector();
    virtual ~Collector() = default;

    static const char* GetGCPhaseName(GCPhase phase);

    // Initializer and finalizer.
    virtual void Init() = 0;
    virtual void Fini() {}
    const char* GetCollectorName() const;

    // This pure virtual function implements the trigger of GC.
    // reason: Reason for GC.
    // async:  Trigger from unsafe context, e.g., holding a lock, in the middle of an allocation.
    //         In order to prevent deadlocks, async trigger only add one async gc task and will not block.
    void RequestGC(GCReason reason, bool async);

    virtual GCPhase GetGCPhase() const { return gcPhase.load(std::memory_order_acquire); }

    virtual void SetGCPhase(const GCPhase phase) { gcPhase.store(phase, std::memory_order_release); }

    // determine how we treat new object during gc.
    virtual void MarkNewObject(BaseObject*) {}

    virtual void FixObject(BaseObject&) const {}

    virtual void RunGarbageCollection(uint64_t, GCReason) = 0;

    virtual GCStats& GetGCStats() { std::abort(); }

    virtual BaseObject* ForwardObject(BaseObject*) { std::abort(); }

    virtual bool ShouldIgnoreRequest(GCRequest& quest) = 0;
    virtual bool IsFromObject(BaseObject*) const { std::abort(); }
    virtual bool IsGhostFromObject(BaseObject*) const { std::abort(); }
    virtual bool IsUnmovableFromObject(BaseObject*) const { std::abort(); }
    virtual BaseObject* FindToVersion(BaseObject* obj) const = 0;

    virtual bool TryUpdateRefField(BaseObject*, RefField<>&, BaseObject*&) const { std::abort(); }
    virtual bool TryForwardRefField(BaseObject*, RefField<>&, BaseObject*&) const { std::abort(); }
    virtual bool TryUntagRefField(BaseObject*, RefField<>&, BaseObject*&) const { std::abort(); }
    virtual bool TryTagRefField(BaseObject*, RefField<>&, BaseObject*) const { std::abort(); }
    virtual RefField<> GetAndTryTagRefField(BaseObject*) const { std::abort(); }

    virtual bool IsOldPointer(RefField<>&) const { std::abort(); }
    virtual bool IsCurrentPointer(RefField<>&) const { std::abort(); }
    virtual void AddRawPointerObject(BaseObject*) { std::abort(); }
    virtual void RemoveRawPointerObject(BaseObject*) { std::abort(); }
    virtual void ResolveCycleRef() { std::abort(); }

    BaseObject* FindLatestVersion(BaseObject* obj) const
    {
        if (obj == nullptr) {
            return nullptr;
        }

        auto to = FindToVersion(obj);
        if (to != nullptr) {
            return to;
        }
        return obj;
    };

protected:
    virtual void RequestGCInternal(GCReason, bool) { std::abort(); }

    CollectorType collectorType = CollectorType::NO_COLLECTOR;
    std::atomic<GCPhase> gcPhase = { GCPhase::GC_PHASE_IDLE };
};
} // namespace MapleRuntime

#endif // MRT_COLLECTOR_H
