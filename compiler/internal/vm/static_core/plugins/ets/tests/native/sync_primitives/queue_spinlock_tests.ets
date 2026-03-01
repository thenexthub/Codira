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

native function spinlockCreate(): Object;
native function spinlockGuard(spinlock: Object, callback: Object): void;
native function spinlockIsHeld(spinlock: Object): boolean;

function runConcurrently(coroutinesCount: number, callback: () => void) {
    let spinlock = spinlockCreate();
    let jobs = new Array<Job<boolean>>;
    let guard = () => {
        spinlockGuard(spinlock, callback);
        return true;
    };
    for (let i = 0; i < coroutinesCount; ++i) {
        let j = launch<boolean, () => boolean>(guard);
        jobs.push(j);
    }
    for (let job of jobs) {
        job.Await();
    }
}

function exclusiveAccess(): void {
    const coroutinesCount = 10;
    const workload = 100500;
    let count = 0;
    runConcurrently(coroutinesCount, () => {
        for (let i = 0; i < workload; ++i) {
            count++;
        }
    });
    arktest.assertTrue(count == workload * coroutinesCount);
}

function objectAllocationsUnderLock() {
    const coroutinesCount = 10;
    const workload = 1000;
    let objects = Array.create<Object>(workload, new Object());
    // Checks that the deadlock did not occur due to GC
    runConcurrently(coroutinesCount, () => {
        for (let i = 0; i < workload; ++i) {
            objects[i] = new Object();
        }
    });
}

function throwingExceptionUnderLock() {
    let spinlock = spinlockCreate();
    let errorCatched = false;
    try {
        spinlockGuard(spinlock, () => { throw new Error(); });
    } catch (e) {
        errorCatched = true;
        arktest.assertTrue(!spinlockIsHeld(spinlock));
    }
    arktest.assertTrue(errorCatched);
}
