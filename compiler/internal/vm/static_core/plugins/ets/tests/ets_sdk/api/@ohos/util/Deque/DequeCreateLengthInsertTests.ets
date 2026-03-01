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

function main(): int {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Deque create an instance that stores single-type data", testCreateSingleTypeInstance);
    suite.addTest("Deque create an instance that stores multiple types of data", testCreateMultiTypeInstance);
    suite.addTest("Deque length", testDequeLength);
    suite.addTest("Deque length with mixed types", testDequeLengthMixedTypes);
    suite.addTest("Deque length with large double of elements", testDequeLengthLargeNumber);
    suite.addTest("Deque insert elements at the front of the deque", testInsertElementsFront);
    suite.addTest("Deque insert object at the front of the deque", testInsertObjectFront);
    suite.addTest("Deque insert every type elements at the front of the deque", testInsertAllTypesFront);
    suite.addTest("Deque insert elements at the end of the deque", testInsertElementsEnd);
    suite.addTest("Deque insert object at the end of the deque", testInsertObjectEnd);

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
    static readonly testInt29: int = 29;
    static readonly testNumber1w: int = 10000;
    static readonly testObject1: Temp = { name: "Dylon", age: 13 };
    static readonly testObject2: Temp = { name: "Joe", age: 14 };
    static readonly testObject3: Temp = { name: "Lucy", age: 15 };
    static readonly testString1: string = "string";
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testCreateSingleTypeInstance() {
    let deque: Deque<double>;
    arktest.expectNoThrow(() => {deque = new Deque<double>()});
}

function testCreateMultiTypeInstance() {
    let deque: Deque<double | string>;
    arktest.expectNoThrow(() => {deque = new Deque<double | string>()});
}

function testDequeLength() {
    let deque = new Deque<double>();
    arktest.assertEQ(deque.length, 0, "Initial deque length should be 0");
    deque.insertEnd(TestData.testNumber3);
    deque.insertEnd(TestData.testNumber1);
    deque.insertEnd(TestData.testNumber2);
    arktest.assertEQ(deque.length, TestData.testNumber3, "Deque length should be 3 after inserting three elements at the end");
    deque.popLast();
    deque.popLast();
    arktest.assertEQ(deque.length, TestData.testNumber1, "Deque length should be 1 after popping two elements from the end");
}

function testDequeLengthMixedTypes() {
    let deque = new Deque<double | string | boolean>();
    arktest.assertEQ(deque.length, 0, "Initial deque length should be 0");
    deque.insertEnd(TestData.testNumber3);
    deque.insertEnd(TestData.testString1);
    deque.insertEnd(true);
    arktest.assertEQ(deque.length, TestData.testNumber3,
        "Deque length should be 3 after inserting three elements of mixed types at the end");
    deque.popLast();
    deque.popLast();
    arktest.assertEQ(deque.length, TestData.testNumber1, "Deque length should be 1 after popping two elements from the end");
}

function testDequeLengthLargeNumber() {
    let deque = new Deque<double>();
    for (let i: int = 0; i < TestData.testNumber1w; i++) {
        deque.insertEnd(TestData.testNumber1);
    }
    arktest.assertEQ(deque.length, TestData.testNumber1w, "Deque length should be 10000 after adding elements");
}

function testInsertElementsFront() {
    let deque = new Deque<double>();
    for (let i: int = 0; i < TestData.forNumber.length; i++) {
        deque.insertFront(TestData.forNumber[i]);
    }
    let countForEach = 0;
    deque.forEach((value: double) => {
        arktest.assertEQ(value, TestData.forNumber[TestData.testInt29 - countForEach++], "Element should match");
    });
    arktest.assertEQ(countForEach, deque.length, "Deque length same as foreach");
    arktest.assertEQ(deque.length, TestData.forNumber.length, "Deque length same as TestData.forNumber");
}

function testInsertObjectFront() {
    let deque = new Deque<object>();
    deque.insertFront(TestData.testObject1);
    arktest.assertEQ(deque.getFirst(), TestData.testObject1, "Get first from non-empty deque should return first element")

    deque.insertFront(TestData.testObject2);
    arktest.assertEQ(deque.getFirst(), TestData.testObject2, "Get first from non-empty deque should return first element")

    deque.insertFront(TestData.testObject3);
    arktest.assertEQ(deque.getFirst(), TestData.testObject3, "Get first from non-empty deque should return first element")
}

function testInsertAllTypesFront() {
    let deque = new Deque<double | string | object>();
    deque.insertFront(TestData.testObject1);
    arktest.assertEQ(deque.getFirst(), TestData.testObject1, "Get first from non-empty deque should return first element")

    deque.insertFront(TestData.testString1);
    arktest.assertEQ(deque.getFirst(), TestData.testString1, "Get first from non-empty deque should return first element")

    deque.insertFront(TestData.testNumber1);
    arktest.assertEQ(deque.getFirst(), TestData.testNumber1, "Get first from non-empty deque should return first element")
}

function testInsertElementsEnd() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let countForEach = 0;
    deque.forEach((value: double) => {
        arktest.assertEQ(value, TestData.forNumber[countForEach++], "Element should match");
    });
    arktest.assertEQ(countForEach, deque.length, "Deque length same as foreach");
    arktest.assertEQ(deque.length, TestData.forNumber.length, "Deque length same as TestData.forNumber");
}

function testInsertObjectEnd() {
    let deque = new Deque<object>();
    deque.insertEnd(TestData.testObject1);
    arktest.assertEQ(deque.getLast(), TestData.testObject1, "Get last from non-empty deque should return last element")

    deque.insertEnd(TestData.testObject2);
    arktest.assertEQ(deque.getLast(), TestData.testObject2, "Get last from non-empty deque should return last element")

    deque.insertEnd(TestData.testObject3);
    arktest.assertEQ(deque.getLast(), TestData.testObject3, "Get last from non-empty deque should return last element")
}

function initializeDeque<T>(deque: Deque<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        deque.insertEnd(sourceArray[i]);
    }
}
