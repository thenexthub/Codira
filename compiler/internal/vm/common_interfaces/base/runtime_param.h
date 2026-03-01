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

#ifndef COMMON_INTERFACES_BASE_RUNTIME_PARAM_H
#define COMMON_INTERFACES_BASE_RUNTIME_PARAM_H

#include "base/common.h"

namespace common {
/*
* @struct HeapParam
* @brief Data structure for Arkcommon heap configuration parameters,\n
* including the heap size, region size at runtime, and etc.
*/
struct HeapParam {
    /*
    * The reference value of region size, measured in KB, default to 64 KB, must be in range [4KB, 64KB].
    * It will be set to default value if assigned with 0.
    */
    size_t regionSize;

    /*
    * The maximum size of arkcommon heap, measured in KB, default to 256 * 1024 KB, must >= 4MB.
    * It will be set to default value if assigned with 0.
    */
    size_t heapSize;

    /*
    * Threshold used to determine whether a region is exempted (i.e., will not be forwarded).
    * If the percentage of live objects in a region is greater than this value, this region will not be exempted.
    * Default to 0.8, must be in range (0, 1].
    * It will be set to default value if assigned with 0.
    */
    double exemptionThreshold;

    /*
    * A hint to guide collector to release physical memory to OS.
    * heap utilization = heap-used-memory / total-heap-memory.
    * During each gc, collector determines how much memory should be cached,
    * and let the heap utilization be close to this value.
    * Default to 0.80, must be in range (0, 1].
    * It will be set to default value if assigned with 0.
    */
    double heapUtilization;

    /*
    * The ratio to expand heap after each GC.
    * GC is probably triggered more often if this value is set to an improperly small number.
    * Default to 0.15, must > 0.
    * It will be set to default value if assigned with 0.
    */
    double heapGrowth;

    /*
    * The rate of allocating memory from heap.
    * this value is the lower bound of the real allocation rate.
    * allocator maybe wait some time if this value is set with an improperly small number.
    * Mesured in MB/s, default to 10240 MB/s, must be > 0 MB/s.
    * It will be set to default value if assigned with 0.
    */
    double allocationRate;

    /*
    * The maximum wait time when allocating memory from heap.
    * The latter alloction will wait a number of time if the two alloction interval is less than the wait time.
    * The real wait time is the minimum of allocationWaitTime and the wait time calculated from real alloction rate.
    * Measured in ns, default to 1000 ns, must > 0 ns.
    * It will be set to default value if assigned with 0.
    */
    size_t allocationWaitTime;
};

/*
* @struct GCParam
* @brief Data structure for Arkcommon garbage collection configuration parameters,\n
* including the garbage ratio, garbage collection interval and etc.
*/
struct GCParam {
    /*
    * Set false to disable GC, default is true
    */
    bool enableGC;

    /*
    * Set true swicth to stop-the-world GC, set false swicth to concurrent-copying GC, default is false
    */
    bool enableStwGC;

    /*
    * GC will be triggered when heap allocated size is greater than this threshold.
    * Measured in KB, must be > 0.
    */
    size_t gcThreshold;

    /*
    * The threshold used to determine whether to collect from-space during GC.
    * The from-space will be collected if the percentage of the garbage in from space is greater than this threshold.
    * default to 0.5, must be in range [0.1, 1.0].
    */
    double garbageThreshold;

    /*
    * Minimum interval each GC request will be responded. If two adjacent GC requests with
    * interval less than this value, the latter one is ignored.
    * Measured in ns, default to 150 ms, must be > 0 ms.
    * It will be set default value if the value is 0.
    */
    uint64_t gcInterval;

    /*
    * Minimum interval each backup GC request will be responded.
    * Backup GC will be triggered if there is no GC during this interval.
    * Measured in ns, default to 240 s, must be > 0 s.
    * It will be set default value if the value is 0.
    */
    uint64_t backupGCInterval;

    /*
    * Parameters for adjusting the number of GC threads.
    * The number of gc threads is ((the hardware concurrency / this value) - 1).
    * default to 8, must be > 0.
    * It will be set default value if the value is 0.
    */
    uint32_t gcThreads;

    /*
    * The maximum grow bytes of next heuristic gc threshold, the default value is 32MB;
    */
    size_t maxGrowBytes;

    /*
    * The minimum grow bytes of next heuristic gc threshold, the default value is 8MB;
    */
    size_t minGrowBytes;

    /*
    * Heruistic gc grow bytes multiplier, The default value of foreground is 1.0, background is 2.0.
    */
    double multiplier;

    /*
    * Young gc throughput adjustment factor, the default value is 0.5.
    */
    double ygcRateAdjustment;

    /*
    * The minimum remaining bytes for next heuristic gc, the default value is 128KB.
    */
    size_t kMinConcurrentRemainingBytes;

    /*
    * The maximum remaining bytes for next heuristic gc, the default value is 512KB.
    */
    size_t kMaxConcurrentRemainingBytes;
};

/*
* @struct RuntimeParam
* @brief Data structure for Arkcommon runtime parameters,\n
* including the config information of heap, garbage collection, thread and log.
*/
struct RuntimeParam {
    struct HeapParam heapParam;
    struct GCParam gcParam;
};
}  // namespace common

#endif  // COMMON_INTERFACES_BASE_RUNTIME_PARAM_H
