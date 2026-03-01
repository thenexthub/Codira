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

let mainFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let workerFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let eaFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let eaFinReg2: FinalizationRegistry<undefined>|undefined = undefined;

let eaWorker1: EAWorker = new EAWorker();
let eaWorker2: EAWorker = new EAWorker();

function startGCWithCleanup(cause: GC.Cause) {
    GC.startGC(cause, true);
    Coroutine.Schedule();
}

class FinRegTestSuite extends arktest.ArkTestsuite {
    constructor() {
        super('FinalizationRegistrySuite');
    }

    public before() {
        mainFinReg = undefined;
        workerFinReg = undefined;
        eaFinReg = undefined;
        eaFinReg2 = undefined;
        startGCWithCleanup(GC.Cause.FULL);
    }
}

function register(finreg: FinalizationRegistry<undefined>) {
    finreg.register(new Object(), undefined);
}

function doInWorker(func: () => void): Job<void> {
    let params = new LaunchParams();
    params.workerGroupId = WorkerGroup.generateGroupId(WorkerDomain.GENERAL);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let job = launch<void, ()=>void>(() => {
        func();
    }, params);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
    return job;
}

/**
 * The test creates FinRegs in different domains, triggers GC in the main coro.
 * GC in the main coro must finalize FinRegs in the main and worker domains.
 */
function cleanupInMainTest() {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
        });
    }).Await();
    eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
        });
    }).Await();
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    startGCWithCleanup(GC.Cause.YOUNG);
    arktest.assertEQ(cleanupInMainHappened.get(), true);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), false);
}

/**
 * The test creates FinRegs in different domains, triggers GC in a worker coro.
 * GC in the worker coro must finalize the FinReg in the worker domain.
 */
function cleanupInWorkerTest() {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    let cleanupFinished = new Promise<void>((resolve) => {
        doInWorker(() => {
            workerFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInWorkerHappened.set(true);
                resolve(undefined);
            });
        }).Await();
    });
    eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
        });
    }).Await();
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    doInWorker(() => {
        startGCWithCleanup(GC.Cause.YOUNG);
    }).Await();
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), false);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), false);
}

/**
 * The test creates FinRegs in different domains, triggers GC in a EA coro.
 * GC in the EA coro must finalize the FinReg in the same EA worker.
 */
function cleanupInEAWorkerTest() {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
        });
    }).Await();
    let cleanupFinished = new Promise<void>((resolve) => {
        eaWorker1.run<void>((): void => {
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInEAWorkerHappened.set(true);
                resolve(undefined);
            });
        }).Await();
    });
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.YOUNG);
    }).Await();
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), false);
    arktest.assertEQ(cleanupInWorkerHappened.get(), false);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), true);
}

/**
 * The test creates FinRegs in different EA coros, triggers GC in the first EA coro.
 * GC in the first EA coro must finalize the FinReg in the same EA worker and
 * don't touch the FinReg in another EA coro.
 */
function cleanupInSameEAWorkerTest() {
    let cleanup1Happened = new AtomicFlag(false);
    let cleanup2Happened = new AtomicFlag(false);
    let cleanupFinished = new Promise<void>((resolve) => {
        eaWorker1.run<void>((): void => {
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                cleanup1Happened.set(true);
                resolve(undefined);
            });
        }).Await();
    });
    eaWorker2.run<void>((): void => {
        eaFinReg2 = new FinalizationRegistry<undefined>(() => {
            cleanup2Happened.set(true);
        });
    }).Await();
    register(eaFinReg!);
    register(eaFinReg2!);
    eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.YOUNG);
    }).Await();
    eaWorker2.run<void>((): void => {
        Coroutine.Schedule();
    }).Await();
    await cleanupFinished;
    arktest.assertEQ(cleanup1Happened.get(), true);
    arktest.assertEQ(cleanup2Happened.get(), false);
}

function surviveGCTest() {
    let cleanupFinished = new Promise<void>((resolve) => {
        eaWorker1.run<void>((): void => {
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                resolve(undefined);
            });
        }).Await();
    });
    register(eaFinReg!);
    // This GC must enqueue FinRegNode and don't clean up it.
    startGCWithCleanup(GC.Cause.YOUNG);
    // This GC must not collect FinRegNode.
    startGCWithCleanup(GC.Cause.FULL);
    // This GC should cleanup FinRegNode
    eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.FULL);
    }).Await();
    await cleanupFinished;
}

function main() {
    eaWorker1.start();
    eaWorker2.start();
    try {
        let testsuite = new FinRegTestSuite();
        testsuite.addTest('cleanupInMainTest', cleanupInMainTest);
        testsuite.addTest('cleanupInWorkerTest', cleanupInWorkerTest);
        testsuite.addTest('cleanupInEAWorkerTest', cleanupInEAWorkerTest);
        testsuite.addTest('cleanupInSameEAWorkerTest', cleanupInSameEAWorkerTest);
        testsuite.addTest('surviveGCTest', surviveGCTest);
        return testsuite.run();
    } finally {
        eaWorker1.join();
        eaWorker2.join();
    }
}
