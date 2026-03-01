/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { ArrayList } from "@ohos.util.ArrayList";
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("ArrayList API tests");

    suite.addTest("Testing ArrayList add", testArrayList0001);
    suite.addTest("Testing ArrayList basic operations", testArrayList0002);
    suite.addTest("Testing ArrayList sort", testArrayList0003);
    suite.addTest("Testing ArrayList exception handling for empty container", testArrayList0004);
    suite.addTest("Testing ArrayList key iteration", testArrayList0006);
    suite.addTest("Testing ArrayList insert and remove with floor index", testArrayList0007);

    return suite.run();
}

function testedIterator(arrayList: ArrayList<double>, testArray: Array<double>): boolean {
    let itr = arrayList.$_iterator();
    let tmp = itr.next().value;
    let testArray1 = new Array<double>();
    do {
        testArray1.push(tmp!);
        tmp = itr.next().value;
    } while (tmp != undefined);

    for (let k = 0; k < arrayList.length; k++) {
        if (testArray1[k] !== testArray[k]) {
            return false;
        }
    }
    return true;
}

function testArrayList0001() {
    let arr = new ArrayList<double>();
    arr.add(1);
    arr.add(2);

    for (let i: int = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i], i + 1, `The element at index ${i} should be ${i + 1}`);
    }
}

function testArrayList0002() {
    let arr1 = new ArrayList<double>();
    let testArray = new Array<double>();

    for (let i: int = 0; i < 10; i++) {
        arr1.add(i);
        testArray.push(i);
    }
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(arr1[i], testArray[i], `The element at index ${i} should be ${testArray[i]}`);
    }

    arktest.assertTrue(arr1.has(2), "The ArrayList should contain the value 2");
    arktest.assertEQ(arr1.getCapacity(), 10, "The capacity of the ArrayList should be 10");
    arktest.assertEQ(arr1.getLastIndexOf(1), 1, "The last index of value 1 should be 1");
    arktest.assertEQ(arr1.getIndexOf(5), 5, "The index of value 5 should be 5");

    let array = arr1.convertToArray();
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(array[i], testArray[i], `The element at index ${i} should be ${testArray[i]} in the converted array`);
    }

    let newArrayList = arr1.clone();
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(newArrayList[i], testArray[i],
            `The element at index ${i} should be ${testArray[i]} in the cloned ArrayList`);
    }

    arr1.insert(999, 3);
    testArray.splice(3 as int, 0 as int, 999);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(arr1[i], testArray[i], `The element at index ${i} should be ${testArray[i]} after insertion`);
    }

    arr1.removeByIndex(9);
    testArray.splice(9 as int, 1);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(arr1[i], testArray[i], `The element at index ${i} should be ${testArray[i]} after removal by index`);
    }

    const removeRes = arr1.remove(7);
    testArray.splice(8 as int, 1);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(arr1[i], testArray[i], `The element at index ${i} should be ${testArray[i]} after removal by value`);
    }
    arktest.assertTrue(removeRes, "The remove operation should return true");

    arr1.removeByRange(1, 3);
    testArray.splice(1 as int, 2);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(arr1[i], testArray[i], `The element at index ${i} should be ${testArray[i]} after removal by range`);
    }

    arr1.forEach((value, index) => {
        arktest.assertEQ(value, testArray[index], `The element at index ${index} should be ${testArray[index]} during forEach`);
    });

    let subArrayList = arr1.subArrayList(1, 3);
    const newTestArray = testArray.slice(1, 3);
    for (let i: int = 0; i < subArrayList.length; i++) {
        arktest.assertEQ(newTestArray[i], subArrayList[i],
            `The element at index ${i} should be ${newTestArray[i]} in the subArrayList`);
    }

    let j = 0;
    for (const data of arr1) {
        arktest.assertEQ(data, testArray[j], `The element at index ${j} should be ${testArray[j]} during iteration`);
      j++;
    }

    arktest.assertTrue(testedIterator(arr1, testArray), "The iterator should correctly iterate over all elements");

    arr1.replaceAllElements((item, index) => {
        return item * 2;
    });

    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(arr1[i], testArray[i] * 2,
            `The element at index ${i} should be ${testArray[i] * 2} after replaceAllElements`);
    }
}

function testArrayList0003() {
    let arr2 = new ArrayList<double>();
    arr2.add(4);
    arr2.add(3);
    arr2.add(1);
    arr2.add(2);
    arr2.add(0);
    arr2.sort((a,b) => (a - b).toInt());
    for (let i: int = 0; i < 5; i++) {
        arktest.assertEQ(arr2[i], i, `The element at index ${i} should be ${i} after sorting in ascending order`);
    }

    arr2.clear();
    arktest.assertEQ(arr2.length, 0, "The length of the ArrayList should be 0 after clearing all elements");
    arktest.assertTrue(arr2.isEmpty(), "The ArrayList should be empty after clearing all elements");
    arr2.add(4);
    arr2.add(3);
    arr2.add(1);
    arr2.add(2);
    arr2.add(0);
    arr2.sort((a,b) => (a - b).toInt());
    for (let i: int = 0; i < 5; i++) {
        arktest.assertEQ(arr2[i], i, `The element at index ${i} should be ${i} after re-sorting in ascending order`);
    }

    arr2.insert(50, 0);
    arktest.assertEQ(arr2[0], 50, "The first element should be 50 after inserting 50 at index 0");
    arr2.increaseCapacityTo(20);
    arktest.assertEQ(arr2.getCapacity(), 20, "The capacity of the ArrayList should be 20 after increasing capacity");

    arr2.trimToCurrentLength();
    arktest.assertEQ(arr2.getCapacity(), 6, "The capacity of the ArrayList should match its current length (6) after trimming");
}

function testArrayList0004() {
    let testList = new ArrayList<double>();
    arktest.expectThrow(() => {testList.removeByIndex(0)}, exceptionCheckEmptyContainer);
    arktest.expectThrow(() => {testList.removeByRange(0, 1)}, exceptionCheckEmptyContainer);
    arktest.expectThrow(() => {testList.subArrayList(0, 1)}, exceptionCheckEmptyContainer);
    testList.add(7);
    testList.add(9);
    arktest.assertEQ(testList[1], 9, "The element at index 1 should be 9 after adding 7 and 9");
}

function testArrayList0006() {
    let test1 = new ArrayList<double>();
    for (let k = 0; k < 10; k++) {
        test1.add(k);
    }
    let keyName = "";
    let key: double = 0;
    for (key of test1) {
        keyName += key;
    }
    arktest.assertEQ(keyName, "0123456789", "The concatenated keys should match the sequence '0123456789'");
}

function testArrayList0007() {
    let myAl = new ArrayList<double>();
    myAl.add(1);
    myAl.add(2);
    myAl.add(3);
    myAl.insert(30, Math.floor(1.3).toInt());
    arktest.assertEQ(myAl[1], 30, "The element at index 1 should be 30 after insertion");
    myAl.removeByIndex(Math.floor(1.3).toInt());
    arktest.assertEQ(myAl[1], 2, "The element at index 1 should be 2 after removal");
}

function exceptionCheckEmptyContainer(e: Error): boolean | string {
    if (e instanceof BusinessError) {
        return e.name == "BusinessError" && e.message == `Container is empty`
    }
    return false
}

function exceptionCheckType(e: Error): boolean | string {
    if (e instanceof BusinessError) {
        return e.name == "BusinessError" && e.message.startsWith(`The type of \"index\" must be small integer.`) &&
            e.code == TypeErrorCodeId
    }
    return false
}
