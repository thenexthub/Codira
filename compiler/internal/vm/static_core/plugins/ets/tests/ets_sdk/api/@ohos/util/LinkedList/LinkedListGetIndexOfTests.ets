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

    suite.addTest("LinkedList getLastIndexOf an unique element", testGetLastIndexOfUniqueElement);
    suite.addTest("LinkedList getLastIndexOf an inexistent element", testGetLastIndexOfInexistentElement);
    suite.addTest("LinkedList getLastIndexOf add->getLastIndexOf->removeByIndex", testGetLastIndexOfAddRemoveByIndex);
    suite.addTest("LinkedList getLastIndexOf add->removeFirst->add->getLastIndexOf",
        testGetLastIndexOfAddRemoveFirstAdd);
    suite.addTest("LinkedList getLastIndexOf add->removeLast->add->getLastIndexOf", testGetLastIndexOfAddRemoveLastAdd);
    suite.addTest("LinkedList getLastIndexOf a repeated element", testGetLastIndexOfRepeatedElement);
    suite.addTest("LinkedList getIndexOf an unique element", testGetIndexOfUniqueElement);
    suite.addTest("LinkedList getIndexOf an inexistent element", testGetIndexOfInexistentElement);
    suite.addTest("LinkedList getIndexOf add->getIndexOf->removeByIndex", testGetIndexOfAddRemoveByIndex);
    suite.addTest("LinkedList getIndexOf add->removeFirst->add->getIndexOf", testGetIndexOfAddRemoveFirstAdd);
    suite.addTest("LinkedList getIndexOf a repeated element", testGetIndexOfRepeatedElement);

    return suite.run();
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
    static readonly forNumber2: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayForNumber2 = Array.from<double>(TestData.forNumber2);
    static readonly forNumber4: Number[] = [1, 2, 2, 3, 3];
    static readonly arrayForNumber4 = Array.from<double>(TestData.forNumber4);
}

function testGetLastIndexOfUniqueElement() {
    let linkedList = initializeNumberLinkedList();
    
    arktest.assertEQ(linkedList.getLastIndexOf(TestData.testNumber1), 0, "Last index of 1 should be 0");
    arktest.assertEQ(linkedList.getLastIndexOf(TestData.testNumber3), TestData.testNumber3, "Last index of 3 should be 3");
}

function testGetLastIndexOfInexistentElement() {
    let linkedList = initializeNumberLinkedList();

    arktest.assertEQ(linkedList.getLastIndexOf(TestData.testNumber4), -TestData.testNumber1, "Last index of 4 should be -1");
}

function testGetLastIndexOfAddRemoveByIndex() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber4);
    let result1 = linkedList.removeByIndex(linkedList.getLastIndexOf(TestData.testNumber2));
    let result2 = linkedList.removeByIndex(linkedList.getLastIndexOf(TestData.testNumber2));
    arktest.assertEQ(result1, result2, "The two result should be equal");
    let result3 = linkedList.removeByIndex(linkedList.getLastIndexOf(TestData.testNumber3));
    let result4 = linkedList.removeByIndex(linkedList.getLastIndexOf(TestData.testNumber3));
    arktest.assertEQ(result3, result4, "The two result should be equal");
}

function testGetLastIndexOfAddRemoveFirstAdd() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    let elem = linkedList.removeFirst();
    linkedList.add(elem!);
    arktest.assertEQ(linkedList.getLastIndexOf(elem!), linkedList.length - TestData.testNumber1,
        "The index should be length - 1");
}

function testGetLastIndexOfAddRemoveLastAdd() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    let elem = linkedList.removeLast();
    linkedList.add(elem!);
    arktest.assertEQ(linkedList.getLastIndexOf(elem!), linkedList.length - TestData.testNumber1,
        "The index should be length - 1");
}

function testGetLastIndexOfRepeatedElement() {
    let linkedList = initializeNumberLinkedList();

    arktest.assertEQ(linkedList.getLastIndexOf(TestData.testNumber2), TestData.testNumber2, "Last index of 2 should be 2");
}

function testGetIndexOfUniqueElement() {
    let linkedList = initializeNumberLinkedList();

    arktest.assertEQ(linkedList.getIndexOf(TestData.testNumber1), 0, "First index of 1 should be 0");
    arktest.assertEQ(linkedList.getIndexOf(TestData.testNumber3), TestData.testNumber3, "First index of 3 should be 3");
}

function testGetIndexOfInexistentElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber4);
    arktest.assertEQ(linkedList.getIndexOf(TestData.testNumber4), -TestData.testNumber1, "First index of 4 should be -1");
}

function testGetIndexOfAddRemoveByIndex() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber4);
    let result1 = linkedList.removeByIndex(linkedList.getIndexOf(TestData.testNumber2));
    let result2 = linkedList.removeByIndex(linkedList.getIndexOf(TestData.testNumber2));
    arktest.assertEQ(result1, result2, "The two result should be equal");
    let result3 = linkedList.removeByIndex(linkedList.getIndexOf(TestData.testNumber3));
    let result4 = linkedList.removeByIndex(linkedList.getIndexOf(TestData.testNumber3));
    arktest.assertEQ(result3, result4, "The two result should be equal");
}

function testGetIndexOfAddRemoveFirstAdd() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    let elem = linkedList.removeFirst();
    linkedList.add(elem!);
    arktest.assertEQ(linkedList.getIndexOf(elem!), linkedList.length - TestData.testNumber1, "The index should be length - 1");
}

function testGetIndexOfRepeatedElement() {
    let linkedList = initializeNumberLinkedList();

    arktest.assertEQ(linkedList.getIndexOf(TestData.testNumber2), TestData.testNumber1, "First index of 2 should be 1");
}

function initializeNumberLinkedList() {
    let linkedList = new LinkedList<double>();
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testNumber2);
    linkedList.add(TestData.testNumber2);
    linkedList.add(TestData.testNumber3);

    return linkedList;
}
