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

    suite.addTest("ArrayList convertToArray with empty container", testConvertToArrayWithEmptyContainer);
    suite.addTest("ArrayList convertToArray", testConvertToArray);
    suite.addTest("ArrayList convertToArray with change convertArray", testConvertToArrayWithChange);
    suite.addTest("ArrayList convertToArray and check whether the array is normal", testConvertToArrayAndCheckArray);
    suite.addTest("ArrayList convertToArray and check whether the array is normal",
        testConvertToArrayAndCheckArrayWithCat);
    suite.addTest("ArrayList isEmpty", testIsEmptyAfterAddingElement);
    suite.addTest("ArrayList isEmpty with empty container", testIsEmptyInitially);
    suite.addTest("ArrayList index getter", testIndexGetter);
    suite.addTest("ArrayList index setter", testIndexSetter);
    suite.addTest("ArrayList index out of range", testIndexOutOfRange);
    suite.addTest("ArrayList index about for...of", testIndexForOfLoop);

    return suite.run();
}

class Dog {
    age: double = 0;
    name: string = "";

    constructor(age: double, name: string) {
        this.age = age;
        this.name = name;
    }
}

class Cat {
    age1: double = 0;
    name1: string = "";

    constructor(age: double, name: string) {
        this.age1 = age;
        this.name1 = name;
    }
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber6: int = 6;
    static readonly testNumber8: int = 8;
    static readonly testString1: string = "string";
    static readonly testString4: string = "%";
    static readonly testDouble1 = 0.4;
    static readonly testDouble2 = 0.7;
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result3: Number[] = [3, 1, 2, 3, 4, 5, 3];
    static readonly result4: Number[] = [3, 1, 2, 3, 4, 5];
}

function testConvertToArrayWithEmptyContainer() {
    let arrayList = new ArrayList<double>();
    let array = arrayList.convertToArray();
    arktest.assertEQ(array.length, arrayList.length, "Converted array should have the same length as ArrayList");
    arktest.assertEQ(array.length, 0, "Converted array should have length 0");
}

function testConvertToArray() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    let array = arrayList.convertToArray();
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(array[i], arrayList[i], `Element at index ${i} should match`);
    }
    arktest.assertEQ(array.length, arrayList.length, "Converted array should have the same length as ArrayList");
}

function testConvertToArrayWithChange() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.result4);
    let array = arrayList.convertToArray();
    arktest.assertTrue(array instanceof Array, "The result of convertToArray should be Array");
    array.push(TestData.testNumber3);
    for (let i: int = 0; i < array.length; i++) {
        arktest.assertEQ(array[i], TestData.result3[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], TestData.result4[i], `Element at index ${i} should match`);
    }
    arktest.assertEQ(array.length, TestData.testNumber6 + TestData.testNumber1,
        "Converted array should have the correct length after modification");
    arktest.assertEQ(arrayList.length, TestData.testNumber6,
        "Original ArrayList should remain unchanged after array modification");
}

function testConvertToArrayAndCheckArray() {
    let arrayList = new ArrayList<Dog>();
    let dog = new Dog(TestData.testNumber1, TestData.testString1);
    arrayList.add(dog);
    let array = arrayList.convertToArray();
    array.filter((value: Dog) => {
        arktest.assertTrue(value instanceof Dog, "The converted array's filter normally");
        return true;
    })
}

function testConvertToArrayAndCheckArrayWithCat() {
    let arrayList = new ArrayList<Dog | Cat>();
    let dog = new Dog(TestData.testNumber1, TestData.testString1);
    arrayList.add(dog);
    let array = arrayList.convertToArray();
    let cat = new Cat(TestData.testNumber2, TestData.testString4);
    array.fill(cat);
    arktest.assertFalse(array[0] instanceof Dog, "The converted array's fill normally");
}

function testIsEmptyAfterAddingElement() {
    let arrayList = new ArrayList<double>();
    arrayList.add(TestData.testNumber3);
    let result: boolean = arrayList.isEmpty();
    arktest.assertFalse(result, "ArrayList should not be empty after adding an element");
}

function testIsEmptyInitially() {
    let arrayList = new ArrayList<double>();
    let result: boolean = arrayList.isEmpty();
    arktest.assertTrue(result, "ArrayList should be empty initially");
}

function testIndexGetter() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], i + TestData.testNumber1, `Element at index ${i} should match`);
    }
}

function testIndexSetter() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    for (let i: int = 0; i < arrayList.length; i++) {
        arrayList[i] = TestData.testNumber3;
    }
    let arrayForEach = new Array<double>();
    arrayList.forEach((value: double) => {
        arrayForEach.push(value);
    });
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayForEach[i], TestData.testNumber3, `Element at index ${i} should match`);
    }
    arktest.assertEQ(arrayList.length, arrayForEach.length, "ArrayList length should match arrayForEach length");
}

function testIndexOutOfRange() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
            `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: 8`;
        }
        return false;
    }
    arktest.expectThrow(() => {arrayList[TestData.testNumber8]}, exceptionCheck);
}


function testIndexForOfLoop() {
    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.forNumber);
    let elem: double = TestData.testNumber1;
    for (let item of arrayList) {
        arktest.assertEQ(item, elem++, "Element mismatch during iteration");
    }
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
