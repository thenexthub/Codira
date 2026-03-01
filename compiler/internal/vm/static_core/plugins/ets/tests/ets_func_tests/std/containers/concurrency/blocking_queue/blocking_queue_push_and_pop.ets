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

type BlockingQueueI = containers.BlockingQueue<int>;
type BQConstructorI = (capacity: int) => BlockingQueueI;
type TestFunction = (constr: BQConstructorI) => void;
type BlockingQueue = containers.LinkedBlockingQueue<int> | containers.ArrayBlockingQueue<int>;

function addLaunch<T> (arr: Array<Job<T>>, func: () => T): void {
    arr.push(launch <T, () => T>(func));
}

function awaitAll <T> (arr: Array<Job<T>>): Array<T> {
    let res: Array<T> = new Array<T> (arr.length);
    for (let i = 0; i < arr.length; ++ i)
        res[i] = arr[i].Await();
    return res;
}

function sumArrInto (to: FixedArray<int>, from: FixedArray<int>): void {
    arktest.assertEQ(to.length, from.length);
    for (let i = 0; i < to.length; ++ i) {
        to[i] += from[i];
    }
}

function subArrInto(to: FixedArray<int>, from: FixedArray<int>): void {
    arktest.assertEQ(to.length, from.length);
    for (let i = 0; i < to.length; ++ i) {
        to[i] -= from[i];
    }
}

function assertEQAll(array: FixedArray<int>, value: int): void {
    for (let i=0; i < array.length; ++ i)
        arktest.assertEQ(array[i], value)
}

function testOrderPopAndPushMany(constr: BQConstructorI) {
    let queue = constr(200);
    const EXTRA = -1;
    let pushers: Job<void>[] = [];
    let poppers: Job<void>[] = [];

    const PUSH_COUNT = 70;
    const JOB_COUNT = 7;

    let makePushF = (seed: int) => () => {
        for (let i = 0; i < PUSH_COUNT; ++i) {
            queue.push(seed * PUSH_COUNT + i);
        }
        queue.push(EXTRA);
    };

    let popF = () => {
        let prev: FixedArray<int> = new FixedArray<int>(JOB_COUNT)
        for (let i = 0; i < JOB_COUNT; ++ i ) prev[i] = -1;
        while (true) {
            let val = queue.pop();
            if (val == EXTRA) break;
            let i = val / PUSH_COUNT;
            val = val - i * PUSH_COUNT;
            arktest.assertLT(prev[i], val);
            prev[i] = val;
            Coroutine.Schedule();
        }
    };

    for (let i = 0; i < JOB_COUNT; ++i) {
        addLaunch(pushers, makePushF(i));
        addLaunch(poppers, popF);
    }

    awaitAll(pushers);
    awaitAll(poppers);
}

function testPopAndPushCount(constr: BQConstructorI): void {
    let queue = constr(200);
    const EXTRA = -1;
    let pushers: Job<void>[] = [];
    let poppers: Job<FixedArray<int>>[] = [];

    const PUSH_COUNT = 100;
    const JOB_COUNT = 10;

    let pushFunction: () => void = () => {
        for (let i = 0; i < PUSH_COUNT; ++i)
            queue.push(i);
        queue.push(EXTRA);
    };

    let popFunction: () => FixedArray<int> = () => {
        let counter: FixedArray<int> = new FixedArray<int>(PUSH_COUNT)
        while (true) {
            let val = queue.pop();
            if (val == EXTRA) break;
            ++ counter[val];
            Coroutine.Schedule();
        }
        return counter;
    };

    for (let i = 0; i < JOB_COUNT; ++i) {
        addLaunch(pushers, pushFunction);
        addLaunch(poppers, popFunction);
    }

    awaitAll<void>(pushers);
    let finalCounter: FixedArray<int> = new FixedArray<int>(PUSH_COUNT)
    let resCounters = awaitAll<FixedArray<int>>(poppers);

    for (let i = 0; i < JOB_COUNT; ++ i)
        sumArrInto(finalCounter, resCounters[i]);

    assertEQAll(finalCounter, JOB_COUNT);
    arktest.assertTrue(queue.isEmpty());
}

function main(): int {
    let tests: Array<[String, TestFunction]> = [
        ["testOrderPopAndPushMany", testOrderPopAndPushMany],
        ["testPopAndPushCount", testPopAndPushCount],
    ];

    let LBQTestSuitecase = new arktest.ArkTestsuite("LinkedBlockingQueueTest")
    let ABQTestSuitecase = new arktest.ArkTestsuite("ArrayBlockingQueueTest")

    let LBQconstr: BQConstructorI = (capacity: int) => new containers.LinkedBlockingQueue<int>(capacity);
    let ABQconstr: BQConstructorI = (capacity: int) => new containers.ArrayBlockingQueue<int>(capacity);

    let suites: Array<[arktest.ArkTestsuite, BQConstructorI]> = [
        [LBQTestSuitecase, LBQconstr],
        [ABQTestSuitecase, ABQconstr],
    ]

    for (let j = 0; j < suites.length; ++ j) {
        let arktestsuite = suites[j][0];
        let constr = suites[j][1];
        for (let i = 0; i < tests.length; ++ i) {
            let name = tests[i][0];
            let testF = tests[i][1];
            arktestsuite.addTest(name, () => testF(constr));
        }
    }

    let codeLBQ = LBQTestSuitecase.run();
    let codeABQ = ABQTestSuitecase.run();
    if (codeLBQ == 0)
        return codeABQ;
    return codeLBQ;
}

