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

import {LinkedList} from '@ohos.util.LinkedList';
import {BusinessError} from '@ohos.base';

const OutOfBoundsErrorCodeId: int = 10200001;
const EmptyContainerErrorCodeId: int = 10200010;

function main() {
    const suite = new arktest.ArkTestsuite('LinkedList API tests');

    suite.addTest('LinkedList removeByIndex with an index within the container\'s length range',
        testRemoveByIndexWithinRange);
    suite.addTest('LinkedList removeByIndex with an index outside the container\'s length range',
        testRemoveByIndexOutsideRange);
    suite.addTest('LinkedList removeByIndex with removing undefined', testRemoveByIndexUndefined);
    suite.addTest('LinkedList removeByIndex with an index at the container\'s length range',
        testRemoveByIndexAtBoundary);
    suite.addTest('LinkedList remove the first element from an empty container', testRemoveFirstFromEmptyContainer);
    suite.addTest('LinkedList remove the first element from a non-empty container',
        testRemoveFirstFromNonEmptyContainer);
    suite.addTest('LinkedList remove the last element from an empty container', testRemoveLastFromEmptyContainer);
    suite.addTest('LinkedList remove the last element from a non-empty container', testRemoveLastFromNonEmptyContainer);
    suite.addTest('LinkedList remove an unique element', testRemoveUniqueElement);
    suite.addTest('LinkedList remove an inexistent element', testRemoveInexistentElement);
    suite.addTest('LinkedList remove a repeated element', testRemoveRepeatedElement);

    return suite.run();
}

class Temp {
    public age: double = 0;
    public name: string = '';
}

class TestData {
    public static readonly testNumber1: int = 1;
    public static readonly testNumber2: int = 2;
    public static readonly testNumber5: int = 5;
    public static readonly testNumber6: int = 6;
    public static readonly testNumber20: int = 20;
    public static readonly testNumber1w: int = 10000;
    public static readonly forNumber1: Number[] = [
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
    ];
    public static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
    public static readonly forNumber2: Number[] = [1, 2, 3, 4, 5];
    public static readonly arrayForNumber2 = Array.from<double>(TestData.forNumber2);
    public static readonly forNumber3: Number[] = [1, 2, 4, 2, 3];
    public static readonly arrayForNumber3 = Array.from<double>(TestData.forNumber3);
}

function testRemoveByIndexWithinRange() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.removeByIndex(TestData.testNumber1), TestData.testNumber1, 'Removed element should be 1');
    arktest.assertEQ(linkedList.get(0), 0, 'Element at index 0 should be 0');
    for (let i: int = TestData.testNumber1; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1,
        'LinkedList length should be - 1');
}

function testRemoveByIndexOutsideRange() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "index" is out of range. It must be >= 0 && <= 20. Received value is: 10000' &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    let exceptionCheck1 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "index" is out of range. It must be >= 0 && <= 20. Received value is: -6' &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {linkedList.removeByIndex(TestData.testNumber1w.toInt())}, exceptionCheck);
    arktest.expectThrow(() => {linkedList.removeByIndex(-TestData.testNumber6.toInt())}, exceptionCheck1);
}

function testRemoveByIndexUndefined() {
    let linkedList = new LinkedList<double | string | boolean | null | object | undefined>();
    linkedList.add(undefined);
    linkedList.add(TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.removeByIndex(0), undefined, 'The result should be undefined');
}

function testRemoveByIndexAtBoundary() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(
        linkedList.removeByIndex(TestData.arrayForNumber1.length - TestData.testNumber1),
        TestData.testNumber20,
        'Removed element should - 1'
    );
    arktest.assertEQ(linkedList.removeByIndex(0), 0, 'Removed element should be 0');
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber2,
        'LinkedList length should be - 2');
}

function testRemoveFirstFromEmptyContainer() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.removeFirst(), 0, 'Removed first element should be 0');
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1, 'LinkedList length should - 1');
}

function testRemoveFirstFromNonEmptyContainer() {
    let linkedList = new LinkedList<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message == 'Container is empty' &&
                e.code == EmptyContainerErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {linkedList.removeFirst()}, exceptionCheck);
}

function testRemoveLastFromEmptyContainer() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.removeLast(), TestData.testNumber20, 'Removed first element should be 20');
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber1[i], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1, 'LinkedList length should - 1');
}

function testRemoveLastFromNonEmptyContainer() {
    let linkedList = new LinkedList<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message == 'Container is empty' &&
                e.code == EmptyContainerErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {linkedList.removeLast()}, exceptionCheck);
}

function testRemoveUniqueElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    arktest.assertTrue(linkedList.remove(TestData.testNumber1), 'Element 1 should be removed');
    arktest.assertTrue(linkedList.remove(TestData.testNumber5), 'Element 5 should be removed');
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber2[i + TestData.testNumber1], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber2.length - TestData.testNumber2, 'LinkedList length should - 2');
}

function testRemoveInexistentElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    arktest.assertFalse(linkedList.remove(TestData.testNumber6), 'Element 6 is inexistent');
    for (let i: int = 0; i < linkedList.length; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber2[i], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber2.length, 'LinkedList length should should not change');
}

function testRemoveRepeatedElement() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    arktest.assertTrue(linkedList.remove(TestData.testNumber2), 'Element 2 should be removed');
    arktest.assertEQ(linkedList.get(0), TestData.testNumber1, 'First element should be 1');
    for (let i: int = TestData.testNumber1; i < linkedList.length - TestData.testNumber1; i++) {
        arktest.assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, 'LinkedList length should - 1');
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}
