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

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("List getIndexOf an unique element", testGetIndexOfUniqueElement);
    suite.addTest("List getIndexOf an inexistent element", testGetIndexOfInexistentElement);
    suite.addTest("List getIndexOf add->getIndexOf->removeByIndex", testGetIndexOfAddRemoveByIndex);
    suite.addTest("List getIndexOf a repeated element", testGetIndexOfRepeatedElement);
    suite.addTest("List equal two containers with same elements", testEqualSameElement);
    suite.addTest("List equal two containers with different elements", testEqualDifferentElement);
    suite.addTest("List equal same container", testEqualSameContainer);
    suite.addTest("List removeByIndex with an index within the container's length range", testRemoveByIndexWithinRange);
    suite.addTest("List removeByIndex with an index outside the container's length range",
        testRemoveByIndexOutsideRange);
    suite.addTest("List removeByIndex with removing undefined", testRemoveByIndexUndefined);
    suite.addTest("List removeByIndex with an index at the container's length range", testRemoveByIndexAtBoundary);

    return suite.run();
}

function initializeList(list: List<double>, sourceArray: Array<double>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber6: int = 6;
    static readonly testNumber20: int = 20;
    static readonly testNumber1w: int = 10000;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
    static readonly forNumber2: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayForNumber2 = Array.from<double>(TestData.forNumber2);
    static readonly forNumber4: Number[] = [1, 2, 2, 3, 3];
    static readonly arrayForNumber4 = Array.from<double>(TestData.forNumber4);
}

function testGetIndexOfUniqueElement() {
    let list = new List<double>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    arktest.assertEQ(list.getIndexOf(TestData.testNumber1), 0, "First index of 1 should be 0");
    arktest.assertEQ(list.getIndexOf(TestData.testNumber3), TestData.testNumber3, "First index of 3 should be 3");
}

function testGetIndexOfInexistentElement() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber4);
    arktest.assertEQ(list.getIndexOf(TestData.testNumber4), -TestData.testNumber1, "First index of 4 should be -1");
}

function testGetIndexOfAddRemoveByIndex() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber4);
    let result1 = list.removeByIndex(list.getIndexOf(TestData.testNumber2));
    let result2 = list.removeByIndex(list.getIndexOf(TestData.testNumber2));
    arktest.assertEQ(result1, result2, "The two result should be equal");
    let result3 = list.removeByIndex(list.getIndexOf(TestData.testNumber3));
    let result4 = list.removeByIndex(list.getIndexOf(TestData.testNumber3));
    arktest.assertEQ(result3, result4, "The two result should be equal");
}

function testGetIndexOfRepeatedElement() {
    let list = new List<double>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    arktest.assertEQ(list.getIndexOf(TestData.testNumber2), TestData.testNumber1, "First index of 2 should be 1");
}

function testEqualSameElement(){
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber2);
    let list1= new List<double>();
    initializeList(list1, TestData.arrayForNumber2);
    arktest.assertTrue(list.equal(list1), "Lists should be equal");
}

function testEqualDifferentElement(){
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber2);
    let list1 = new List<double>();
    initializeList(list1, TestData.arrayForNumber4);
    arktest.assertFalse(list.equal(list1), "Lists should not be equal");
}

function testEqualSameContainer(){
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber2);
    arktest.assertTrue(list.equal(list), "List should be equal to itself");
}

function testRemoveByIndexWithinRange() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    arktest.assertEQ(list.removeByIndex(TestData.testNumber1), TestData.testNumber1, "Removed element should be 1");
    arktest.assertEQ(list.get(0), 0, "Element at index 0 should be 0");
    for (let i: int = TestData.testNumber1; i < list.length; i++) {
        arktest.assertEQ(list.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length - TestData.testNumber1, "List length should be - 1");
}

function testRemoveByIndexOutsideRange() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 20. Received value is: 10000" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    let exceptionCheckTwo = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 20. Received value is: -6" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.removeByIndex(TestData.testNumber1w)}, exceptionCheck);
    arktest.expectThrow(() => {list.removeByIndex(-TestData.testNumber6)}, exceptionCheckTwo);
}

function testRemoveByIndexUndefined() {
    let list = new List<double | string | boolean | null | object | undefined>();
    list.add(undefined);
    list.add(TestData.arrayForNumber1);
    arktest.assertEQ(list.removeByIndex(0), undefined, "The result should be undefined");
}

function testRemoveByIndexAtBoundary() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber1);
    arktest.assertEQ(list.removeByIndex(TestData.arrayForNumber1.length - TestData.testNumber1), TestData.testNumber20,
        "Removed element should - 1");
    arktest.assertEQ(list.removeByIndex(0), 0, "Removed element should be 0");
    for (let i: int = 0; i < list.length; i++) {
        arktest.assertEQ(list.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length - TestData.testNumber2, "List length should be - 2");
}
