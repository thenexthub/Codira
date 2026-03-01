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

const elementsNum: int = 10;
const stressNum: int = 100;
let f1: int | undefined = 0;
let f2: int | undefined = 0;

let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueGetFirstTest");

    ArrayBlockingQueueTestsuite.addTest("GetFirstTest001", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        let res = queueInt.getFirst();
        arktest.assertEQ(res, 0);
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTest002", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        queueInt.pop();
        let res = queueInt.getFirst();
        arktest.assertEQ(res, 1);
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, elementsNum - 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstAndSizeAndPopAndPollTest009", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 20 - i);
            queueInt.pop();
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstAndIsEmptyAndPopAndPollTest010", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            queueInt.pop();
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.isEmpty(), true);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstStressTest005", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            arktest.assertEQ(queueInt.getFirst(), i);
            queueInt.pop();
        }
        arktest.assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt001", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        let frontNum = 0;
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000).toInt();
            queueInt.push(el);
            if (i == 0) {
                frontNum = el;
            }
        }
        arktest.assertEQ(queueInt.getFirst(), frontNum);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt002", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        let p1 = launch<void, () => void>(queueIntGetFirstdata);
        let frontNum: int = 10;
        queueInt.push(frontNum);
        p1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt003", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch<void, () => void>(queueIntGetFirstdata);
        let frontNum: int = 10;
        queueInt.push(frontNum);
        p1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt004", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        let p1 = launch<void, () => void>(queueIntGetFirstF1data);
        queueInt.push(10);
        queueInt.push(20);
        p1.Await();
        let p2 = launch<void, () => void>(queueIntGetEndB1data);
        p2.Await();
        arktest.assertEQ(queueInt.size, 2);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt005", () =>
    {
        f1 = 0;
        f2 = 0;
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        arktest.assertTrue(queueInt.isEmpty());
        let p1 = launch<void, () => void>(queueIntAddTwodata);
        let p2 = launch<void, () => void>(queueIntGetFirstF1data);
        let p3 = launch<void, () => void>(queueIntGetFirstF2data);
        p1.Await();
        p2.Await();
        p3.Await();
        arktest.assertTrue(f1 == 10 || f1 == undefined);
        arktest.assertTrue(f2 == 10 || f2 == undefined);
        arktest.assertFalse(queueInt.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt006", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        queueInt.push(10);
        queueInt.push(30);
        let p1 = launch<void, () => void>(queueIntGetFirstdata);
        queueInt.push(20);
        p1.Await();
        let p2 = launch<void, () => void>(queueIntGetFirstdata);
        p2.Await();
        arktest.assertEQ(queueInt.getFirst(), 10);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntGetEnddata() {
    queueInt.getEnd();
}

function queueIntGetEndB1data() {
    queueInt.getEnd();
}

function queueIntGetFirstF1data() {
    f1 = queueInt.getFirst();
}

function queueIntGetFirstF2data() {
    f2 = queueInt.getFirst();
}

function queueIntAddTwodata() {
    queueInt.push(10);
    queueInt.push(20);
}

function queueIntGetFirstdata() {
    queueInt.getFirst();
}
