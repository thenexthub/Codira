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

function main(): int {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Deque has", testDequeHasElement);
    suite.addTest("Deque has with inexistent element", testDequeHasInexistentElement);
    suite.addTest("Deque has with string element", testDequeHasStringElement);
    suite.addTest("Deque has with object element", testDequeHasObjectElement);
    suite.addTest("Deque pop first element from an empty deque", testDequePopFirstFromEmpty);
    suite.addTest("Deque pop first element", testDequePopFirstElement);
    suite.addTest("Deque pop last element from an empty deque", testDequePopLastFromEmpty);
    suite.addTest("Deque pop last element", testDequePopLastElement);
    suite.addTest("Deque forEach with no parameter function", testDequeForEachNoParam);
    suite.addTest("Deque forEach with function with value", testDequeForEachWithValue);
    suite.addTest("Deque forEach with function with value and index", testDequeForEachWithValueAndIndex);
    suite.addTest("Deque forEach with function with value, index and tempArrayList",
        testDequeForEachWithValueIndexAndTempArrayList);
    suite.addTest("Deque forEach with change the elements", testDequeForEachChangeElements);
    suite.addTest("Deque JSON stringify", testDequeJSONStringify);

    return suite.run();
}

class Temp {
    name: string = "";
    age: double = 0;
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber3: int = 3;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber31: double = 31;
    static readonly testObject1: Temp = { name: "Dylon", age: 13 };
    static readonly testString1: string = "string";
    static readonly testString2: string = "a";
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
    static readonly result1 = `{"0":{"name":"Dylon","age":13},"1":"a","2":1}`; 
}

function testDequeHasElement() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let result: boolean = deque.has(TestData.testNumber3);
    arktest.assertTrue(result, "Deque should contain the element");
}

function testDequeHasInexistentElement() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let result: boolean = deque.has(TestData.testNumber31);
    arktest.assertFalse(result, "Deque should not contain the element");
}

function testDequeHasStringElement() {
    let deque = new Deque<string>();
    deque.insertFront(TestData.testString1);
    arktest.assertTrue(deque.has(TestData.testString1), "Deque should contain the element")
}

function testDequeHasObjectElement() {
    let deque = new Deque<object>();
    deque.insertFront(TestData.testObject1);
    arktest.assertTrue(deque.has(TestData.testObject1), "Deque should contain the element")
}

function testDequePopFirstFromEmpty() {
    let deque = new Deque<double>();
    arktest.assertEQ(deque.length, 0, "Initial deque length should be 0");
    try {
        let result = deque.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testDequePopFirstElement() {
    let deque = new Deque<double>();
    deque.insertEnd(TestData.testNumber1);
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        deque.insertEnd(TestData.testNumber3);
    }
    arktest.assertEQ(deque.length, TestData.testNumber6, "Deque length should be 6 after inserting six elements");
    arktest.assertEQ(deque.popFirst(), TestData.testNumber1, "Pop first from non-empty deque should return first element");
    arktest.assertEQ(deque.length, TestData.testNumber5, "Pop first from non-empty deque should update length");
    let countForEach = 0;
    deque.forEach((value: double) => {
        arktest.assertEQ(value, TestData.testNumber3, "Element should match");
        countForEach++;
    });
    arktest.assertEQ(countForEach, deque.length, "Deque length same as foreach");
}

function testDequePopLastFromEmpty() {
    let deque = new Deque<double>();
    arktest.assertEQ(deque.length, 0, "Initial deque length should be 0");
    try {
        let result = deque.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testDequePopLastElement() {
    let deque = new Deque<double>();
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        deque.insertEnd(TestData.testNumber3);
    }
    deque.insertEnd(TestData.testNumber1);
    arktest.assertEQ(deque.length, TestData.testNumber6, "Deque length should be 6 after inserting six elements");
    arktest.assertEQ(deque.popLast(), TestData.testNumber1, "Pop last from non-empty deque should return last element");
    arktest.assertEQ(deque.length, TestData.testNumber5, "Pop last from non-empty deque should update length");
    let countForEach = 0;
    deque.forEach((value: double) => {
        arktest.assertEQ(value, TestData.testNumber3, "Element should match");
        countForEach++;
    });
    arktest.assertEQ(countForEach, deque.length, "Deque length same as foreach");
}

function testDequeForEachNoParam() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let forEachCount: int = 0;
    deque.forEach(() => {
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, deque.length, "ForEach count should match deque length");
}

function testDequeForEachWithValue() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let forEachCount: int = 0;
    let forEachFlag: double = 0;
    deque.forEach((value: double) => {
        if (value != deque.popFirst()) {
            forEachFlag = TestData.testNumber1.toDouble();
        }
        forEachCount++;
    });
    arktest.assertEQ(forEachFlag, 0, "ForEach should iterate and modify elements correctly");
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match deque length");
}

function testDequeForEachWithValueAndIndex() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let forEachCount: int = 0;
    let forEachFlag: double = 0;
    deque.forEach((value: double, index: int) => {
        if (value != deque.popFirst() || index != forEachCount) {
            forEachFlag = TestData.testNumber1.toDouble();
        }
        forEachCount++;
    });
    arktest.assertEQ(forEachFlag, 0, "ForEach should iterate and modify elements correctly");
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match deque length");
}

function testDequeForEachWithValueIndexAndTempArrayList() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);
    let forEachCount: int = 0;
    let forEachFlag: double = 0;
    deque.forEach((value: double, index: int, tempDeque: Deque<double>) => {
        arktest.assertEQ(deque, tempDeque, "TempDeque should be this");
        let elem1 = tempDeque.getFirst();
        let elem2 = deque.popFirst();
        if (value != elem2 || index != forEachCount || elem1 != elem2) {
            forEachFlag = TestData.testNumber1.toDouble();
        }
        forEachCount++;
    });
    arktest.assertEQ(forEachFlag, 0, "ForEach should iterate and modify elements correctly");
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match deque length");
}

function testDequeForEachChangeElements() {
    let deque = new Deque<double>();
    initializeDeque(deque, TestData.forNumber);

    deque.forEach((value: double) => {
        value = 0;
    });
    for (let i: int = 0; i < TestData.forNumber.length; i++) {
        arktest.assertEQ(deque.popFirst(), TestData.forNumber[i], "ForEach should keep all elements the same");
    }
}

function testDequeJSONStringify() {
    let deque: Deque<double | string | object> = new Deque<double | string | object>();
    deque.insertEnd(TestData.testObject1);
    deque.insertEnd(TestData.testString2);
    deque.insertEnd(TestData.testNumber1);
    let jsonResult = JSON.stringify(deque);
    arktest.assertEQ(jsonResult, TestData.result1, `jsonResult should be ${TestData.result1}`);
}

function initializeDeque<T>(deque: Deque<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        deque.insertEnd(sourceArray[i]);
    }
}
