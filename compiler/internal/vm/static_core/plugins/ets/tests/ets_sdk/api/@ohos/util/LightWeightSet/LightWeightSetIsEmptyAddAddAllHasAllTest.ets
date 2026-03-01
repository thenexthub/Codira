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
    const suite = new arktest.ArkTestsuite("LightWeightSet IsEmpty Add AddAll HasAll API tests")

    suite.addTest("Test LightWeightSet isEmpty() on a newly created set.", testIsEmptyOnANewlyCreatedSet)
    suite.addTest("Test LightWeightSet isEmpty() after adding multiple elements.",
        testIsEmptyAfterAddingMultipleElements)
    suite.addTest("Test LightWeightSet isEmpty() after adding and then removing an element.",
        testIsEmptyAfterAddingAndThenRemovingAnElement)
    suite.addTest("Test LightWeightSet isEmpty() after adding elements and then calling clear().",
        testIsEmptyAfterAddingElementsAndThenCallingClear)
    suite.addTest("Test LightWeightSet add() with a double.", testAddWithANumber)
    suite.addTest("Test LightWeightSet add() with a string.", testAddWithAString)
    suite.addTest("Test LightWeightSet addAll() with an empty set to an empty set.",
        testAddAllWithAnEmptySetToAnEmptySet)
    suite.addTest("Test LightWeightSet hasAll() when the target set contains all elements of the source set.",
        testHasAllWhenTargetContainsAllSourceElements)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0100
function testIsEmptyOnANewlyCreatedSet() {
    let lightWeightSet = new LightWeightSet<double | string>();
    let result = lightWeightSet.isEmpty();
    arktest.assertTrue(result, "Expected isEmpty() on a new set to be true");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0200
function testIsEmptyAfterAddingMultipleElements() {
    let lightWeightSet = new LightWeightSet<double | string>();
    lightWeightSet.add(1)
    lightWeightSet.add(2)
    lightWeightSet.add(3)
    let result = lightWeightSet.isEmpty();
    arktest.assertFalse(result, "Expected isEmpty() after adding elements to be false");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0300
function testIsEmptyAfterAddingAndThenRemovingAnElement() {
    let lightWeightSet = new LightWeightSet<double | string>();
    lightWeightSet.add(3)
    lightWeightSet.remove(3)
    let result = lightWeightSet.isEmpty();
    arktest.assertTrue(result, "Expected isEmpty() to be true after adding and removing the same element");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0400
function testIsEmptyAfterAddingElementsAndThenCallingClear() {
    let lightWeightSet = new LightWeightSet<double | string>();
    lightWeightSet.add("a")
    lightWeightSet.add("b")
    lightWeightSet.add("c")
    lightWeightSet.clear()
    let result = lightWeightSet.isEmpty();
    arktest.assertTrue(result, "Expected isEmpty() to be true after calling clear()");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0500
function testAddWithANumber() {
    let lightWeightSet = new LightWeightSet<double | string>();
    const MY_MAX = 10240;
    let result = lightWeightSet.add(MY_MAX);
    arktest.assertTrue(result, "Expected add(double) to return true when adding a new element");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0600
function testAddWithAString() {
    let lightWeightSet = new LightWeightSet<string>();
    let result = lightWeightSet.add("squirrel");
    arktest.assertTrue(result, "Expected add(string) to return true when adding a new element");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0700
function testAddAllWithAnEmptySetToAnEmptySet() {
    let lightWeightSet = new LightWeightSet<double>();
    let value: double = 10240;
    lightWeightSet.add(value);
    let set = new LightWeightSet<double>();
    let result = set.addAll(set);
    arktest.assertFalse(result, "The result should be false as no elements were added");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0800
function testHasAllWhenTargetContainsAllSourceElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let set = new LightWeightSet<string>();
    set.add("sparrow");
    let result = lightWeightSet.hasAll(set);
    arktest.assertTrue(result,
        "Expected hasAll to be true when the target set contains all elements of the source set");
}
