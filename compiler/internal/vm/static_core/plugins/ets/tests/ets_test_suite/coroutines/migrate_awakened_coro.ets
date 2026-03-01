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

import { launch } from "std/concurrency"
import { Job } from "std/core"

let isLaunchMigrated: AtomicFlag = new AtomicFlag(false);

async function awaitee(): Promise<boolean> {
    return new Promise<boolean>((resolve: (v: boolean) => void) => {
        setTimeout(() => { resolve(true); }, 10);
    });
}

// this task actively waits on atomic to inscrease worker's loadFactor
function task(wId: int) {
    arktest.assertEQ(wId, CoroutineExtras.getWorkerId(), 'scheduled on wrong worker');
    while (!isLaunchMigrated.get()) {
        Coroutine.Schedule();
        arktest.assertEQ(wId, CoroutineExtras.getWorkerId(), "this coro shouldn't migrate in this test");
    }

}

function loadThisWorker(wId: int) {
    let otherId = (wId == 1) ? 2 : 1;
    let wGroup: LaunchParams = {
        abortFlag: true,
        workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [wId])
    };
    const WORKLOAD = 5;
    for (let i = 0; i < WORKLOAD; i++) {
        launch<void, (wId: int) => void>(task, wGroup, wId);
    }
}

function testLaunch(): void {
    let wId = CoroutineExtras.getWorkerId();
    arktest.assertTrue(wId == 1 || wId == 2);

    loadThisWorker(wId);
    waitForCompletion(awaitee);

    // migrate to the worker with the least load after await
    if (wId != CoroutineExtras.getWorkerId()) {
        isLaunchMigrated.set(true);
    }
}

function checkLaunchMigration(): void {
    launch<void, () => void>(testLaunch).Await();
}

function testAwaitMigration(): void {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    checkLaunchMigration();
    let res: boolean = isLaunchMigrated.get();
    isLaunchMigrated.set(true); // to finish waiting coroutines
    arktest.assertEQ(res, true);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.migrate_awakened_coro');
    testSuite.addTest('testAwaitMigration', testAwaitMigration);
    let res = testSuite.run();
    return res;
}
