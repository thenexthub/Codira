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
    const suite = new arktest.ArkTestsuite("TreeMap Set Get API tests")

    suite.addTest("TreeMap set data", testSetData)
    suite.addTest("TreeMap set data with same key and different values", testSetDataWithSameKeyDifferentValues)
    suite.addTest("TreeMap set empty string data", testSetEmptyStringData)
    suite.addTest("TreeMap loop set data", testLoopSetData)
    suite.addTest("TreeMap get data", testGetData)
    suite.addTest("TreeMap get not set data", testGetNonExistingData)
    suite.addTest("TreeMap get data from the null treeMap", testGetDataFromEmptyTreeMap)
    suite.addTest("TreeMap get empty string data when treeMap is empty", testGetEmptyStringDataFromEmptyTreeMap)

    return suite.run()
}

class MyObject extends Object {
    size: double;
    constructor(size: double) {
        super();
        this.size = size;
    }
}

function testSetData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("a", "123");
    let result = treeMap.get("a");
    arktest.assertEQ(result, "123", "The treeMap should get the value by key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap Length should be 1");
}

function testSetDataWithSameKeyDifferentValues() {
    let treeMap: TreeMap<MyObject, string> =
        new TreeMap<MyObject, string>((firstValue: MyObject, secondValue: MyObject): double => {
            return secondValue.size - firstValue.size
        });
    let obj: MyObject = new MyObject(1);
    let obj2: MyObject = new MyObject(1);
    treeMap.set(obj, "123");
    treeMap.set(obj2, "456");
    let result = treeMap.get(obj);
    arktest.assertEQ(result, "456", "The treeMap should get the value by key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap Length should be 1");
}

function testSetEmptyStringData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("", "123");
    let length = treeMap.length;
    let result = treeMap.get("");
    arktest.assertEQ(result, "123", "The treeMap should get the value by key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap Length should be 1");
}

function testLoopSetData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let arr = 'abc';
    for (let i: int = 0; i < 1000; i++) {
        treeMap.set(arr + i, "123");
    }
    let length = treeMap.length;
    for (let k = 0; k < 1000; k++) {
        let result = treeMap.get(arr + k);
        arktest.assertEQ(result, "123", "The treeMap should get the value by key");
    }
    arktest.assertEQ(length, 1000, "The treeMap Length should be 1");

}

function testGetData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.get("aa");
    arktest.assertEQ(result, "123", "The treeMap should return the correct value for the key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testGetNonExistingData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.get("bb");
    arktest.assertEQ(result, undefined, "The treeMap should return undefined for the key that is not set");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testGetDataFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.get("aa");
    arktest.assertEQ(result, undefined, "The treeMap should return undefined for the key in an empty map");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testGetEmptyStringDataFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.get("");
    arktest.assertEQ(result, undefined, "The treeMap should return undefined for the key in an empty map");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}
