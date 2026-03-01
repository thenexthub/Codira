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
    const suite = new arktest.ArkTestsuite("LightWeightMap Set Get API tests")

    suite.addTest("LightWeightMap set valid data", testSetValidData)
    suite.addTest("LightWeightMap set larger double value", testSetLargeNumberValue)
    suite.addTest("LightWeightMap get valid key", testGetValidKey)
    suite.addTest("LightWeightMap get not set key", testGetNonExistingKey)
    suite.addTest("LightWeightMap get key from the null lightWeightMap", testGetKeyFromEmptyMap)
    suite.addTest("LightWeightMap get blank key", testGetBlankKey)
    suite.addTest("LightWeightMap get key when contains complex value", testGetKeyWithComplexValue)
    suite.addTest("LightWeightMap get complex key", testGetComplexKey)
    suite.addTest("LightWeightMap get special key", testGetSpecialKey)
    suite.addTest("LightWeightMap get same key when value is different", testGetSameKeyWithDifferentValue)
    suite.addTest("LightWeightMap get empty string", testGetEmptyStringKey)

    return suite.run()
}

function testSetValidData() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.get("squirrel");
    arktest.assertEQ(result, 123, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testSetLargeNumberValue() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", Number.MAX_VALUE);
    let result = lightWeightMap.get("squirrel");
    arktest.assertEQ(result, Number.MAX_VALUE, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetValidKey() {
    let lightWeightMap: LightWeightMap<double, double> = new LightWeightMap<double, double>();
    lightWeightMap.set(123, 356);
    let result = lightWeightMap.get(123);
    arktest.assertEQ(result, 356, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetNonExistingKey() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 356);
    let result = lightWeightMap.get("squirrel");
    arktest.assertEQ(result, undefined, "The lightWeightMap should return undefined for the key that is not set");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetKeyFromEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result = lightWeightMap.get("sparrow");
    arktest.assertEQ(result, undefined, "The lightWeightMap should return undefined for the key in an empty map");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testGetBlankKey() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 356);
    let result = lightWeightMap.get(" sparrow ");
    arktest.assertEQ(result, undefined, "The lightWeightMap should return undefined for the key that is not set");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

interface ComplexValue {
    name: string;
    gender: string;
    age: number;
}

function testGetKeyWithComplexValue() {
    const value: ComplexValue = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap: LightWeightMap<string, ComplexValue> = new LightWeightMap<string, ComplexValue>();
    lightWeightMap.set("squirrel", value);
    let result = lightWeightMap.get("squirrel");
    arktest.assertEQ(result, value, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

interface ComplexKey {
    name: string;
    gender: string;
    age: number;
}

function testGetComplexKey() {
    const key: ComplexKey = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap: LightWeightMap<ComplexKey, double> = new LightWeightMap<ComplexKey, double>();
    lightWeightMap.set(key, 123);
    let result = lightWeightMap.get(key);
    arktest.assertEQ(result, 123, "The lightWeightMap should return the correct value for the complex key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetSpecialKey() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set('鸿蒙', 123);
    let result = lightWeightMap.get('鸿蒙');
    arktest.assertEQ(result, 123, "The lightWeightMap should return the correct value for the special key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetSameKeyWithDifferentValue() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("squirrel", 456);
    let result = lightWeightMap.get("squirrel");
    arktest.assertEQ(result, 456, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetEmptyStringKey() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set('', 123);
    let result = lightWeightMap.get('');
    arktest.assertEQ(result, 123, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}
