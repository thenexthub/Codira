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

    suite.addTest("LinkedList forEach without passing any parameters", testForEachNoParams);
    suite.addTest("LinkedList forEach with a single parameter", testForEachSingleParam);
    suite.addTest("LinkedList forEach with two parameters", testForEachTwoParams);
    suite.addTest("LinkedList forEach with three parameters", testForEachThreeParams);
    suite.addTest("LinkedList forEach an empty container", testForEachEmptyContainer);
    suite.addTest("LinkedList forEach with changing the value", testForEachChangeValue);
    suite.addTest("LinkedList iterator with an empty container", testIteratorEmptyContainer);
    suite.addTest("LinkedList iterator with a non-empty container", testIteratorNonEmptyContainer);

    return suite.run();
}

class Temp {
    age: double = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber20: int = 20;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
}

function testForEachNoParams() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: int = 0;
    linkedList.forEach(() => { count++ });
    arktest.assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachSingleParam() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: int = 0;
    linkedList.forEach((value: double) => {
        arktest.assertEQ(linkedList.get(count++), value, `Element should be equal`);
    });
    arktest.assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachTwoParams() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: int = 0;
    linkedList.forEach((value: double, index: int) => {
        arktest.assertEQ(linkedList.get(count), value, `Element should be equal`);
        arktest.assertEQ(count++, index, `The index should be equal`);
    });
    arktest.assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachThreeParams() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: int = 0;
    linkedList.forEach((value: double, index: int, tempLinkedList: LinkedList<double>) => {
        arktest.assertEQ(linkedList, tempLinkedList, "TempLinkedList should be this");
        arktest.assertEQ(linkedList.get(count), value, `Element should be equal`);
        arktest.assertEQ(count++, index, `The index should be equal`);
    });
    arktest.assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachEmptyContainer() {
    let linkedList = new LinkedList<double>();
    let count: int = 0;
    linkedList.forEach(() => { count++ });
    arktest.assertEQ(count, 0, "ForEach count should be 0");
}

function testForEachChangeValue() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    linkedList.forEach((value: double) => {
        value = TestData.testNumber1;
    });
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber1[i], "ForEach should keep all elements the same");
    }
}

function testIteratorEmptyContainer() {
    let linkedList = new LinkedList<double>();
    let count: int = 0;
    for (let item of linkedList) {
        count++;
    }
    arktest.assertEQ(count, 0, "Iterator count should be 0");
}

function testIteratorNonEmptyContainer() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: int = 0;
    let iter = linkedList.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    while (!temp.done) {
        arktest.assertEQ(linkedList.get(count++), temp.value, `Element should be equal`);
        temp = iter.next();
    }
    arktest.assertEQ(count, linkedList.length, "Iterator count should match linkedList length");
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}
