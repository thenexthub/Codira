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

function main() {
    const suite = new arktest.ArkTestsuite('LinkedList Create, Length, and Add API tests');

    suite.addTest('LinkedList create a single-type container', testCreateSingleTypeContainer);
    suite.addTest('LinkedList create a composite-type container', testCreateCompositeTypeContainer);
    suite.addTest('LinkedList length syncs with element count changes for single-type', testLengthSyncSingleType);
    suite.addTest('LinkedList length syncs with element count changes for multi-type', testLengthSyncMultiType);
    suite.addTest('LinkedList length with add large value', testLengthWithLargeValue);
    suite.addTest('LinkedList add elements of multiple types to the container', testAddMultipleTypes);
    suite.addTest('LinkedList add special characters to the container', testAddSpecialCharacters);
    suite.addTest('LinkedList add an element of the declared type to the first position', testAddFirstDeclaredType);
    suite.addTest('LinkedList add null to the first position', testAddFirstNull);
    suite.addTest('LinkedList add special characters to the first', testAddFirstSpecialCharacters);

    return suite.run();
}

class Temp {
    public age: double = 0;
    public name: string = '';
}

class TestData {
    public static readonly testNumber1: int = 1;
    public static readonly testNumber2: int = 2;
    public static readonly testNumber3: int = 3;
    public static readonly testNumber4: int = 4;
    public static readonly testNumber5: int = 5;
    public static readonly testNumber6: int = 6;
    public static readonly testNumber1w: int = 10000;
    public static readonly testString1: string = 'string';
    public static readonly testString2: string = '!"#$%\'()+,-./:;<=>?@[\\]^_`{|}~';
    public static readonly testObject1: Temp = { age: 10, name: 'zhang' };
    public static readonly forNumber1: Number[] = [
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
    ];
    public static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
}

function testCreateSingleTypeContainer() {
    let linkedList: LinkedList<double>;
    arktest.expectNoThrow(() => {linkedList = new LinkedList<double>()});
}

function testCreateCompositeTypeContainer() {
    let linkedList: LinkedList<double | string | boolean | null | object | undefined>;
    arktest.expectNoThrow(() => {linkedList = new LinkedList<double | string | boolean | null | object | undefined>()});
}

function testLengthSyncSingleType() {
    let linkedList = new LinkedList<double>();
    arktest.assertEQ(linkedList.length, 0, 'LinkedList length should be 0');
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length,
        'LinkedList length should be TestData.arrayForNumber1 length');
    linkedList.remove(TestData.testNumber1);
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1, 'LinkedList length should - 1');
}

function testLengthSyncMultiType() {
    let linkedList = new LinkedList<double | string | boolean | null | object | undefined>();
    arktest.assertEQ(linkedList.length, 0, 'LinkedList length should be 0');
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testString1);
    linkedList.add(true);
    linkedList.add(null);
    linkedList.add(TestData.testObject1);
    linkedList.add(undefined);
    arktest.assertEQ(linkedList.length, TestData.testNumber6, 'LinkedList length should be 6');
    linkedList.remove(TestData.testNumber1);
    arktest.assertEQ(linkedList.length, TestData.testNumber5, 'LinkedList length should be 5');
}

function testLengthWithLargeValue() {
    let linkedList = new LinkedList<double>();
    for (let i: int = 0; i < TestData.testNumber1w; i++) {
        linkedList.add(TestData.testNumber1);
    }
    arktest.assertEQ(linkedList.length, TestData.testNumber1w, 'LinkedList length should be 10000 after adding elements');
}

function testAddMultipleTypes() {
    let linkedList = new LinkedList<double | string | boolean | null | object | undefined>();
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testString1);
    linkedList.add(true);
    linkedList.add(null);
    linkedList.add(TestData.testObject1);
    linkedList.add(undefined);

    arktest.assertEQ(linkedList.length, TestData.testNumber6, 'LinkedList length should be 6');
    arktest.assertEQ(linkedList.get(0), TestData.testNumber1, 'Element at index 0 should match');
    arktest.assertEQ(linkedList.get(TestData.testNumber1), TestData.testString1, 'Element at index 1 should match');
    arktest.assertEQ(linkedList.get(TestData.testNumber2), true, 'Element at index 2 should match');
    arktest.assertEQ(linkedList.get(TestData.testNumber3), null, 'Element at index 3 should match');
    arktest.assertEQ(linkedList.get(TestData.testNumber4), TestData.testObject1, 'Element at index 4 should match');
    arktest.assertEQ(linkedList.get(TestData.testNumber5), undefined, 'Element at index 5 should match');
}

function testAddSpecialCharacters() {
    let linkedList = new LinkedList<string>();
    linkedList.add(TestData.testString2);
    arktest.assertEQ(linkedList.get(0), TestData.testString2, 'Add successful');
    arktest.assertEQ(linkedList.length, TestData.testNumber1, 'LinkedList length should be 1');
}

function testAddFirstDeclaredType() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    linkedList.addFirst(TestData.testNumber4);
    arktest.assertEQ(linkedList.get(0), TestData.testNumber4, 'First element should be 4');
    for (let i: int = 0; i < TestData.arrayForNumber1.length; i++) {
        arktest.assertEQ(linkedList.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length + TestData.testNumber1, 'LinkedList length should + 1');
}

function testAddFirstNull() {
    let linkedList = new LinkedList<double | null>();
    for (let i: int = 0; i < TestData.arrayForNumber1.length; i++) {
        linkedList.add(TestData.arrayForNumber1[i]);
    }
    linkedList.addFirst(null);
    arktest.assertEQ(linkedList.get(0), null, 'First element should be null');
    for (let i: int = 0; i < TestData.arrayForNumber1.length; i++) {
        arktest.assertEQ(linkedList.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], 'Element should match');
    }
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length + TestData.testNumber1,
        'LinkedList length should be + 1');
}

function testAddFirstSpecialCharacters() {
    let linkedList = new LinkedList<string>();
    linkedList.addFirst(TestData.testString2);
    arktest.assertEQ(linkedList.get(0), TestData.testString2, 'Add successful');
    arktest.assertEQ(linkedList.length, TestData.testNumber1, 'LinkedList length should be 1');
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}
