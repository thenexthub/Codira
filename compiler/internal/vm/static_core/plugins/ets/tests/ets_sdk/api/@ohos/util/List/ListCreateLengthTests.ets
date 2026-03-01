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

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("List create a single-type container", testCreateSingleTypeContainer);
    suite.addTest("List create a composite-type container", testCreateCompositeTypeContainer);
    suite.addTest("List length syncs with element count changes for single-type", testLengthSyncSingleType);
    suite.addTest("List length syncs with element count changes for multi-type", testLengthSyncMultiType);
    suite.addTest("List length with add large value", testLengthWithLargeValue);
    suite.addTest("Testing List constructor support dataType", testListConstructor0001);

    return suite.run();
}

class Temp {
    age: double = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumberLarge: double = 10000;
    static readonly testString1: string = "string";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
}

function initializeList(list: List<double>, sourceArray: Array<double>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

function testCreateSingleTypeContainer() {
    let list: List<double>;
    arktest.expectNoThrow(() => {list = new List<double>()});
}

function testCreateCompositeTypeContainer() {
    let list: List<double | string | boolean | null | object | undefined>;
    arktest.expectNoThrow(() => {list = new List<double | string | boolean | null | object | undefined>()});
}

function testLengthSyncSingleType() {
    let list = new List<double>();
    arktest.assertEQ(list.length, 0, "List length should be 0");
    initializeList(list, TestData.arrayForNumber1);
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length, "List length should be TestData.arrayForNumber1 length");
    list.remove(TestData.testNumber1);
    arktest.assertEQ(list.length, TestData.arrayForNumber1.length - TestData.testNumber1, "List length should - 1");
}

function testLengthSyncMultiType() {
    let list = new List<double | string | boolean | null | object | undefined>();
    arktest.assertEQ(list.length, 0, "List length should be 0");
    list.add(TestData.testNumber1);
    list.add(TestData.testString1);
    list.add(true);
    list.add(null);
    list.add(TestData.testObject1);
    list.add(undefined);
    arktest.assertEQ(list.length, TestData.testNumber6, "List length should be 6");
    list.remove(TestData.testNumber1);
    arktest.assertEQ(list.length, TestData.testNumber5, "List length should be 5");
}

function testLengthWithLargeValue() {
    let list = new List<double>();
    for (let i: int = 0; i < TestData.testNumberLarge; i++) {
        list.add(TestData.testNumber1);
    }
    arktest.assertEQ(list.length, TestData.testNumberLarge, "List length should be 10000 after adding elements");
}

function testListConstructor0001() {
    let list = new List<double | string | Array<double | string | boolean> | boolean | bigint |
    Int32Array | undefined | null | object>();
    arktest.assertEQ(list.length, 0);
}
