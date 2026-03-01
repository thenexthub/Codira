/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import {Queue} from "@ohos.util.Queue";
import { BusinessError } from "@ohos.base";

function main() {
    const suite = new arktest.ArkTestsuite("Queue API tests");

    suite.addTest("Queue forEach with no parameter function", testQueueForEachNoParam);
    suite.addTest("Queue forEach with function with value", testQueueForEachWithValue);
    suite.addTest("Queue forEach with function with value and index", testQueueForEachWithValueAndIndex);
    suite.addTest("Queue forEach with function with value, index and tempQueue", testQueueForEachWithAllParams);
    suite.addTest("Queue forEach with change the elements", testQueueForEachChangeElements);
    suite.addTest("Queue get first element from an empty queue", testQueueGetFirstEmpty);
    suite.addTest("Queue get first element", testQueueGetFirst);
    suite.addTest("Queue iterate with empty queue", testQueueIteratorEmpty);
    suite.addTest("Queue iterate", testQueueIterator);
    suite.addTest("Queue interface composition testing", testQueueInterfaceComposition);

    return suite.run();
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber3: int = 3;
    static readonly testNumber6: int = 6;
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testQueueForEachNoParam() {
    let queue = new Queue<double>();
    initializeQueue(queue, TestData.forNumber);
    let forEachCount: int = 0;
    queue.forEach(() => {
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match queue length");
}

function testQueueForEachWithValue() {
    let queue = new Queue<double>();
    initializeQueue(queue, TestData.forNumber);
    let forEachCount: int = 0;
    let forEachFlag: double = 0;
    queue.forEach((value: double) => {
        if (value != queue.pop()) {
            forEachFlag = TestData.testNumber1.toDouble();
        }
        forEachCount++;
    });
    arktest.assertEQ(forEachFlag, 0, "ForEach should iterate and modify elements correctly");
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match queue length");
}

function testQueueForEachWithValueAndIndex() {
    let queue = new Queue<double>();
    initializeQueue(queue, TestData.forNumber);
    let forEachCount: int = 0;
    let forEachFlag: double = 0;
    queue.forEach((value: double, index: int) => {
        if (value != queue.pop() || index != forEachCount) {
            forEachFlag = TestData.testNumber1.toDouble();
        }
        forEachCount++;
    });
    arktest.assertEQ(forEachFlag, 0, "ForEach should iterate and modify elements correctly");
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match queue length");
}

function testQueueForEachWithAllParams() {
    let queue = new Queue<double>();
    initializeQueue(queue, TestData.forNumber);
    let forEachCount: int = 0;
    let forEachFlag: double = 0;
    queue.forEach((value: double, index: int, tempQueue: Queue<double>) => {
        arktest.assertEQ(queue, tempQueue, "TempQueue should be this");
        let elem1 = tempQueue.getFirst();
        let elem2 = queue.pop();
        if (value != elem2 || index != forEachCount || elem1 != elem2) {
            forEachFlag = TestData.testNumber1.toDouble();
        }
        forEachCount++;
    });
    arktest.assertEQ(forEachFlag, 0, "ForEach should iterate and modify elements correctly");
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match queue length");
}

function testQueueForEachChangeElements() {
    let queue = new Queue<double>();
    initializeQueue(queue, TestData.forNumber);

    queue.forEach((value: double) => {
        value = 0;
    });
    for (let i: int = 0; i < TestData.forNumber.length; i++) {
        arktest.assertEQ(queue.pop(), TestData.forNumber[i], "ForEach should keep all elements the same");
    }
}

function testQueueGetFirstEmpty() {
    let queue = new Queue<double>();
    arktest.assertEQ(queue.length, 0, "Initial queue length should be 0");
    try {
        let first = queue.getFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testQueueGetFirst() {
    let queue = new Queue<double>();
    queue.add(TestData.testNumber1);
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        queue.add(TestData.testNumber3);
    }
    arktest.assertEQ(queue.length, TestData.testNumber6, "Queue length should be 6 after inserting six elements");
    arktest.assertEQ(queue.getFirst(), TestData.testNumber1, "Get first from non-empty queue should return first element");
}

function testQueueIteratorEmpty() {
    let queue = new Queue<double>();
    let elem: double = 0;
    for (let item of queue) {
        elem = TestData.testNumber1;
    }
    let iter = queue.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    arktest.assertEQ(elem, 0, "Iterator over empty queue should not iterate any elements");
}

function testQueueIterator() {
    let queue = new Queue<double>();
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        queue.add(i);
    }
    let elem1: int = TestData.testNumber1;
    for (let item of queue) {
        arktest.assertEQ(item, elem1++, "Iterator should iterate elements in correct order");
    }
    arktest.assertEQ(elem1, TestData.testNumber6, "Iterator should iterate all elements correctly");
    let iter = queue.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    let elem2: int = TestData.testNumber1;
    while (!temp.done) {
        arktest.assertEQ(temp.value, elem2++, "Iterator should iterate elements in correct order");
        temp = iter.next();
    }
    arktest.assertEQ(elem2, TestData.testNumber6, "Iterator should iterate all elements correctly");
}

function testQueueInterfaceComposition() {
    let queue = new Queue<double>();
    queue.add(TestData.testNumber3);
    arktest.assertEQ(queue.length, TestData.testNumber1, "Queue length should be 1 after inserting one element");

    initializeQueue(queue, TestData.forNumber);

    arktest.assertEQ(queue.getFirst(), queue.pop(), "Pop from Queue should return first element");

    let forEachCount: int = 0;
    queue.forEach(() => {
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match Queue length");
}

function initializeQueue<T>(queue: Queue<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        queue.add(sourceArray[i]);
    }
}
