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
    const suite = new arktest.ArkTestsuite("LightWeightMap SetAll Remove Clear API tests")

    suite.addTest("LightWeightMap setAll data from map2 to map1", testSetAllFromMap2ToMap1)
    suite.addTest("LightWeightMap setAll data from empty map2 to map1", testSetAllFromEmptyMap2ToMap1)
    suite.addTest("LightWeightMap remove key", testRemoveExistingKey)
    suite.addTest("LightWeightMap remove not set key", testRemoveNonExistingKey)
    suite.addTest("LightWeightMap remove key from the null LightWeightMap", testRemoveKeyFromEmptyMap)
    suite.addTest("LightWeightMap remove large double key", testRemoveLargeNumberKey)
    suite.addTest("LightWeightMap clear data", testClearData)
    suite.addTest("LightWeightMap clear the null LightWeightMap", testClearEmptyMap)
    suite.addTest("LightWeightMap clear data and set data again", testClearAndSetDataAgain)

    return suite.run()
}

function testSetAllFromMap2ToMap1() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    newLightWeightMap.setAll(lightWeightMap);
    let result = newLightWeightMap.get("sparrow");
    arktest.assertEQ(result, 356, "The newLightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    arktest.assertEQ(newLightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testSetAllFromEmptyMap2ToMap1() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.setAll(newLightWeightMap);
    let result = lightWeightMap.get("sparrow");
    arktest.assertEQ(result, 356, "The lightWeightMap should return the correct value for the key");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    arktest.assertEQ(newLightWeightMap.length, 0, "The newLightWeightMap length should be 0");
}

function testRemoveExistingKey() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.remove("squirrel");
    arktest.assertEQ(result, 123, "The lightWeightMap should remove the key and return its value");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testRemoveNonExistingKey() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.remove("test");
    arktest.assertEQ(result, undefined, "The lightWeightMap should return undefined for the key that is not set");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testRemoveKeyFromEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result = lightWeightMap.remove("squirrel");
    arktest.assertEQ(result, undefined, "The lightWeightMap should return undefined for the key in an empty map");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testRemoveLargeNumberKey() {
    let lightWeightMap: LightWeightMap<double, string> = new LightWeightMap<double, string>();
    lightWeightMap.set(Number.MAX_VALUE, "squirrel");
    let result = lightWeightMap.remove(Number.MAX_VALUE);
    arktest.assertEQ(result, "squirrel", "The lightWeightMap should remove the key and return its value");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testClearData() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result1 = lightWeightMap.isEmpty();
    lightWeightMap.clear();
    let result2 = lightWeightMap.get("squirrel");
    arktest.assertEQ(result1, false, "The lightWeightMap should not be empty");
    arktest.assertEQ(result2, undefined, "The lightWeightMap should be cleared");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testClearEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.clear();
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testClearAndSetDataAgain() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.clear();
    let result = lightWeightMap.isEmpty();
    lightWeightMap.set("a", 123);
    lightWeightMap.set("b", 356);
    arktest.assertEQ(result, true, "The lightWeightMap should be empty");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}
