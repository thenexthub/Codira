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
    const suite = new arktest.ArkTestsuite("LightWeightSet Constructor IsEmpty Add API tests")

    suite.addTest("LightWeightSet construct default empty lightWeightSet", testConstructorCreatesEmptySet)
    suite.addTest("LightWeightSet isEmpty is true", testIsEmptyTrue)
    suite.addTest("LightWeightSet isEmpty is false", testIsEmptyFalse)
    suite.addTest("LightWeightSet isEmpty is true after remove data", testIsEmptyTrueAfterRemove)
    suite.addTest("LightWeightSet isEmpty is true after clear data", testIsEmptyTrueAfterClear)
    suite.addTest("LightWeightSet add data", testAddData)
    suite.addTest("LightWeightSet add Max data", testAddMaxData)
    suite.addTest("LightWeightSet add exist data", testAddExistData)

    return suite.run()
}

function testConstructorCreatesEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    arktest.assertTrue(lightWeightSet instanceof LightWeightSet,
        "The lightWeightSet should be of the same type as the LightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testIsEmptyTrue() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let result = lightWeightSet.isEmpty();
    arktest.assertEQ(result, true, "The lightWeightSet should be empty");
    arktest.assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testIsEmptyFalse() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.isEmpty();
    arktest.assertEQ(result, false, "The lightWeightSet should not be empty");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testIsEmptyTrueAfterRemove() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add(3);
    lightWeightSet.remove(3)
    let result = lightWeightSet.isEmpty();
    arktest.assertEQ(result, true, "The lightWeightSet should be empty");
    arktest.assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testIsEmptyTrueAfterClear() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa")
    lightWeightSet.add("bb")
    lightWeightSet.add("cc")
    lightWeightSet.clear()
    let result = lightWeightSet.isEmpty();
    arktest.assertEQ(result, true, "The lightWeightSet should  be empty");
    arktest.assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testAddData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.has("aa");
    arktest.assertEQ(result, true, "The lightWeightSet should contain the added value");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testAddMaxData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    const MY_MAX = 10240;
    let result = lightWeightSet.add(MY_MAX);
    arktest.assertEQ(result, true, "The lightWeightSet should contain the added value");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testAddExistData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");

    let result = lightWeightSet.has("aa");
    arktest.assertEQ(result, true, "The lightWeightSet should contain the added value");

    result = lightWeightSet.add("aa");
    arktest.assertEQ(result, false, "The lightWeightSet should not add exist value");

    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}