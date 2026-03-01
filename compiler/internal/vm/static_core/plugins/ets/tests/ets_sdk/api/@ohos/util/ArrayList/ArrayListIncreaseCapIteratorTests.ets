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

function main() {
    const suite = new arktest.ArkTestsuite("ArrayList API tests");

    suite.addTest("ArrayList increaseCapacityTo with bigger value", testIncreaseCapacityToBiggerValue);
    suite.addTest("ArrayList increaseCapacityTo with smaller value", testIncreaseCapacityToSmallerValue);
    suite.addTest("ArrayList increaseCapacityTo", testIncreaseCapacityTo);
    suite.addTest("ArrayList trimToCurrentLength", testTrimToCurrentLength);
    suite.addTest("ArrayList iterator with empty container", testIteratorEmptyContainer);
    suite.addTest("ArrayList iterator", testIterator);
    suite.addTest("ArrayList API composition testing", testAPIComposition);
    // NOTE(akmaev): started to fail in runtime after primitives refactoring patch
    // suite.addTest("ArrayList the add, insert, and removeByIndex APIs are mixed", testAddInsertRemoveByIndex);
    suite.addTest("ArrayList the getIndexOf, removeByIndex, and has APIs are mixed", testGetIndexOfRemoveByIndexHas1);
    suite.addTest("ArrayList the getIndexOf, removeByIndex, and has APIs are mixed", testGetIndexOfRemoveByIndexHas2);

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
    static readonly testNumber6: int = 6;
    static readonly testNumber10: int = 10;
    static readonly testNumber12: int = 12;
    static readonly testNumber18: int = 18;
    static readonly testNumber1w: int = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "";
    static readonly testString3: string = "`";
    static readonly testString4: string = "%";
    static readonly testString5: string = `"`;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, 2, undefined];
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result1: Number[] = [5, 4, 3, 2, 1];
    static readonly arrayResult8 = `{"0":"string","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true,"6":[0,1,2,null]}`;
}

function testIncreaseCapacityToBiggerValue() {
    let arrayList = new ArrayList<double>();
    for (let i: int = 0; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let cap: double = arrayList.getCapacity();
    arrayList.increaseCapacityTo(TestData.testNumber12);
    let cap1: double = arrayList.getCapacity();
    for (let i: int = 0; i < TestData.testNumber3; i++) {
        arrayList.add(TestData.forNumber[i]);
    }
    let cap2: double = arrayList.getCapacity();
    initializeArrayList(arrayList, TestData.forNumber);
    arktest.assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    arktest.assertEQ(cap1, TestData.testNumber12, "ArrayList capacity should be 12 after increaseCapacityTo");
    arktest.assertEQ(cap2, TestData.testNumber12, "ArrayList capacity should remain 12 after adding elements");
    arktest.assertEQ(arrayList.getCapacity(), TestData.testNumber18,
        "ArrayList capacity should be 18 after adding more elements");

    arrayList.increaseCapacityTo(TestData.testNumber1w);
    arktest.assertEQ(arrayList.getCapacity(), TestData.testNumber1w,
        "ArrayList capacity should be 10000 after increaseCapacityTo");
}

function testIncreaseCapacityToSmallerValue() {
    let arrayList = new ArrayList<double>();
    for (let i: int = 1; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let cap: double = arrayList.getCapacity();
    arrayList.increaseCapacityTo(TestData.testNumber2);
    arktest.assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    arktest.assertEQ(arrayList.getCapacity(), TestData.testNumber10,
        "ArrayList capacity should remain unchanged when increasing to a smaller value");
}

function testIncreaseCapacityTo() {
    let arrayList = new ArrayList<number>();
    for (let i: number = 0; i < 5; i++) {
        arrayList.add(i);
    }
    arktest.assertEQ(arrayList.getCapacity(), 10);
    arrayList.increaseCapacityTo(-1);
    arktest.assertEQ(arrayList.getCapacity(), 10);
    arrayList.increaseCapacityTo(8);
    arktest.assertEQ(arrayList.getCapacity(), 8);
}

function testTrimToCurrentLength() {
    let arrayList = new ArrayList<double>();
    for (let i: int = 1; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let cap: double = arrayList.getCapacity();
    arrayList.trimToCurrentLength();
    arktest.assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    arktest.assertEQ(arrayList.getCapacity(), arrayList.length, "ArrayList capacity should be trimmed to current length");
}

function testIteratorEmptyContainer() {
    let arrayList = new ArrayList<double>();
    let elem: double = 0;
    for (let item of arrayList) {
        elem = TestData.testNumber1;
    }
    let iter = arrayList.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    arktest.assertEQ(elem, 0, "Iterator over empty ArrayList should not iterate any elements");
}

function testIterator() {
    let arrayList = new ArrayList<double>();
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let elem1: int = TestData.testNumber1;
    for (let item of arrayList) {
        arktest.assertEQ(item, elem1++, "Element mismatch during iteration");
    }
    let iter = arrayList.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    let elem2: int = TestData.testNumber1;
    while (!temp.done) {
        arktest.assertEQ(temp.value, elem2++, "Element mismatch during iteration");
        temp = iter.next();
    }
    arktest.assertEQ(elem1, TestData.testNumber6, "Iterator should iterate all elements correctly");
    arktest.assertEQ(elem2, TestData.testNumber6, "Iterator should iterate all elements correctly");
}

function testAPIComposition() {
    extendTestInterfaceComposition();

    let arrayList = new ArrayList<double>();
    initializeArrayList(arrayList, TestData.result1);
    let array = arrayList.convertToArray();
    arrayList.sort();
    array.sort();
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], array[i], `Element at index ${i} should match`);
    }

    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    arrayList.clone();
    arktest.assertEQ(arrayList.getCapacity(), TestData.testNumber10, "ArrayList capacity should be updated correctly");
    arrayList.increaseCapacityTo(TestData.testNumber12);
    arktest.assertEQ(arrayList.getCapacity(), TestData.testNumber12, "ArrayList capacity should be updated correctly");
    arrayList.trimToCurrentLength();
    arktest.assertEQ(arrayList.getCapacity(), arrayList.length, "ArrayList capacity should be updated correctly");

    arrayList.replaceAllElements((): double => {
        return TestData.testNumber3;
    });

    arrayList.clear();
    let elem: double = 0;
    for (let item of arrayList) {
        elem = TestData.testNumber1;
    }
    arktest.assertEQ(elem, 0, "Iterator over empty ArrayList should not iterate any elements");
}

