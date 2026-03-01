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
    const suite = new arktest.ArkTestsuite("TreeSet PopFirst PopLast Values API tests")

    suite.addTest("Test popFirst() on an empty TreeSet.", testPopFirstOnEmptySet)
    suite.addTest("Test popFirst() on a TreeSet with multiple elements (natural string order).",
        testPopFirstWithMultipleElementsNaturalOrder)
    suite.addTest("Test popFirst() on a TreeSet with a custom reverse string comparator.",
        testPopFirstWithCustomReverseComparator)
    suite.addTest("Test popFirst() on a TreeSet with a single element.", testPopFirstWithSingleElement)
    suite.addTest("Test popFirst() on a TreeSet that becomes empty after a remove operation.",
        testPopFirstAfterRemoveMakesEmpty)
    suite.addTest("Test popFirst() on a TreeSet after clear().", testPopFirstAfterClear)
    suite.addTest("Test popLast() on an empty TreeSet.", testPopLastOnEmptySet)
    suite.addTest("Test popLast() on a TreeSet with multiple elements.", testPopLastWithMultipleElements)
    suite.addTest("Test popLast() on a TreeSet with a custom reverse string comparator.",
        testPopLastWithCustomReverseComparator)
    suite.addTest("Test popLast() on a TreeSet with a single element.", testPopLastWithSingleElement)
    suite.addTest("Test popLast() on a TreeSet that becomes empty after a remove operation.",
        testPopLastAfterRemoveMakesEmpty)
    suite.addTest("Test popLast() on a TreeSet after clear().", testPopLastAfterClear)
    suite.addTest("Test values() iterator on an empty TreeSet.", testValuesOnEmptySet)
    suite.addTest("Test values() iterator on a TreeSet with multiple elements.", testValuesWithMultipleElements)
    suite.addTest("Test values() iterator on a TreeSet with a custom reverse string comparator.",
        testValuesWithCustomReverseComparator)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREESET_5600
function testPopFirstOnEmptySet() {
    let treeSet = new TreeSet<string>();
    try {
        let result = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_5700
function testPopFirstWithMultipleElementsNaturalOrder() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.popFirst();
    let result = treeSet.has("sparrow");
    arktest.assertFalse(result, "Expected has('sparrow') to be false after popFirst() removed it (natural order)");
}

//Test cases ported from ArkTS 1.0:TREESET_5800
function testPopFirstWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    treeSet.popFirst();
    let result = treeSet.has("d");
    arktest.assertFalse(result, "Expected has('d') to be false after popFirst() removed it (custom reverse order)");
}

//Test cases ported from ArkTS 1.0:TREESET_5900
function testPopFirstWithSingleElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.popFirst();
    let result = treeSet.has("squirrel");
    arktest.assertFalse(result, "Expected has('squirrel') to be false after popFirst() removed it (single element)");
}

//Test cases ported from ArkTS 1.0:TREESET_6000
function testPopFirstAfterRemoveMakesEmpty() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.remove("squirrel");
    try {
        let result = treeSet.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    let result = treeSet.has("squirrel");
    arktest.assertFalse(result,
        "Expected has('squirrel') to be false after remove() and then popFirst() on an empty set");
}

//Test cases ported from ArkTS 1.0:TREESET_6100
function testPopFirstAfterClear() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.clear();
    try {
        let result = treeSet.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    let resultOne = treeSet.has("squirrel");
    arktest.assertFalse(resultOne, "Expected has('squirrel') to be false after clear() and popFirst()");
    let resultTwo = treeSet.has("sparrow");
    arktest.assertFalse(resultTwo, "Expected has('sparrow') to be false after clear() and popFirst()");
}

//Test cases ported from ArkTS 1.0:TREESET_6200
function testPopLastOnEmptySet() {
    let treeSet = new TreeSet<string>();
    try {
        let result = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

//Test cases ported from ArkTS 1.0:TREESET_6300
function testPopLastWithMultipleElements() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.popLast();
    let result = treeSet.has("squirrel");
    arktest.assertFalse(result, "Expected has('squirrel') to be false after popLast() removed it (natural order)");
}

//Test cases ported from ArkTS 1.0:TREESET_6400
function testPopLastWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    treeSet.popLast();
    let result = treeSet.has("a");
    arktest.assertFalse(result, "Expected has('a') to be false after popLast() removed it (custom reverse order)");
}

//Test cases ported from ArkTS 1.0:TREESET_6500
function testPopLastWithSingleElement() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.popLast();
    let result = treeSet.has("squirrel");
    arktest.assertFalse(result, "Expected has('squirrel') to be false after popLast() removed it (single element)");
}

//Test cases ported from ArkTS 1.0:TREESET_6600
function testPopLastAfterRemoveMakesEmpty() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.remove("squirrel");
    try {
        let result = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    let result = treeSet.has("squirrel");
    arktest.assertFalse(result,
        "Expected has('squirrel') to be false after remove() and then popLast() on an empty set");
}

//Test cases ported from ArkTS 1.0:TREESET_6700
function testPopLastAfterClear() {
    let treeSet = new TreeSet<string>();
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.clear();
    try {
        let result = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    let resultOne = treeSet.has("squirrel");
    arktest.assertFalse(resultOne, "Expected has('squirrel') to be false after clear() and popLast()");
    let resultTwo = treeSet.has("sparrow");
    arktest.assertFalse(resultTwo, "Expected has('sparrow') to be false after clear() and popLast()");
}

//Test cases ported from ArkTS 1.0:TREESET_7100
function testValuesOnEmptySet() {
    let treeSet = new TreeSet<string>();
    let it = treeSet.values();
    let result = it.next().done;
    arktest.assertTrue(result, "Expected iterator.next().done to be true for an empty TreeSet");
}

//Test cases ported from ArkTS 1.0:TREESET_7200
function testValuesWithMultipleElements() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let res = treeSet.values();
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
}

//Test cases ported from ArkTS 1.0:TREESET_7300
function testValuesWithCustomReverseComparator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let res = treeSet.values();
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
}
