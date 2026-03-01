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
    const suite = new arktest.ArkTestsuite("TreeMap Replace ForEach API tests")

    suite.addTest("Test TreeMap replace() with an empty string key on an empty map.",
        testReplaceWithEmptyStringKeyOnEmptyMap)
    suite.addTest("Test TreeMap replace() with a non-existing key on an empty map.",
        testReplaceWithNonExistingKeyOnEmptyMap)
    suite.addTest("Test TreeMap replace() with an existing key and the same value.",
        testReplaceWithExistingKeySameValue)
    suite.addTest("Test TreeMap replace() with an existing key and a different value.",
        testReplaceWithExistingKeyDifferentValue)
    suite.addTest("Test TreeMap replace() with a non-existing key on a non-empty map.",
        testReplaceWithNonExistingKeyOnNonEmptyMap)
    suite.addTest("Test TreeMap forEach() on an empty map.", testForEachOnEmptyMap)
    suite.addTest("Test TreeMap forEach() and check values and length (natural string order).", testForEachNaturalOrder)
    suite.addTest("Test TreeMap forEach() with a custom reverse string comparator and check values and length.",
        testForEachCustomReverseComparator)
    suite.addTest("Test TreeMap forEach(), remove an element, then check values and length.", testForEachThenRemove)
    suite.addTest("Test TreeMap forEach(), replace an element's value, then check values and length.",
        testForEachThenReplace)
    suite.addTest("Test TreeMap forEach(), clear the map, then check length.", testForEachThenClear)
    suite.addTest("Test TreeMap forEach() with no parameters in callback, clear the map, then check length.",
        testForEachNoCallbackParamsThenClear)
    suite.addTest("Test TreeMap forEach() with only value parameter in callback.", testForEachOnlyValueCallbackParam)
    suite.addTest("Test TreeMap forEach() check value existence, then clear and check length.",
        testForEachCheckValueExistenceThenClear)
    suite.addTest("Test TreeMap forEach() check value existence and original map state after forEach.",
        testForEachCheckValueExistenceAndMapState)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:TREEMAP_6800
function testReplaceWithEmptyStringKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.replace("", 123);
    arktest.assertFalse(result, "Expected replace on an empty map with an empty string key to return false");
}

//Test cases ported from ArkTS 1.0:TREEMAP_7100
function testReplaceWithNonExistingKeyOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let result = treeMap.replace('key', 123);
    arktest.assertFalse(result, "Expected replace on an empty map with a non-existing key to return false");
}

//Test cases ported from ArkTS 1.0:TREEMAP_7200
function testReplaceWithExistingKeySameValue() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("sparrow", 123);
    let result = treeMap.replace("sparrow", 123);
    let result1 = treeMap.get("sparrow");
    arktest.assertTrue(result, "Expected replace with an existing key and same value to return true");
    arktest.assertEQ(result1, 123);
}

//Test cases ported from ArkTS 1.0:TREEMAP_7300
function testReplaceWithExistingKeyDifferentValue() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("sparrow", 123);
    let result = treeMap.replace("sparrow", 456);
    let result1 = treeMap.get("sparrow");
    arktest.assertTrue(result, "Expected replace with an existing key and different value to return true");
    arktest.assertEQ(result1, 456);
}

//Test cases ported from ArkTS 1.0:TREEMAP_7400
function testReplaceWithNonExistingKeyOnNonEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set("sparrow", 123);
    let result = treeMap.replace("sparrow1", 456);
    let result1 = treeMap.get("sparrow1");
    arktest.assertFalse(result, "Expected replace with a non-existing key on a non-empty map to return false");
    arktest.assertEQ(result1, undefined);
}