function testAddInsertRemoveByIndex() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
        Object>();
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testObject1);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    arrayList.insert(TestData.testString1, 0);
    arrayList.add(TestData.testObject2);
    arrayList.removeByIndex(0);
    arktest.assertEQ(JSON.stringify(arrayList), TestData.arrayResult8, "Element should be equal");
}

function testGetIndexOfRemoveByIndexHas1() {
    let arrayList = new ArrayList<double | string>();
    arrayList.add(TestData.testString1);
    arrayList.insert(TestData.testString2, 0);
    arrayList.insert(TestData.testString3, arrayList.length);
    let startIndex = arrayList.getIndexOf(TestData.testString2);
    arrayList.removeByIndex(startIndex);
    let endIndex = arrayList.getIndexOf(TestData.testString3);
    arrayList.removeByIndex(endIndex);
    arktest.assertFalse(arrayList.has(TestData.testString2), "Elements should be removed");
    arktest.assertFalse(arrayList.has(TestData.testString3), "Elements should be removed");
}

function testGetIndexOfRemoveByIndexHas2() {
    let arrayList = new ArrayList<double | string>();
    arrayList.insert(TestData.testString3, 0);
    arrayList.insert(TestData.testString4, TestData.testNumber1);
    arrayList.insert(TestData.testString5, TestData.testNumber2);
    let index1 = arrayList.getIndexOf(TestData.testString3);
    arrayList.removeByIndex(index1);
    let index2 = arrayList.getIndexOf(TestData.testString4);
    arrayList.removeByIndex(index2);
    let index3 = arrayList.getIndexOf(TestData.testString5);
    arrayList.removeByIndex(index3);
    arktest.assertFalse(arrayList.has(TestData.testString3), "The element should be removed");
    arktest.assertFalse(arrayList.has(TestData.testString4), "The element should be removed");
    arktest.assertFalse(arrayList.has(TestData.testString5), "The element should be removed");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}

function extendTestInterfaceComposition() {
    let arrayList = new ArrayList<double | string | boolean | undefined | null | Object>();
    arktest.assertEQ(arrayList.length, 0, "Initial ArrayList length should be 0");

    let result: boolean = arrayList.add(TestData.testNumber3);
    arktest.assertTrue(result, "ArrayList add should return true");

    arrayList.insert(TestData.testNumber1, 0);
    result = arrayList.has(TestData.testNumber3);
    arktest.assertTrue(result, "ArrayList should contain the element");

    let startIndex = arrayList.getIndexOf(TestData.testNumber1);
    arrayList.removeByIndex(startIndex);
    arktest.assertFalse(arrayList.has(TestData.testNumber1), "Elements should be removed");

    arrayList.remove(TestData.testNumber3);
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testObject1);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    let endIndex = arrayList.getLastIndexOf(true);
    arrayList.removeByIndex(endIndex);
    arktest.assertFalse(arrayList.has(true), "Elements should be removed");

    arrayList.removeByRange(0, arrayList.length - 1);
    arktest.assertFalse(arrayList.isEmpty(), "ArrayList should not be empty");
}
