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

import {LinkedList} from "@ohos.util.LinkedList";

function main() {
    const suite = new arktest.ArkTestsuite("LinkedList API tests");

    suite.addTest("LinkedList removeFirstFound with an unique element", testRemoveFirstFoundUniqueElement);
    suite.addTest("LinkedList removeFirstFound with an inexistent element", testRemoveFirstFoundInexistentElement);
    suite.addTest("LinkedList removeFirstFound with a repeated element", testRemoveFirstFoundRepeatedElement);
    suite.addTest("LinkedList removeLastFound with an unique element", testRemoveLastFoundUniqueElement);
    suite.addTest("LinkedList removeLastFound with an inexistent element", testRemoveLastFoundInexistentElement);
    suite.addTest("LinkedList removeLastFound add->getLastIndexOf->removeLastFound",
        testRemoveLastFoundAddGetLastIndexOf);
    suite.addTest("LinkedList removeLastFound with a repeated element", testRemoveLastFoundRepeatedElement);

    return suite.run();
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
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
    static readonly testNumber6: int = 6;
    static readonly forNumber3: Number[] = [1, 2, 4, 2, 3];
    static readonly arrayForNumber3 = Array.from<double>(TestData.forNumber3);
}

function testRemoveFirstFoundUniqueElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertTrue(linkedList.removeFirstFound(TestData.testNumber1), "First found element 1 should be removed");
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveFirstFoundInexistentElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertFalse(linkedList.removeFirstFound(TestData.testNumber6), "Element 6 is inexistent");
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i], "Element should match");
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length, "LinkedList length should should not change");
}

function testRemoveFirstFoundRepeatedElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertTrue(linkedList.removeFirstFound(TestData.testNumber2), "First found element 2 should be removed");
    arktest.assertEQ(linkedList.get(0), TestData.testNumber1, "First element should be 1");
    for (let i: int = TestData.testNumber1; i < linkedList.length - TestData.testNumber1; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveLastFoundUniqueElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertTrue(linkedList.removeLastFound(TestData.testNumber1), "Last found element 1 should be removed");
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveLastFoundInexistentElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertFalse(linkedList.removeLastFound(TestData.testNumber6), "Element 6 is inexistent");
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i], "Element should match");
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length, "LinkedList length should should not change");
}

function testRemoveLastFoundAddGetLastIndexOf() {
    let linkedList = new LinkedList<double>();
    linkedList.add(TestData.testNumber2);
    let index1 = linkedList.getLastIndexOf(TestData.testNumber2);
    linkedList.add(TestData.testNumber2);
    let index2 = linkedList.getLastIndexOf(TestData.testNumber2);
    linkedList.add(TestData.testNumber2);
    let index3 = linkedList.getLastIndexOf(TestData.testNumber2);
    linkedList.add(TestData.testNumber3);
    arktest.assertTrue(index1 !== index2 && index2 !== index3, "The index is not the same");
    linkedList.removeLastFound(TestData.testNumber2);
    arktest.assertEQ(linkedList.getLastIndexOf(TestData.testNumber2), index2, "The index should be index2");
}

function testRemoveLastFoundRepeatedElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertTrue(linkedList.removeLastFound(TestData.testNumber2), "Last found element 2 should be removed");
    for (let i: int = 0; i < linkedList.length - TestData.testNumber1; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i], "Element should match");
    }
    arktest.assertEQ(linkedList.get(TestData.testNumber3), TestData.testNumber3, "Last element should be 3");
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}
