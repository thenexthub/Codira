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

import {HashSet} from "@ohos.util.HashSet";

function main(): int {
    const suite = new arktest.ArkTestsuite("HashSet Length Constructor IsEmpty API tests")

    suite.addTest("Testing HashSet length is 0", testLengthOnNewSet)
    suite.addTest("Testing HashSet has data", testLengthAfterAdd)
    suite.addTest("Testing HashSet remove->add->clear", testLengthAfterRACSequence)
    suite.addTest("Testing HashSet add duplicate data length result", testLengthWithDuplicates)
    suite.addTest("Testing HashSet constructor add sample data", testConstructorAndAddSimpleType)
    suite.addTest("Testing HashSet constructor add custom type data", testConstructorAndAddCustomType)
    suite.addTest("Testing HashSet empty data", testIsEmptyOnNewSet)
    suite.addTest("Testing HashSet has data", testIsEmptyAfterAdd)
    suite.addTest("Testing HashSet has data and remove", testIsEmptyAfterAddRemove)
    suite.addTest("Testing HashSet has data and clear", testIsEmptyAfterAddClear)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHSET_LENGTH_0100
function testLengthOnNewSet() {
    const hashSet = new HashSet<double>();
    arktest.assertEQ(hashSet.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHSET_LENGTH_0200
function testLengthAfterAdd() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertEQ(hashSet.length, 1);
}

//Test cases ported from ArkTS 1.0:HASHSET_LENGTH_0300
function testLengthAfterRACSequence() {
    const hashSet = new HashSet<double>();
    hashSet.remove(1);
    arktest.assertFalse(hashSet.has(1),
        "Expected HashSet.has(1) to return false as 1 was not added or removed from an empty set");
    hashSet.add(1);
    arktest.assertTrue(hashSet.has(1), "Expected HashSet.has(1) to return true after adding 1");
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHSET_LENGTH_0400
function testLengthWithDuplicates() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.add(1);
    arktest.assertEQ(hashSet.length, 1);
}

//Test cases ported from ArkTS 1.0:HASHSET_CONSTRUCTOR_0100
function testConstructorAndAddSimpleType() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertEQ(hashSet.length, 1);
}

class Tmp {
    name: number = 0
}

//Test cases ported from ArkTS 1.0:HASHSET_CONSTRUCTOR_0200
function testConstructorAndAddCustomType() {
    const hashSet = new HashSet<Tmp>();
    let param: Tmp = {
        name: 1
    };
    hashSet.add(param);
    let result = hashSet.has(param);
    arktest.assertTrue(result, "Expected HashSet to contain the added custom type element");
}

//Test cases ported from ArkTS 1.0:HASHSET_IS_EMPTY_0100
function testIsEmptyOnNewSet() {
    const hashSet = new HashSet<double>();
    arktest.assertTrue(hashSet.isEmpty(), "Expected HashSet to be empty after initialization");
}

//Test cases ported from ArkTS 1.0:HASHSET_IS_EMPTY_0200
function testIsEmptyAfterAdd() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertFalse(hashSet.isEmpty(), "Expected HashSet to not be empty after adding an element");
}

//Test cases ported from ArkTS 1.0:HASHSET_IS_EMPTY_0300
function testIsEmptyAfterAddRemove() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.remove(1);
    arktest.assertFalse(hashSet.has(1), "Expected HashSet.has(1) to return false after removing 1");
    arktest.assertTrue(hashSet.isEmpty(), "Expected HashSet to be empty after removing the only element");
}

//Test cases ported from ArkTS 1.0:HASHSET_IS_EMPTY_0400
function testIsEmptyAfterAddClear() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.clear();
    arktest.assertTrue(hashSet.isEmpty(), "Expected HashSet to be empty after clearing all elements");
}
