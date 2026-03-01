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

const elementsNum: int = 10;
const testNum: int = 20;
const stressNum: int = 100;

let mQueueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(elementsNum);
let sMTQueueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(stressNum);

function main(): int {
    let LinkedBlockingQueueTestsuite = new arktest.ArkTestsuite("LinkedBlockingQueueAddTest");

    LinkedBlockingQueueTestsuite.addTest("AddTest001", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("AddTest002", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(elementsNum);
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.add(testNum), false);
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("ThreadsAddTest001", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let jobs = new Array<Job<void>>(5);
        for (let i = 0; i < 5; i++) {
            jobs[i] = launch<void, () => void>(queueAddData);
        }
        for (let i: int = 0; i < 5; i++) {
            jobs[i].Await();
        }
        arktest.assertEQ(mQueueInt.getFirst(), testNum);
    });
    LinkedBlockingQueueTestsuite.addTest("TheadsAddAndPopTest001", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let jobs = new Array<Job<void>>(6);
        let p = 0;
        for (let i = 0; i < 3; i++) {
            jobs[p++] = launch<void, () => void>(queueAddData);
            jobs[p++] = launch<void, () => void>(queuePopData);
        }
        for (let i: int = 0; i < 6; i++) {
            jobs[i].Await();
        }
        arktest.assertTrue(mQueueInt.size <= 3);
    });
    LinkedBlockingQueueTestsuite.addTest("TheadsAddAndGetFirstTest003", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let jobs = new Array<Job<void>>(10);
        let p = 0;
        for (let i = 0; i < 5; i++) {
            jobs[p++] = launch<void, () => void>(queueAddData);
        }
        for (let i = 5; i < 10; i++) {
            jobs[p++] = launch<void, () => void>(queueGetFirstData);
        }
        for (let i: int = 0; i < 10; i++) {
            jobs[i].Await();
        }
        arktest.assertEQ(mQueueInt.getFirst(), testNum);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPopAndRemainingCapacityTest001", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let res = queueCapacityData();
        arktest.assertEQ(res, elementsNum);
        for (let i = 0; i < 5; i++) {
            queueAddData();
        }
        res = queueRemainingCapacityData();
        arktest.assertEQ(res, elementsNum - 5);
        queuePushData();
        arktest.assertEQ(mQueueInt.size, 6);
        arktest.assertEQ(mQueueInt.getFirst(), testNum);
        for (let i = 0; i < 2; i++) {
            queuePopData();
        }
        arktest.assertEQ(mQueueInt.size, 4);
        queuePollData();
        arktest.assertEQ(mQueueInt.size, 3);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndPopTest002", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            arktest.assertEQ(queueInt.size, 20 - i);
            if (!queueInt.isEmpty()) {
                queueInt.pop();
            }
        }
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndPollTest003", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            arktest.assertEQ(queueInt.size, 20 - i);
            arktest.assertEQ(queueInt.poll(), i);
        }
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndGetFirstAndPollTest004", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            arktest.assertEQ(queueInt.getFirst(), i);
            arktest.assertEQ(queueInt.poll(), i);
        }
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndGetFirstAndIsEnTest005", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            arktest.assertEQ(queueInt.getFirst(), i);
            arktest.assertEQ(queueInt.isEmpty(), false);
            arktest.assertEQ(queueInt.poll(), i);
        }
        arktest.assertEQ(queueInt.isEmpty(), true);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPopAndPollTest012", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < 20; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            if (!queueInt.isEmpty()) {
                queueInt.pop();
            }
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.isEmpty(), true);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPopAndPollAndGetSizeTest014", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < 20; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 20 - i);
            if (!queueInt.isEmpty()) {
                queueInt.pop();
            }
        }
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("AddStressTest001", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.size, stressNum);
    });

    return LinkedBlockingQueueTestsuite.run();
}

function queueCapacityData(): int {
    return mQueueInt.capacity;
}

function queueRemainingCapacityData(): int {
    return mQueueInt.remainingCapacity();
}

function queuePushData() {
    mQueueInt.push(testNum);
}

function queueGetFirstData() {
    mQueueInt.getFirst();
}

function queuePopData() {
    if (!mQueueInt.isEmpty()) {
        mQueueInt.pop();
    }
}

function queuePollData() {
    mQueueInt.poll();
}

function queueAddData() {
    mQueueInt.add(testNum);
}
