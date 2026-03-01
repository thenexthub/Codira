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

const TEST_ITERATIONS = 10;
const MAX_WORKERS_COUNT = 128;

function workerGroupAssertEq(a: WorkerGroupId, b: WorkerGroupId) {
    arktest.assertEQ(a.length, 2);
    arktest.assertEQ(b.length, 2);
    arktest.assertEQ(a[0], b[0]);
    arktest.assertEQ(a[1], b[1]);
}

function workerGroupAssertNe(a: WorkerGroupId, b: WorkerGroupId) {
    arktest.assertEQ(a.length, 2);
    arktest.assertEQ(b.length, 2);
    arktest.assertFalse(a[0] == b[0] && a[1] == b[1]);
}

function verifyWorkerIdCallback(expected: Array<WorkerId>) {
    const currId = getWorkerId();
    arktest.assertTrue(expected.includes(currId), `Error: current worker id: ${currId}, expected worker id: ${expected}`);
}

function getWorkerId(): int {
    return CoroutineExtras.getWorkerId();
}

// 1. Generates group ID using domain and hint
// 2. Launches callback using generated group
// 3. Verifies that callback launched on worker with ID from list of expected IDs
function verifyWorkerId(domain: WorkerDomain, hint: Array<WorkerId>, expected: Array<WorkerId>, mayThrow: boolean = false) {
    const params: LaunchParams = {workerGroupId: WorkerGroup.generateGroupId(domain, hint)};
    workerGroupAssertNe(params.workerGroupId, WorkerGroup.INVALID_ID);
    if (mayThrow) {
        try {
            launch<void, (expected: Array<WorkerId>) => void>(verifyWorkerIdCallback, params, expected).Await();
        } catch (e) {
            arktest.assertEQ((e as Error).message, 'Unable to launch coroutine: no suitable worker was found');
        }
    } else {
        launch<void, (expected: Array<WorkerId>) => void>(verifyWorkerIdCallback, params, expected).Await();
    }
}


function testGeneralDomain() {
    const mainId = getWorkerId();

    const ea = new EAWorker();
    ea.start();
    const eaId = ea.run<int>(getWorkerId).Await();

    // Collect some general worker IDs
    let generalWorkerIds = new Set<WorkerId>();
    for (let i = 0; i < TEST_ITERATIONS; i++) {
        const params: LaunchParams = {workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL)};
        workerGroupAssertNe(params.workerGroupId, WorkerGroup.INVALID_ID);
        launch<void, () => void>((): void => { generalWorkerIds.add(getWorkerId()); }, params).Await();
    }
    arktest.assertFalse(generalWorkerIds.has(mainId));
    const generalWorkerIdsArr = Array.from<WorkerId>(generalWorkerIds);

    // Collect all possible general worker IDs
    const possibleWorkerIdsArr = new Array<WorkerId>();
    for (let i: WorkerId = 0; i < MAX_WORKERS_COUNT; ++i) {
        if (i != eaId && i != mainId) {
            possibleWorkerIdsArr.push(i);
        }
    }

    // Test that for GENERAL domain coroutines are executed only on workers from "generalWorkerIds"
    // Test that eaId and mainId hints are ignored
    // Test that general worker ID hint is not ignored
    const test = (mayThrow: boolean) => {
        for (let id of generalWorkerIds) {
            verifyWorkerId(WorkerDomain.GENERAL, [id, eaId, mainId], [id], mayThrow);
        }
        verifyWorkerId(WorkerDomain.GENERAL, [], possibleWorkerIdsArr, mayThrow);
        verifyWorkerId(WorkerDomain.GENERAL, [eaId], possibleWorkerIdsArr, mayThrow);
        verifyWorkerId(WorkerDomain.GENERAL, [mainId], possibleWorkerIdsArr, mayThrow);
    };

    let jobs = Array<Job<void>>();
    for (let i = 0; i < TEST_ITERATIONS; i++) {
        test(false);
        launch<void, () => void>((): void => { test(false); }).Await();
        ea.run<void>(() => { test(false); }).Await();
        jobs.push(launch<void, () => void>((): void => { test(true); }));
        jobs.push(ea.run<void>(() => { test(true); }));
    }

    ea.join();

    for (let job of jobs) {
        job.Await();
    }
}

