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
    const suite = new arktest.ArkTestsuite("TreeSet GetLowerValue GetHigherValue Clear API tests")

    suite.addTest("Test getLowerValue() for an empty string on an empty TreeSet.",
        testGetLowerValueEmptyStringOnEmptySet)
    suite.addTest("Test getLowerValue() for a double on an empty TreeSet.", testGetLowerValueNumberOnEmptySet1)
    suite.addTest("Test getLowerValue() for a double on an empty TreeSet.", testGetLowerValueNumberOnEmptySet2)
    suite.addTest("Test getLowerValue() on a TreeSet with multiple elements.", testGetLowerValueWithMultipleElements)
    suite.addTest("Test getLowerValue() on a TreeSet with a custom reverse string comparator.",
        testGetLowerValueWithCustomReverseComparator)
    suite.addTest("Test getHigherValue() for an empty string on an empty TreeSet.",
        testGetHigherValueEmptyStringOnEmptySet)
    suite.addTest("Test getHigherValue() for a double on an empty TreeSet.", testGetHigherValueNumberOnEmptySet1)
    suite.addTest("Test getHigherValue() for a double on an empty TreeSet.", testGetHigherValueNumberOnEmptySet2)
    suite.addTest("Test getHigherValue() on a TreeSet with multiple elements.", testGetHigherValueWithMultipleElements)
    suite.addTest("Test getHigherValue() on a TreeSet with a custom reverse string comparator.",
        testGetHigherValueWithCustomReverseComparator)
    suite.addTest("Test clear() on an empty TreeSet and check length and has().", testClearOnEmptySet)
    suite.addTest("Test clear() on a TreeSet with elements and check length and has() for previous elements.",
        testClearOnPopulatedSet)
    suite.addTest("Test clear() on a TreeSet with a custom comparator and elements, then check length and has().",
        testClearOnPopulatedSetWithCustomComparator)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREESET_4200
function testGetLowerValueEmptyStringOnEmptySet() {
    let treeSet = new TreeSet<string>();
    try {
        let result = treeSet.getLowerValue("");
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_4500
function testGetLowerValueNumberOnEmptySet1() {
    let treeSet = new TreeSet<double>();
    try {
        let result = treeSet.getLowerValue(123);
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_4600
function testGetLowerValueNumberOnEmptySet2() {
    let treeSet = new TreeSet<double>();
    try {
        let result = treeSet.getLowerValue(123);
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_4700
function testGetLowerValueWithMultipleElements() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    let result = treeSet.getLowerValue("squirrel");
    arktest.assertEQ(result, "sparrow");
}

//Test cases ported from ArkTS 1.0:TREESET_4800
function testGetLowerValueWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    let result = treeSet.getLowerValue("c");
    arktest.assertEQ(result, "d");
}

//Test cases ported from ArkTS 1.0:TREESET_4900
function testGetHigherValueEmptyStringOnEmptySet() {
    let treeSet = new TreeSet<string>();
    try {
        let result = treeSet.getHigherValue("");
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_5200
function testGetHigherValueNumberOnEmptySet1() {
    let treeSet = new TreeSet<double>();
    try {
        let result = treeSet.getHigherValue(123);
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_5300
function testGetHigherValueNumberOnEmptySet2() {
    let treeSet = new TreeSet<double>();
    try {
        let result = treeSet.getHigherValue(123);
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_5400
function testGetHigherValueWithMultipleElements() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    let result = treeSet.getHigherValue("sparrow");
    arktest.assertEQ(result, "squirrel");
}

//Test cases ported from ArkTS 1.0:TREESET_5500
function testGetHigherValueWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    let result = treeSet.getHigherValue("c");
    arktest.assertEQ(result, "b");
}

//Test cases ported from ArkTS 1.0:TREESET_6800
function testClearOnEmptySet() {
    let treeSet = new TreeSet<string>();
    treeSet.clear();
    let result = treeSet.has("");
    arktest.assertFalse(result, "Expected has('') to return false after clearing an empty TreeSet");
    arktest.assertEQ(treeSet.length, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_6900
function testClearOnPopulatedSet() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.clear();
    let result = treeSet.has("squirrel");
    arktest.assertFalse(result, "Expected has('squirrel') to return false after clear()");
    let resultOne = treeSet.has("sparrow");
    arktest.assertFalse(resultOne, "Expected has('sparrow') to return false after clear()");
    arktest.assertEQ(treeSet.length, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_7000
function testClearOnPopulatedSetWithCustomComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    treeSet.clear();
    let result = treeSet.has("a");
    arktest.assertFalse(result, "Expected has('a') to return false after clear()");
    let resultOne = treeSet.has("b");
    arktest.assertFalse(resultOne, "Expected has('b') to return false after clear()");
    let resultTwo = treeSet.has("c");
    arktest.assertFalse(resultTwo, "Expected has('c') to return false after clear()");
    let resultThree = treeSet.has("d");
    arktest.assertFalse(resultThree, "Expected has('d') to return false after clear()");
    arktest.assertEQ(treeSet.length, 0);
}
