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
    const suite = new arktest.ArkTestsuite("HashMap Constructor IsEmpty Clear API tests")

    suite.addTest("HashMap construct create default empty hashMap", testConstructorCreatesEmptyHashMap)
    suite.addTest("HashMap construct create hashMap add custom type data", testConstructorWithCustomTypeData)
    suite.addTest("HashMap isEmpty is true", testIsEmptyTrue)
    suite.addTest("HashMap isEmpty is false", testIsEmptyFalse)
    suite.addTest("HashMap isEmpty is true when remove the data", testIsEmptyAfterRemovingData)
    suite.addTest("HashMap isEmpty is true when clear the data", testIsEmptyAfterClearingData)
    suite.addTest("HashMap clear the data", testClearData)
    suite.addTest("HashMap clear the null hashMap", testClearEmptyHashMap)
    suite.addTest("HashMap clear data and set data again", testClearAndSetDataAgain)

    return suite.run()
}

function testConstructorCreatesEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    arktest.assertTrue(hashMap instanceof HashMap, "The hashMap should be of the same type as the HashMap");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

class Tmp {
    name: number = 0
}
function testConstructorWithCustomTypeData() {
    let hashMap: HashMap<string, Tmp> = new HashMap<string, Tmp>();
    let param: Tmp = {
        name: 1
    };
    hashMap.set("squirrel", param);
    let result = hashMap.hasValue(param);
    arktest.assertTrue(hashMap instanceof HashMap, "The hashMap should be of the same type as the HashMap");
    arktest.assertEQ(result, true, "The hashMap contains the specified value");
}

function testIsEmptyTrue() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let result = hashMap.isEmpty();
    arktest.assertEQ(result, true, "The hashMap should be empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testIsEmptyFalse() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result = hashMap.isEmpty();
    arktest.assertEQ(result, false, "The hashMap should not be empty");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testIsEmptyAfterRemovingData() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.isEmpty();
    hashMap.remove("squirrel")
    let result2 = hashMap.isEmpty();
    arktest.assertEQ(result1, false, "The hashMap should not be empty");
    arktest.assertEQ(result2, true, "The hashMap should be empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testIsEmptyAfterClearingData() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.isEmpty();
    hashMap.clear()
    let result2 = hashMap.isEmpty();
    arktest.assertEQ(result1, false, "The hashMap should not be empty");
    arktest.assertEQ(result2, true, "The hashMap should be empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testClearData() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    hashMap.clear();
    let result = hashMap.get("squirrel");
    arktest.assertEQ(result, undefined, "The hashMap should be cleared");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testClearEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.clear();
    arktest.assertEQ(hashMap.length, 0, "Empty HashMap should remain empty after clear");
}

function testClearAndSetDataAgain() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("ji", 12);
    hashMap.clear();
    hashMap.set("ji", 13);
    let result = hashMap.get("ji");
    arktest.assertEQ(result, 13, "The hashMap should return the correct value for the key");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}
