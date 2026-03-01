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
    const suite = new arktest.ArkTestsuite("LightWeightMap Clear Keys Values API tests")

    suite.addTest("Testing LightWeightMap clear data", testClearNonEmptyMap)
    suite.addTest("Testing LightWeightMap add data after clear data", testClearThenAdd)
    suite.addTest("Testing LightWeightMap clear data for a empty LightWeightMap", testClearEmptyMap)
    suite.addTest("Testing LightWeightMap get all key", testKeysNonEmptyMap)
    suite.addTest("Testing LightWeightMap get all key for an empty LightWeightMap", testKeysEmptyMap)
    suite.addTest("Testing LightWeightMap get all value", testValuesNonEmptyMap)
    suite.addTest("Testing LightWeightMap get all value for an empty LightWeightMap", testValuesEmptyMap)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_CLEAR_0100
function testClearNonEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    lightWeightMap.clear();
    arktest.assertTrue(lightWeightMap.isEmpty(), "Expected LightWeightMap to be empty after calling clear()");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_CLEAR_0200
function testClearThenAdd() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.clear();
    lightWeightMap.set("a", 123);
    lightWeightMap.set("b", 356);
    arktest.assertEQ(lightWeightMap.length, 2);
    arktest.assertEQ(lightWeightMap.get("a"), 123);
    arktest.assertEQ(lightWeightMap.get("b"), 356);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_CLEAR_0300
function testClearEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.clear();
    arktest.assertTrue(lightWeightMap.isEmpty(),
        "Expected an initially empty LightWeightMap to remain empty after calling clear()");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_KEYS_0100
function testKeysNonEmptyMap() {
    let arr = new Array<string>();
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let iter: IterableIterator<string> = lightWeightMap.keys();
    let temp: IteratorResult<string> = iter.next();
    while (!temp.done) {
        arr.push(temp.value!);
        temp = iter.next();
    }
    arr.sort();
    arktest.assertEQ(arr[0], "sparrow");
    arktest.assertEQ(arr[1], "squirrel");
    arktest.assertEQ(arr.length, 2);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_KEYS_0200
function testKeysEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    let iter: IterableIterator<string> = lightWeightMap.keys();
    let temp: IteratorResult<string> = iter.next();
    arktest.assertTrue(temp.done, "Expected the iterator to be done for an empty LightWeightMap's keys");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_VALUES_0100
function testValuesNonEmptyMap() {
    let arr = new Array<number>();
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let iter: IterableIterator<number> = lightWeightMap.values();
    let temp: IteratorResult<number> = iter.next();
    while (!temp.done) {
        arr.push(temp.value!);
        temp = iter.next();
    }
    arr.sort();
    arktest.assertEQ(arr[0], 123);
    arktest.assertEQ(arr[1], 356);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_VALUES_0200
function testValuesEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    let iter: IterableIterator<number> = lightWeightMap.values();
    let temp: IteratorResult<number> = iter.next();
    arktest.assertTrue(temp.done, "Expected the iterator to be done for an empty LightWeightMap's values");
}
