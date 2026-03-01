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

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeSet Constrcutor IsEmpty Has API tests")

    suite.addTest("Test TreeSet constructor and add a single element.", testConstructorAndAddSingleElement)
    suite.addTest("Test TreeSet constructor with a custom comparator and add elements.",
        testConstructorWithCustomComparatorAndAddElements)
    suite.addTest("Test TreeSet constructor with custom objects and a custom comparator.",
        testConstructorWithCustomObjectsAndCustomComparator)
    suite.addTest("Test TreeSet constructor with custom objects and default comparator (object references).",
        testConstructorWithCustomObjectsAndDefaultComparator)
    suite.addTest("Test isEmpty() on a newly created, empty TreeSet.", testIsEmptyOnNewSet)
    suite.addTest("Test isEmpty() on a TreeSet after adding an element.", testIsEmptyAfterAdd)
    suite.addTest("Test isEmpty() on a TreeSet after adding and then removing an element.",
        testIsEmptyAfterAddAndRemove)
    suite.addTest("Test has() for an empty string on an empty TreeSet.", testHasEmptyStringOnEmptySet)
    suite.addTest("Test has() for an empty string on a TreeSet containing a different element.",
        testHasEmptyStringOnNonEmptySetWithoutIt)
    suite.addTest("Test has() for a double on an empty TreeSet.", testHasNumberOnEmptySet)
    suite.addTest("Test has() for an existing element in a TreeSet.", testHasExistingElement)
    suite.addTest("Test has() for a non-existing element in a TreeSet.", testHasNonExistingElement)
    suite.addTest("Test has() and length after multiple add, remove, and clear operations.",
        testHasAndLengthAfterMultipleOperations)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREESET_0100
function testConstructorAndAddSingleElement() {
    const treeSet = new TreeSet<double>();
    treeSet.add(65536);
    arktest.assertEQ(treeSet.length, 1);
}

//Test cases ported from ArkTS 1.0:TREESET_0200
function testConstructorWithCustomComparatorAndAddElements() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    arktest.assertEQ(treeSet.length, 3);
}

class TestEntry {
    id: number = 0;
}

//Test cases ported from ArkTS 1.0:TREESET_0300
function testConstructorWithCustomObjectsAndCustomComparator() {

    let ts1: TreeSet<TestEntry> = new TreeSet<TestEntry>((t1: TestEntry, t2: TestEntry): double => {
        return t1.id - t2.id;
    });
    let entry1: TestEntry = {
        id: 0
    };
    let entry2: TestEntry = {
        id: 1
    }
    ts1.add(entry1);
    ts1.add(entry2);
    arktest.assertEQ(ts1.length, 2);
}

//Test cases ported from ArkTS 1.0:TREESET_0400
function testConstructorWithCustomObjectsAndDefaultComparator() {
    let ts1 = new TreeSet<TestEntry>();
    let entry1: TestEntry = {
        id: 0
    };
    let entry2: TestEntry = {
        id: 1
    }
    ts1.add(entry1);
    ts1.add(entry2);
    arktest.assertEQ(ts1.length, 1);
}

//Test cases ported from ArkTS 1.0:TREESET_0500
function testIsEmptyOnNewSet() {
    const treeSet = new TreeSet<string | double | boolean | Object>();
    let result = treeSet.isEmpty();
    arktest.assertTrue(result, "Expected isEmpty() to return true for a new TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_0600
function testIsEmptyAfterAdd() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.isEmpty();
    arktest.assertFalse(result, "Expected isEmpty() to return false after adding an element");
}

//Test cases ported from ArkTS 1.0:TREESET_0700
function testIsEmptyAfterAddAndRemove() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.remove("squirrel");
    let result = treeSet.isEmpty();
    arktest.assertTrue(result, "Expected isEmpty() to return true after adding and then removing the only element");
}

//Test cases ported from ArkTS 1.0:TREESET_0800
function testHasEmptyStringOnEmptySet() {
    let treeSet = new TreeSet<string | double | boolean | Object>();
    let result = treeSet.has("");
    arktest.assertFalse(result, "Expected has('') to return false for an empty TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_0900
function testHasEmptyStringOnNonEmptySetWithoutIt() {
    let treeSet = new TreeSet<string | double | boolean | Object>();
    treeSet.add("squirrel");
    let result = treeSet.has("");
    arktest.assertFalse(result, "Expected has('') to return false when the TreeSet contains 'squirrel' but not ''");
}

//Test cases ported from ArkTS 1.0:TREESET_1400
function testHasNumberOnEmptySet() {
    let treeSet = new TreeSet<string | double | boolean | Object>();
    let result = treeSet.has(123);
    arktest.assertFalse(result, "Expected has(123) to return false for an empty TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_1500
function testHasExistingElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.has("squirrel");
    arktest.assertTrue(result, "Expected has('squirrel') to return true when 'squirrel' is in the TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_1600
function testHasNonExistingElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.has("AA");
    arktest.assertFalse(result, "Expected has('AA') to return false when 'AA' is not in the TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_1700
function testHasAndLengthAfterMultipleOperations() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let resultOne = treeSet.has("squirrel");
    arktest.assertTrue(resultOne, "Expected has('squirrel') to be true after adding 'squirrel'");
    arktest.assertEQ(treeSet.length, 1);
    treeSet.add("aa");
    let resultTwo = treeSet.has("aa");
    arktest.assertTrue(resultTwo, "Expected has('aa') to be true after adding 'aa'");
    arktest.assertEQ(treeSet.length, 2);
    treeSet.remove("squirrel");
    let resultThree = treeSet.has("squirrel");
    arktest.assertFalse(resultThree, "The result should be false");
    arktest.assertEQ(treeSet.length, 1);
    treeSet.add("squirrel");
    treeSet.clear();
    let resultFive = treeSet.has("aa");
    arktest.assertFalse(resultFive, "Expected has('aa') to be false after clear()");
    let resultSix = treeSet.has("squirrel");
    arktest.assertFalse(resultSix, "Expected has('squirrel') to be false after clear()");
    arktest.assertEQ(treeSet.length, 0);
}
