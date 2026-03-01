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
    const suite = new arktest.ArkTestsuite("LightWeightSet Has Remove Clear API tests")

    suite.addTest("Test LightWeightSet has interface with an existing double", testHasWithAnExistingNumber)
    suite.addTest("Test LightWeightSet has interface with a large string", testHasWithALargeString)
    suite.addTest("Test LightWeightSet remove interface with an existing string", testRemoveWithAnExistingString)
    suite.addTest("Test LightWeightSet remove interface with a non-existing string", testRemoveWithANonExistingString)
    suite.addTest("Test LightWeightSet remove interface with a large double", testRemoveWithALargeNumber)
    suite.addTest("Test LightWeightSet clear interface and check length", testClearAndCheckLength)
    suite.addTest("Test LightWeightSet add after clear", testClearAddAfterClear)
    suite.addTest("Test LightWeightSet length after clear on an empty set", testClearLengthAfterClearOnAnEmptySet)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_0900
function testHasWithAnExistingNumber() {
    let lightWeightSet = new LightWeightSet<double>();
    lightWeightSet.add(123);
    let result = lightWeightSet.has(123);
    arktest.assertTrue(result, "Expected has(123) to return true for an existing element");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_1200
function testHasWithALargeString() {
    let lightWeightSet = new LightWeightSet<string>();
    let str: string = '';
    for (let index = 0; index < 10000; index++) {
        str.concat(index + '')
    }
    lightWeightSet.add(str);
    let result = lightWeightSet.has(str);
    arktest.assertTrue(result, "Expected has(largeString) to return true for an existing large string");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_2700
function testRemoveWithAnExistingString() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.remove("sparrow");
    arktest.assertEQ(result, "sparrow");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3000
function testRemoveWithANonExistingString() {
    let lightWeightSet = new LightWeightSet<string>();
    let result = lightWeightSet.remove("88");
    arktest.assertEQ(result, undefined);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3200
function testRemoveWithALargeNumber() {
    let lightWeightSet = new LightWeightSet<double>();
    let value: double = 1024000;
    lightWeightSet.add(value);
    let result = lightWeightSet.remove(value);
    arktest.assertEQ(result, value);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3900
function testClearAndCheckLength() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    lightWeightSet.clear();
    let result = lightWeightSet.length;
    arktest.assertEQ(result, 0);
}

//Test cases ported from ArkTS 1.0:testLightWeightSet0040
function testClearAddAfterClear() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    lightWeightSet.clear();
    let result = lightWeightSet.add("squirrel");
    arktest.assertTrue(result, "Expected add('squirrel') after clear to return true");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4100
function testClearLengthAfterClearOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.clear();
    let result = lightWeightSet.length;
    arktest.assertEQ(result, 0);
}
