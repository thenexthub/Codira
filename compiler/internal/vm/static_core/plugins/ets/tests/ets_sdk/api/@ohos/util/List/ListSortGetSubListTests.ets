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

import {List} from '@ohos.util.List';
import {BusinessError} from '@ohos.base';

const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite('LinkedList API tests');

    suite.addTest('List sort with different types in ascending order', testSortDifferentTypesAscending);
    suite.addTest('List sort with object in ascending order', testSortObjectAscending);
    suite.addTest('List sort with different types in descending order', testSortDifferentTypesDescending);
    suite.addTest('List sort with object in descending order', testSortObjectDescending);
    suite.addTest('List getSubList', testGetSubList);
    suite.addTest('List getSubList with start index out of range', testGetSubListStartIndexOutOfRange);
    suite.addTest('List getSubList with end index out of range', testGetSubListEndIndexOutOfRange);
    suite.addTest('List getSubList with end index smaller than start index',
        testGetSubListEndIndexSmallerThanStartIndex);
    suite.addTest('List getSubList with change getSubList', testGetSubListWithChange);
    suite.addTest('List getSubList with empty container', testGetSubListEmptyContainer);
    suite.addTest('List getSubList with start index = 0 and end index = length', testGetSubListFullRange);
    suite.addTest('List getSubList with start index = end index', testGetSubListWithEqualStartAndEndIndex);

    return suite.run();
}

function initializeList<T>(list: List<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

class Temp {
    public age: double = 0;
    public name: string = '';
}

class TestData {
    public static readonly testNumber1: int = 1;
    public static readonly testNumber3: int = 3;
    public static readonly testNumber5: int = 5;
    public static readonly testNumber8: int = 8;
    public static readonly testObject1: Temp = { age: 10, name: 'zhang' };
    public static readonly testObject2: Temp = { age: 8, name: 'li' };
    public static readonly testObject3: Temp = { age: 12, name: 'wang' };
    public static readonly testObject4: Temp = { age: 1, name: 'liu' };
    public static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    public static readonly arrayForNumber = Array.from<double>(TestData.forNumber);
    public static readonly forString: String[] = ['ssss', '2', '%', 'Z'];
    public static readonly arrayForString = Array.from<String>(TestData.forString);
    public static readonly forBoolean: Boolean[] = [true, false, false, true];
    public static readonly arrayForBoolean = Array.from<boolean>(TestData.forBoolean);
    public static readonly forObject: Object[] = [[1], [2, 3, 4], [1, 2], [5]];
    public static readonly arrayForObject = Array.from<Object>(TestData.forObject);
    public static readonly result1: Number[] = [5, 4, 3, 2, 1];
    public static readonly arrayResult1 = Array.from<double>(TestData.result1);
    public static readonly result6: Number[] = [2, 3, 4];
    public static readonly arrayResult6 = Array.from<double>(TestData.result6);
    public static readonly result7: Number[] = [2, 3, 4, 5];
    public static readonly arrayResult7 = Array.from<double>(TestData.result7);
}

function testSortDifferentTypesAscending() {
    let list1 = new List<double>();
    initializeList(list1, TestData.arrayResult1);
    list1.sort((a: double, b: double) => a - b);
    for (let i: int = 0; i < list1.length; i++) {
        arktest.assertEQ(list1.get(i), TestData.arrayForNumber[i], `Element at index ${i} should match`);
    }

    let list2 = new List<string>();
    initializeList(list2, TestData.arrayForString);
    let array2 = list2.convertToArray();
    list2.sort((a: string, b: string) => a.compareTo(b));
    array2.sort((a: string, b: string) => a.compareTo(b));
    for (let i: int = 0; i < list2.length; i++) {
        arktest.assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }

    let list3 = new List<boolean>();
    initializeList(list3, TestData.arrayForBoolean);
    let array3 = list3.convertToArray();
    list3.sort((a: boolean, b: boolean) => (a ? 1 : 0) - (b ? 1 : 0));
    array3.sort((a: boolean, b: boolean) => (a ? 1 : 0) - (b ? 1 : 0));
    for (let i: int = 0; i < list3.length; i++) {
        arktest.assertEQ(list3.get(i), array3[i], `Element at index ${i} should match`);
    }
}

let arrStrCmp = (a: object, b: object): int => {
    let sa = a.toString();
    let sb = b.toString();
    return sb.compareTo(sa);
}

let listStrCmp = (a: object, b: object): double => {
    let sa = a.toString();
    let sb = b.toString();
    return sb.compareTo(sa);
}

function testSortObjectAscending() {
    let list1 = new List<object>();
    initializeList(list1, TestData.arrayForObject);
    let array1 = list1.convertToArray();
    list1.sort(listStrCmp);
    array1.sort(arrStrCmp);
    for (let i: int = 0; i < list1.length; i++) {
        arktest.assertEQ(list1.get(i), array1[i], `Element at index ${i} should match`);
    }

    let list2 = new List<Temp>();
    list2.add(TestData.testObject1);
    list2.add(TestData.testObject2);
    list2.add(TestData.testObject3);
    list2.add(TestData.testObject4);
    let array2 = list2.convertToArray();
    list2.sort((a: Temp, b: Temp) => (a.age - b.age).toInt());
    array2.sort((a: Temp, b: Temp) => (a.age - b.age).toInt());
    for (let i: int = 0; i < list2.length; i++) {
        arktest.assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }
}

function testSortDifferentTypesDescending() {
    let list1 = new List<double>();
    initializeList(list1, TestData.arrayForNumber);
    list1.sort((a: double, b: double) => b - a);
    for (let i: int = 0; i < list1.length; i++) {
        arktest.assertEQ(list1.get(i), TestData.arrayResult1[i], `Element at index ${i} should match`);
    }

    let list2 = new List<string>();
    initializeList(list2, TestData.arrayForString);
    let array2 = list2.convertToArray();
    list2.sort((a: string, b: string) => b.compareTo(a));
    array2.sort((a: string, b: string) => b.compareTo(a));
    for (let i: int = 0; i < list2.length; i++) {
        arktest.assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }

    let list3 = new List<boolean>();
    initializeList(list3, TestData.arrayForBoolean);
    let array3 = list3.convertToArray();
    list3.sort((a: boolean, b: boolean) => (b ? 1 : 0) - (a ? 1 : 0));
    array3.sort((a: boolean, b: boolean) => (b ? 1 : 0) - (a ? 1 : 0));
    for (let i: int = 0; i < list3.length; i++) {
        arktest.assertEQ(list3.get(i), array3[i], `Element at index ${i} should match`);
    }
}

function testSortObjectDescending() {
    let list1 = new List<object>();
    initializeList(list1, TestData.arrayForObject);
    let array1 = list1.convertToArray();
    list1.sort(listStrCmp);
    array1.sort(arrStrCmp);
    for (let i: int = 0; i < list1.length; i++) {
        arktest.assertEQ(list1.get(i), array1[i], `Element at index ${i} should match`);
    }

    let list2 = new List<Temp>();
    list2.add(TestData.testObject1);
    list2.add(TestData.testObject2);
    list2.add(TestData.testObject3);
    list2.add(TestData.testObject4);
    let array2 = list2.convertToArray();
    list2.sort((a: Temp, b: Temp) => (b.age - a.age).toInt());
    array2.sort((a: Temp, b: Temp) => (b.age - a.age).toInt());
    for (let i: int = 0; i < list2.length; i++) {
        arktest.assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }
}

function testGetSubList() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber);
    let getSubList = list.getSubList(TestData.testNumber1, TestData.testNumber5 - TestData.testNumber1);
    for (let i: int = 0; i < list.length; i++) {
        arktest.assertEQ(list.get(i), TestData.arrayForNumber[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < getSubList.length; i++) {
        arktest.assertEQ(getSubList.get(i), TestData.arrayResult6[i], `Element at index ${i} should match`);
    }
}

function testGetSubListStartIndexOutOfRange() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "fromIndex" is out of range. It must be >= 0 && <= 4. Received value is: -1' &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.getSubList(-TestData.testNumber1, TestData.testNumber5)}, exceptionCheck);
}

