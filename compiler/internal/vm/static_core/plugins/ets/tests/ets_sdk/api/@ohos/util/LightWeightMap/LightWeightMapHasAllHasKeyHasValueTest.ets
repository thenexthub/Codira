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
    const suite = new arktest.ArkTestsuite("LightWeightMap HasAll HasKey HasValue API tests")

    suite.addTest("LightWeightMap hasAll the map1 has all data of the map2",
        testHasAllWithAllData)
    suite.addTest("LightWeightMap hasAll the map1 has not set data compared with map2",
        testHasAllWithNotSetData)
    suite.addTest("LightWeightMap hasAll the map1 not has all data of the map2 which is out range",
        testHasAllWithOutOfRangeData)
    suite.addTest("LightWeightMap hasAll the map1 has all data of the null map2",
        testHasAllWithEmptyMap)
    suite.addTest("LightWeightMap hasKey hasValue designate key and value",
        testHasKeyAndValueWithExisting)
    suite.addTest("LightWeightMap hasKey hasValue not set key and value",
        testHasKeyAndValueWithNonExisting)
    suite.addTest("LightWeightMap hasKey hasValue from the null LightWeightMap",
        testHasKeyAndValueWithEmptyMap)
    suite.addTest("LightWeightMap hasKey hasValue blank key and value",
        testHasKeyAndValueWithBlank)
    suite.addTest("LightWeightMap hasKey hasValue complex key and value",
        testHasKeyAndValueWithComplex)

    return suite.run()
}

function testHasAllWithAllData() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    newLightWeightMap.set("sparrow", 356);
    let result = lightWeightMap.hasAll(newLightWeightMap);
    arktest.assertEQ(result, true, "The lightWeightMap should contain all data from another lightWeightMap");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    arktest.assertEQ(newLightWeightMap.length, 1, "The newLightWeightMap length should be 1");
}

function testHasAllWithNotSetData() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    newLightWeightMap.set("test", 777);
    let result = lightWeightMap.hasAll(newLightWeightMap);
    arktest.assertEQ(result, false, "The lightWeightMap should not contain data that are not set in another lightWeightMap");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    arktest.assertEQ(newLightWeightMap.length, 1, "The newLightWeightMap length should be 1");
}

function testHasAllWithOutOfRangeData() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    newLightWeightMap.set("test", 777);
    newLightWeightMap.set("squirrel", 123);
    newLightWeightMap.set("sparrow", 356);
    let result = lightWeightMap.hasAll(newLightWeightMap);
    arktest.assertEQ(result, false,
        "The lightWeightMap should not contain data that are out of range in another lightWeightMap");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    arktest.assertEQ(newLightWeightMap.length, 3, "The newLightWeightMap length should be 3");
}

function testHasAllWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result = lightWeightMap.hasAll(newLightWeightMap);
    arktest.assertEQ(result, true, "The lightWeightMap should contain all data from an empty lightWeightMap");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    arktest.assertEQ(newLightWeightMap.length, 0, "The newLightWeightMap length should be 0");
}

function testHasKeyAndValueWithExisting() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result1 = lightWeightMap.hasKey("squirrel");
    let result2 = lightWeightMap.hasValue(123);
    arktest.assertEQ(result1, true, "The lightWeightMap should contain the key");
    arktest.assertEQ(result2, true, "The lightWeightMap should contain the value");
    arktest.assertEQ(lightWeightMap.length, 1, "LightWeightMap length should be 1");
}

function testHasKeyAndValueWithNonExisting() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result1 = lightWeightMap.hasKey("sparrow");
    let result2 = lightWeightMap.hasValue(456);
    arktest.assertEQ(result1, false, "The lightWeightMap should not contain the key that is not set");
    arktest.assertEQ(result2, false, "The lightWeightMap should not contain the value that is not set");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testHasKeyAndValueWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result1 = lightWeightMap.hasKey("squirrel");
    let result2 = lightWeightMap.hasValue(123);
    arktest.assertEQ(result1, false, "The lightWeightMap should not contain any key when the lightWeightMap is empty");
    arktest.assertEQ(result2, false, "The lightWeightMap should not contain any value when the lightWeightMap is empty");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testHasKeyAndValueWithBlank() {
    let lightWeightMap: LightWeightMap<string, string> = new LightWeightMap<string, string>();
    lightWeightMap.set('squirrel', 'sparrow');
    let result1 = lightWeightMap.hasKey(' squirrel ');
    let result2 = lightWeightMap.hasValue(' sparrow ');
    arktest.assertEQ(result1, false, "The lightWeightMap should not contain the key");
    arktest.assertEQ(result2, false, "The lightWeightMap should not contain the value");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

interface ComplexValue {
    name: string;
    gender: string;
    age: number;
}

function testHasKeyAndValueWithComplex() {
    const value: ComplexValue = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };

    const key: ComplexValue = {
        name: 'sparrow',
        gender: 'female',
        age: 20,
    };
    let lightWeightMap: LightWeightMap<ComplexValue, ComplexValue> = new LightWeightMap<ComplexValue, ComplexValue>();
    lightWeightMap.set(key, value);
    let result1 = lightWeightMap.hasKey(key);
    let result2 = lightWeightMap.hasValue(value);
    arktest.assertEQ(result1, true, "The lightWeightMap should contain the key");
    arktest.assertEQ(result2, true, "The lightWeightMap should contain the value");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}
