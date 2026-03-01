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
    const suite = new arktest.ArkTestsuite("TreeMap HasKey HasValue SetAll ForEach API tests")

    suite.addTest("TreeMap hasKey hasValue key and value", testHasKeyAndValueExisting)
    suite.addTest("TreeMap hasKey hasValue not set key and value", testHasKeyAndValueNonExisting)
    suite.addTest("TreeMap hasKey hasValue key and value in the null TreeMap", testHasKeyAndValueEmptyTreeMap)
    suite.addTest("TreeMap setAlldata from treeMap2 to treeMap1 when treeMap2 is empty", testSetAllFromEmptyTreeMap)
    suite.addTest("TreeMap setAlldata from treeMap2 to treeMap1", testSetAllFromTreeMap)
    suite.addTest("TreeMap forEach the treeMap with no reference", testForEachNoReference)
    suite.addTest("TreeMap forEach the treeMap with reference is value", testForEachValueReference)
    suite.addTest("TreeMap forEach the treeMap with reference are value and key", testForEachValueKeyReference)
    suite.addTest("TreeMap forEach the treeMap with three references", testForEachThreeReferences)

    return suite.run()
}

function testHasKeyAndValueExisting() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let hasK = treeMap.hasKey("aa");
    let hasV = treeMap.hasValue("123");
    arktest.assertEQ(hasK, true, "The treeMap should contain the key");
    arktest.assertEQ(hasV, true, "The treeMap should contain the value");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testHasKeyAndValueNonExisting() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let hasK = treeMap.hasKey("bb");
    let hasV = treeMap.hasValue("456");
    arktest.assertEQ(hasK, false, "The treeMap should not contain the key that is not set");
    arktest.assertEQ(hasV, false, "The treeMap should not contain the value that is not set");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testHasKeyAndValueEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let hasK = treeMap.hasKey("aa");
    let hasV = treeMap.hasValue("123");
    arktest.assertEQ(hasK, false, "The treeMap should not contain any key when the treeMap is empty");
    arktest.assertEQ(hasV, false, "The treeMap should not contain any value when the treeMap is empty");
    arktest.assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testSetAllFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let treeMapp: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMapp.setAll(treeMap);
    arktest.assertEQ(treeMap.length, treeMapp.length, "The treeMap should set all elements from another treeMap");
    arktest.assertEQ(treeMap.length, 1, "The treeMap length should be 1");
    arktest.assertEQ(treeMapp.length, 1, "The treeMapp length should be 1");
}

function testSetAllFromTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("HIG", "456");
    treeMap.set("DEF", "123");
    treeMap.set("XYZ", "789");
    let newMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    newMap.set("demo", '123');
    newMap.setAll(treeMap);
    arktest.assertEQ(treeMap.length, 3, "The treeMap length should be 1");
    arktest.assertEQ(newMap.length, 4, "The newMap should set all elements from another treeMap");
}

function testForEachNoReference() {
    let treeMap: TreeMap<string, double> =
        new TreeMap<string, double>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", 123);
    treeMap.set("bb", 456);
    let flag = false;
    treeMap.forEach(() => {
        if (treeMap.get("aa") == 123 && treeMap.get("bb") == 456) {
            flag = true
        }
        arktest.assertEQ(flag, true, "The treeMap should iterate over all elements");
    });
}

function testForEachValueReference() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let flag = false;
    treeMap.forEach((value: string): void => {
        flag = treeMap.get("aa") === "123" && treeMap.get("bb") === "456";
    });
    arktest.assertEQ(flag, true, "The treeMap should iterate over all elements and check values");
}

function testForEachValueKeyReference() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let flag = false;
    treeMap.forEach((value: string, key: string): void => {
        flag = treeMap.get(key) === value;
        arktest.assertEQ(flag, true, "The treeMap should iterate over all elements and check keys and values");
    });
}

function testForEachThreeReferences() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let flag = false;
    treeMap.forEach((value: string, key: string, treeMap: TreeMap<string, string>): void => {
        flag = treeMap.get(key) === value;
        arktest.assertEQ(flag, true, "The treeMap should iterate over all elements and check keys, values, and the map itself");
    });
}
