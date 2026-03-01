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
    const suite = new arktest.ArkTestsuite("LightWeightMap GetIndexOfKey GetIndexOfValue API tests")

    suite.addTest("Testing LightWeightMap set difference length string key and get index by key",
        testGetIndexOfKeyExisting)
    suite.addTest("Testing LightWeightMap get index by not existed key", testGetIndexOfKeyNonExisting)
    suite.addTest("Testing LightWeightMap get index by complex key", testGetIndexOfKeyByComplexKey)
    suite.addTest("Testing LightWeightMap get index by key", testGetIndexOfKeySimple)
    suite.addTest("Testing LightWeightMap set difference length string key and get index by value",
        testGetIndexOfValueExisting)
    suite.addTest("Testing LightWeightMap get index by not existed value", testGetIndexOfValueNonExisting)
    suite.addTest("Testing LightWeightMap get index by value", testGetIndexOfValueSimple)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFKEY_0100
function testGetIndexOfKeyExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("a", 123);
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("b", 356);
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("g", 4356);
    lightWeightMap.forEach((value: number, key: string) => {
        arktest.assertTrue(lightWeightMap.hasKey(key), "Expected map to have key during forEach iteration");
        arktest.assertTrue(lightWeightMap.hasValue(value),
            "Expected map to have value for key during forEach iteration");
    });
    arktest.assertNE(lightWeightMap.getIndexOfKey("squirrel"), -1);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFKEY_0200
function testGetIndexOfKeyNonExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertEQ(lightWeightMap.getIndexOfKey("a"), -1);
}

interface ComplexKey {
    name: string;
    gender: string;
    age: number;
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFKEY_0300
function testGetIndexOfKeyByComplexKey() {
    const key: ComplexKey = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap = new LightWeightMap<ComplexKey, number>();
    lightWeightMap.set(key, 123);
    arktest.assertNE(lightWeightMap.getIndexOfKey(key), -1);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFKEY_0500
function testGetIndexOfKeySimple() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    arktest.assertNE(lightWeightMap.getIndexOfKey("sparrow"), -1);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFVALUE_0100
function testGetIndexOfValueExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("a", 100);
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("b", 356);
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("g", 456);
    lightWeightMap.forEach((value: number, key: string) => {
        arktest.assertTrue(lightWeightMap.hasKey(key), "Expected map to have key during forEach iteration");
        arktest.assertTrue(lightWeightMap.hasValue(value),
            "Expected map to have value for key during forEach iteration");
    });
    arktest.assertNE(lightWeightMap.getIndexOfValue(123), -1);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFVALUE_0300
function testGetIndexOfValueNonExisting() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertEQ(lightWeightMap.getIndexOfValue(124), -1);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETINDEXOFVALUE_0400
function testGetIndexOfValueSimple() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    arktest.assertNE(lightWeightMap.getIndexOfValue(123), -1);
}