//Test cases ported from ArkTS 1.0:TREEMAP_8500
function testForEachOnEmptyMap() {
    let treeMap = new TreeMap<string, double>();
    let count = 0;
    treeMap.forEach((value?: double, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 0);
}

//Test cases ported from ArkTS 1.0:TREEMAP_8600
function testForEachNaturalOrder() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    let count = 0;
    treeMap.forEach((value?: double, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, 123);
    arktest.assertEQ(res.next().value, 456);
    arktest.assertEQ(res.next().value, 789);
    arktest.assertEQ(res.next().value, 666);
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_8700
function testForEachCustomReverseComparator() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    treeMap.set('AAA', '123');
    treeMap.set('BBB', '456');
    treeMap.set('CCC', '789');
    treeMap.set('DDD', '666');
    let count = 0;
    treeMap.forEach((value?: string, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, '666');
    arktest.assertEQ(res.next().value, '789');
    arktest.assertEQ(res.next().value, '456');
    arktest.assertEQ(res.next().value, '123');
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_8800
function testForEachThenRemove() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    let count = 0;
    treeMap.forEach((value?: double, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    treeMap.remove("BBB");
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, 123);
    arktest.assertEQ(res.next().value, 789);
    arktest.assertEQ(res.next().value, 666);
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 3);
}

//Test cases ported from ArkTS 1.0:TREEMAP_8900
function testForEachThenReplace() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    let count = 0;
    treeMap.forEach((value?: double, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    treeMap.replace("BBB", 555);
    let res = treeMap.values();
    arktest.assertEQ(res.next().value, 123);
    arktest.assertEQ(res.next().value, 555);
    arktest.assertEQ(res.next().value, 789);
    arktest.assertEQ(res.next().value, 666);
    arktest.assertTrue(res.next().done, "Expected values() iterator to be done after iterating all elements");
    arktest.assertEQ(treeMap.length, 4);
}

//Test cases ported from ArkTS 1.0:TREEMAP_9000
function testForEachThenClear() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    let count = 0;
    treeMap.forEach((value?: double, key?: string): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
    arktest.assertTrue(treeMap.entries().next().done, "Expected TreeMap entries iterator to be done after clear");
}

//Test cases ported from ArkTS 1.0:TREEMAP_9700
function testForEachNoCallbackParamsThenClear() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    treeMap.set('BBB', 456);
    treeMap.set('CCC', 789);
    treeMap.set('DDD', 666);
    let count = 0;
    treeMap.forEach((): void => {
        count++;
    });
    arktest.assertEQ(count, 4);
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
    arktest.assertTrue(treeMap.entries().next().done, "Expected TreeMap entries iterator to be done after clear");
}

//Test cases ported from ArkTS 1.0:TREEMAP_9800
function testForEachOnlyValueCallbackParam() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    let count = 0;
    treeMap.forEach((value): void => {
        count++;
        arktest.assertTrue(treeMap.hasValue(value), "Expected TreeMap to have the iterated value");
    });
    arktest.assertEQ(count, 1);
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
    arktest.assertTrue(treeMap.entries().next().done, "Expected TreeMap entries iterator to be done after clear");
}

//Test cases ported from ArkTS 1.0:TREEMAP_9900
function testForEachCheckValueExistenceThenClear() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    let count = 0;
    treeMap.forEach((value, index, treeMap: TreeMap<string, double>): void => {
        count++;
        arktest.assertTrue(treeMap.hasValue(value), "Expected TreeMap to have the iterated value");
    });
    arktest.assertEQ(count, 1);
    treeMap.clear();
    arktest.assertEQ(treeMap.length, 0);
    arktest.assertTrue(treeMap.entries().next().done, "Expected TreeMap entries iterator to be done after clear");
}

//Test cases ported from ArkTS 1.0:TREEMAP_100000
function testForEachCheckValueExistenceAndMapState() {
    let treeMap = new TreeMap<string, double>();
    treeMap.set('AAA', 123);
    let count = 0;
    treeMap.forEach((value, index, treeMap: TreeMap<string, double>): void => {
        count++;
        arktest.assertTrue(treeMap.hasValue(value), "The result should be true");
    });
    arktest.assertEQ(count, 1);
    arktest.assertEQ(treeMap.get('AAA'), 123);
}
