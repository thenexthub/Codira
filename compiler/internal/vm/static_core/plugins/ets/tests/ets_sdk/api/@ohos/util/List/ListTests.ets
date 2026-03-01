/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

import {List} from '@ohos.util.List';
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;
const TypeErrorCodeId: int = 401;

// Test cases ported from ArkTS 1.0: container_list.js
function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("Testing List copy and manipulation operations", testCopyArray);
    suite.addTest("Testing List basic operations and methods", testBasicOperationsAndMethods);
    suite.addTest("Testing List sort and clear operations", testSortAndClearOperations);
    suite.addTest("Testing List getSubList functionality", testGetSubListFunctionality);
    suite.addTest("Testing List with index access", testIndexAccess);
    suite.addTest("Testing List exception handling with empty container", testExceptionHandlingWithEmptyContainer);
    suite.addTest("Testing List methods with Math.floor indices", testMethodsWithMathFloorIndices);

    return suite.run();
}

function testCopyArray() {
    let myList = new List<string>();
    let result = [
        "hello9",
        "hello10",
        "hello0",
        "hello1",
        "hello2",
        "hello3",
        "hello4",
        "hello5",
        "hello6",
        "hello7",
        "hello8",
        "hello10",
        "hello9"
    ]
    let list = new List<string>();
    for (let i: int = 0; i <= 10; i++) {
        list.add("hello" + i);
    }
    myList = list;
    let tmp = myList;
    tmp.add("hello10");
    myList = tmp;
    let tmp2 = myList;
    tmp2.remove("hello10");
    tmp2.insert("hello10", 0);
    myList = tmp2;
    let tmp3 = myList;
    tmp3.add("hello9");
    myList = tmp3;
    let tmp4 = myList;
    tmp4.remove("hello9");
    tmp4.insert("hello9", 0);
    myList = tmp4;
    for (let i: int = 0; i < myList.length; i++) {
        arktest.assertEQ(myList.get(i), result[i]);
    }
}

function testBasicOperationsAndMethods() {
    let list = new List<double>();
    let testArray: Array<double> = new Array<double>();
    arktest.assertTrue(list.isEmpty(), "The list should be empty initially");
    for (let i: int = 0; i < 10; i++) {
        list.add(i);
        testArray.push(i);
    }

    let s = Math.trunc(1.33);
    arktest.assertFalse(list.isEmpty(), "The list should not be empty after adding elements");
    arktest.assertEQ(list.get(1), 1);
    arktest.assertEQ(list.get(s.toInt()), 1);
    arktest.assertTrue(list.has(8), "The list should contain the double 8");
    arktest.assertFalse(list.has(123), "The list should not contain the double 123");

    let list1 = new List<double>();
    let testArray2 = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        list1.add(i)
        testArray2.push(i);
    }

    arktest.assertTrue(list.equal(list1), "The two lists should be equal initially");
    list.add(10);
    testArray.push(10);
    arktest.assertFalse(list.equal(list1), "The two lists should not be equal after adding an element to one of them");
    arktest.assertEQ(list.getLastIndexOf(1), 1);
    arktest.assertEQ(list.getIndexOf(5), 5);

    list.removeByIndex(10);
    testArray.splice(10, 1);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list[i], testArray[i]);
    }

    list.remove(9);
    testArray.splice(9, 1);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list[i], testArray[i]);
        testArray[i] = testArray[i] * 2;
    }

    list.replaceAllElements((item, index) => {
        return item * 2;
    })

    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list[i], testArray[i]);
    }
    arktest.assertEQ(list.getFirst(), 0);
    arktest.assertEQ(list.getLast(), 16);
    list.insert(999, 3);
    testArray.splice(3, 0, 999);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list[i], testArray[i]);
    }

    list.set(5, 888);
    testArray[5] = 888;
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list[i], testArray[i]);
    }

    let subList = list.getSubList(1, 3);
    let newTestArray = testArray.slice(1, 3);
    for (let i: int = 0; i < subList.length; i++) {
        arktest.assertEQ(newTestArray[i], subList[i]);
    }

    let arr = list.convertToArray();
    for (let i: int = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i], testArray[i]);
    }

    let i: int = 0;
    for (let data of list) {
        arktest.assertEQ(data, testArray[i]);
        i++;
    }

    list1.forEach((i, d) => {
        arktest.assertEQ(d, testArray2[i.toInt()]);
    })

    try {
        let result = list1.get(200);
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.code, 10200001);
    }
    arktest.assertEQ(list1.getLastIndexOf(3333), -1);

    arktest.expectThrow(() => {list1.removeByIndex(99)});

    arktest.assertFalse(list1.remove(888), "The remove method should return false for a non-existing element 888");
}

function testSortAndClearOperations() {
    let list2 = new List<double>();
    list2.add(4);
    list2.add(3);
    list2.add(1);
    list2.add(2);
    list2.add(0);
    list2.sort((a,b) => a-b);
    for (let i: int = 0; i < 5; i++) {
        arktest.assertEQ(list2[i], i);
    }
    list2.clear();
    arktest.assertEQ(list2.length, 0);

    let testArrayClear = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        list2.add(i);
        testArrayClear.push(i);
    }
    for (let i: int = 0; i < testArrayClear.length; i++) {
        arktest.assertEQ(list2[i], testArrayClear[i]);
    }
}

function testGetSubListFunctionality() {
    let list6 = new List<double>();
    let testArray1 = new Array<double>();
    for (let i: int = 0; i < 4; i++) {
        list6.add(i);
        testArray1.push(i);
    }
    let subList11 = list6.getSubList(1, 4);
    let newTestArrayList1 = testArray1.slice(1, 4);
    for (let i: int = 0; i < subList11.length; i++) {
        arktest.assertEQ(newTestArrayList1[i], subList11[i]);
    }
}

function testIndexAccess() {
    let testList = new List<double>();
    testList.add(7);
    testList.add(8);
    arktest.assertEQ(testList[1], 8);
}

function testExceptionHandlingWithEmptyContainer() {
    let empty_l = new List<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => {empty_l.set(0, 1)}, exceptionCheck);
    let exceptionCheck1 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => {empty_l.removeByIndex(0)}, exceptionCheck1);
    let exceptionCheck2 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => {empty_l.getSubList(0, 1)}, exceptionCheck2);
}

function testMethodsWithMathFloorIndices() {
    let myList1 = new List<double>();
    myList1.add(1);
    myList1.add(2);
    myList1.add(3);
    myList1.insert(999, Math.floor(1.5).toInt());
    arktest.assertEQ(myList1.get(Math.floor(1.5).toInt()), 999);
    myList1.set(Math.floor(1.5).toInt(), 888);
    myList1.removeByIndex(Math.floor(1.5).toInt());
    arktest.assertEQ(myList1.length, 3);
}
