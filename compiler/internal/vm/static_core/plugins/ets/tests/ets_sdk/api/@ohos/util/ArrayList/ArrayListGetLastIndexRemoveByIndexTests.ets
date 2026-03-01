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

import {ArrayList} from "@ohos.util.ArrayList";
import {BusinessError} from "@ohos.base";

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("ArrayList API tests");

    suite.addTest("ArrayList getLastIndexOf with unique element", testGetLastIndexOfUniqueElement);
    suite.addTest("ArrayList getLastIndexOf with repeated element", testGetLastIndexOfRepeatedElement);
    suite.addTest("ArrayList getLastIndexOf with inexistent element", testGetLastIndexOfNonExistentElement);
    suite.addTest("ArrayList removeByIndex", testRemoveByIndex);
    suite.addTest("ArrayList removeByIndex with 0 and length - 1", testRemoveByIndexWithBoundaryIndices);
    suite.addTest("ArrayList removeByIndex with out of range index", testRemoveByIndexOutOfRange);
    suite.addTest("ArrayList removeByIndex with empty container", testRemoveByIndexEmptyContainer);
    suite.addTest("ArrayList removeByIndex with the same index", testRemoveByIndexSameIndex);

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
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber12: int = 12;
    static readonly testString1: string = "string";
    static readonly testString2: string = "";
    static readonly testString3: string = "`";
    static readonly testString4: string = "%";
    static readonly testString5: string = `"`;
    static readonly testDouble1: double = 0.4;
    static readonly testDouble2: double = 0.7;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, 2, undefined];
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result5: Number[] = [1, 2, 3, 5];
    static readonly result6: Number[] = [2, 3, 4];
    static readonly result7: Number[] = [2, 3, 4, 5];
}

function testGetLastIndexOfUniqueElement() {
    let arrayList = new ArrayList<double>();
    arrayList.add(TestData.testNumber1);
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arrayList.add(TestData.testNumber2);
    arktest.assertEQ(arrayList.getLastIndexOf(TestData.testNumber1), 0, "Last index of first element should be 0");
    arktest.assertEQ(arrayList.getLastIndexOf(TestData.testNumber2), TestData.testNumber6,
        "Last index of last element should be 6");
}

function testGetLastIndexOfRepeatedElement() {
    let arrayList = new ArrayList<double>();
    arrayList.add(TestData.testNumber1);
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arktest.assertEQ(arrayList.getLastIndexOf(TestData.testNumber3), TestData.testNumber5, "Last index of element should be 5");
}

function testGetLastIndexOfNonExistentElement() {
    let arrayList = new ArrayList<double>();
    arrayList.add(TestData.testNumber1);
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arktest.assertEQ(arrayList.getLastIndexOf(TestData.testNumber5), -TestData.testNumber1,
        "Last index of non-existent element should be -1");
}

function testRemoveByIndex() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByIndex(TestData.testNumber3);
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], TestData.result5[i], `Element at index ${i} should match`);
    }
    arktest.assertEQ(arrayList.length, TestData.testNumber5 - TestData.testNumber1,
        "ArrayList length should be 4 after removeByIndex operation");
}

function testRemoveByIndexWithBoundaryIndices() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByIndex(0);
    arrayList.removeByIndex(TestData.testNumber3);
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], TestData.result6[i], `Element at index ${i} should match`);
    }
    arktest.assertEQ(arrayList.length, TestData.testNumber3, "ArrayList length should be 3 after removeByIndex operations");
}

function testRemoveByIndexOutOfRange() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
                `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: 12`;
        }
        return false;
    }
    let exceptionCheck1 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
                `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: -12`;
        }
        return false;
    }
    arktest.expectThrow(() => {arrayList.removeByIndex(TestData.testNumber12)}, exceptionCheck);
    arktest.expectThrow(() => {arrayList.removeByIndex(-TestData.testNumber12)}, exceptionCheck1);
}

function testRemoveByIndexEmptyContainer() {
    let arrayList = new ArrayList<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == `Container is empty`;
        }
        return false;
    }
    arktest.expectThrow(() => {arrayList.removeByIndex(0)}, exceptionCheck);
}

function testRemoveByIndexSameIndex() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
        Object>();
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testObject1);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    arrayList.add(TestData.testObject2);
    arrayList.add(TestData.testString2);
    arrayList.add(TestData.testString3);
    arrayList.add(TestData.testString4);
    arrayList.add(TestData.testString5);
    let array = arrayList.convertToArray();
    let index1 = arrayList.getIndexOf(TestData.testString2);
    let result = arrayList.removeByIndex(index1);
    let result1 = arrayList.removeByIndex(index1);
    arktest.assertNE(result, result1, "The elements removed twice are different");

    let index2 = arrayList.getIndexOf(TestData.testString4);
    let result2 = arrayList.removeByIndex(index2);
    let result3 = arrayList.removeByIndex(index2);
    arktest.assertNE(result2, result3, "The elements removed twice are different");
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], array[i], `Element at index ${i} should match`);
    }
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
