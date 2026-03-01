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
    const suite = new arktest.ArkTestsuite("HashMap API HasKey HasValue tests")

    suite.addTest("HashMap hasKey and hasValue designate key and value", testHasKeyHasValueWithExistingKeyAndValue)
    suite.addTest("HashMap hasKey and hasValue not set key and value", testHasKeyHasValueWithNonExistingKeyAndValue)
    suite.addTest("HashMap hasKey and hasValue key and value from the null hashMap", testHasKeyHasValueWithEmptyHashMap)
    suite.addTest("HashMap hasKey and hasValue empty string", testHasKeyHasValueWithEmptyStringKey)
    suite.addTest("HashMap hasKey and hasValue data when data is exchange", testHasKeyHasValueAfterDataExchange)
    suite.addTest("HashMap hasKey and hasValue when clear data", testHasKeyHasValueAfterClearingData)

    return suite.run()
}

function testHasKeyHasValueWithExistingKeyAndValue() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.hasKey("squirrel");
    let result2 = hashMap.hasValue(123);
    arktest.assertEQ(result1, true, "The hashMap contains the specified key");
    arktest.assertEQ(result2, true, "The hashMap contains the specified value");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueWithNonExistingKeyAndValue() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.hasKey("sparrow");
    let result2 = hashMap.hasValue(456);
    arktest.assertEQ(result1, false, "The hashMap not contains the specified key that is not set");
    arktest.assertEQ(result2, false, "The hashMap not contains the specified value that is not set");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueWithEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let result1 = hashMap.hasKey("squirrel");
    let result2 = hashMap.hasValue(123);
    arktest.assertEQ(result1, false, "The hashMap not contains the specified key when the hashMap is empty");
    arktest.assertEQ(result2, false, "The hashMap not contains the specified value when the hashMap is empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testHasKeyHasValueWithEmptyStringKey() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("", "");
    let result1 = hashMap.hasKey("");
    let result2 = hashMap.hasValue("");
    arktest.assertEQ(result1, true, "The hashMap contains the empty string key");
    arktest.assertEQ(result2, true, "The hashMap contains the empty string value");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueAfterDataExchange() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    let result1 = hashMap.hasKey("hello");
    let result2 = hashMap.hasValue("ji");
    arktest.assertEQ(result1, false, "The hashMap not contains the specified key that is not set");
    arktest.assertEQ(result2, false, "The hashMap not contains the specified value that is not set");
    arktest.assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueAfterClearingData() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    let result1 = hashMap.isEmpty();
    hashMap.clear();
    let result2 = hashMap.hasKey("ji");
    let result3 = hashMap.hasValue("hello");
    arktest.assertEQ(result1, false, "The hashMap should not be empty");
    arktest.assertEQ(result2, false, "The hashMap not contains the specified key when the hashMap is empty");
    arktest.assertEQ(result3, false, "The hashMap not contains the specified value when the hashMap is empty");
    arktest.assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}
