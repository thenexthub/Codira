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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_FIX_HEAP_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_FIX_HEAP_H

#include <functional>
#include <stack>
#include <vector>
#include "common_components/taskpool/task.h"
namespace common {

class ArkCollector;
class MarkingCollector;
class RegionDesc;
class RegionList;
class BaseObject;

/**
 * Enum representing different types of heap region fixing tasks
 */
enum FixRegionType {
    FIX_OLD_REGION,         // Fix all rset objects
    FIX_RECENT_OLD_REGION,  // Fix all rset objects before copyline
    FIX_RECENT_REGION,      // Fix all objects before copyline
    FIX_REGION,             // Fix all survived objects
    FIX_TO_REGION,          // Fix objects in to region
};

/**
 * Task unit for parallel heap fixing operations
 */
struct FixHeapTask final {
    RegionDesc *region;
    FixRegionType type;

    FixHeapTask(RegionDesc *region, FixRegionType type) noexcept : region(region), type(type) {}

    // Explicitly delete copy
    FixHeapTask(const FixHeapTask &) = delete;
    FixHeapTask &operator=(const FixHeapTask &) = delete;

    // Default move operations
    FixHeapTask(FixHeapTask &&) = default;
    FixHeapTask &operator=(FixHeapTask &&) = default;
};

using FixHeapTaskList = std::vector<FixHeapTask>;

/**
 * Worker class for parallel heap fixing operations
 */
class FixHeapWorker : public common::Task {
public:
    /**
     * Result structure containing the collected garbages and stats of heap fixing operations
     */
    struct Result {
        std::vector<std::tuple<RegionDesc *, BaseObject *, size_t>> monoSizeNonMovableGarbages;
        std::vector<std::pair<BaseObject *, size_t>> polySizeNonMovableGarbages;
        size_t numProcessedRegions = 0;
    };

    FixHeapWorker(ArkCollector *collector, TaskPackMonitor &monitor, Result &result,
                  std::function<FixHeapTask *()> &next) noexcept
        : Task(0), collector_(collector), monitor_(monitor), result_(result), getNextTask_(next)
    {
    }

    /**
     * Dispatches a region fixing task based on its type
     */
    void DispatchRegionFixTask(FixHeapTask *task);

    bool Run([[maybe_unused]] uint32_t threadIndex) override;

    /**
     * Collect fix heap tasks from a region list
     */
    static void CollectFixHeapTasks(FixHeapTaskList &taskList, RegionList &regionList, FixRegionType type);

private:
    /**
     * Enum defining how to handle dead objects in a region
     */
    enum DeadObjectHandlerType {
        FILL_FREE,             // Fill in free object immediately
        COLLECT_MONOSIZE_NONMOVABLE,  // Collect mono size non-movable objects (to be added to freelist)
        COLLECT_POLYSIZE_NONMOVABLE,        // Collect non-movable objects (to be filled free later)
        IGNORED,               // Ignore dead objects
    };

    void FixOldRegion(RegionDesc *region);
    void FixRecentOldRegion(RegionDesc *region);
    void FixToRegion(RegionDesc *region);

    template <DeadObjectHandlerType HandlerType>
    void FixRegion(RegionDesc *region);

    template <DeadObjectHandlerType HandlerType>
    void FixRecentRegion(RegionDesc *region);

    ArkCollector *collector_;
    TaskPackMonitor &monitor_;
    Result &result_;
    std::function<FixHeapTask *()> getNextTask_;
};

/**
 * Worker class for collecting garbages units after heap fixing operations
 */
class PostFixHeapWorker : public common::Task {
public:
    PostFixHeapWorker(FixHeapWorker::Result &result, TaskPackMonitor &monitor) noexcept

        : Task(0), monitor_(monitor), result_(result)
    {
    }

    /**
     * Performs post-processing cleanup tasks
     */
    void PostClearTask();

    // During fix phase we also collect the entire empty regions into garbage list from non-movable region.
    // However, we can only do it during post-fix because those region can contains metadata for getObjectSize
    // Hence we cache empty regions in those two stack and duirng post fix we collect the region as garbage,
    static std::stack<std::pair<RegionList*, RegionDesc *>> emptyRegionsToCollect;
    static void AddEmptyRegionToCollectDuringPostFix(RegionList *list, RegionDesc *region);
    static void CollectEmptyRegions();

    bool Run([[maybe_unused]] uint32_t threadIndex) override;

private:
    TaskPackMonitor &monitor_;
    FixHeapWorker::Result &result_;
};

};  // namespace common
#endif
