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
    const suite = new arktest.ArkTestsuite("LightWeightSet GetValueAt API tests")

    suite.addTest("LightWeightSet getValueAt value by index", testGetValueAtValidIndex)
    suite.addTest("LightWeightSet getValueAt value by boundary index", testGetValueAtBoundaryIndex)
    suite.addTest("LightWeightSet getValueAt value by left exceeded index", testGetValueAtLeftExceededIndex)
    suite.addTest("LightWeightSet getValueAt value by right exceeded index", testGetValueAtRightExceededIndex)
    suite.addTest("LightWeightSet getValueAt value by index of null lightWeightSet", testGetValueAtIndexOfEmptySet)
    suite.addTest("LightWeightSet getValueAt value by left float index", testGetValueAtLeftFloatIndex)
    suite.addTest("LightWeightSet getValueAt value by right float index", testGetValueAtRightFloatIndex)
    suite.addTest("LightWeightSet getValueAt long string value", testGetValueAtLongString)

    return suite.run()
}

function testGetValueAtValidIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.getValueAt(1);
    arktest.assertEQ(result, "bb", "The lightWeightSet should return the value at the specified index");
    arktest.assertEQ(lightWeightSet.length, 3, "The lightWeightSet length should be 3");
}

function testGetValueAtBoundaryIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.getValueAt(0);
    arktest.assertEQ(result, "aa", "The lightWeightSet should return the value at the boundary index");
    arktest.assertEQ(lightWeightSet.length, 3, "LightWeightSet length should be 3");
}

function testGetValueAtLeftExceededIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getValueAt(-2);
    arktest.assertEQ(result, undefined);
}

function testGetValueAtRightExceededIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getValueAt(2);
    arktest.assertEQ(result, undefined);
}

function testGetValueAtIndexOfEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let result = lightWeightSet.getValueAt(4);
    arktest.assertEQ(result, undefined);
}

function testGetValueAtLeftFloatIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getValueAt(Math.floor(1.2).toInt());
    arktest.assertEQ(result, "bb", "The lightWeightSet should return the value at the floored index");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testGetValueAtRightFloatIndex() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getValueAt(Math.floor(1.8).toInt());
    arktest.assertEQ(result, "bb", "The lightWeightSet should return the value at the floored index");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testGetValueAtLongString() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let str1 = "sparrow"
    let str2 = "squirrel"
    let str3 = "s"
    lightWeightSet.add(str1);
    lightWeightSet.add(str2);
    lightWeightSet.add(str3);
    let result = lightWeightSet.getValueAt(1);
    arktest.assertEQ(result, "sparrow", "The lightWeightSet should return the value at the specified index");
}