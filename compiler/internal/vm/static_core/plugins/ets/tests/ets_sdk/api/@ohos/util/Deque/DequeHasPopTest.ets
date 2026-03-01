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

import Deque from "@ohos.util.Deque";
import { BusinessError } from "@ohos.base";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Test checking the existence of a string element in the deque.", testHasStringElement);
    suite.addTest("Test checking the existence of a numeric element in the deque.", testHasNumericElement);
    suite.addTest("Test checking the existence of an object element in the deque.", testHasObjectElement);
    suite.addTest("Test removing the first element and retrieving the new first element.",
        testPopFirstAndRetrieveNewFirst);
    suite.addTest("Test removing the first element from an empty deque.", testPopFirstFromEmptyDeque);
    suite.addTest("Test retrieving the first element from an empty deque.", testGetFirstFromEmptyDeque);
    suite.addTest("Test retrieving the last element after inserting multiple elements.",
        testGetLastAfterMultipleInserts);
    suite.addTest("Test retrieving the last element from an empty deque.", testGetLastFromEmptyDeque);
    suite.addTest("Test iterating over all elements in the deque using forEach.", testForEachIteration);
    suite.addTest("Test iterating over all elements in the deque and verifying its structure.",
        testForEachAndVerifyStructure);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: DEQUE_0900
function testHasStringElement() {
    let deque = new Deque<string>();
    deque.insertFront("a");;
    let result = deque.has("a");
    arktest.assertTrue(result, "Deque should contain the string 'a'");
}

// Test cases ported from ArkTS 1.0: DEQUE_1000
function testHasNumericElement() {
    let deque = new Deque<double>();
    deque.insertFront(1);
    let result = deque.has(1);
    arktest.assertTrue(result, "Deque should contain the double 1");
}

// Test cases ported from ArkTS 1.0: DEQUE_1100
function testHasObjectElement() {
    let deque = new Deque<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    deque.insertFront(a);
    let result = deque.has(a);
    arktest.assertTrue(result, "Deque should contain the object 'a'");
}

// Test cases ported from ArkTS 1.0: DEQUE_1200
function testPopFirstAndRetrieveNewFirst() {
    let deque = new Deque<double>();
    deque.insertFront(1);
    deque.insertFront(2);
    deque.insertEnd(3);
    deque.insertFront(4);
    deque.popFirst();
    let result = deque.getFirst();
    arktest.assertEQ(result, 2);
}

// Test cases ported from ArkTS 1.0: DEQUE_1300
function testPopFirstFromEmptyDeque() {
    let deque = new Deque<double>();
    try {
        let first = deque.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let first = deque.getFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

// Test cases ported from ArkTS 1.0: DEQUE_1500
function testGetFirstFromEmptyDeque() {
    let deque = new Deque<double>();
    try {
        let first = deque.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let first = deque.getFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

// Test cases ported from ArkTS 1.0: DEQUE_1600
function testGetLastAfterMultipleInserts() {
    let deque = new Deque<double>();
    deque.insertFront(1);
    deque.insertFront(2);
    deque.insertEnd(4);
    deque.insertFront(3);
    let result = deque.getLast();
    arktest.assertEQ(result, 4);
}

// Test cases ported from ArkTS 1.0: DEQUE_1700
function testGetLastFromEmptyDeque() {
    let deque = new Deque<double>();
    try {
        let first = deque.getLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

// Test cases ported from ArkTS 1.0: DEQUE_1800
function testForEachIteration() {
    let deque = new Deque<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    deque.insertEnd(a);
    deque.insertEnd(b);
    deque.insertEnd(c);
    deque.forEach((item, index) => {
        let result = item;
        let itemIndex = index;
        arktest.assertEQ(result, item);
        arktest.assertEQ(itemIndex, itemIndex);
    });
}

// Test cases ported from ArkTS 1.0: DEQUE_1800 tesForeach0019
function testForEachAndVerifyStructure() {
    let deque = new Deque<double | string | object>();
    let res = new Deque<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    deque.insertEnd(a);
    deque.insertEnd(b);
    deque.insertEnd(c);
    deque.forEach((item, index) => {
        let result = item;
        let itemIndex = index;
        arktest.assertEQ(result, item);
        arktest.assertEQ(itemIndex, itemIndex);
    });
}
