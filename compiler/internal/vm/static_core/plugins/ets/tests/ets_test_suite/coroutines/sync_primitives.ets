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

type callbackT = (id: int) => void;

class Event {
    public Wait() {
        while (this.flag.get() != true) {
            Coroutine.Schedule();
        }
    }

    public Fire() {
        this.flag.set(true);
    }

    private flag = new AtomicFlag(false);
};

let event = new Event();

class SharedJob<T> {
    constructor(jobs: Array<T>) {
        this.sharedJob = new CompletableJob<void>();
        SharedJob.completeJob(jobs, this.sharedJob);
    }

    public Await() {
        this.sharedJob.Await();
    }

    private static async completeJob<T>(jobs: Array<T>, sharedJob: CompletableJob<void>) {
        Coroutine.Schedule();
        for (let job of jobs) {
            (job as Job<void>).Await();
        }
        sharedJob.finish();
    }

    private sharedJob: CompletableJob<void>;
}

function runConcurrently(callback: callbackT, coroutines: number) {
    let jobs = new Array<Job<void>>(coroutines.toInt());
    for (let i = 0; i < coroutines; ++i) {
        const id = i.toInt();
        jobs[id] = launch<void, callbackT>(callback, id);
    }
    return new SharedJob<Job<void>>(jobs);
}

// The test checks exclusive access to the critical section
function exclusiveLockTest(locker: Lock) {
    const coroutines = 10;
    const workload = 1000;

    let count = 0;
    runConcurrently((id: int) => {
        for (let i = 0; i < workload; ++i) {
            locker.lock();
            count++;
            locker.unlock();
        }
    }, coroutines).Await();
    arktest.assertEQ(count, coroutines * workload);
}

// The test checks that there is no deadlock with the ReadLock
function sharedLockTest() {
    const coroutines = 10;

    let rLock = new RWLock().readLock();
    let jobs = new Array<CompletableJob<void>>(coroutines);
    for (let i: int = 0; i < coroutines; ++i) {
        jobs[i] = new CompletableJob<void>();
    }
    let sharedJob = new SharedJob<CompletableJob<void>>(jobs);
    runConcurrently((id: int) => {
        rLock.lock();
        jobs[id].finish();
        sharedJob.Await();
        rLock.unlock();
    }, coroutines).Await();
}

// The test checks whether the correct error was thrown
function illegalLockStateTest() {
    let arrError = new Array<Object>();
    let rwLock = new RWLock();
    try {
        rwLock.readLock().lock();
        rwLock.writeLock().unlock();
    } catch (e) {
        rwLock.readLock().unlock();
        arrError.push(e);
    }

    try {
        rwLock.writeLock().lock();
        rwLock.readLock().unlock();
    } catch (e) {
        rwLock.writeLock().unlock();
        arrError.push(e);
    }

    try {
        rwLock.readLock().unlock();
    } catch (e) {
        arrError.push(e);
    }

    try {
        rwLock.writeLock().unlock();
    } catch (e) {
        arrError.push(e);
    }

    try {
        new Mutex().unlock();
    } catch (e) {
        arrError.push(e);
    }

    arktest.assertEQ(arrError.length, 5);
    for (let error of arrError) {
        arktest.assertTrue(error instanceof IllegalLockStateError);
    }
}

// The test checks whether read-write locks are mutually exclusive
function readWriteMutualExclusionTest() {
    let rwLock = new RWLock();
    let readLock = rwLock.readLock();
    let writeLock = rwLock.writeLock();

    let writeAcquired = false;
    let writeLockAttempted = false;

    readLock.lock();

    let job = launch<void, () => void>(() => {
        writeLockAttempted = true;
        event.Fire();
        writeLock.lock();
        writeAcquired = true;
        writeLock.unlock();
    });
    event.Wait();

    arktest.assertTrue(writeLockAttempted);
    arktest.assertFalse(writeAcquired);

    readLock.unlock();
    job.Await();
    arktest.assertTrue(writeAcquired);
}

function exclusiveMutexLockTest() {
    exclusiveLockTest(new Mutex());
}

function exclusiveRWLockTest() {
    exclusiveLockTest(new RWLock().writeLock());
}

function main() {
    let testSuite = new arktest.ArkTestsuite('coroutines.sync_primitives');
    testSuite.addTest('exclusiveMutexLockTest', exclusiveMutexLockTest);
    testSuite.addTest('exclusiveRWLockTest', exclusiveRWLockTest);
    testSuite.addTest('sharedLockTest', sharedLockTest);
    testSuite.addTest('illegalLockStateTest', illegalLockStateTest);
    testSuite.addTest('readWriteMutualExclusionTest', readWriteMutualExclusionTest);

    return testSuite.run();
}
