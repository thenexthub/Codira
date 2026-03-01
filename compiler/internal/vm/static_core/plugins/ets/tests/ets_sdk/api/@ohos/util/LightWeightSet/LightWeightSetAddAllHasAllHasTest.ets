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
    const suite = new arktest.ArkTestsuite("LightWeightSet AddAll HasAll Has API tests")

    suite.addTest("LightWeightSet addAll data", testAddAllData)
    suite.addTest("LightWeightSet addAll empty data", testAddAllEmptyData)
    suite.addTest("LightWeightSet addAll with empty set as source", testAddAllWithNonEmptySet)
    suite.addTest("LightWeightSet addAll with overlapping sets", testAddAllWithMultipleSets)
    suite.addTest("LightWeightSet hasAll data", testHasAllData)
    suite.addTest("LightWeightSet hasAll not set data", testHasAllNotSetData)
    suite.addTest("LightWeightSet hasAll data lightWeightSet out of range", testHasAllOutOfRangeData)
    suite.addTest("LightWeightSet hasAll data lightWeightSet is empty", testHasAllEmptyData)
    suite.addTest("LightWeightSet has value", testHasValue)
    suite.addTest("LightWeightSet has not set value", testHasNotSetValue)
    suite.addTest("LightWeightSet has value of the null lightWeightSet", testHasValueOfEmptySet)
    suite.addTest("LightWeightSet has string value", testHasStringValue)

    return suite.run()
}

function testAddAllData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let lightWeightSett: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSett.add("aa");
    lightWeightSett.add("bb");
    let result = lightWeightSet.addAll(lightWeightSett);
    arktest.assertEQ(result, true, "The lightWeightSet should add all elements from another lightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
    arktest.assertEQ(lightWeightSett.length, 2, "The lightWeightSet length should be 2");
}

function testAddAllEmptyData() {
    let set: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let result = set.addAll(set);
    arktest.assertEQ(result, false, "The lightWeightSet should add all elements from another lightWeightSet");
}

function testAddAllWithNonEmptySet() {
    let setA: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let setB: LightWeightSet<double | string> = new LightWeightSet<double | string>();

    setA.add("aa");
    setB.add("bb");
    let result = setB.addAll(setA);
    arktest.assertEQ(result, true, "The lightWeightSet should add all elements from another lightWeightSet");
}

function testAddAllWithMultipleSets() {
    let setA: LightWeightSet<string> = new LightWeightSet<string>();
    let setB: LightWeightSet<string> = new LightWeightSet<string>();

    setA.add("sparrow");
    setA.add("s");

    setB.add("s");
    setB.add("squirrel");

    arktest.assertEQ(setA.addAll(setB), true, "The addAll method should return true when new elements are added");
    arktest.assertEQ(setA.length, 3, "The set length should be 3 after adding all elements");

    let setC: LightWeightSet<string> = new LightWeightSet<string>();
    setC.add("s");

    arktest.assertEQ(setA.addAll(setC), false, "The addAll method should return false when no new elements are added");
}

function testHasAllData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let lightWeightSett: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSett.add("aa");
    let result = lightWeightSet.hasAll(lightWeightSett);
    arktest.assertEQ(result, true, "The lightWeightSet should contain all elements from another lightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
    arktest.assertEQ(lightWeightSett.length, 1, "The lightWeightSett length should be 1");
}

function testHasAllNotSetData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let lightWeightSett: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSett.add("cc");
    let result = lightWeightSet.hasAll(lightWeightSett);
    arktest.assertEQ(result, false,
        "The lightWeightSet should not contain elements that are not set in another lightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
    arktest.assertEQ(lightWeightSett.length, 1, "The lightWeightSett length should be 1");
}

function testHasAllOutOfRangeData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let lightWeightSett: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSett.add("aa");
    lightWeightSett.add("bb");
    lightWeightSett.add("cc");
    let result = lightWeightSet.hasAll(lightWeightSett);
    arktest.assertEQ(result, false,
        "The lightWeightSet should not contain elements that are out of range in another lightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
    arktest.assertEQ(lightWeightSett.length, 3, "The lightWeightSett length should be 3");
}

function testHasAllEmptyData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let lightWeightSett: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSett.add("aa");
    lightWeightSett.add("bb");
    let result = lightWeightSett.hasAll(lightWeightSet);
    arktest.assertEQ(result, true, "The lightWeightSet should add all elements from an empty lightWeightSet");
    arktest.assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
    arktest.assertEQ(lightWeightSett.length, 2, "The lightWeightSett length should be 2");
}

function testHasValue() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add(123);
    let result = lightWeightSet.has(123);
    arktest.assertEQ(result, true, "The lightWeightSet should contain the value");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testHasNotSetValue() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add(123);
    let result = lightWeightSet.has(456);
    arktest.assertEQ(result, false, "The lightWeightSet should not contain the value that is not set");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testHasValueOfEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let result = lightWeightSet.has(456);
    arktest.assertEQ(result, false, "The lightWeightSet should not contain any value when the lightWeightSet is empty");
    arktest.assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testHasStringValue() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let str: string = ''
    for (let index = 0; index < 10000; index++) {
        str.concat(index + '')
    }
    lightWeightSet.add(str);
    let result = lightWeightSet.has(str);
    arktest.assertEQ(result, true, "The lightWeightSet should contain the value");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}