function testExactIdDomain() {
    // Collect some worker IDs (main + ea + workers on which "launch" launches callbacks)
    let workerIds = new Set<WorkerId>();
    const ea = new EAWorker();
    ea.start();
    const eaId = ea.run<int>(getWorkerId).Await();
    workerIds.add(eaId);
    const mainId = getWorkerId();
    workerIds.add(mainId);
    for (let i = 0; i < TEST_ITERATIONS; i++) {
        launch<void, () => void>((): void => { workerIds.add(getWorkerId()); }).Await();
    }
    const workerIdsArr = Array.from<WorkerId>(workerIds);

    // Test that for EXACT_ID domain any particular id hint (including main ID and EA ID) is not ignored
    const test = (mayThrow: boolean) => {
        for (let id of workerIds) {
            verifyWorkerId(WorkerDomain.EXACT_ID, [id], [id], mayThrow);
            verifyWorkerId(WorkerDomain.EXACT_ID, [workerIdsArr[0], workerIdsArr[1], id], [workerIdsArr[0], workerIdsArr[1], id], mayThrow);
        }
    };

    let jobs = Array<Job<void>>();
    for (let i = 0; i < TEST_ITERATIONS; i++) {
        test(false);
        launch<void, () => void>((): void => { test(false); }).Await();
        ea.run<void>(() => { test(false); }).Await();
        jobs.push(launch<void, () => void>((): void => { test(true); }));
        jobs.push(ea.run<void>(() => { test(true); }));
    }

    ea.join();

    // Test that for EXACT_ID domain with empty hint ANY_ID worker group is generated
    workerGroupAssertEq(WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, []), WorkerGroup.ANY_ID);
    workerGroupAssertEq(WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID), WorkerGroup.ANY_ID);

    for (let job of jobs) {
        job.Await();
    }
}

function testMainDomain() {
    const mainId = getWorkerId();
    const ea = new EAWorker();
    ea.start();

    // Test that for MAIN domain any hint is ignored and coroutines are always executed on main worker
    const test = (mayThrow: boolean) => {
        verifyWorkerId(WorkerDomain.MAIN, [], [mainId], mayThrow);
        verifyWorkerId(WorkerDomain.MAIN, [1, 2, 3], [mainId], mayThrow);
    };

    let jobs = Array<Job<void>>();
    for (let i = 0; i < TEST_ITERATIONS; i++) {
        test(false);
        launch<void, () => void>((): void => { test(false); }).Await();
        ea.run<void>(() => { test(false); }).Await();
        jobs.push(launch<void, () => void>((): void => { test(true); }));
        jobs.push(ea.run<void>(() => { test(true); }));
    }

    ea.join();

    for (let job of jobs) {
        job.Await();
    }
}

function testEADomain() {
    // Test that without created EA workers group ID is invalid
    workerGroupAssertEq(WorkerGroup.generateGroupId(WorkerDomain.EA), WorkerGroup.INVALID_ID);

    const mainId = getWorkerId();

    const ea1 = new EAWorker();
    ea1.start();
    const ea2 = new EAWorker();
    ea2.start();
    const eaId1 = ea1.run<int>(getWorkerId).Await();
    const params1: LaunchParams = {workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EA, [eaId1])};
    const eaId2 = ea2.run<int>(getWorkerId).Await();
    const params2: LaunchParams = {workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EA, [eaId2])};

    // Test that for EA domain coroutines are always executed on EA workers
    // Test hint: non-EA worker IDs are ignored and EA worker IDs are not ignored
    const test = (mayThrow: boolean) => {
        verifyWorkerId(WorkerDomain.EA, [eaId1, mainId], [eaId1], mayThrow);
        verifyWorkerId(WorkerDomain.EA, [eaId2, mainId], [eaId2], mayThrow);
        verifyWorkerId(WorkerDomain.EA, [], [eaId1, eaId2], mayThrow);
        verifyWorkerId(WorkerDomain.EA, [eaId1, eaId2, mainId, 0, 1, 2], [eaId1, eaId2], mayThrow);
    }

    // NOTE: await all jobs so eaworkers won't be destroyed before generateGroupId + launch
    for (let i = 0; i < TEST_ITERATIONS; i++) {
        test(false);
        launch<void, () => void>((): void => { test(false); }).Await();
        ea1.run<void>(() => { test(false); }).Await();
        ea2.run<void>(() => { test(false); }).Await();
    }

    ea1.join();
    ea2.join();
}

function testInvalidGroupId() {
    const params: LaunchParams = {workerGroupId: WorkerGroup.INVALID_ID};
    arktest.expectThrow(() => { launch<void, () => void>((): void => {}, params).Await(); }, (e: Error) => {
        return (e as Error).message === 'Cannot launch with invalid group id!';
    });
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.worker_group');
    testSuite.addTest('testEADomain', testEADomain);
    testSuite.addTest('testGeneralDomain', testGeneralDomain);
    testSuite.addTest('testExactIdDomain', testExactIdDomain);
    testSuite.addTest('testMainDomain', testMainDomain);
    testSuite.addTest('testInvalidGroupId', testInvalidGroupId);
    return testSuite.run();
}