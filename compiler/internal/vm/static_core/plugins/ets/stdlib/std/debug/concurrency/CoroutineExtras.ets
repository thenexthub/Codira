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

package std.debug.concurrency;

/**
 * @internal
 */
export final class CoroutineExtras {
    public static POLICY_ANY = 0;
    public static POLICY_NON_MAIN = 1;

    // returns coroutine worker id, which is the same for coros which are running within the same worker thread
    public static native getWorkerId(): int;
    // sets default coroutine affinity
    public static native setSchedulingPolicy(policy: int): void;
    // returns coroutine id
    public static native getCoroutineId(): int;
    // returns true if current worker is main, false otherwise
    public static native isMainWorker(): boolean;
    /**
     * This method changes the size of the worker pool
     * @param scaler is a parameter that determines the number of workers
     * that will be created or removed (in this case scaler is less than zero)
     */
    public static native scaleWorkersPool(scaler: int): void;

    // taskpool api
    // stop taskpool all workers after executes all taskpool tasks, for testSuite
    public static native stopTaskpool(): void;
    // increase taskpool workers
    public static native increaseTaskpoolWorkersToN(workersNum: int): void;
    // checks if worker has external scheduler
    public static native workerHasExternalScheduler(): boolean;
    // is external timer enabled
    public static native isExternalTimerEnabled(): boolean;
    // is taskpool using launch mode
    public static native isTaskpoolUsingLaunchMode(): boolean;
    // is taskpool supporting interop
    public static native isTaskpoolSupportingInterop(): boolean;
    // set taskpool shrink interval
    public static native setTaskPoolTriggerShrinkInterval(interval: int): void;
    // set taskpool idle threshold
    public static native setTaskPoolIdleThreshold(threshold: int): void;
    // get taskpool workers num
    public static native getTaskPoolWorkersNum(): int;
    // set taskpool workers limit
    public static native setTaskPoolWorkersLimit(limit: int): void;
    // retrigger taskpool shrink
    public static native retriggerTaskPoolShrink(): void;
    // get taskpool workers limit based on the device
    public static native getTaskPoolWorkersLimit(): int;
    // dump currently stored unhandled failed jobs
    public static native dumpUnhandledFailedJobs(): void;
}
