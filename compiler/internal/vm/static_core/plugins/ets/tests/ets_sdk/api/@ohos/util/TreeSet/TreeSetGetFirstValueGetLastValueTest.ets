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

import {TreeSet} from "@ohos.util.TreeSet";
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeSet GetFirstValue GetLastValue API tests")

    suite.addTest("TreeSet getFirstValue getLastValue first and last value",
        testGetFirstValueGetLastValueSingleElement)
    suite.addTest("TreeSet getFirstValue getLastValue first and last value from the treeSet with a data of undefined",
        testGetFirstValueGetLastValueUndefinedTreeSet)
    suite.addTest("TreeSet getFirstValue getLastValue first and last value from the null treeSet",
        testGetFirstValueGetLastValueEmptyTreeSet)
    suite.addTest("TreeSet getFirstValue getLastValue first and last value from the TreeSet with a set of data",
        testGetFirstValueGetLastValueMultipleElements)
    suite.addTest("TreeSet getFirstValue getLastValue first and last value by descending",
        testGetFirstValueGetLastValueDescendingOrder)
    suite.addTest("TreeSet getFirstValue getLastValue first and last object value",
        testGetFirstValueGetLastValueObjectValues)
    suite.addTest("TreeSet getFirstValue getLastValue first and last object value from the TreeSet with a set of data",
        testGetFirstValueGetLastValueMultipleObjectValues)

    return suite.run()
}

class MyObject extends Object {
    size: double;
    constructor(size: double) {
        super();
        this.size = size;
    }
}

function testGetFirstValueGetLastValueSingleElement() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    arktest.assertEQ(result1, "aa", "The treeSet should return the correct first value");
    arktest.assertEQ(result2, "bb", "The treeSet should return the correct last value");
    arktest.assertEQ(treeSet.length, 2, "The treeSet length should be 2")
}

function testGetFirstValueGetLastValueUndefinedTreeSet() {
    let treeSet: TreeSet<undefined> = new TreeSet<undefined>();
    treeSet.add(undefined);
    let result1 = treeSet.getFirstValue();
    arktest.assertEQ(result1, undefined);
    let result2 = treeSet.getLastValue();
    arktest.assertEQ(result2, undefined);
}

function testGetFirstValueGetLastValueEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    try {
        let result1 = treeSet.getFirstValue();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let result2 = treeSet.getLastValue();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0")
}

function testGetFirstValueGetLastValueMultipleElements() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    arktest.assertEQ(result1, "aa", "The treeSet should return the correct first value");
    arktest.assertEQ(result2, "aa", "The treeSet should return the correct last value");
    arktest.assertEQ(treeSet.length, 1, "The treeSet length should be 1")
}

function testGetFirstValueGetLastValueDescendingOrder() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    treeSet.add("dd");
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    arktest.assertEQ(result1, "dd", "The treeSet should return the correct first value");
    arktest.assertEQ(result2, "aa", "The treeSet should return the correct last value");
    arktest.assertEQ(treeSet.length, 4, "The treeSet length should be 4")
}

function testGetFirstValueGetLastValueObjectValues() {
    let treeSet: TreeSet<MyObject> =
        new TreeSet<MyObject>((firstValue: MyObject, secondValue: MyObject): double => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    let obj_sec: MyObject = new MyObject(2);
    let obj_trd: MyObject = new MyObject(3);
    treeSet.add(obj);
    treeSet.add(obj_sec);
    treeSet.add(obj_trd);
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    arktest.assertEQ(result1, obj, "The treeSet should return the correct first value");
    arktest.assertEQ(result2, obj_trd, "The treeSet should return the correct last value");
    arktest.assertEQ(treeSet.length, 3, "The treeSet length should be 3")
}

function testGetFirstValueGetLastValueMultipleObjectValues() {
    let treeSet: TreeSet<MyObject> =
        new TreeSet<MyObject>((firstValue: MyObject, secondValue: MyObject): double => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    treeSet.add(obj);
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    arktest.assertEQ(result1, obj, "The treeSet should return the correct first value");
    arktest.assertEQ(result2, obj, "The treeSet should return the correct last value");
    arktest.assertEQ(treeSet.length, 1, "The treeSet length should be 1")
}
