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

function main(): int {
    const suite = new arktest.ArkTestsuite("HashMap Remove Replace API tests")

    suite.addTest("HashMap remove key", testRemoveExistingKey)
    suite.addTest("HashMap remove not set key", testRemoveNonExistingKey)
    suite.addTest("HashMap remove key from the null hashMap", testRemoveKeyFromEmptyHashMap)
    suite.addTest("HashMap remove empty string", testRemoveEmptyStringKey)
    suite.addTest("HashMap remove when clear data", testRemoveKeyAfterClearingHashMap)
    suite.addTest("HashMap replace new value", testReplaceExistingKeyWithNewValue)
    suite.addTest("HashMap replace new value when key not set", testReplaceNonExistingKey)
    suite.addTest("HashMap replace new value to the null hashMap", testReplaceKeyInEmptyHashMap)
    suite.addTest("HashMap replace empty string", testReplaceEmptyStringKey)
    suite.addTest("HashMap replace new value when data is not change", testReplaceKeyWithSameValue)

    return suite.run()
}

function testRemoveExistingKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result = hashMap.remove("squirrel");
    arktest.assertEQ(result, 123, "The hashMap should remove the key and return its value");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testRemoveNonExistingKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result = hashMap.remove("test");
    arktest.assertEQ(result, undefined, "The hashMap should return undefined when key is not set");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testRemoveKeyFromEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let result = hashMap.remove("squirrel");
    arktest.assertEQ(result, undefined, "The hashMap should return undefined when the map is empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testRemoveEmptyStringKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set('', 11);
    let result = hashMap.remove('');
    arktest.assertEQ(result, 11, "The hashMap should remove the key and return its value");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testRemoveKeyAfterClearingHashMap() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    let result1 = hashMap.isEmpty();
    hashMap.clear();
    let result2 = hashMap.remove("ji");
    arktest.assertEQ(result1, false, "The hashMap should not be empty");
    arktest.assertEQ(result2, undefined, "The hashMap should return undefined when the map is empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testReplaceExistingKeyWithNewValue() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.replace("squirrel", 789);
    let result2 = hashMap.get("squirrel");
    arktest.assertEQ(result1, true, "The hashMap should replace the value by the key");
    arktest.assertEQ(result2, 789, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testReplaceNonExistingKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result = hashMap.replace("test", 789);
    arktest.assertEQ(result, false, "The hashMap should return false by the key that is not set");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testReplaceKeyInEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let result = hashMap.replace("test", 789);
    arktest.assertEQ(result, false, "The hashMap should return false by key when the map is empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testReplaceEmptyStringKey() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set('', 123)
    let result = hashMap.replace('', 789);
    arktest.assertEQ(result, true, "The hashMap should replace the value by the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testReplaceKeyWithSameValue() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.replace("squirrel", 123);
    let result2 = hashMap.get("squirrel");
    arktest.assertEQ(result1, true, "The hashMap should replace the value by the key");
    arktest.assertEQ(result2, 123, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}
