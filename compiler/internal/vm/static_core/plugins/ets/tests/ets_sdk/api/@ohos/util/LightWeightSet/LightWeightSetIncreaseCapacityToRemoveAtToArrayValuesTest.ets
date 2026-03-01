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
    const suite = new arktest.ArkTestsuite("LightWeightSet IncreaseCapacityTo RemoveAt ToArray Values API tests")

    suite.addTest("Test LightWeightSet increaseCapacityTo interface", testIncreaseCapacityToInterface)
    suite.addTest("Test LightWeightSet removeAt interface with a valid index", testRemoveAtWithAValidIndex)
    suite.addTest("Test LightWeightSet removeAt interface with a negative index", testRemoveAtWithANegativeIndex)
    suite.addTest("Test LightWeightSet removeAt interface with an out-of-bounds index",
        testRemoveAtWithAnOutOfBoundsIndex)
    suite.addTest("Test LightWeightSet toArray() with multiple string elements", testToArrayWithMultipleStringElements)
    suite.addTest("Test LightWeightSet toArray().length on an empty set", testToArrayLengthOnAnEmptySet)
    suite.addTest("Test LightWeightSet toArray() after adding duplicate string elements",
        testToArrayAfterAddingDuplicateStringElements)
    suite.addTest("Test LightWeightSet toArray() with an object element", testToArrayWithAnObjectElement)
    suite.addTest("Test LightWeightSet toArray() with a large double element", testToArrayWithALargeNumberElement)
    suite.addTest("Test LightWeightSet iteration using toArray() and for...of",
        testToArrayIterationUsingToArrayAndForOf)
    suite.addTest("Test LightWeightSet values() iterator with multiple string elements",
        testValuesIteratorWithMultipleStringElements)
    suite.addTest("Test LightWeightSet values() iterator on an empty set", testValuesIteratorOnAnEmptySet)
    suite.addTest("Test LightWeightSet values() iterator with many concatenated string elements",
        testValuesIteratorWithManyConcatenatedStringElements)
    suite.addTest("Test LightWeightSet values() iterator with many double elements",
        testValuesIteratorWithManyNumberElements)

    return suite.run()
}

class C1 {
    name: string = ""
    age: string = ""
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_2000
function testIncreaseCapacityToInterface() {
    let lightWeightSet = new LightWeightSet<double>();
    arktest.expectNoThrow(() => { lightWeightSet.increaseCapacityTo(10) });
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3300
function testRemoveAtWithAValidIndex() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.removeAt(1);
    arktest.assertTrue(result, "Expected removeAt(1) to return true");
}

//Test cases ported from ArkTS 1.0:testLightWeightSet0034
function testRemoveAtWithANegativeIndex() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.removeAt(-1);
    arktest.assertFalse(result, "Expected removeAt(-1) to return false for a negative index");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3500
function testRemoveAtWithAnOutOfBoundsIndex() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.removeAt(3);
    arktest.assertFalse(result, "Expected removeAt(3) to return false for an out-of-bounds index");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4900
function testToArrayWithMultipleStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.toArray();
    let itemArr = ["sparrow", "squirrel"];
    arktest.assertEQ(result[0], itemArr[0]);
    arktest.assertEQ(result[1], itemArr[1]);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5000
function testToArrayLengthOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let result = lightWeightSet.toArray().length;
    arktest.assertEQ(result, 0);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5100
function testToArrayAfterAddingDuplicateStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let result = lightWeightSet.toArray();
    let itemArr = ["squirrel"];
    arktest.assertEQ(result[0], itemArr[0]);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5200
function testToArrayWithAnObjectElement() {
    let lightWeightSet = new LightWeightSet<object>();
    let a: C1 = {
        name: "Dylon", age: "Dylon"
    };
    lightWeightSet.add(a);
    let result = lightWeightSet.toArray();
    arktest.assertEQ((result[0] as C1).name, a.name);
    arktest.assertEQ((result[0] as C1).age, a.age);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5300
function testToArrayWithALargeNumberElement() {
    let element = 0;
    let lightWeightSet = new LightWeightSet<double>();
    for (let index = 0; index < 1024; index++) {
        element = element += index;
    }
    lightWeightSet.add(element);
    let result = lightWeightSet.toArray();
    arktest.assertEQ(result[0], element);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_6400
function testToArrayIterationUsingToArrayAndForOf() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let nums: Array<string> = lightWeightSet.toArray()
    let itemArr = ["sparrow", "squirrel"];
    let index = 0;
    for (let item of nums) {
        arktest.assertEQ(item, itemArr[index]);
        index++;
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5400
function testValuesIteratorWithMultipleStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let iter = lightWeightSet.values();
    let itemArr = ["sparrow", "squirrel"];
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(iter.next().value, itemArr[index]);
        index++;
    }
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5500
function testValuesIteratorOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let iter = lightWeightSet.values();
    arktest.assertEQ(iter.next().value, undefined);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5600
function testValuesIteratorWithManyConcatenatedStringElements() {
    let element = ''
    let lightWeightSet = new LightWeightSet<string>();
    for (let index = 0; index < 100; index++) {
        element = element.concat('squirrel')
        lightWeightSet.add(element)
    }
    let iter = lightWeightSet.values();
    let index = 0;
    let result = iter.next().value!.length % 'squirrel'.length;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(result, 0);
        index++;
    }
    arktest.assertEQ(index, 100);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_5700
function testValuesIteratorWithManyNumberElements() {
    let lightWeightSet = new LightWeightSet<double>();
    for (let index = 0; index < 1024; index++) {
        lightWeightSet.add(index++)
    }
    let iter = lightWeightSet.values();
    let index = 0;
    let expected = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(iter.next().value, expected);
        expected += 2;
        index++;
    }
}
