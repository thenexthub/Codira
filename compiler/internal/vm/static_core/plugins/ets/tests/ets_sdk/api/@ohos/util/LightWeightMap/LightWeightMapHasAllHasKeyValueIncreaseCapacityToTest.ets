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
    const suite = new arktest.ArkTestsuite("LightWeightMap HasAll HasKey HasValue IncreaseCapacityTo API tests")

    suite.addTest("Testing LightWeightMap contain other LightWeightMap data", testHasAllWithSubset)
    suite.addTest("Testing LightWeightMap has key", testHasKeyExisting)
    suite.addTest("Testing LightWeightMap has not existed key", testHasKeyNonExisting)
    suite.addTest("Testing LightWeightMap has existed key with blank", testHasKeyNonExistingWithSpaces)
    suite.addTest("Testing LightWeightMap has value", testHasValueExisting)
    suite.addTest("Testing LightWeightMap has not existed value", testHasValueNonExisting)
    suite.addTest("Testing LightWeightMap has existed value with blank", testHasValueNonExistingWithSpaces)
    suite.addTest("Testing LightWeightMap has complex value", testHasValueComplex)
    suite.addTest("Testing LightWeightMap has large string value", testHasValueLargeString)
    suite.addTest("Testing LightWeightMap capacity", testIncreaseCapacityToPositive)
    suite.addTest("Testing LightWeightMap capacity to 0", testIncreaseCapacityToZero)
    suite.addTest("Testing LightWeightMap capacity to negative number", testIncreaseCapacityToNegative)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASALL_0100
function testHasAllWithSubset() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let map = new LightWeightMap<string, number>();
    map.set("sparrow", 356);
    arktest.assertTrue(lightWeightMap.hasAll(map),
        "Expected hasAll to return true when the map contains all elements of the specified map");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASKEY_0100
function testHasKeyExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertTrue(lightWeightMap.hasKey("squirrel"),
        "Expected hasKey('squirrel') to return true as the key exists");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASKEY_0200
function testHasKeyNonExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertFalse(lightWeightMap.hasKey('sparrow'),
        "Expected hasKey('sparrow') to return false as the key does not exist");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASKEY_0300
function testHasKeyNonExistingWithSpaces() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertFalse(lightWeightMap.hasKey(' squirrel '),
        "Expected hasKey(' squirrel ') to return false as the key with surrounding spaces does not exist");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASVALUE_0100
function testHasValueExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertTrue(lightWeightMap.hasValue(123), "Expected hasValue(123) to return true as the value exists");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASVALUE_0200
function testHasValueNonExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertFalse(lightWeightMap.hasValue(124),
        "Expected hasValue(124) to return false as the value does not exist");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASVALUE_0300
function testHasValueNonExistingWithSpaces() {
    let lightWeightMap = new LightWeightMap<string, string>();
    lightWeightMap.set('squirrel', 'sparrow');
    arktest.assertFalse(lightWeightMap.hasValue(' sparrow '),
        "Expected hasValue(' sparrow ') to return false as the value with surrounding spaces does not exist");
}

interface ComplexValue {
    name: string;
    gender: string;
    age: number;
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASVALUE_0500
function testHasValueComplex() {
    const value: ComplexValue = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap = new LightWeightMap<string, ComplexValue>();
    lightWeightMap.set('key', value);
    arktest.assertTrue(lightWeightMap.hasValue(value),
        "Expected hasValue(complexValue) to return true as the complex value exists");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_HASVALUE_0600
function testHasValueLargeString() {
    let value = '';
    for (let i: number = 0; i < 10000; i++) {
        value += i;
    }
    let lightWeightMap = new LightWeightMap<string, string>();
    lightWeightMap.set('key', value);
    arktest.assertTrue(lightWeightMap.hasValue(value),
        "Expected hasValue(largeStringValue) to return true as the large string value exists");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_INCREASECAPACITYTO_0100
function testIncreaseCapacityToPositive() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.increaseCapacityTo(10);
    arktest.assertEQ(lightWeightMap.length, 0);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_INCREASECAPACITYTO_0200
function testIncreaseCapacityToZero() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.increaseCapacityTo(0);
    arktest.assertEQ(lightWeightMap.length, 0);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_INCREASECAPACITYTO_0300
function testIncreaseCapacityToNegative() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.increaseCapacityTo(-1);
    arktest.assertEQ(lightWeightMap.length, 0);
}
