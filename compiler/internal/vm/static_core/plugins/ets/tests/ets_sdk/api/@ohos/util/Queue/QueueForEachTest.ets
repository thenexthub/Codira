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

import { Queue } from "@ohos.util.Queue";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Queue API tests");

    suite.addTest("Testing Queue forEach iteration with array", testQueueForEachWithArray);
    suite.addTest("Testing Queue [Symbol.iterator] with array (iterator usage)", testQueueSymbolIteratorUsage);
    suite.addTest("Testing Queue length with pop", testQueueLengthWithPop);
    suite.addTest("Testing Queue forEach iteration with array (length validation)", testQueueForEachLengthValidation);
    suite.addTest("Testing Queue forEach iteration with array (value validation)", testQueueForEachValueValidation);
    suite.addTest("Testing Queue forEach iteration with array (value and index validation)",
        testQueueForEachValueAndIndexValidation);
    suite.addTest("Testing Queue forEach iteration with single element", testQueueForEachSingleElement);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: QUEUE_1000
function testQueueForEachWithArray() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let expectedValues = [a, b, c];
    let count = 0;
    queue.forEach((value, index): void => {
        arktest.assertEQ(value, expectedValues[count]);
        arktest.assertEQ(index, count);
        count++;
    });
    arktest.assertEQ(count, queue.length);
}

// Test cases ported from ArkTS 1.0: QUEUE_[SYMBOL.ITERATOR]_1100
function testQueueSymbolIteratorUsage() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let expectedValues = [a, b, c];
    let iter = queue.$_iterator();
    let temp = iter.next();
    for (let i: int = 0; i < 3; i++) {
        arktest.assertFalse(temp.done, `Iteration ${i + 1}: done should be false`);
        arktest.assertEQ(temp.value, expectedValues[i]);
        temp = iter.next();
    }
    arktest.assertTrue(temp.done, `After 3 iterations, done should be true`);
    arktest.assertEQ(queue.length, 3);
}

// Test cases ported from ArkTS 1.0: QUEUE_[SYMBOL.ITERATOR]_1200
function testQueueLengthWithPop() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let expectedValues = [a, b, c];
    let idx = 0;
    while (queue.length) {
        let item = queue.pop();
        arktest.assertEQ(item, expectedValues[idx]);
        idx++;
        arktest.assertEQ(queue.length, 3 - idx);
    }
}

// Test cases ported from ArkTS 1.0: QUEUE_1300
function testQueueForEachLengthValidation() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let count = 0;
    queue.forEach((): void => {
        arktest.assertEQ(queue.length, 3);
        count++;
    });
    arktest.assertEQ(count, queue.length);
}

// Test cases ported from ArkTS 1.0: QUEUE_1400
function testQueueForEachValueValidation() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    queue.add(a);
    let count = 0;
    queue.forEach((value): void => {
        arktest.assertEQ(queue.getFirst(), value);
        count++;
    });
    arktest.assertEQ(count, queue.length);
}

// Test cases ported from ArkTS 1.0: QUEUE_1500
function testQueueForEachValueAndIndexValidation() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    queue.add(a);
    let count = 0;
    queue.forEach((value, index, queue): void => {
        arktest.assertEQ(queue.getFirst(), value);
        count++;
    });
    arktest.assertEQ(count, queue.length);

}

// Test cases ported from ArkTS 1.0: QUEUE_1600
function testQueueForEachSingleElement() {
    let queue = new Queue<double | string | object>();
    queue.add(1);
    let count = 0;
    queue.forEach((value, index, queue): void => {
        arktest.assertEQ(queue.getFirst(), value);
        count++;
    });
    arktest.assertEQ(queue.getFirst(), 1);
    arktest.assertEQ(count, queue.length);
}
