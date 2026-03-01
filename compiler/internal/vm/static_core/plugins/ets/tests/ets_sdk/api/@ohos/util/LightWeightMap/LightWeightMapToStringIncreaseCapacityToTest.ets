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
    const suite = new arktest.ArkTestsuite("LightWeightMap ToString IncreaseCapacityTo API tests")

    suite.addTest("LightWeightMap toString output data as a string",
        testToStringOutputData)
    suite.addTest("LightWeightMap toString output data as a string for the empty LightWeightMap",
        testToStringEmptyMap)
    suite.addTest("LightWeightMap toString output data as a string when in loop",
        testToStringInLoop)
    suite.addTest("LightWeightMap increaseCapacityTo increase the capacity",
        testIncreaseCapacity)
    suite.addTest("LightWeightMap increaseCapacityTo increase the capacity less than the length of LightWeightMap",
        testIncreaseCapacityLessThanLength)
    suite.addTest("LightWeightMap increaseCapacityTo increase the left float capacity of LightWeightMap",
        testIncreaseCapacityLeftFloat)
    suite.addTest("LightWeightMap increaseCapacityTo increase the right float capacity of LightWeightMap",
        testIncreaseCapacityRightFloat)

    return suite.run()
}

function testToStringOutputData() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    let result = lightWeightMap.toString();
    arktest.assertEQ(result, "gull:111,sparrow:123", "The lightWeightMap should return the correct string representation");
}

function testToStringEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result = lightWeightMap.toString();
    arktest.assertEQ(result, '', "The lightWeightMap should return the correct string representation");
}

function testToStringInLoop() {
    const VALUE = 'VALUE';
    let lightWeightMap: LightWeightMap<double, string> = new LightWeightMap<double, string>();
    for (let i: int = 0; i < 5; i++) {
        lightWeightMap.set(i, VALUE);
    }
    let result = lightWeightMap.toString();
    arktest.assertEQ(result, '0:VALUE,1:VALUE,2:VALUE,3:VALUE,4:VALUE',
        "The lightWeightMap should return the correct string representation");
}

function testIncreaseCapacity() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    arktest.expectNoThrow(() => { lightWeightMap.increaseCapacityTo(2) });
    arktest.expectNoThrow(() => { lightWeightMap.increaseCapacityTo(10) });
}

function testIncreaseCapacityLessThanLength() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    arktest.expectNoThrow(() => { lightWeightMap.increaseCapacityTo(1) });
}

function testIncreaseCapacityLeftFloat() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    arktest.expectNoThrow(() => { lightWeightMap.increaseCapacityTo(Math.floor(1.2).toInt()) });
}

function testIncreaseCapacityRightFloat() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    arktest.expectNoThrow(() => { lightWeightMap.increaseCapacityTo(Math.floor(3.8).toInt()) });
}
