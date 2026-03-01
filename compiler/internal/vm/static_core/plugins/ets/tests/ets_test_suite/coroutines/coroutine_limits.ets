/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class CoroutineLimit {
    coroCount: int;
    static readonly MAX_COROUTINE_COUNT: int = 7000;
};

function startTest(limitTest: (limit: CoroutineLimit) => void) {
    let limit = new CoroutineLimit();
    try {
        limitTest(limit);
    } catch (e) {
        arktest.assertTrue(e instanceof CoroutinesLimitExceedError);
        arktest.assertEQ(limit.coroCount, CoroutineLimit.MAX_COROUTINE_COUNT);
    }
    Coroutine.Schedule();
}

function asyncLimitTest() {
    let asyncLimit = (limit: CoroutineLimit) => {
        let aFunc: (() => Promise<void>) | null = null;
        aFunc = async () => {
            limit.coroCount++;
            await aFunc!();
        }
        waitForCompletion(aFunc!);
    };
    startTest(asyncLimit);
}

function timerLimitTest() {
    let timerLimit = (limit: CoroutineLimit) => {
        while (true) {
            setTimeout(() => {}, 0);
            limit.coroCount++;
        }
    };
    startTest(timerLimit);
}

function launchLimitTest() {
    let launchLimit = (limit: CoroutineLimit) => {
        let wGroup: LaunchParams = { workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [0]) };
        while (true) {
            launch(() => {}, wGroup);
            limit.coroCount++;
        }
    };
    startTest(launchLimit);
}

function promiseLimitTest() {
    let promiseLimit = (limit: CoroutineLimit) => {
        let p = Promise.resolve(undefined);
        while (true) {
            p.then(() => {});
            limit.coroCount++;
        }
    };
    startTest(promiseLimit);
}

function main() {
    let testSuite = new arktest.ArkTestsuite('coroutine_limits');
    testSuite.addTest('asyncLimitTest', asyncLimitTest);
    testSuite.addTest('timerLimitTest', timerLimitTest);
    testSuite.addTest('launchLimitTest', launchLimitTest);
    testSuite.addTest('promiseLimitTest', promiseLimitTest);
    testSuite.run();
}
