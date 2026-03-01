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

const elementsNum: int = 100;
let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueGetEndTest");

    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt001", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000).toInt();
            queueInt.push(el);
            arktest.assertEQ(el, queueInt.getEnd());
        }
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt002", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch<void, () => void>(queueIntGetEnddata);
        queueInt.push(elementsNum);
        p1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt003", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch<void, () => void>(queueIntGetEnddata);
        queueInt.push(elementsNum);
        p1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt004", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch<void, () => void>(queueIntGetEnddata);
        let a1: int = 10;
        let a2: int = 20;
        queueInt.push(a1);
        queueInt.push(a2);
        p1.Await();
        arktest.assertEQ(queueInt.size, 2);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt005", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        let p1 = launch<void, () => void>(queueIntGetEndB1data);
        let p2 = launch<void, () => void>(queueIntAddTwodata);
        let p3 = launch<void, () => void>(queueIntGetEndB2data);
        let p4 = launch<void, () => void>(queueIntAddTwodata);
        p1.Await();
        p2.Await();
        p3.Await();
        p4.Await();
        arktest.assertFalse(queueInt.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt006", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        queueInt.push(10);
        let p1 = launch<void, () => void>(queueIntGetEnddata);
        let p2 = launch<void, () => void>(queueIntGetEnddata);
        queueInt.push(20);
        p1.Await();
        p2.Await();
        let p3 = launch<void, () => void>(queueIntGetEnddata);
        p3.Await();
        arktest.assertEQ(queueInt.getEnd(), 20);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntGetEnddata() {
    queueInt.getEnd();
}

function queueIntGetEndB1data() {
    queueInt.getEnd();
}

function queueIntGetEndB2data() {
    queueInt.getEnd();
}

function queueIntAddTwodata() {
    queueInt.push(10);
    queueInt.push(20);
}
