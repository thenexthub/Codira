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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightSet ForEach Iterator Entries API tests")

    suite.addTest("Test LightWeightSet forEach() with removal of elements during iteration",
        testForEachWithRemovalOfElementsDuringIteration)
    suite.addTest("Test LightWeightSet forEach() callback arguments", testForEachCallbackArguments)
    suite.addTest("Test LightWeightSet forEach() on an empty set", testForEachOnAnEmptySet)
    suite.addTest("Test LightWeightSet forEach() checks length", testForEachChecksLength)
    suite.addTest("Test LightWeightSet forEach() checks has(value)", testForEachChecksHasValue)
    suite.addTest("Test LightWeightSet forEach() checks getIndexOf(value) against index",
        testForEachChecksGetIndexOfValueAgainstIndex)
    suite.addTest("Test LightWeightSet forEach() with all arguments, checks getIndexOf",
        testForEachWithAllArgumentsChecksGetIndexOf)
    suite.addTest("Test LightWeightSet forEach() similar to previous, checks getIndexOf",
        testForEachSimilarToPreviousChecksGetIndexOf)
    suite.addTest("Test LightWeightSet $_iterator() with multiple string elements",
        test$_iteratorWithMultipleStringElements)
    suite.addTest("Test LightWeightSet $_iterator() after adding duplicate string elements",
        test$_iteratorAfterAddingDuplicateStringElements)
    suite.addTest("Test LightWeightSet $_iterator() on an empty set", test$_iteratorOnAnEmptySet)
    suite.addTest("Test LightWeightSet entries() with multiple string elements.", testEntriesWithMultipleStringElements)
    suite.addTest("Test LightWeightSet entries() after adding duplicate string elements.",
        testEntriesAfterAddingDuplicateStringElements)
    suite.addTest("Test LightWeightSet entries() on an empty set.", testEntriesOnAnEmptySet)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5800
function testForEachWithRemovalOfElementsDuringIteration() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("sparrow");
    lightWeightSet.add("gull");
    lightWeightSet.forEach((value ?: string, index ?: string) => {
        lightWeightSet.remove(value!);
    });
    arktest.assertFalse(lightWeightSet.has("gull"),
        "Expected 'gull' not to be present in the set after forEach removal");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5900
function testForEachCallbackArguments() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("sparrow");
    lightWeightSet.add("gull");
    lightWeightSet.forEach((value?: string, key?: string) => {
        arktest.assertTrue(lightWeightSet.has(key!), "Expected set to have the key during forEach iteration");
        arktest.assertTrue(lightWeightSet.has(value!), "Expected set to have the value during forEach iteration");
    });
}

//Test cases ported from ArkTS 1.0:testLightWeightSet0060
function testForEachOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.forEach((value?: string, key?: string) => {
        arktest.assertFalse(lightWeightSet.has(key!),
            "Expected set not to have the key in forEach callback for an empty set");
        arktest.assertFalse(lightWeightSet.has(value!),
            "Expected set not to have the value in forEach callback for an empty set");
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6800
function testForEachChecksLength() {
    let lightWeightSet = new LightWeightSet<double>();
    for (let i: int = 0; i < 10; i++) {
        lightWeightSet.add(i);
    }
    lightWeightSet.forEach(() => {
        arktest.assertEQ(lightWeightSet.length, 10);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6900
function testForEachChecksHasValue() {
    let lightWeightSet = new LightWeightSet<double>();
    for (let i: int = 0; i < 10; i++) {
        lightWeightSet.add(i);
    }
    lightWeightSet.forEach((value) => {
        arktest.assertTrue(lightWeightSet.has(value), "Expected set to have value value during forEach iteration");
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_7000
function testForEachChecksGetIndexOfValueAgainstIndex() {
    let lightWeightSet = new LightWeightSet<double>();
    for (let i: int = 0; i < 10; i++) {
        lightWeightSet.add(i);
    }
    lightWeightSet.forEach((value, index) => {
        arktest.assertEQ(lightWeightSet.getIndexOf(value), index);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_7100
function testForEachWithAllArgumentsChecksGetIndexOf() {
    let lightWeightSet = new LightWeightSet<double>();
    for (let i: int = 0; i < 10; i++) {
        lightWeightSet.add(i);
    }
    lightWeightSet.forEach((value, index, lightWeightSet: LightWeightSet<double>) => {
        arktest.assertEQ(lightWeightSet.getIndexOf(value), index);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_7200
function testForEachSimilarToPreviousChecksGetIndexOf() {
    let lightWeightSet = new LightWeightSet<double>();
    for (let i: int = 0; i < 10; i++) {
        lightWeightSet.add(i);
    }
    lightWeightSet.forEach((value, index: double, lightWeightSet: LightWeightSet<double>) => {
        arktest.assertEQ(lightWeightSet.getIndexOf(value), index);
    });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6500
function test$_iteratorWithMultipleStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let iter = lightWeightSet.$_iterator();
    let temp: IteratorResult<string> = iter.next();
    let itemArr = ["sparrow", "squirrel"]
    let i: int = 0;
    while (!temp.done) {
        arktest.assertEQ(temp.value, itemArr[i]);
        i++;
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6600
function test$_iteratorAfterAddingDuplicateStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let iter = lightWeightSet.$_iterator().next();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(iter.value, "squirrel");
        index++;
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6700
function test$_iteratorOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let iter = lightWeightSet.$_iterator().next();
    let index = 0;
    while (index < lightWeightSet.length) {
        index++;
    }
    arktest.assertEQ(index, 0);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6100
function testEntriesWithMultipleStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let iter = lightWeightSet.entries();
    let temp = iter.next();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(temp.value![0], temp.value![1]);
        index++;
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6200
function testEntriesAfterAddingDuplicateStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let iter = lightWeightSet.entries();
    let temp = iter.next();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(temp.value![0], temp.value![1]);
        index++;
        temp = iter.next();
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6300
function testEntriesOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let iter = lightWeightSet.entries();
    let index = 0;
    arktest.assertTrue(iter.next().done, "Expected iterator on empty set to be done immediately");
    while (index < lightWeightSet.length) {
        index++;
    }
}
