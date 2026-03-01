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

import { LinkedList } from "@ohos.util.LinkedList";
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

// Test cases ported from ArkTS 1.0: container_linked_list.js
function main() {
    const suite = new arktest.ArkTestsuite("LinkedList API tests");

    suite.addTest("Testing LinkedList basic CRUD operations", testLinkedListBasicCRUD);
    suite.addTest("Testing LinkedList remove operations (removeFirstFound, removeLast, removeLastFound)",
        testLinkedListRemoveOperations);
    suite.addTest("Testing LinkedList iteration and custom iterator", testLinkedListIteration);
    suite.addTest("Testing LinkedList exception handling for empty list", testLinkedListExceptionHandling);
    suite.addTest("Testing LinkedList operations with Math.floor indices", testLinkedListMathFloorIndices);

    return suite.run();
}

function testLinkedListBasicCRUD() {
    let list = new LinkedList<double>();
    let testArray = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        list.add(i);
        testArray.push(i);
    }
    arktest.assertTrue(list.has(8), "The LinkedList should contain the double 8");
    arktest.assertTrue(list.has(2), "The LinkedList should contain the double 2");
    arktest.assertEQ(list.getLastIndexOf(1), 1);
    arktest.assertEQ(list.getIndexOf(5), 5);

    list.removeByIndex(9);

    testArray.splice(9, 1);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list.get(i), testArray[i]);
    }

    const removeRes = list.remove(8);
    testArray.splice(8, 1);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list.get(i), testArray[i]);
    }
    arktest.assertTrue(removeRes, "The remove method should return true for a successfully removed element");
    arktest.assertEQ(list.getFirst(), 0);
    arktest.assertEQ(list.getLast(), 7);

    list.insert(3, 999);
    testArray.splice(3, 0, 999);
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list.get(i), testArray[i]);
    }

    list.set(5, 888);
    testArray[5] = 888;
    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(list.get(i), testArray[i]);
    }

    let cloneList = list.clone();
    arktest.assertEQ(list.length, cloneList.length);

    for (let i: int = 0; i < cloneList.length; i++) {
        arktest.assertEQ(list.get(i), cloneList.get(i));
    }

    list.addFirst(1111);
    arktest.assertEQ(list.getFirst(), 1111);

    const removeFirstRes = list.removeFirst();
    arktest.assertEQ(removeFirstRes, 1111);

    let i: int = 0;
    for (const data of list) {
        arktest.assertEQ(data, testArray[i]);
        i++;
    }

    let list1 = new LinkedList<double>();
    let testArray1 = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        list1.add(i);
        testArray1.push(i);
    }

    list1.forEach((d, i) => {
        arktest.assertEQ(d, testArray1[i]);
    });

    list1.clear();
    arktest.assertEQ(list1.length, 0);
    for (let i: int = 0; i < 10; i++) {
        list1.add(i);
    }

    list1.forEach((d, i) => {
        arktest.assertEQ(d, testArray1[i]);
    });
    try {
        let result = list.get(1232);
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.code, 10200001);
    }
    arktest.assertEQ(list.getLastIndexOf(3333), -1);

    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 9. Received value is: 99" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => { list1.removeByIndex(99) }, exceptionCheck);

    const arr = list.convertToArray();
    for (let i: int = 1; i < arr.length; i++) {
        arktest.assertEQ(arr[i], testArray[i]);
    }
}

function testLinkedListRemoveOperations() {
    let removeList = new LinkedList<double>();
    for (let i: int = 0; i < 10; i++) {
        removeList.add(i);
    }
    for (let i: int = 9; i >= 0; i--) {
        removeList.add(i);
    }
    let removeResult = removeList.removeFirstFound(2);
    arktest.assertTrue(removeResult,
        "The removeFirstFound method should return true for a successfully removed element");
    arktest.assertEQ(removeList.getIndexOf(2), 16);

    let removeResult1 = removeList.removeLast();
    arktest.assertEQ(removeResult1, 0);
    arktest.assertEQ(removeList.getLastIndexOf(0), 0);

    let removeResult2 = removeList.removeLastFound(3);
    arktest.assertTrue(removeResult2,
        "The removeLastFound method should return true for a successfully removed element");
    arktest.assertEQ(removeList.getIndexOf(3), 2);
}

function testLinkedListIteration() {
    let list2 = new LinkedList<double>();
    let testArray2 = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        list2.add(i);
        testArray2.push(i);
    }
    let j = 0;
    for (const data of list2) {
        arktest.assertEQ(data, testArray2[j]);
        j++;
    }

    let itr = list2.$_iterator();
    let tmp = itr.next().value;
    let arr3 = new Array<double>();
    do {
        arr3.push(tmp!);
        tmp = itr.next().value;
    } while (tmp != undefined);

    for (let k = 0; k < list2.length; k++) {
        arktest.assertEQ(arr3[k], testArray2[k]);
    }
}

function testLinkedListExceptionHandling() {
    let empty_ll = new LinkedList<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => { empty_ll.set(0, 1) }, exceptionCheck);
    let exceptionCheck1 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => { empty_ll.removeByIndex(0) }, exceptionCheck1);
}

function testLinkedListMathFloorIndices() {
    let myList1 = new LinkedList<double>();
    myList1.add(1);
    myList1.add(2);
    myList1.add(3);
    myList1.insert(Math.floor(1.5).toInt(), 999);
    arktest.assertEQ(myList1.get(Math.floor(1.5).toInt()), 999);
    myList1.set(Math.floor(1.5).toInt(), 888);
    myList1.removeByIndex(Math.floor(1.5).toInt());
    arktest.assertEQ(myList1.length, 3);
}