function testGetSubListEndIndexOutOfRange() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "toIndex" is out of range. It must be > 0 && <= 5. Received value is: 8' &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.getSubList(TestData.testNumber1, TestData.testNumber8)}, exceptionCheck);
}

function testGetSubListEndIndexSmallerThanStartIndex() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "fromIndex" is out of range. It must be >= 0 && <= 0. Received value is: 5' &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.getSubList(TestData.testNumber5, TestData.testNumber1)}, exceptionCheck);
}

function testGetSubListWithChange() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber);
    let getSubList = list.getSubList(TestData.testNumber1, TestData.testNumber5 - TestData.testNumber1);
    getSubList.add(TestData.testNumber5);
    for (let i: int = 0; i < list.length; i++) {
        arktest.assertEQ(list.get(i), TestData.arrayForNumber[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < getSubList.length; i++) {
        arktest.assertEQ(getSubList.get(i), TestData.arrayResult7[i], `Element at index ${i} should match`);
    }
}

function testGetSubListEmptyContainer() {
    let list = new List<double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message == 'Container is empty';
        }
        return false;
    }

    arktest.expectThrow(() => {list.getSubList(TestData.testNumber1, TestData.testNumber3)}, exceptionCheck);
}

function testGetSubListFullRange() {
    let list = new List<double>();
    initializeList(list, TestData.arrayForNumber);
    let getSubList = list.getSubList(0, TestData.testNumber5);
    for (let i: int = 0; i < getSubList.length; i++) {
        arktest.assertEQ(getSubList.get(i), list.get(i), `Element at index ${i} should match`);
    }
    arktest.assertEQ(getSubList.length, TestData.testNumber5);
}

function testGetSubListWithEqualStartAndEndIndex() {
    let list = new List<double>();
    initializeList(list, TestData.forNumber);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == 'BusinessError' && e.message ==
                'The value of "fromIndex" is out of range. It must be >= 0 && <= 0. Received value is: 1' &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }

    arktest.expectThrow(() => {list.getSubList(TestData.testNumber1, TestData.testNumber1)}, exceptionCheck);
}
