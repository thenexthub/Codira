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
    const suite = new arktest.ArkTestsuite("LightWeightMap Iterator GetIndexOfKey GetIndexOfValue API tests")

    suite.addTest("LightWeightMap $_iteraor return iterator",
        testIteratorReturnsIterator)
    suite.addTest("LightWeightMap $_iteraor return iterator with null LightWeightMap",
        testIteratorWithEmptyMap)
    suite.addTest("LightWeightMap getIndexOfKey getIndexOfValue key and value",
        testGetIndexOfKeyAndValue)
    suite.addTest("LightWeightMap getIndexOfKey getIndexOfValue not set key and value",
        testGetIndexOfKeyAndValueNonExisting)
    suite.addTest("LightWeightMap getIndexOfKey getIndexOfValue key and value of null LightWeightMap",
        testGetIndexOfKeyAndValueEmptyMap)
    suite.addTest("LightWeightMap getIndexOfKey getIndexOfValue key and value when the value repeated",
        testGetIndexOfKeyAndValueRepeated)
    suite.addTest("LightWeightMap getIndexOfKey getIndexOfValue different length key and value",
        testGetIndexOfKeyAndValueDifferentLength)
    suite.addTest("LightWeightMap getIndexOfKey getIndexOfValue complex key and value",
        testGetIndexOfKeyAndValueComplex)

    return suite.run()
}

function testIteratorReturnsIterator() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    lightWeightMap.set("test", 111);
    lightWeightMap.set("apply", 222);
    let iterator = lightWeightMap.$_iterator();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "gull,357,test,111,apply,222,sparrow,123";
    arktest.assertEQ(str, strAll, "The lightWeightMap $_iterator should return the correct key-value pairs");
}

function testIteratorWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let iteratorSetValues = lightWeightMap.$_iterator();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The lightWeightMap iterator.done should true for an empty map");
}

function testGetIndexOfKeyAndValue() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let result1 = lightWeightMap.getIndexOfKey("sparrow");
    let result2 = lightWeightMap.getIndexOfValue(357);
    arktest.assertEQ(result1, 1, "The lightWeightMap should return the correct index for the key");
    arktest.assertEQ(result2, 0, "The lightWeightMap should return the correct index for the value");
}

function testGetIndexOfKeyAndValueNonExisting() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let result1 = lightWeightMap.getIndexOfKey("test");
    let result2 = lightWeightMap.getIndexOfValue(111);
    arktest.assertEQ(result1, -1, "The lightWeightMap should return -1 for a key that is not set");
    arktest.assertEQ(result2, -1, "The lightWeightMap should return -1 for a value that is not set");
}

function testGetIndexOfKeyAndValueEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let result1 = lightWeightMap.getIndexOfKey("test");
    let result2 = lightWeightMap.getIndexOfValue(111);
    arktest.assertEQ(result1, -1, "The lightWeightMap should return -1 for a key in an empty map");
    arktest.assertEQ(result2, -1, "The lightWeightMap should return -1 for a value in an empty map");
}

function testGetIndexOfKeyAndValueRepeated() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 123);
    let result1 = lightWeightMap.getIndexOfKey("sparrow");
    let result2 = lightWeightMap.getIndexOfValue(123);
    arktest.assertEQ(result1, 1, "The lightWeightMap should return the correct index for the key");
    arktest.assertEQ(result2, 0, "The lightWeightMap should return the first index for the repeated value");
}

function testGetIndexOfKeyAndValueDifferentLength() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("a", 123);
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    lightWeightMap.set("b", 35);
    lightWeightMap.set("g", 4356);
    let result1 = lightWeightMap.getIndexOfKey("sparrow");
    let result2 = lightWeightMap.getIndexOfValue(35);
    arktest.assertNE(result1, -1, "The lightWeightMap should return the correct index for the key");
    arktest.assertNE(result2, -1, "The lightWeightMap should return the correct index for the value");
}

interface Complex {
    name: string;
    gender: string;
    age: number;
}

function testGetIndexOfKeyAndValueComplex() {
    const key: Complex = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };

    const value: Complex = {
        name: 'sparrow',
        gender: 'female',
        age: 20,
    };
    let lightWeightMap: LightWeightMap<Complex, Complex> = new LightWeightMap<Complex, Complex>();
    lightWeightMap.set(key, value);
    let result1 = lightWeightMap.getIndexOfKey(key);
    let result2 = lightWeightMap.getIndexOfValue(value);
    arktest.assertNE(result1, -1, "The lightWeightMap should return the correct index for the key");
    arktest.assertNE(result2, -1, "The lightWeightMap should return the correct index for the value");
}
