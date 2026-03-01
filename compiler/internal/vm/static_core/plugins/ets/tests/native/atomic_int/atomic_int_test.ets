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

class AtomicInt {
    constructor(v: int) {
        this.value = v;
    }

    public native set(v: int): void;
    public native get(): int;

    public native exchange(v: int): int;
    public native compareAndSwap(expected: int, newValue: int): int;

    public native fetchAndAdd(v: int): int;
    public native fetchAndSub(v: int): int;

    private value: int;
}

let workers: EAWorker[] = []
const kThreadCount = 10
const kIterations = 1000

function main(): int {
    let AtomicIntTestsuite = new arktest.ArkTestsuite("AtomicIntTest")
    for (let i = 0; i < kThreadCount; i++) {
        let worker = new EAWorker()
        workers.push(worker)
        worker.start()
    }
    AtomicIntTestsuite.addTest("fetchAndAddIsThreadSafe", fetchAndAddThreadSafeTest)
    AtomicIntTestsuite.addTest("fetchAndSubIsThreadSafe", fetchAndSubThreadSafeTest)
    AtomicIntTestsuite.addTest("compareAndSetOnlyOneWins", compareAndSetOnlyOneWinsTest)

    // testVisiablity many times to ensure visibility issues are caught
    let res = AtomicIntTestsuite.run()
    for (let i: int = 0; i < 1000; i++) {
        try {
            testVisiablity();
        } catch (e) {
            console.log("testVisiablity failed with error: " + e.message + " at iteration " + i);
            res = 4;  // return a failure code if visibility test fails
            break;    // stop after the first failure to avoid flooding the output
        }
    }
    for (let worker of workers) {
        worker.join()
    }
    return res
}

function fetchAndAddThreadSafeTest() {
    let atomicInteger = new AtomicInt(0)
    let jobs: Job<void>[] = [];
    for (let worker of workers) {
        let job = worker.run<void>((atomicInt: AtomicInt) => {
            for (let j = 0; j < kIterations; j++) {
                atomicInt.fetchAndAdd(1)
            }
        }, atomicInteger)
        jobs.push(job)
    }
    for (let job of jobs) {
        job.Await()
    }
    arktest.assertEQ(atomicInteger.get(), kThreadCount * kIterations)
}

function fetchAndSubThreadSafeTest() {
    let atomicInteger = new AtomicInt(0)
    let jobs: Job<void>[] = [];
    for (let worker of workers) {
        let job = worker.run<void>((atomicInt: AtomicInt) => {
            for (let j = 0; j < kIterations; j++) {
                atomicInt.fetchAndSub(1)
            }
        }, atomicInteger)
        jobs.push(job)
    }
    for (let job of jobs) {
        job.Await()
    }
    arktest.assertEQ(atomicInteger.get(), -kThreadCount * kIterations)
}

function compareAndSetOnlyOneWinsTest() {
    let atomicInteger = new AtomicInt(0)
    let winnerCounter = new AtomicInt(0)
    let jobs: Job<void>[] = [];
    for (let worker of workers) {
        let job = worker.run<void>((atomicInt: AtomicInt) => {
            if (atomicInt.compareAndSwap(0, 1) == 0) {
                winnerCounter.fetchAndAdd(1)
            }
        }, atomicInteger)
        jobs.push(job)
    }
    for (let job of jobs) {
        job.Await()
    }
    arktest.assertEQ(winnerCounter.get(), 1)
    arktest.assertEQ(atomicInteger.get(), 1)
}

let a: int = 0;

function testVisiablity() {
    let atomicInteger = new AtomicInt(0)
    let worker_a = workers[0]
    let worker_b = workers[1]

    let job_a = worker_a.run<void>((atomicInt: AtomicInt) => {
        a = 1;
        atomicInt.set(42);
    }, atomicInteger)

    let job_b = worker_b.run<void>((atomicInt: AtomicInt) => {
        if (atomicInt.get() === 42) {
            arktest.assertEQ(a, 1, "assert a == 1 failed in testVisiablity");
        } else {
            // passed without assertion, this is expected if job_b runs before job_a
        }
    }, atomicInteger)
    job_a.Await()
    job_b.Await()
    a = 0;  // Reset a for next test
}
