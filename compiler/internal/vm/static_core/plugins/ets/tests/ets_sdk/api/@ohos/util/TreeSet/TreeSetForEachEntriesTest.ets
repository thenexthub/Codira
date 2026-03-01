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
    const suite = new arktest.ArkTestsuite("TreeSet ForEach Entries API tests")

    suite.addTest("Test forEach() on an empty TreeSet.", testForEachOnEmptySet)
    suite.addTest("Test forEach() on a populated TreeSet and verify elements using a subsequent values() iterator.",
        testForEachOnPopulatedSetAndVerifyWithValuesIterator)
    suite.addTest("Test forEach() verify elements using a subsequent values() iterator.",
        testForEachAndVerifyWithValuesIterator)
    suite.addTest("Test iterating with values() after removing an element.", testValuesIteratorAfterRemove)
    suite.addTest("Test clear() on a populated TreeSet after an empty forEach() call, and check length.",
        testClearAfterEmptyForEach)
    suite.addTest("Test clear() on a populated TreeSet after an empty forEach() call, and check length.",
        testClearAfterEmptyForEachNoParams)
    suite.addTest("Test forEach() by verifying each element has(value) in the callback, then clear and check length.",
        testForEachVerifyHasInCallbackThenClear)
    suite.addTest("Test forEach() and check has(value), then clear and check length.",
        testForEachCheckHasThenClear)
    suite.addTest("Test forEach() and check has(value) then check getFirstValue().",
        testForEachCheckHasThenCheckFirstValue)
    suite.addTest("Test entries() iterator on an empty TreeSet.", testEntriesOnEmptySet)
    suite.addTest("Test entries() iterator on a populated TreeSet and verify key-value pairs.",
        testEntriesOnPopulatedSetAndVerifyPairs)
    suite.addTest("Test entries() iterator and verify key-value pairs.", testEntriesAndVerifyPairs)
    suite.addTest("Test behavior of values() iterator after an element is removed.",
        testValuesIteratorBehaviorAfterRemove)
    suite.addTest("Test clear() on a populated TreeSet after an entries() call, and check length.",
        testClearAfterEntries)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREESET_7400
function testForEachOnEmptySet() {
    let treeSet = new TreeSet<string>();
    let count = 0;
    treeSet.forEach((value?: string, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_7500
function testForEachOnPopulatedSetAndVerifyWithValuesIterator() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value?: string, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    let res = treeSet.values();
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
}

//Test cases ported from ArkTS 1.0:TREESET_7600
function testForEachAndVerifyWithValuesIterator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value?: string, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    let res = treeSet.values();
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
    arktest.assertEQ(treeSet.length, 4);
}

//Test cases ported from ArkTS 1.0:TREESET_7700
function testValuesIteratorAfterRemove() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value?: string, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    let res = treeSet.values();
    treeSet.remove('BBB')
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
    arktest.assertEQ(treeSet.length, 3);
}

//Test cases ported from ArkTS 1.0:TREESET_7800
function testClearAfterEmptyForEach() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value?: string, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    treeSet.clear()
    arktest.assertEQ(treeSet.length, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_7900
function testClearAfterEmptyForEachNoParams() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_8500
function testForEachVerifyHasInCallbackThenClear() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value): void => {
        count++;
        arktest.assertTrue(treeSet.has(value), "Expected treeSet.has(value) to be true within forEach callback");
    });
    arktest.assertEQ(count, 4);
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_8600
function testForEachCheckHasThenClear() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value, index, treeSet: TreeSet<string>): void => {
        count++;
        arktest.assertTrue(treeSet.has(value), 
            "Expected treeSet.has(value) to be true within forEach callback (with index and set parameters)");
    });
    arktest.assertEQ(count, 4);
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0);
}

//Test cases ported from ArkTS 1.0:TREESET_8700
function testForEachCheckHasThenCheckFirstValue() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    let count = 0;
    treeSet.forEach((value, index, treeSet: TreeSet<string>): void => {
        count++;
        arktest.assertTrue(treeSet.has(value),
            "Expected treeSet.has(value) to be true within forEach callback (checking getFirstValue)");
    });
    arktest.assertEQ(count, 4);
    arktest.assertEQ(treeSet.getFirstValue(), 'AAA');
}

//Test cases ported from ArkTS 1.0:TREESET_7900
function testEntriesOnEmptySet() {
    let treeSet = new TreeSet<string>();
    let it = treeSet.entries();
    let result = it.next().done;
    arktest.assertTrue(result, "Expected iterator.next().done to be true for an empty TreeSet (entries)");
}

//Test cases ported from ArkTS 1.0:TREESET_8000
function testEntriesOnPopulatedSetAndVerifyPairs() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    treeSet.entries()
    let res = treeSet.values();
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
    arktest.assertEQ(treeSet.length, 4);
}

//Test cases ported from ArkTS 1.0:TREESET_8100
function testEntriesAndVerifyPairs() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string, secondValue: string): double => {
        return firstValue.compareTo(secondValue)
    });
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    treeSet.entries();
    let res = treeSet.values();
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertEQ(res.next().value, 'CCC');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
    arktest.assertEQ(treeSet.length, 4);
}

//Test cases ported from ArkTS 1.0:TREESET_8200
function testValuesIteratorBehaviorAfterRemove() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    treeSet.entries();
    let res = treeSet.values();
    treeSet.remove("CCC");
    arktest.assertEQ(res.next().value, 'AAA');
    arktest.assertEQ(res.next().value, 'BBB');
    arktest.assertEQ(res.next().value, 'DDD');
    arktest.assertTrue(res.next().done, "Expected iterator to be done after iterating all elements");
    arktest.assertEQ(treeSet.length, 3);
}

//Test cases ported from ArkTS 1.0:TREESET_8300
function testClearAfterEntries() {
    let treeSet = new TreeSet<string>();
    treeSet.add('AAA');
    treeSet.add('BBB');
    treeSet.add('CCC');
    treeSet.add('DDD');
    treeSet.entries();
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0);
}
