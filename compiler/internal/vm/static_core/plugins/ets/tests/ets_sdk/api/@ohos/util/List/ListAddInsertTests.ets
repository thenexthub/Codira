/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

import {List} from '@ohos.util.List';
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

class Temp1 {
    age: double = 0
    name: string = ""
    constructor(age: double, name: string) {
        this.age = age;
        this.name = name;
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("List add elements of multiple types to the container", testAddMultipleTypes);
    suite.addTest("List add special characters to the container", testAddSpecialCharacters);
    suite.addTest("List insert an element within the container's length range", testInsertWithinRange);
    suite.addTest("List insert an element at the boundary of the container", testInsertAtBoundary);
    suite.addTest("List insert insert->addFirst->getIndexOf->removeByIndex->has",
        testInsertGetIndexOfRemoveByIndexHas);
    suite.addTest("List insert elements at index > length", testInsertIndexGreaterThanLength);
    suite.addTest("List insert elements at index < 0", testInsertIndexLessThanZero);
    suite.addTest("Testing List Add different dataType data", testListAddVariousDataTypes);
    suite.addTest("Testing List add special string", testListAddSpecialString);
    suite.addTest("Testing List JSON stringify", testListJsonStringify);

    return suite.run();
}

class Temp {
    age: double = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber7: int = 7;
    static readonly testNumber8: int = 8;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
}

function testAddMultipleTypes() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(TestData.testNumber1);
    list.add(TestData.testString1);
    list.add(true);
    list.add(null);
    list.add(TestData.testObject1);
    list.add(undefined);

    arktest.assertEQ(list.length, TestData.testNumber6, "List length should be 6");
    arktest.assertEQ(list.get(0), TestData.testNumber1, "Element at index 0 should match");
    arktest.assertEQ(list.get(TestData.testNumber1), TestData.testString1, "Element at index 1 should match");
    arktest.assertEQ(list.get(TestData.testNumber2), true, "Element at index 2 should match");
    arktest.assertEQ(list.get(TestData.testNumber3), null, "Element at index 3 should match");
    arktest.assertEQ(list.get(TestData.testNumber4), TestData.testObject1, "Element at index 4 should match");
    arktest.assertEQ(list.get(TestData.testNumber5), undefined, "Element at index 5 should match");
}

function testAddSpecialCharacters() {
    let list = new List<string>();
    list.add(TestData.testString2);
    arktest.assertEQ(list.get(0), TestData.testString2, "Add successful");
    arktest.assertEQ(list.length, TestData.testNumber1, "List length should be 1");
}

function testInsertWithinRange() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.insert(TestData.testNumber1, TestData.testNumber1);
    list.insert(TestData.testString1, TestData.testNumber2);
    list.insert(true, TestData.testNumber3);
    list.insert(null, TestData.testNumber4);
    list.insert(TestData.testObject1, TestData.testNumber5);
    list.insert(undefined, TestData.testNumber6);

    arktest.assertEQ(list.length, TestData.testNumber8, "List length should be 8");
    arktest.assertEQ(list.get(0), TestData.testNumber1, "Element at index 0 should match");
    arktest.assertEQ(list.get(TestData.testNumber1), TestData.testNumber1, "Element at index 6 should match");
    arktest.assertEQ(list.get(TestData.testNumber2), TestData.testString1, "Element at index 1 should match");
    arktest.assertEQ(list.get(TestData.testNumber3), true, "Element at index 2 should match");
    arktest.assertEQ(list.get(TestData.testNumber4), null, "Element at index 3 should match");
    arktest.assertEQ(list.get(TestData.testNumber5), TestData.testObject1, "Element at index 4 should match");
    arktest.assertEQ(list.get(TestData.testNumber6), undefined, "Element at index 5 should match");
    arktest.assertEQ(list.get(TestData.testNumber7), TestData.testNumber2, "Element at index 7 should match");
}

function testInsertAtBoundary() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    list.insert(0, TestData.arrayForNumber1.length);
    list.insert(0, 0);

    for (let i: int = 0; i < TestData.arrayForNumber1.length; i++) {
        arktest.assertEQ(list.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], "Element should match");
    }
    arktest.assertEQ(list.get(0), 0, "Element should be 0");
    arktest.assertEQ(list.get(list.length - TestData.testNumber1), 0, "Element should be 0");
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length + TestData.testNumber2, "List length should + 2");
}

function testInsertGetIndexOfRemoveByIndexHas() {
    let list = new List<double | string>();
    list.insert(TestData.testString2, 0);
    let index = list.getIndexOf(TestData.testString2);
    arktest.assertEQ(index, 0, "The index of the first element should be 0");
    list.removeByIndex(index);
    arktest.assertFalse(list.has(TestData.testString2), "The element should be removed");
}

function testInsertIndexGreaterThanLength() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 21. Received value is: 10000" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.insert(TestData.testNumber6, 10000)}, exceptionCheck);
}

function testInsertIndexLessThanZero() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 21. Received value is: -1" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.insert(TestData.testNumber6, -TestData.testNumber1)}, exceptionCheck);
}

function initializeList(list: List<double>, sourceArray: Array<double>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

// Test cases ported from ArkTS 1.0: LIST_ADD_0100
function testListAddVariousDataTypes() {
    let list = new List<double | string | boolean | undefined | null | Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.insert("insert", 0);
    list.add([0, 1, 2, 4]);
    list.removeByIndex(0);
    for (let i: int = 0; i < 4; i++) {
        arktest.assertEQ((list.get(6)! as Array<int>)[i], [0, 1, 2, 4][i]);
    }
    arktest.assertEQ(list.get(5), true);
    arktest.assertEQ(list.get(4), null);
    arktest.assertEQ(list.get(3), undefined);
    arktest.assertEQ(list.get(2), param);
    arktest.assertEQ(list.get(1), 1);
    arktest.assertEQ(list.get(0), "zhang san");
}

// Test cases ported from ArkTS 1.0: LIST_ADD_0200
function testListAddSpecialString() {
    let list = new List<string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    list.add(param);
    arktest.assertEQ(list.get(0), param);
}

function testListJsonStringify() {
    let list = new List<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    list.add("zhang san");
    list.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    list.add(param);
    list.add(undefined);
    list.add(null);
    list.add(true);
    list.insert("insert", 0);
    list.add([0, 1, 2, undefined]);
    list.removeByIndex(0);
    let jsonResult = JSON.stringify(list);
    let result = `{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true,"6":[0,1,2,null]}`;
    arktest.assertEQ(jsonResult, result);
}
