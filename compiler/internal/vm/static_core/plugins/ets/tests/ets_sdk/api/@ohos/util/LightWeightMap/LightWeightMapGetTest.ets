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
    const suite = new arktest.ArkTestsuite("LightWeightMap Get API tests")

    suite.addTest("Testing LightWeightMap get value by key", testGetExistingKey)
    suite.addTest("Testing LightWeightMap get value by not existed key", testGetNonExistingKey)
    suite.addTest("Testing LightWeightMap get value by existed key with blank", testGetNonExistingKeyWithSpaces)
    suite.addTest("Testing LightWeightMap get complex value by key", testGetComplexValue)
    suite.addTest("Testing LightWeightMap get value by complex key", testGetByComplexKey)
    suite.addTest("Testing LightWeightMap get value by large string key", testGetByLargeStringKey)
    suite.addTest("Testing LightWeightMap get value by special string key", testGetBySpecialStringKey)
    suite.addTest("Testing LightWeightMap set difference value by same key", testGetAfterReSetSameKey)
    suite.addTest("Testing LightWeightMap set key with empty string", testGetByEmptyStringKey)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0100
function testGetExistingKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    arktest.assertEQ(lightWeightMap.get("sparrow"), 356);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0200
function testGetNonExistingKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertEQ(lightWeightMap.get("sparrow"), undefined);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0300
function testGetNonExistingKeyWithSpaces() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertEQ(lightWeightMap.get(" squirrel "), undefined);
}

interface ComplexValue {
    name: string;
    gender: string;
    age: number;
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0500
function testGetComplexValue() {
    const value: ComplexValue = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap = new LightWeightMap<string, ComplexValue>();
    lightWeightMap.set("squirrel", value);
    arktest.assertEQ(lightWeightMap.get("squirrel"), value);
}

interface ComplexKey {
    name: string;
    gender: string;
    age: number;
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0700
function testGetByComplexKey() {
    const key: ComplexKey = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap = new LightWeightMap<ComplexKey, number>();
    lightWeightMap.set(key, 123);
    arktest.assertEQ(lightWeightMap.get(key), 123);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0800
function testGetByLargeStringKey() {
    let key = '';
    for (let i: number = 0; i < 10000; i++) {
        key += i;
    }
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set(key, 123);
    arktest.assertEQ(lightWeightMap.get(key), 123);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0900
function testGetBySpecialStringKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('鸿蒙', 123);
    arktest.assertEQ(lightWeightMap.get('鸿蒙'), 123);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_1000
function testGetAfterReSetSameKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("squirrel", 456);
    arktest.assertEQ(lightWeightMap.get("squirrel"), 456);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_1100
function testGetByEmptyStringKey() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set('', 123);
    arktest.assertEQ(lightWeightMap.get(''), 123);
}
