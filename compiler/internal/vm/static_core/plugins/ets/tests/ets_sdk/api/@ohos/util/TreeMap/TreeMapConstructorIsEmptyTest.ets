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

import TreeMap from "@ohos.util.TreeMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeMap Constructor IsEmpty API tests")

    suite.addTest("TreeMap construct default empty string treeMap", testConstructDefaultEmptyStringTreeMap)
    suite.addTest("TreeMap construct default empty double treeMap", testConstructDefaultEmptyNumberTreeMap)
    suite.addTest("TreeMap construct default empty object treeMap", testConstructDefaultEmptyObjectTreeMap)
    suite.addTest("TreeMap construct no comparator treeMap", testConstructNoComparatorTreeMap)
    suite.addTest("TreeMap isEmpty is true", testIsEmptyReturnsTrue)
    suite.addTest("TreeMap isEmpty is false", testIsEmptyReturnsFalse)
    suite.addTest("TreeMap isEmpty is true after remove data", testIsEmptyReturnsTrueAfterRemove)

    return suite.run()
}

class MyObject extends Object {
    size: double;
    constructor(size: double) {
        super();
        this.size = size;
    }
}

function testConstructDefaultEmptyStringTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    arktest.assertTrue(treeMap instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testConstructDefaultEmptyNumberTreeMap() {
    let treeMap: TreeMap<double, double> =
        new TreeMap<double, double>((firstValue: double, secondValue: double): double => {
            return secondValue - firstValue
        });
    treeMap.set(111, 999)
    treeMap.set(222, 888)
    treeMap.set(333, 777)
    arktest.assertTrue(treeMap instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    arktest.assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testConstructDefaultEmptyObjectTreeMap() {
    let treeMapo: TreeMap<MyObject, MyObject> =
        new TreeMap<MyObject, MyObject>((firstValue: MyObject, secondValue: MyObject): double => {
            return secondValue.size - firstValue.size
        });
    let obj: MyObject = new MyObject(1);
    let obj2: MyObject = new MyObject(2);
    treeMapo.set(obj, obj2);
    arktest.assertTrue(treeMapo instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    arktest.assertEQ(treeMapo.length, 1, "The treeMap length should be 1");
}

function testConstructNoComparatorTreeMap() {
    let treeMap: TreeMap<double, double> = new TreeMap<double, double>();
    treeMap.set(111, 999)
    treeMap.set(222, 888)
    treeMap.set(333, 777)
    arktest.assertTrue(treeMap instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    arktest.assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testIsEmptyReturnsTrue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.isEmpty();
    arktest.assertEQ(result, true, "The treeMap should be empty");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testIsEmptyReturnsFalse() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.isEmpty();
    arktest.assertEQ(result, false, "The treeMap should not be empty");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testIsEmptyReturnsTrueAfterRemove() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result1 = treeMap.isEmpty();
    treeMap.remove("aa");
    let result2 = treeMap.isEmpty();
    arktest.assertEQ(result1, false, "The treeMap should not be empty");
    arktest.assertEQ(result2, true, "The treeMap should be empty");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}
