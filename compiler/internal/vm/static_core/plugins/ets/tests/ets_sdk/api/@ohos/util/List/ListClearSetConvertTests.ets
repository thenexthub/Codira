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
const EmptyContainerErrorCodeId: int = 10200010;

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("List clear a non-empty container", testClearNonEmptyContainer);
    suite.addTest("List clear an empty container", testClearEmptyContainer);
    suite.addTest("List set multiple types of data in the container", testSetMultipleTypes);
    suite.addTest("List set elements when the container is empty", testSetElementsInEmptyContainer);
    suite.addTest("List set getIndexOf->set->removeByIndex", testSetGetIndexOfRemoveByIndex);
    suite.addTest("List set getIndexOf->set->has", testSetGetIndexOfHas);
    suite.addTest("List set set->getLast", testSetGetLast);
    suite.addTest("List set set->getFirst", testSetGetFirst);
    suite.addTest("List convertToArray with a non-empty container", testConvertToArrayNonEmptyContainer);
    suite.addTest("List convertToArray with an empty container", testConvertToArrayEmptyContainer);
    suite.addTest("List convertToArray with change array", testConvertToArrayChangeArray);

    return suite.run();
}

function initializeList(list: List<double>, sourceArray: Array<double>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
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
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, null];
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
}

function testClearNonEmptyContainer() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length, "List length should be same as TestData.arrayForNumber1");
    list.clear();
    arktest.assertEQ(list.length, 0, "List length should be 0 after clear");
}

function testClearEmptyContainer() {
    let list = new List<double>();
    arktest.assertEQ(list.length, 0, "List length should be 0");
    list.clear();
    arktest.assertEQ(list.length, 0, "List length should be 0 after clear");
}

function testSetMultipleTypes() {
    let list = new List<double | string | boolean | null | object | undefined>();
    for (let i: int = 0; i < TestData.testNumber6; i++) {
        list.add(i);
    }

    arktest.assertEQ(list.set(0, TestData.testString1), TestData.testString1, "Set successful");
    arktest.assertEQ(list.set(TestData.testNumber1, TestData.testNumber3), TestData.testNumber3, "Set successful");
    arktest.assertEQ(list.set(TestData.testNumber2, null), null, "Set successful");
    arktest.assertEQ(list.set(TestData.testNumber3, true), true, "Set successful");
    arktest.assertEQ(list.set(TestData.testNumber4, TestData.testObject2), TestData.testObject2, "Set successful");
    arktest.assertEQ(list.set(TestData.testNumber5, undefined), undefined, "Set successful");
}

function testSetElementsInEmptyContainer() {
    let list = new List<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty" && e.code == EmptyContainerErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.set(0, TestData.testNumber3)}, exceptionCheck);
}

function testSetGetIndexOfRemoveByIndex() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let index = list.getIndexOf(TestData.testObject1);
    let elem1 = list.set(Math.floor(index).toInt(), TestData.testString2);
    let elem2 = list.removeByIndex(Math.floor(index).toInt());
    arktest.assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testSetGetIndexOfHas() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let index = list.getIndexOf(TestData.testObject1);
    let elem = list.set(Math.floor(index).toInt(), TestData.testString2);
    arktest.assertTrue(list.has(elem), "The element set successful");
}

function testSetGetLast() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let elem1 = list.set(list.length - TestData.testNumber1, TestData.testString2);
    let elem2 = list.getLast();
    arktest.assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testSetGetFirst() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let elem1 = list.set(0, TestData.testString2);
    let elem2 = list.getFirst();
    arktest.assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testConvertToArrayNonEmptyContainer() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    let array = list.convertToArray();
    arktest.assertTrue(Array.isArray(array), "The result should be an Array");
    arktest.assertEQ(array.length, list.length, "Array length should be same as List length");
    for (let i: int = 0; i < TestData.arrayForNumber1.length; i++) {
        arktest.assertEQ(array[i], list.get(i), `Element at index ${i} should be equal`);
    }
}

function testConvertToArrayEmptyContainer() {
    let list = new List<double>();
    let array = list.convertToArray();
    arktest.assertEQ(array.length, 0, "Array length should be 0");
}

function testConvertToArrayChangeArray() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    let array = list.convertToArray();
    array.push(TestData.testNumber1);
    arktest.assertEQ(array.length, list.length + TestData.testNumber1, "Array length should be List length + 1");
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length, "List length should not change");
    for (let i: int = 0; i < TestData.arrayForNumber1.length; i++) {
        arktest.assertEQ(list.get(i), TestData.arrayForNumber1[i], `Element at index ${i} should be equal`);
    }
}
