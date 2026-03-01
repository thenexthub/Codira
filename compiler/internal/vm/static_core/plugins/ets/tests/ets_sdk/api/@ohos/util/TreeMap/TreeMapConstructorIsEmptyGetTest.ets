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
    const suite = new arktest.ArkTestsuite("TreeMap API Constructor IsEmpty Get tests")

    suite.addTest("Test TreeMap constructor (no comparator) and set() with double keys and check length.",
        testConstructorNoComparatorSetNumberKeys)
    suite.addTest("Test TreeMap constructor with custom string comparator and set() with string keys and check length.",
        testConstructorCustomStringComparatorSetStringKeys)
    suite.addTest("Test TreeMap constructor with custom object comparator and set() with custom object keys.",
        testConstructorCustomObjectComparatorSetCustomObjectKeys)
    suite.addTest("Test TreeMap constructor (no comparator) and set() with custom object keys, then check length.",
        testConstructorNoComparatorSetCustomObjectKeys)
    suite.addTest("Test TreeMap isEmpty() on a newly created map.", testIsEmptyOnNewlyCreatedMap)
    suite.addTest("Test TreeMap isEmpty() after adding an element.", testIsEmptyAfterAddingElement)
    suite.addTest("Test TreeMap isEmpty() after adding and then removing an element.",
        testIsEmptyAfterAddingAndRemovingElement)
    suite.addTest("Test TreeMap get() with an empty string key on an empty map.", testGetWithEmptyStringKeyOnEmptyMap)
    suite.addTest("Test TreeMap get() with a non-existing key on an empty map.", testGetWithNonExistingKeyOnEmptyMap)
    suite.addTest("Test TreeMap get() with an existing key.", testGetWithExistingKey)
    suite.addTest("Test TreeMap get() with a non-existing key on a non-empty map.",
        testGetWithNonExistingKeyOnNonEmptyMap)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREEMAP_0100
function testConstructorNoComparatorSetNumberKeys() {
    let treeMap = new TreeMap<double, double>();
    treeMap.set(111, 999)
    treeMap.set(222, 888)
    treeMap.set(333, 777)
    arktest.assertEQ(treeMap.length, 3);
}

//Test cases ported from ArkTS 1.0:TREEMAP_0200
function testConstructorCustomStringComparatorSetStringKeys() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set("AAA", "111")
    treeMap.set("BBB", "222")
    treeMap.set("CCC", "333")
    arktest.assertEQ(treeMap.length, 3);
}


class TestEntry {
    id: number = 0;
}

//Test cases ported from ArkTS 1.0:TREEMAP_0300
function testConstructorCustomObjectComparatorSetCustomObjectKeys() {
    let ts1: TreeMap<TestEntry, string> =
        new TreeMap<TestEntry, string>((t1: TestEntry, t2: TestEntry): double => {
            return t2.id - t1.id;
        });
    let entry1: TestEntry = {
        id: 0
    };
    let entry2: TestEntry = {
        id: 1
    }
    ts1.set(entry1, "0");
    ts1.set(entry2, "1");
    arktest.assertEQ(ts1.get(entry1), "0");
    arktest.assertEQ(ts1.get(entry2), "1");
    arktest.assertEQ(ts1.length, 2);
}

//Test cases ported from ArkTS 1.0:TREEMAP_0400
function testConstructorNoComparatorSetCustomObjectKeys() {
    let ts1 = new TreeMap<TestEntry, string>();
    let entry1: TestEntry = {
        id: 0
    };
    let entry2: TestEntry = {
        id: 1
    }
    ts1.set(entry1, "0");
    ts1.set(entry2, "1");
    arktest.assertEQ(ts1.get(entry1), "1");
    arktest.assertEQ(ts1.get(entry2), "1");
    arktest.assertEQ(ts1.length, 1);
}

//Test cases ported from ArkTS 1.0:testIsEmpty0001
function testIsEmptyOnNewlyCreatedMap() {
    let treeMap = new TreeMap<double, double>();
    let result = treeMap.isEmpty();
    arktest.assertTrue(result, "Expected isEmpty() to be true for a newly created TreeMap");
}

//Test cases ported from ArkTS 1.0:testIsEmpty0002
function testIsEmptyAfterAddingElement() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.isEmpty();
    arktest.assertFalse(result, "Expected isEmpty() to be false after adding an element");
}

//Test cases ported from ArkTS 1.0:testIsEmpty0003
function testIsEmptyAfterAddingAndRemovingElement() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.isEmpty();
    arktest.assertFalse(result, "Expected isEmpty() to be false after adding an element");
    treeMap.remove("squirrel");
    let result1 = treeMap.isEmpty();
    arktest.assertTrue(result1, "Expected isEmpty() to be true after removing the only element");
}

//Test cases ported from ArkTS 1.0:TREEMAP_1800
function testGetWithEmptyStringKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.get('');
    arktest.assertEQ(result, undefined);
}

//Test cases ported from ArkTS 1.0:TREEMAP_2100
function testGetWithNonExistingKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.get("key");
    arktest.assertEQ(result, undefined);
}

//Test cases ported from ArkTS 1.0:TREEMAP_2200
function testGetWithExistingKey() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.get("squirrel");
    arktest.assertEQ(result, 123);
}

//Test cases ported from ArkTS 1.0:TREEMAP_2300
function testGetWithNonExistingKeyOnNonEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("squirrel", 123);
    let result = treeMap.get("sparrow");
    arktest.assertEQ(result, undefined);
}
