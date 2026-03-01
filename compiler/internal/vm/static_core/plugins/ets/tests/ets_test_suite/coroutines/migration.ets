/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let isLaunchMigrated: AtomicFlag = new AtomicFlag(false);

function BackOff() {
    let x = 0;
    for (let i = 0; i < 1000; i++) {
        x++;
    }
}

// this task, if was scheduled correctly (on same worker), actively waits for someone's migration
function task(wId: int, isLaunchedOnSameWorker: AtomicFlag) {
    if (wId == CoroutineExtras.getWorkerId()) {
        isLaunchedOnSameWorker.set(true);
        while (!isLaunchMigrated.get()) {
            Coroutine.Schedule();
            if (wId != CoroutineExtras.getWorkerId()) {
                isLaunchMigrated.set(true);
            }
        }
        return;
    }
    while (!isLaunchedOnSameWorker.get()) {
        Coroutine.Schedule();
    }
}

// Don't trigger scheduler, just wait for other coroutines to migrate from this worker
function spinLoopWithNoYield(wId: int) {
    let i = 0;
    arktest.assertEQ(wId, CoroutineExtras.getWorkerId(), 'This function is intended to be called from the same worker');
    while (!isLaunchMigrated.get()) {
        BackOff();
        if (i++ > 1000000) {
            console.error('no migration for too long');
            break;
        }
    }
}

function loadThisWorker(wId: int) {
    let isLaunchedOnSameWorker: AtomicFlag = new AtomicFlag(false);
    while (!isLaunchedOnSameWorker.get()) {
        launch<void, (wId: int, flg: AtomicFlag) => void>(task, wId, isLaunchedOnSameWorker);
        Coroutine.Schedule();
    }
}

function testLaunch(): void {
    let wId = CoroutineExtras.getWorkerId();
    arktest.assertTrue(wId == 1 || wId == 2);

    loadThisWorker(wId);
    if (wId != CoroutineExtras.getWorkerId()) {
        isLaunchMigrated.set(true);
    } else {
        spinLoopWithNoYield(wId);
    }
}

function checkLaunchMigration(): void {
    // scheduled on worker 1 or 2
    launch<void, () => void>(testLaunch).Await();
}

function testMigration(): void {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    checkLaunchMigration();
    let res: boolean = isLaunchMigrated.get();
    isLaunchMigrated.set(true); // to finish waiting coroutines
    arktest.assertEQ(res, true);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.migration');
    testSuite.addTest('testMigration', testMigration);
    let res = testSuite.run();
    return res;
}
