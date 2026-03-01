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
    const suite = new arktest.ArkTestsuite("LightWeightMap RemoveAt API tests")

    suite.addTest("LightWeightMap removeAt data by index", testRemoveAtValidIndex)
    suite.addTest("LightWeightMap removeAt data by boundary index", testRemoveAtBoundaryIndex)
    suite.addTest("LightWeightMap removeAt data by left exceeded index", testRemoveAtLeftExceededIndex)
    suite.addTest("LightWeightMap removeAt data by right exceeded index", testRemoveAtRightExceededIndex)
    suite.addTest("LightWeightMap removeAt data by index of null lightWeightMap", testRemoveAtIndexOfEmptyMap)
    suite.addTest("LightWeightMap removeAt data by left float index", testRemoveAtLeftFloatIndex)
    suite.addTest("LightWeightMap removeAt data by right float index", testRemoveAtRightFloatIndex)

    return suite.run()
}

function testRemoveAtValidIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(1);
    arktest.assertEQ(result, true, "The lightWeightMap should remove the element at the specified index");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testRemoveAtBoundaryIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(2);
    arktest.assertEQ(result, true, "The lightWeightMap should remove the element at the boundary index");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testRemoveAtLeftExceededIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(-1);
    arktest.assertEQ(result, false, "The lightWeightMap should not remove the element at the left exceeded index");
    arktest.assertEQ(lightWeightMap.length, 3, "The lightWeightMap length should be 3");
}

function testRemoveAtRightExceededIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(5);
    arktest.assertEQ(result, false, "The lightWeightMap should not remove the element at the right exceeded index");
    arktest.assertEQ(lightWeightMap.length, 3, "The lightWeightMap length should be 3");
}

function testRemoveAtIndexOfEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result = lightWeightMap.removeAt(5);
    arktest.assertFalse(result, "The lightWeightMap should not remove the element with empty lightWeightMap");
    arktest.assertEQ(lightWeightMap.length, 0);
}

function testRemoveAtLeftFloatIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(Math.floor(1.2).toInt());
    arktest.assertEQ(result, true, "The lightWeightMap should remove the element at the floored left float index");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testRemoveAtRightFloatIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(Math.floor(1.8).toInt());
    arktest.assertEQ(result, true, "The lightWeightMap should remove the element at the floored right float index");
    arktest.assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}
