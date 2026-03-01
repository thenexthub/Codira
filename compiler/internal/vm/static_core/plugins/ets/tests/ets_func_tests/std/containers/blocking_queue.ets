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
let queueInt: containers.BlockingQueue<int> = new containers.ArrayBlockingQueue<int>(100);
let queueObj: containers.BlockingQueue<Object> = new containers.LinkedBlockingQueue<Object>(100);
let queueStr: containers.BlockingQueue<String> = new containers.LinkedBlockingQueue<String>(100);

function wait(): int {
    let i = 1
    for (let j = 0; j < 3000; j++) {
        i += j
    }
    return i
}

function pusherInt() {
    for (let i = 0; i < elementsNum; i++) {
        wait();
        let el = (random() * 10000).toInt();
        testArrayInt[i] = el;
        queueInt.push(el);
    }
}

function pusherObj() {
    for (let i = 0; i < elementsNum; i++) {
        wait();
        let el = new Object();
        testArrayObj[i] = el;
        queueObj.push(el);
    }
}

function pusherStr() {
    for (let i = 0; i < elementsNum; i++) {
        wait();
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
        if (el === null) {
            continue;
        }
        arr[i] = queueInt.pop();
        arktest.assertEQ(el, arr[i]);
    }
    return arr;
}

function popperObj(): Array<Object> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueObj.getFirst();
        if (el === null) {
            continue;
        }
        arr[i] = queueObj.pop();
        arktest.assertEQ(el, arr[i]);
    }
    return arr;
}

function popperStr(): Array<String> {
    let arr = new Array<String>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueStr.getFirst();
        if (el === null) {
            continue;
        }
        arr[i] = queueStr.pop();
        arktest.assertEQ(el, arr[i]);
    }
    return arr;
}

function concurrentPushPopTestInt() {
    let p1 = launch<void, () => void>(pusherInt);
    p1.Await();
    let arrayToCheck = popperInt();
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ(testArrayInt[i], arrayToCheck[i]);
    }
    arktest.assertEQ(queueInt.size, 0);
}

function concurrentPushPopTestObj() {
    let p1 = launch<void, () => void>(pusherObj);
    p1.Await();
    let arrayToCheck = popperObj();
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ(testArrayObj[i], arrayToCheck[i]);
    }
    arktest.assertEQ(queueObj.size, 0);
}

function concurrentPushPopTestStr() {
    let p1 = launch<void, () => void>(pusherStr);
    p1.Await();
    let arrayToCheck = popperStr();
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
    }
    arktest.assertEQ(queueStr.size, 0);
}

function simpleQueuePushPopFrontBackSizeEmptyTest() {
    let arr: Array<int> = new Array<int>(elementsNum);
    for (let i: int = 0; i < arr.length; ++i) {
        let el = (random() * 10000).toInt();
        arr[i] = el;
        queueInt.push(el);
        arktest.assertEQ(el, (queueInt as containers.ArrayBlockingQueue<int>).getEnd());
    }
    let ind: int = 0;
    while (!queueInt.isEmpty()) {
        let el = queueInt.getFirst();
        arktest.assertEQ(el, queueInt.pop());
        arktest.assertEQ(el, arr[ind++]);
    }
    arktest.assertEQ(ind, arr.length);
    arktest.assertEQ(queueInt.size, 0);
}

function main() {
    simpleQueuePushPopFrontBackSizeEmptyTest();
    concurrentPushPopTestInt(); // Concurrent test for Int type
    concurrentPushPopTestObj(); // Concurrent test for Object type
    concurrentPushPopTestStr(); // Concurrent test for String type
}
