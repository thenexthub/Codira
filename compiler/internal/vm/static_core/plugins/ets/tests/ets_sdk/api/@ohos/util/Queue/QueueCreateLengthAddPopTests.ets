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

    suite.addTest("Queue create an instance that stores single-type data", testQueueCreateSingleType);
    suite.addTest("Queue create an instance that stores multiple types of data", testQueueCreateMultipleTypes);
    suite.addTest("Queue length", testQueueLengthBasic);
    suite.addTest("Queue length with mixed types", testQueueLengthMixedTypes);
    suite.addTest("Queue length with large double of elements", testQueueLengthLargeNumber);
    suite.addTest("Queue add", testQueueAddElements);
    suite.addTest("Queue pop first element from an empty queue", testQueuePopEmpty);
    suite.addTest("Queue pop first element", testQueuePopFirstElement);

    return suite.run();
}

class Temp {
    name: string = "";
    age: double = 0;
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber1w: int = 10000;
    static readonly testObject1: Temp = { name: "Dylon", age: 13 };
    static readonly testString1: string = "string";
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testQueueCreateSingleType() {
    let queue: Queue<double>;
    arktest.expectNoThrow(() => {queue = new Queue<double>()});
}

function testQueueCreateMultipleTypes() {
    let queue: Queue<double | string | object>;
    arktest.expectNoThrow(() => {queue = new Queue<double | string | object>()});
}

function testQueueLengthBasic() {
    let queue = new Queue<double>();
    arktest.assertEQ(queue.length, 0, "Initial queue length should be 0");
    queue.add(TestData.testNumber3);
    queue.add(TestData.testNumber1);
    queue.add(TestData.testNumber2);
    arktest.assertEQ(queue.length, TestData.testNumber3, "Queue length should be 3 after adding three elements");
    queue.pop();
    queue.pop();
    arktest.assertEQ(queue.length, TestData.testNumber1, "Queue length should be 1 after popping two elements");
}

function testQueueLengthMixedTypes() {
    let queue = new Queue<double | string | boolean>();
    arktest.assertEQ(queue.length, 0, "Initial queue length should be 0");
    queue.add(TestData.testNumber3);
    queue.add(TestData.testString1);
    queue.add(true);
    arktest.assertEQ(queue.length, TestData.testNumber3, "Queue length should be 3 after adding three elements of mixed types");
    queue.pop();
    queue.pop();
    arktest.assertEQ(queue.length, TestData.testNumber1, "Queue length should be 1 after popping two elements");
}

function testQueueLengthLargeNumber() {
    let queue = new Queue<double>();
    for (let i: int = 0; i < TestData.testNumber1w; i++) {
        queue.add(TestData.testNumber1);
    }
    arktest.assertEQ(queue.length, TestData.testNumber1w, "Queue length should be 10000 after adding elements");
}

function testQueueAddElements() {
    let queue = new Queue<double>();
    initializeQueue(queue, TestData.forNumber);
    let countForEach = 0;
    queue.forEach((value: double) => {
        arktest.assertEQ(value, TestData.forNumber[countForEach++], "Element should match");
    });
    arktest.assertEQ(countForEach, queue.length, "Queue length same as foreach");
    arktest.assertEQ(queue.length, TestData.forNumber.length, "Queue length same as TestData.forNumber");
}

function testQueuePopEmpty() {
    let queue = new Queue<double>();
    arktest.assertEQ(queue.length, 0, "Initial queue length should be 0");
    try {
        let result = queue.pop();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testQueuePopFirstElement() {
    let queue = new Queue<double>();
    queue.add(TestData.testNumber1);
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        queue.add(TestData.testNumber3);
    }
    arktest.assertEQ(queue.length, TestData.testNumber6, "Queue length should be 6 after inserting six elements");
    arktest.assertEQ(queue.pop(), TestData.testNumber1, "Pop from non-empty queue should return first element");
    arktest.assertEQ(queue.length, TestData.testNumber5, "Pop from non-empty queue should update length");
    let countForEach = 0;
    queue.forEach((value: double) => {
        arktest.assertEQ(value, TestData.testNumber3, "Element should match");
        countForEach++;
    });
    arktest.assertEQ(countForEach, queue.length, "Queue length same as foreach");
}

function initializeQueue<T>(queue: Queue<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        arktest.assertTrue(queue.add(sourceArray[i]), `Failed to add element at index ${i}`);
    }
}
