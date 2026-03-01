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

let testArrayInt: Array<int> = new Array<int>(elementsNum);
let testArrayObj: Array<Object> = new Array<Object>(elementsNum);
let testArrayStr: Array<String> = new Array<String>(elementsNum);
let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
let queueObj: containers.ArrayBlockingQueue<Object> = new containers.ArrayBlockingQueue<Object>(200);
let queueStr: containers.ArrayBlockingQueue<String> = new containers.ArrayBlockingQueue<String>(200);
let queueIntPopN: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueuePopTest");

    ArrayBlockingQueueTestsuite.addTest("ConcurrentPushPopTestStr", () =>
    {
        let p1 = launch<void, () => void>(pusherStr);
        let arrayToCheck = popperStr();
        p1.Await();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        arktest.assertEQ(queueStr.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt001", () =>
    {
        let queueIntPop1: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        arktest.assertEQ(queueIntPop1.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPop1.push(i);
        }
        arktest.assertEQ(queueIntPop1.getFirst(), 0);
        queueIntPop1.pop();
        arktest.assertEQ(queueIntPop1.size, elementsNum - 1);
        arktest.assertTrue(queueIntPop1.getFirst() != 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt002", () =>
    {
        let queueIntPop2: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPop2.push(i);
        }
        let num = queueIntPop2.getFirst();
        arktest.assertEQ(num, queueIntPop2.pop());
        arktest.assertEQ(queueIntPop2.size, elementsNum - 1);
        while (!queueIntPop2.isEmpty()) {
            let el = queueIntPop2.getFirst();
            arktest.assertEQ(el, queueIntPop2.pop());
        }
        let p1 = launch<void, (queueIntPop2: containers.ArrayBlockingQueue<int>) => void>(ArrayBlockingQueuePop, queueIntPop2);
        let i = queueIntPop2.pop();
        p1.Await();
        arktest.assertEQ(i, 10);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt003", () =>
    {
        let p1 = launch<void, () => void>(queueIntPopNdata);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPopN.push(i);
        }
        p1.Await();
        arktest.assertEQ(queueIntPopN.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt004", () =>
    {
        let queueIntPop: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        arktest.assertEQ(queueIntPop.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000).toInt();
            queueIntPop.push(el);
        }
        arktest.assertEQ(queueIntPop.size, elementsNum);
        let j: int = 1;
        for (let i = 0; i < elementsNum; i++) {
            queueIntPop.pop();
            arktest.assertEQ(queueIntPop.size, elementsNum - j);
            j++;
        }
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt005", () =>
    {
        while (!queueIntPopN.isEmpty()) {
            queueIntPopN.pop();
        }
        let jobs = new Array<Job<void>>(100);
        for (let i: int = 0; i < elementsNum; ++i) {
           jobs[i] = launch<void, () => void>(queueIntPopdata); 
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPopN.push(i);
        }
        for (let i: int = 0; i < 100; i++) {
            jobs[i].Await();
        }
        arktest.assertEQ(queueIntPopN.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt006", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        let p1 = launch<void, () => void>(queueIntFrontF1data);
        let p2 = launch<void, () => void>(queueIntPopFdata);
        p1.Await();
        p2.Await();
        arktest.assertEQ(queueInt.size, elementsNum - 1);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function pusherInt() {
    for (let i = 0; i < elementsNum; i++) {
        let el = (random() * 10000).toInt();
        testArrayInt[i] = el;
        queueInt.push(el);
    }
}

function pusherObj() {
    for (let i = 0; i < elementsNum; i++) {
        let el = new Object();
        testArrayObj[i] = el;
        queueObj.push(el);
    }
}

function pusherStr() {
    for (let i = 0; i < elementsNum; i++) {
        if (i == 0) {
            let el = String("abacaba");
            testArrayStr[i] = el;
            queueStr.push(el);
        } else {
            let el = testArrayStr[i - 1] + "aba";
            testArrayStr[i] = el;
            queueStr.push(el);
        }
    }
}

function popperInt(): Array<int> {
    let arr = new Array<int>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueInt.getFirst();
        arr[i] = queueInt.pop();
        arktest.assertEQ(el, arr[i]);
    }
    return arr;
}

function popperObj(): Array<Object> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueObj.getFirst();
        arr[i] = queueObj.pop();
        arktest.assertEQ(el, arr[i]);
    }
    return arr;
}

function popperStr(): Array<String> {
    let arr = new Array<String>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arr[i] = queueStr.pop();
    }
    return arr;
}

function queueIntPopdata() {
    queueIntPopN.pop();
}

function queueIntPopFdata() {
    queueInt.pop();
}

function ArrayBlockingQueuePop (queueIntPop2: containers.ArrayBlockingQueue<int>) {
    queueIntPop2.push(10);
}

function queueIntPopNdata () {
    for (let i: int = 0; i < elementsNum; ++i) {
        queueIntPopN.pop();
    }
}

function queueIntFrontF1data() {
    queueInt.getFirst();
}
