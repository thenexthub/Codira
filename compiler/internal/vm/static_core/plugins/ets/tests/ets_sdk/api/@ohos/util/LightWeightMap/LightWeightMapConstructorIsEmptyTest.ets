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
    const suite = new arktest.ArkTestsuite("LightWeightMap Constructor IsEmpty API tests")

    suite.addTest("LightWeightMap construct create default empty lightWeightMap",
        testConstructorCreatesEmptyLightWeightMap)
    suite.addTest("LightWeightMap isEmpty is true",
        testIsEmptyTrue)
    suite.addTest("LightWeightMap isEmpty is false",
        testIsEmptyFalse)
    suite.addTest("LightWeightMap isEmpty is true after remove data",
        testIsEmptyTrueAfterRemove)
    suite.addTest("LightWeightMap isEmpty is true after clear data",
        testIsEmptyTrueAfterClear)

    return suite.run()
}

function testConstructorCreatesEmptyLightWeightMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertTrue(lightWeightMap instanceof LightWeightMap,
        "The lightWeightMap should be of the same type as the LightWeightMap");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testIsEmptyTrue() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result = lightWeightMap.isEmpty();
    arktest.assertEQ(result, true, "The lightWeightMap should be empty");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testIsEmptyFalse() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.isEmpty();
    arktest.assertEQ(result, false, "The lightWeightMap should not be empty");
    arktest.assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testIsEmptyTrueAfterRemove() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.remove("squirrel");
    let result = lightWeightMap.isEmpty();
    arktest.assertEQ(result, true, "The lightWeightMap should be empty");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testIsEmptyTrueAfterClear() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.clear();
    let result = lightWeightMap.isEmpty();
    arktest.assertEQ(result, true, "The lightWeightMap should be empty");
    arktest.assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}
