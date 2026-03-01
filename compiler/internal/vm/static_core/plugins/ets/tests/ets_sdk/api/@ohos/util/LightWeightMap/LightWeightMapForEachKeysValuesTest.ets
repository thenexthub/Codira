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

import LightWeightMap from "@ohos.util.LightWeightMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightMap ForEach Keys Values API tests")

    suite.addTest("LightWeightMap forEach the lightWeightMap with no reference",
        testForEachNoReference)
    suite.addTest("LightWeightMap forEach the lightWeightMap with reference is value",
        testForEachValueReference)
    suite.addTest("LightWeightMap forEach the lightWeightMap with reference are value and key",
        testForEachKeyValueReference)
    suite.addTest("LightWeightMap forEach the lightWeightMap with three references",
        testForEachThreeReferences)
    suite.addTest("LightWeightMap keys return iterator by key",
        testKeysIterator)
    suite.addTest("LightWeightMap keys return iterator by key with null lightWeightMap",
        testKeysIteratorWithEmptyMap)
    suite.addTest("LightWeightMap values return iterator by value",
        testValuesIterator)
    suite.addTest("LightWeightMap values return iterator by value with null lightWeightMap",
        testValuesIteratorWithEmptyMap)

    return suite.run()
}

function testForEachNoReference() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let count: int = 0;
    lightWeightMap.forEach(() => {
        count++;
    });
    arktest.assertEQ(count, 2, "The lightWeightMap forEach should iterate over all elements");
}

function testForEachValueReference() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let flag: boolean = false;
    lightWeightMap.forEach((value: double) => {
        flag = lightWeightMap.hasValue(value);
        arktest.assertEQ(flag, true, "The lightWeightMap forEach should iterate over all values");
    });
}

function testForEachKeyValueReference() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let flag1: boolean = false;
    let flag2: boolean = false;
    lightWeightMap.forEach((value: double, key: string) => {
        flag1 = lightWeightMap.hasValue(value);
        flag2 = lightWeightMap.hasKey(key);
        arktest.assertEQ(flag1, true, "The lightWeightMap forEach should iterate over all values");
        arktest.assertEQ(flag2, true, "The lightWeightMap forEach should iterate over all keys");
    });
}

function testForEachThreeReferences() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let flag1: boolean = false;
    let flag2: boolean = false;
    lightWeightMap.forEach((value: double, key: string, lightWeightMap: LightWeightMap<string, double>) => {
        flag1 = lightWeightMap.hasValue(value);
        flag2 = lightWeightMap.hasKey(key);
        arktest.assertEQ(flag1, true, "The lightWeightMap forEach should iterate over all elements with three references");
        arktest.assertEQ(flag2, true, "The lightWeightMap forEach should iterate over all elements with three references");
    });
}

function testKeysIterator() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let iteratorSetValues = lightWeightMap.keys();
    arktest.assertEQ(iteratorSetValues.next().value, "gull", "The lightWeightMap keys iterator should return the correct key");
    arktest.assertEQ(iteratorSetValues.next().value, "sparrow",
        "The lightWeightMap keys iterator should return the correct key");
    arktest.assertTrue(iteratorSetValues.next().done,
        "The lightWeightMap keys iterator.done should true at the end");
}

function testKeysIteratorWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let iteratorSetValues = lightWeightMap.keys();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The lightWeightMap keys iterator.done should true for an empty map");
}

function testValuesIterator() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let iteratorSetValues = lightWeightMap.values();
    arktest.assertEQ(iteratorSetValues.next().value, 357, "The lightWeightMap values iterator should return the correct value");
    arktest.assertEQ(iteratorSetValues.next().value, 123, "The lightWeightMap values iterator should return the correct value");
    arktest.assertTrue(iteratorSetValues.next().done,
        "The lightWeightMap values iterator.done should true at the end");
}

function testValuesIteratorWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let iteratorSetValues = lightWeightMap.values();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The lightWeightMap values iterator.done should true for an empty map");
}
