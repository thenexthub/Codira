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
    const suite = new arktest.ArkTestsuite("HashMap Length Constructor IsEmpty API tests")

    suite.addTest("Testing HashMap length is 0", testLengthNewMap)
    suite.addTest("Testing HashMap has data check length", testLengthAfterSet)
    suite.addTest("Testing HashMap remove->set->clear check length", testLengthAfterRemoveSetClear)
    suite.addTest("Testing HashMap constructor add sample data", testConstructorWithSimpleData)
    suite.addTest("Testing HashMap constructor add custom type data", testConstructorWithCustomData)
    suite.addTest("Testing HashMap empty data", testIsEmptyNewMap)
    suite.addTest("Testing HashMap has data", testIsEmptyAfterSet)
    suite.addTest("Testing HashMap has data and remove", testIsEmptyAfterSetRemove)
    suite.addTest("Testing HashMap has data and clear", testIsEmptyAfterSetClear)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHMAP_LENGTH_0100
function testLengthNewMap() {
    const hashMap = new HashMap<string, double>();
    arktest.assertEQ(hashMap.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHMAP_LENGTH_0200
function testLengthAfterSet() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("li", 1);
    arktest.assertEQ(hashMap.length, 1);
}

//Test cases ported from ArkTS 1.0:HASHMAP_LENGTH_0300
function testLengthAfterRemoveSetClear() {
    const hashMap = new HashMap<string, double>();
    hashMap.remove("zhang")
    hashMap.set("li", 1);
    hashMap.clear();
    arktest.assertEQ(hashMap.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHMAP_CONSTRUCTOR_0100
function testConstructorWithSimpleData() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("li", 1);
    arktest.assertEQ(hashMap.length, 1);
    arktest.assertTrue(hashMap.hasKey('li'), "hasKey('li') should return true when the key exists");
    arktest.assertEQ(hashMap.get('li'), 1);
}

class Tmp {
    name: number = 0
}

//Test cases ported from ArkTS 1.0:HASHMAP_CONSTRUCTOR_0200
function testConstructorWithCustomData() {
    const hashMap = new HashMap<string, Tmp>();
    let param: Tmp = {
        name: 1
    };
    hashMap.set("li", param);
    let result = hashMap.hasValue(param);
    arktest.assertTrue(result, "HashMap should contain the set custom object value");
}

//Test cases ported from ArkTS 1.0:HASHMAP_IS_EMPTY_0100
function testIsEmptyNewMap() {
    const hashMap = new HashMap<string, double>();
    arktest.assertTrue(hashMap.isEmpty(), "A newly created HashMap should be empty");
}

//Test cases ported from ArkTS 1.0:HASHMAP_IS_EMPTY_0200
function testIsEmptyAfterSet() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1)
    arktest.assertFalse(hashMap.isEmpty(), "HashMap should not be empty after adding an element");
}

//Test cases ported from ArkTS 1.0:HASHMAP_IS_EMPTY_0300
function testIsEmptyAfterSetRemove() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1)
    hashMap.remove("ji")
    arktest.assertTrue(hashMap.isEmpty(), "HashMap should be empty after removing the only element");
}

//Test cases ported from ArkTS 1.0:HASHMAP_IS_EMPTY_0400
function testIsEmptyAfterSetClear() {
    const hashMap = new HashMap<string, double>();
    hashMap.set("ji", 1)
    hashMap.clear();
    arktest.assertTrue(hashMap.isEmpty(), "HashMap should be empty after calling clear");
}
