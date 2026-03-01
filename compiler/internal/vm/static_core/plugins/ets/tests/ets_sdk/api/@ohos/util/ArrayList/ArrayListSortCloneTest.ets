/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

    suite.addTest("Testing ArrayList sort by Ascending Order", testArrayListSort0001);
    suite.addTest("Testing ArrayList sort by Descending Order", testArrayListSort0002);
    suite.addTest("Testing ArrayList sort insert string", testArrayListSort0003);
    suite.addTest("Testing ArrayList sort insert 'A'", testArrayListSort0004);
    suite.addTest("Testing ArrayList subArrayList", testArrayListSubArrayList0001);
    suite.addTest("Testing ArrayList insert->add->subArrayList->clear", testArrayListClear0001);
    suite.addTest("Testing ArrayList insert->add->subArrayList->clone", testArrayListClone0001);
    suite.addTest("Testing ArrayList insert->add->subArrayList->clear->clone", testArrayListClone0002);

    return suite.run();
}

function testArrayListSort0001() {
    let arrayList = new ArrayList<double>();
    arrayList.add(1);
    arrayList.add(22);
    arrayList.add(3);
    arrayList.add(24);
    arrayList.add(5);
    arrayList.add(51);
    arrayList.add(8);
    arrayList.add(0);
    arrayList.sort((firstValue, secondValue) => (firstValue - secondValue).toInt());

    const expectedValues = [0, 1, 3, 5, 8, 22, 24, 51];
    for (let i: int = 0; i < expectedValues.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i], `The element at index ${i} should be ${expectedValues[i]}`);
    }
}

function testArrayListSort0002() {
    let arrayList = new ArrayList<double>();
    arrayList.add(1);
    arrayList.add(22);
    arrayList.add(3);
    arrayList.add(24);
    arrayList.add(5);
    arrayList.add(51);
    arrayList.add(8);
    arrayList.add(0);
    arrayList.sort((firstValue, secondValue) => (secondValue - firstValue).toInt());

    const expectedValues = [51, 24, 22, 8, 5, 3, 1, 0];
    for (let i: int = 0; i < expectedValues.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i], `The element at index ${i} should be ${expectedValues[i]}`);
    }
}

function testArrayListSort0003() {
    let arrayList = new ArrayList<double | string>();
    arrayList.add(1);
    arrayList.add(22);
    arrayList.add(3);
    arrayList.add(24);
    arrayList.add(5);
    arrayList.add(51);
    arrayList.add(8);
    arrayList.add(0);
    arrayList.add("zhang san");
    arrayList.sort();

    const expectedValues = [0, 1, 22, 24, 3, 5, 51, 8, "zhang san"];
    for (let i: int = 0; i < expectedValues.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i], `The element at index ${i} should be ${expectedValues[i]}`);
    }
}

function testArrayListSort0004() {
    let arrayList = new ArrayList<double | string>();
    arrayList.add(1);
    arrayList.add(22);
    arrayList.add(3);
    arrayList.add(24);
    arrayList.add(5);
    arrayList.add(51);
    arrayList.add(8);
    arrayList.add(0);
    arrayList.add("A");
    arrayList.sort();

    const expectedValues = [0, 1, 22, 24, 3, 5, 51, 8, "A"];
    for (let i: int = 0; i < expectedValues.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i], `The element at index ${i} should be ${expectedValues[i]}`);
    }
}

function testArrayListSubArrayList0001() {
    let arrayList = new ArrayList<double | string>();
    arrayList.add(1);
    arrayList.add(22);
    arrayList.add(3);
    arrayList.add(24);
    arrayList.add(5);
    arrayList.add(51);
    arrayList.add(8);
    arrayList.add(0);
    let subArrayList = arrayList.subArrayList(1, 5);

    const expectedValues = [22, 3, 24, 5];
    for (let i: int = 0; i < expectedValues.length; i++) {
        arktest.assertEQ(subArrayList[i], expectedValues[i],
            `The element at index ${i} in the subArrayList should be ${expectedValues[i]}`);
    }
}

function testArrayListClear0001() {
    let arrayList = new ArrayList<double | string>();
    arrayList.insert(2, 0);
    arrayList.add(1);
    arrayList.add(22);
    arrayList.subArrayList(1, 2);
    arrayList.clear();
    arktest.assertEQ(arrayList.length, 0, "The length of the ArrayList should be 0 after clearing all elements");
}

function testArrayListClone0001() {
    let arrayList = new ArrayList<double | string>();
    arrayList.insert(2, 0);
    arrayList.add(1);
    arrayList.add(22);
    arrayList.subArrayList(1, 2);
    let result = arrayList.clone();
    arktest.assertEQ(arrayList.length, result.length, "The length of the cloned ArrayList should match the original ArrayList");
}

function testArrayListClone0002() {
    let arrayList = new ArrayList<double | string>();
    arrayList.insert(2, 0);
    arrayList.add(1);
    arrayList.add(22);
    arrayList.subArrayList(1, 2);
    arrayList.clear();
    let result = arrayList.clone();
    arktest.assertEQ(result.length, 0, "The length of the cloned ArrayList should be 0 after clearing the original ArrayList");
}
