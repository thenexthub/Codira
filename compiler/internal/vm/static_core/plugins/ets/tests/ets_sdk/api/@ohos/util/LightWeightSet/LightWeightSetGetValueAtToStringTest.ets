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
    const suite = new arktest.ArkTestsuite("LightWeightSet GetValueAt ToString API tests")

    suite.addTest("Test LightWeightSet getValueAt interface with a valid index", testGetValueAtWithAValidIndex)
    suite.addTest("Test LightWeightSet getValueAt interface with an out-of-bounds index",
        testGetValueAtWithAnOutOfBoundsIndex)
    suite.addTest("Test LightWeightSet getValueAt and getIndexOf consistency", testGetValueAtAndGetIndexOfConsistency)
    suite.addTest("Test LightWeightSet toString() with multiple string elements",
        testToStringWithMultipleStringElements)
    suite.addTest("Test LightWeightSet toString() on an empty set", testToStringOnAnEmptySet)
    suite.addTest("Test LightWeightSet toString() with string value 'null'", testToStringWithStringValueNull)
    suite.addTest("Test LightWeightSet toString() with string value 'undefined'", testToStringWithStringValueUndefined)
    suite.addTest("Test LightWeightSet toString() after adding duplicate string elements",
        testToStringAfterAddingDuplicateStringElements)
    suite.addTest("Test LightWeightSet toString() with a string representing object properties",
        testToStringWithAStringRepresentingObjectProperties)
    suite.addTest("Test LightWeightSet toString() with a double element", testToStringWithANumberElement)

    return suite.run()
}

class C1 {
    name: string = ""
    age: string = ""
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3600
function testGetValueAtWithAValidIndex() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.getValueAt(1);
    arktest.assertEQ(result, "squirrel");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3700
function testGetValueAtWithAnOutOfBoundsIndex() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.getValueAt(3);
    arktest.assertEQ(result, undefined);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_3800
function testGetValueAtAndGetIndexOfConsistency() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.getValueAt(1);
    let result1 = lightWeightSet.getIndexOf("sparrow");
    arktest.assertEQ(result, "squirrel");
    arktest.assertEQ(result1, 0);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4200
function testToStringWithMultipleStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "sparrow,squirrel");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4300
function testToStringOnAnEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4400
function testToStringWithStringValueNull() {
    let lightWeightSet = "null";
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "null");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4500
function testToStringWithStringValueUndefined() {
    let lightWeightSet = "undefined";
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "undefined");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4600
function testToStringAfterAddingDuplicateStringElements() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "squirrel");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4700
function testToStringWithAStringRepresentingObjectProperties() {
    let lightWeightSet = new LightWeightSet<object>();
    let a: C1 = {
        name: "Dylon", age: "Dylon"
    };
    let aStr = `${a.name},${a.age}`;
    lightWeightSet.add(aStr);
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "Dylon,Dylon");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTSET_4800
function testToStringWithANumberElement() {
    let value: double = 1024000;
    let lightWeightSet = new LightWeightSet<double>();
    lightWeightSet.add(value);
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, value + '');
}
