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

const elementsNum: int = 100;

let testArrayInt: Array<int> = new Array<int>(elementsNum);
let testArrayObj: Array<Object> = new Array<Object>(elementsNum);
let testArrayStr: Array<String> = new Array<String>(elementsNum);
let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
let queueObj: containers.ArrayBlockingQueue<Object> = new containers.ArrayBlockingQueue<Object>(200);
let queueStr: containers.ArrayBlockingQueue<String> = new containers.ArrayBlockingQueue<String>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueuePushTest");

    ArrayBlockingQueueTestsuite.addTest("SimpleQueuePushPopFrontBackSizeEmptyTest", () =>
    {
        let arr: Array<int> = new Array<int>(elementsNum);
        for (let i: int = 0; i < arr.length; ++i) {
            let el = (random() * 10000).toInt();
            arr[i] = el;
            queueInt.push(el);
            arktest.assertEQ(el, queueInt.getEnd());
        }
        let ind: int = 0;
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
            arktest.assertEQ(el, arr[ind++]);
        }
        arktest.assertEQ(ind, arr.length);
        arktest.assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("ConcurrentPushPopTestInt", () =>
    {
        let p1 = launch<void, () => void>(pusherInt);
        let arrayToCheck = popperInt();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayInt[i], arrayToCheck[i]);
        }
        p1.Await();
        arktest.assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("ConcurrentPushPopTestObj", () =>
    {
        let p1 = launch<void, () => void>(pusherObj);
        let arrayToCheck = popperObj();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayObj[i], arrayToCheck[i]);
        }
        p1.Await();
        arktest.assertEQ(queueObj.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("ConcurrentPushPopTestStr", () =>
    {
        let p1 = launch<void, () => void>(pusherStr);
        let arrayToCheck = popperStr();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        p1.Await();
        arktest.assertEQ(queueStr.size, 0);
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
        arr[i] = queueInt.pop();
    }
    return arr;
}

function popperObj(): Array<Object> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arr[i] = queueObj.pop();
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

function queueIntFrontdata() {
    queueInt.getFirst();
}
