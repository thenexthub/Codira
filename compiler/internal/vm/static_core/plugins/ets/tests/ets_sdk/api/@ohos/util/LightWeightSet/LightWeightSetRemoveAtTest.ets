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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightSet RemoveAt API tests")

    suite.addTest("LightWeightSet removeAt data by index", testRemoveAtValidIndex)
    suite.addTest("LightWeightSet removeAt data by boundary index", testRemoveAtBoundaryIndex)
    suite.addTest("LightWeightSet removeAt data by left exceeded index", testRemoveAtLeftExceededIndex)
    suite.addTest("LightWeightSet removeAt data by right exceeded index", testRemoveAtRightExceededIndex)
    suite.addTest("LightWeightSet removeAt data by index of null lightWeightSet", testRemoveAtIndexOfEmptySet)
    suite.addTest("LightWeightSet removeAt data by left float index", testRemoveAtLeftFloatIndex)
    suite.addTest("LightWeightSet removeAt data by right float index", testRemoveAtRightFloatIndex)

    return suite.run()
}

function testRemoveAtValidIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.removeAt(1);
    arktest.assertEQ(result, true, "The lightWeightSet should remove the value at the specified index");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtBoundaryIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.removeAt(0);
    arktest.assertEQ(result, true, "The lightWeightSet should remove the value at the boundary index");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtLeftExceededIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(-2);
    arktest.assertEQ(result, false, "The lightWeightSet should not remove the value at an out-of-range index");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtRightExceededIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(2);
    arktest.assertEQ(result, false, "The lightWeightSet should not remove the value at an out-of-range index");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtIndexOfEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let result = lightWeightSet.removeAt(2);
    arktest.assertFalse(result, "The lightWeightSet should not remove with empty lightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 0);
}

function testRemoveAtLeftFloatIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(Math.floor(1.2).toInt());
    arktest.assertEQ(result, true, "The lightWeightSet should remove the value at the floored index");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testRemoveAtRightFloatIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(Math.floor(1.8).toInt());
    arktest.assertEQ(result, true, "The lightWeightSet should remove the value at the floored index");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}
