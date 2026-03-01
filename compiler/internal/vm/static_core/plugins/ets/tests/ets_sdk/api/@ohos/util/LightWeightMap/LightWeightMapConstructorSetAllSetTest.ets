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
    const suite = new arktest.ArkTestsuite("LightWeightMap Constructor SetAll Set API tests")

    suite.addTest("Testing LightWeightMap constructor", testConstructor)
    suite.addTest("Testing LightWeightMap set data from other LightWeightMap", testSetAllFromNonEmptyMap)
    suite.addTest("Testing LightWeightMap set data from other empty LightWeightMap", testSetAllFromEmptyMap)
    suite.addTest("Testing LightWeightMap set string key and larger number value", testSetStringKeyMaxNumberValue)
    suite.addTest("Testing LightWeightMap set large string key and number value", testSetLargeStringKeyNumberValue)
    suite.addTest("Testing LightWeightMap set key and value", testSetStringKeyNumberValue)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_CONSTRUCTOR_0100
function testConstructor() {
    let lightWeightMap = new LightWeightMap<string, number>();
    arktest.assertTrue(!!lightWeightMap, "Expected LightWeightMap instance to be truthy after construction");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SETALL_0100
function testSetAllFromNonEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let map = new LightWeightMap<string, number>();
    map.setAll(lightWeightMap);
    arktest.assertEQ(map.length, 2);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SETALL_0200
function testSetAllFromEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.setAll(new LightWeightMap<string, number>());
    arktest.assertEQ(lightWeightMap.length, 1);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SET_0100
function testSetStringKeyMaxNumberValue() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", Number.MAX_VALUE);
    arktest.assertEQ(lightWeightMap.get("squirrel"), Number.MAX_VALUE);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SET_0200
function testSetLargeStringKeyNumberValue() {
    let key = '';
    for (let i: number = 0; i < 10000; i++) {
        key += i;
    }
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set(key, 123);
    arktest.assertEQ(lightWeightMap.get(key), 123);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SET_0400
function testSetStringKeyNumberValue() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertEQ(lightWeightMap.get("squirrel"), 123);
}
