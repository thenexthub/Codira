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
    const suite = new arktest.ArkTestsuite("TreeMap Remove Replace Clear API tests")

    suite.addTest("TreeMap remove key", testRemoveExistingKey)
    suite.addTest("TreeMap remove root", testRemoveRootEntry)
    suite.addTest("TreeMap remove not set key", testRemoveNonExistingKey)
    suite.addTest("TreeMap remove key from the null treeMap", testRemoveKeyFromEmptyTreeMap)
    suite.addTest("TreeMap replace value", testReplaceExistingValue)
    suite.addTest("TreeMap replace not set key", testReplaceNonExistingKey)
    suite.addTest("TreeMap replace value in the null treeMap", testReplaceValueInEmptyTreeMap)
    suite.addTest("TreeMap replace empty string when treeMap is empty", testReplaceEmptyStringInEmptyTreeMap)
    suite.addTest("TreeMap clear data", testClearData)
    suite.addTest("TreeMap clear the null treeMap", testClearEmptyTreeMap)
    suite.addTest("TreeMap set data after clear the treeMap", testSetDataAfterClearTreeMap)

    return suite.run()
}

function testRemoveExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.remove("aa");
    arktest.assertEQ(result, "123", "The treeMap should remove the element and return its value");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testRemoveRootEntry() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "a123");
    treeMap.set("bb", "b123");
    treeMap.set("cc", "c123");

    let result = treeMap.remove("bb");
    arktest.assertEQ(result, "b123", "The treeMap should remove the element and return its value");
    arktest.assertEQ(treeMap.length, 2, "The treeMap length should be 0");
}

function testRemoveNonExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.remove("bb");
    arktest.assertEQ(result, undefined, "The treeMap should return undefined for a key that is not set");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testRemoveKeyFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.remove("aa");
    arktest.assertEQ(result, undefined, "The treeMap should return undefined for a key in an empty map");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testReplaceExistingValue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result1 = treeMap.replace("aa", "456");
    let result2 = treeMap.get("aa");
    arktest.assertEQ(result1, true, "The treeMap should replace the value for the key");
    arktest.assertEQ(result2, "456", "The treeMap should return the new value for the key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testReplaceNonExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result1 = treeMap.replace("bb", "456");
    let result2 = treeMap.get("aa");
    arktest.assertEQ(result1, false, "The treeMap should return false for a key that is not set");
    arktest.assertEQ(result2, "123", "The treeMap should return the new value for the key");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testReplaceValueInEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.replace("aa", "789");
    arktest.assertEQ(result, false, "The treeMap should return false for a key in an empty map");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testReplaceEmptyStringInEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.replace("", "789");
    arktest.assertEQ(result, false, "The treeMap should return false for a key in an empty map");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testClearData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.isEmpty();
    treeMap.clear();
    arktest.assertEQ(result, false, "The treeMap should not be empty");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testClearEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testSetDataAfterClearTreeMap() {
    let treeMap: TreeMap<string, double> =
        new TreeMap<string, double>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue)
        });
    treeMap.set("aa", 123);
    treeMap.set("bb", 456);
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
    treeMap.set("cc", 111);
    treeMap.set("dd", 222);
    treeMap.set("ee", 333);
    arktest.assertEQ(treeMap.length, 3, "The treeMap length should be 3");
    let iterator = treeMap.$_iterator();
    let str = "";
    for (let i: int = 0; i < 3; i++) {
        str += iterator.next().value!.toString();
        if (i < 2) {
            str += ",";
        }
    }
    let strAll = "ee,333,dd,222,cc,111";
    arktest.assertEQ(str, strAll, "The treeMap should return the correct iterator");
}
