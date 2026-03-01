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

import HashMap from "@ohos.util.HashMap";
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("HashMap Set Get SetAll API tests")

    suite.addTest("HashMap set valid data", testSetValidData)
    suite.addTest("HashMap set empty string", testSetEmptyString)
    suite.addTest("HashMap set undefined", testSetUndefined)
    suite.addTest("HashMap set key length 1024", testSetKeyLength1024)
    suite.addTest("HashMap set data loop 1000", testSetDataLoop1000)
    suite.addTest("HashMap get valid key", testGetValidKey)
    suite.addTest("HashMap get not set key", testGetNotSetKey)
    suite.addTest("HashMap get key from the null hashMap", testGetKeyFromEmptyHashMap)
    suite.addTest("HashMap get empty string", testGetEmptyString)
    suite.addTest("HashMap get data when clear data", testGetDataAfterClearing)
    suite.addTest("HashMap setAll set all data of hashMap2 to hashMap1", testSetAllDataFromHashMap2ToHashMap1)
    suite.addTest("HashMap setAll data when same key diff value", testSetAllSameKeyDiffValue)
    suite.addTest("HashMap setAll same data", testSetAllSameData)

    return suite.run()
}

function testSetValidData() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result = hashMap.get("squirrel");
    arktest.assertEQ(result, 123, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testSetEmptyString() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("", 356);
    let result = hashMap.get("")
    arktest.assertEQ(result, 356, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testSetUndefined() {
    let hashMap: HashMap<string|undefined, double|undefined> = new HashMap<string|undefined, double|undefined>();
    try {
        hashMap.set(undefined, 356);
    } catch (e) {
        arktest.assertEQ(e.code, 401, "The HashMap should throw a BusinessError when key is undefined");
        return;
    }
    arktest.assertEQ(0, 1, "The key cannot be undefined");
}

function testSetKeyLength1024() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let param = 'a'.repeat(2048);
    hashMap.set(param, 356);
    let result = hashMap.get(param)
    arktest.assertEQ(result, 356, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testSetDataLoop1000() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    for (let i: int = 0; i < 1000; i++) {
        hashMap.set("ji" + i, i);
    }
    for (let i: int = 0; i < 1000; i++) {
        let result = hashMap.get("ji" + i)
        arktest.assertEQ(result, i, "The hashMap should return the correct value for the key");
    }
    arktest.assertEQ(hashMap.length, 1000, "The hashMap length should be 1000");
}

function testGetValidKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 356);
    let result = hashMap.get("sparrow");
    arktest.assertEQ(result, 356, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testGetNotSetKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 356);
    let result = hashMap.get("squirrel");
    arktest.assertEQ(result, undefined, "The hashMap should return undefined for the key that is not set");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testGetKeyFromEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let result = hashMap.get("sparrow");
    arktest.assertEQ(result, undefined, "The hashMap should return undefined for the key when the hashMap is empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testGetEmptyString() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set('', "356");
    let result = hashMap.get('');
    arktest.assertEQ(result, "356", "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testGetDataAfterClearing() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("ji", "hello")
    let result1 = hashMap.isEmpty();
    hashMap.clear()
    let result2 = hashMap.get("ji");
    arktest.assertEQ(result1, false, "The hashMap should not be empty");
    arktest.assertEQ(result2, undefined, "The hashMap should return undefined for the key that is not set");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testSetAllDataFromHashMap2ToHashMap1() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    hashMap.set("sparrow", 356);
    let newHashMap: HashMap<string, double> = new HashMap<string, double>();
    newHashMap.setAll(hashMap);
    arktest.assertEQ(newHashMap.length, hashMap.length, "The hashMap should set all elements to another HashMap");
    arktest.assertEQ(hashMap.length, 2, "The hashMap length should be 2");
    arktest.assertEQ(newHashMap.length, 2, "The newHashMap length should be 2");
}

function testSetAllSameKeyDiffValue() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    hashMap.set("sparrow", 356);
    let newHashMap: HashMap<string, double> = new HashMap<string, double>();
    newHashMap.set("sparrow", 100);
    hashMap.setAll(newHashMap);
    let result = hashMap.get("sparrow");
    arktest.assertEQ(result, 100, "The hashMap should set all elements to another HashMap");
    arktest.assertEQ(hashMap.length, 2, "The hashMap length should be 2");
    arktest.assertEQ(newHashMap.length, 1, "The newHashMap length should be 1");
}

function testSetAllSameData() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("sparrow", 356);
    let newHashMap: HashMap<string, double> = new HashMap<string, double>();
    newHashMap.set("sparrow", 356);
    hashMap.setAll(newHashMap);
    let result = hashMap.get("sparrow");
    arktest.assertEQ(result, 356, "The hashMap should set all elements to another HashMap");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
    arktest.assertEQ(newHashMap.length, 1, "The newHashMap length should be 1");
}
