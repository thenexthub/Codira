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
    const suite = new arktest.ArkTestsuite("TreeSet GetFirstValue GetLastValue Add Remove API tests")

    suite.addTest("Test getFirstValue() on an empty TreeSet.", testGetFirstValueOnEmptySet)
    suite.addTest("Test getFirstValue() on a TreeSet with a single element.", testGetFirstValueWithSingleElement)
    suite.addTest("Test getFirstValue() on a TreeSet with multiple elements (natural string order).",
        testGetFirstValueWithMultipleElementsNaturalOrder)
    suite.addTest("Test getFirstValue() on a TreeSet with a custom reverse string comparator.",
        testGetFirstValueWithCustomReverseComparator)
    suite.addTest("Test getLastValue() on an empty TreeSet.", testGetLastValueOnEmptySet)
    suite.addTest("Test getLastValue() on a TreeSet with a single element.", testGetLastValueWithSingleElement)
    suite.addTest("Test getLastValue() on a TreeSet with multiple elements (natural string order).",
        testGetLastValueWithMultipleElementsNaturalOrder)
    suite.addTest("Test getLastValue() on a TreeSet with a custom reverse string comparator.",
        testGetLastValueWithCustomReverseComparator)
    suite.addTest("Test add() with an empty string and check length and has().", testAddEmptyString)
    suite.addTest("Test add() with a non-empty string and check length and has().", testAddNonEmptyString)
    suite.addTest("Test add() by adding multiple elements in a loop and check length and has() for each.",
        testAddMultipleElementsInLoop)
    suite.addTest("Test remove() for an empty string on an empty TreeSet.", testRemoveEmptyStringOnEmptySet)
    suite.addTest("Test remove() for an empty string on a TreeSet that does not contain it.",
        testRemoveEmptyStringNotPresent)
    suite.addTest("Test remove() for a double on an empty TreeSet.", testRemoveNumberOnEmptySet)
    suite.addTest("Test remove() for an existing element in a TreeSet.", testRemoveExistingElement)
    suite.addTest("Test remove() for a non-existing element in a TreeSet that contains other elements.",
        testRemoveNonExistingElementWhenSetNotEmpty)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREESET_1800
function testGetFirstValueOnEmptySet() {
    let treeSet = new TreeSet<string>();
    try {
        let result = treeSet.getFirstValue();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_1900
function testGetFirstValueWithSingleElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.getFirstValue();
    arktest.assertEQ(result, "squirrel");
}

//Test cases ported from ArkTS 1.0:TREESET_2000
function testGetFirstValueWithMultipleElementsNaturalOrder() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    let result = treeSet.getFirstValue();
    arktest.assertEQ(result, "sparrow");
}

//Test cases ported from ArkTS 1.0:TREESET_2100
function testGetFirstValueWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    let result = treeSet.getFirstValue();
    arktest.assertEQ(result, "d");
}

//Test cases ported from ArkTS 1.0:TREESET_2200
function testGetLastValueOnEmptySet() {
    let treeSet = new TreeSet<string>();
    try {
        let result = treeSet.getLastValue();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_2300
function testGetLastValueWithSingleElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.getLastValue();
    arktest.assertEQ(result, "squirrel");
}

//Test cases ported from ArkTS 1.0:TREESET_2400
function testGetLastValueWithMultipleElementsNaturalOrder() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    let result = treeSet.getLastValue();
    arktest.assertEQ(result, "squirrel");
}

//Test cases ported from ArkTS 1.0:TREESET_2500
function testGetLastValueWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    let result = treeSet.getLastValue();
    arktest.assertEQ(result, "a");
}

//Test cases ported from ArkTS 1.0:TREESET_2600
function testAddEmptyString() {
    let treeSet = new TreeSet<string>();
    treeSet.add("");
    arktest.assertEQ(treeSet.length, 1);
    let result = treeSet.has("");
    arktest.assertTrue(result, "Expected has('') to return true after adding an empty string");
}

//Test cases ported from ArkTS 1.0:TREESET_2700
function testAddNonEmptyString() {
    let treeSet = new TreeSet<string>();
    treeSet.add("a");
    arktest.assertEQ(treeSet.length, 1);
    let result = treeSet.has("a");
    arktest.assertTrue(result, "Expected has('a') to return true after adding 'a'");
}

//Test cases ported from ArkTS 1.0:TREESET_3100
function testAddMultipleElementsInLoop() {
    let treeSet = new TreeSet<string>();
    for (let i: int = 0; i < 1000; i++) {
        treeSet.add("sparrow" + i);
        arktest.assertTrue(treeSet.has("sparrow" + i), "The result should be true");
    }
    arktest.assertEQ(treeSet.length, 1000);
}

//Test cases ported from ArkTS 1.0:TREESET_3300
function testRemoveEmptyStringOnEmptySet() {
    let treeSet = new TreeSet<string>();
    let result = treeSet.remove("");
    arktest.assertFalse(result, "Expected remove('') on an empty TreeSet to return false");
}

//Test cases ported from ArkTS 1.0:TREESET_3400
function testRemoveEmptyStringNotPresent() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.remove("");
    arktest.assertFalse(result, "Expected remove('') to return false when '' is not in the TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_3900
function testRemoveNumberOnEmptySet() {
    let treeSet = new TreeSet<double>();
    let result = treeSet.remove(123);
    arktest.assertFalse(result, "Expected remove(123) on an empty TreeSet to return false");
}

//Test cases ported from ArkTS 1.0:TREESET_4000
function testRemoveExistingElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.remove("squirrel");
    arktest.assertTrue(result, "Expected remove('squirrel') to return true when 'squirrel' is in the TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_4100
function testRemoveNonExistingElementWhenSetNotEmpty() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    let result = treeSet.remove("AA");
    arktest.assertFalse(result, "Expected remove('AA') to return false when 'AA' is not in the TreeSet");
}
