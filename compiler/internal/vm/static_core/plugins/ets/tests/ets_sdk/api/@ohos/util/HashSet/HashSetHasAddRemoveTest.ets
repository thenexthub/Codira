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
    const suite = new arktest.ArkTestsuite("HashSet Has Add Remove API tests")

    suite.addTest("Testing HashSet is empty and check if empty string has", testHasEmptyStringOnEmptySet)
    suite.addTest("Testing HashSet is empty and check if value has", testHasValueOnEmptySet)
    suite.addTest("Testing HashSet has 1 and check if 1 has", testHasExistingElement)
    suite.addTest("Testing HashSet has 1 and check if 2 has", testHasNonExistingElement)
    suite.addTest("Testing HashSet remove->add->clear", testHasAfterRACSequence)
    suite.addTest("Testing HashSet data empty string check length =1 and has empty string", testAddEmptyString)
    suite.addTest("Testing HashSet data a check length =1 and has a", testAddShortString)
    suite.addTest("Testing HashSet data string length is 2048 check length =1 and has string", testAddLongString)
    suite.addTest("Testing HashSet data has 1000 check length =1 and has 1000 data", testAddMultipleElements)
    suite.addTest("Testing HashSet remove->add->clear", testAddInRACSequence)
    suite.addTest("Testing HashSet is empty and remove value is empty string", testRemoveEmptyStringFromEmptySet)
    suite.addTest("Testing HashSet is empty and remove value is a", testRemoveValueFromEmptySet)
    suite.addTest("Testing HashSet has 1 and remove value is 1", testRemoveExistingElement)
    suite.addTest("Testing HashSet has 1 and remove value is 2", testRemoveNonExistingElement)
    suite.addTest("Testing HashSet remove->add->clear", testRemoveAfterRACSequence)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHSET_HAS_0100
function testHasEmptyStringOnEmptySet() {
    const hashSet = new HashSet<string>();
    arktest.assertFalse(hashSet.has(""), "Expected HashSet.has('') to return false for an empty HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_HAS_0400
function testHasValueOnEmptySet() {
    const hashSet = new HashSet<string>();
    arktest.assertFalse(hashSet.has('value'), "Expected HashSet.has('value') to return false for an empty HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_HAS_0500
function testHasExistingElement() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertTrue(hashSet.has(1), "Expected HashSet.has(1) to return true after adding 1 to the HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_HAS_0600
function testHasNonExistingElement() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertFalse(hashSet.has(2), "Expected HashSet.has(2) to return false when 2 was not added to the HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_HAS_0700
function testHasAfterRACSequence() {
    const hashSet = new HashSet<double>();
    hashSet.remove(1);
    arktest.assertFalse(hashSet.has(1),
        "Expected HashSet.has(1) to return false as 1 was not added or removed from an empty set");
    hashSet.add(1);
    arktest.assertTrue(hashSet.has(1), "Expected HashSet.has(1) to return true after adding 1");
    hashSet.clear();
    arktest.assertFalse(hashSet.has(1), "Expected HashSet.has(1) to return false after clearing the HashSet");
    arktest.assertEQ(hashSet.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHSET_ADD_0100
function testAddEmptyString() {
    const hashSet = new HashSet<string>();
    hashSet.add("");
    let result = hashSet.length == 1 && hashSet.has("");
    arktest.assertTrue(result, "Expected HashSet to have length 1 and contain an empty string after adding it");
}

//Test cases ported from ArkTS 1.0:HASHSET_ADD_0200
function testAddShortString() {
    const hashSet = new HashSet<string>();
    hashSet.add("a");
    let result = hashSet.length == 1 && hashSet.has("a");
    arktest.assertTrue(result, "Expected HashSet to have length 1 and contain 'a' after adding it");
}

//Test cases ported from ArkTS 1.0:HASHSET_ADD_0500
function testAddLongString() {
    const hashSet = new HashSet<string>();
    let param = 'a'.repeat(2048);
    hashSet.add(param);
    let result = hashSet.length == 1 && hashSet.has(param);
    arktest.assertTrue(result,
        "Expected HashSet to have length 1 and contain the 2048-character string after adding it");
}

//Test cases ported from ArkTS 1.0:HASHSET_ADD_0600
function testAddMultipleElements() {
    const hashSet = new HashSet<double>();
    for (let i: int = 0; i < 1000; i++) {
        hashSet.add(i);
    }
    for (let i: int = 0; i < hashSet.length; i++) {
        arktest.assertTrue(hashSet.has(i), "Expected HashSet to contain element after adding 1000 elements");
    }
    arktest.assertEQ(hashSet.length, 1000);
}

//Test cases ported from ArkTS 1.0:HASHSET_ADD_0700
function testAddInRACSequence() {
    const hashSet = new HashSet<double>();
    hashSet.remove(1);
    arktest.assertFalse(hashSet.has(1),
        "Expected HashSet.has(1) to return false as 1 was not added or removed from an empty set");
    hashSet.add(1);
    arktest.assertTrue(hashSet.has(1), "Expected HashSet.has(1) to return true after adding 1");
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
    arktest.assertFalse(hashSet.has(1), "Expected HashSet not to contain 1 after remove, add, and clear operations");
}

//Test cases ported from ArkTS 1.0:HASHSET_REMOVE_0100
function testRemoveEmptyStringFromEmptySet() {
    const hashSet = new HashSet<string>();
    arktest.assertFalse(hashSet.remove(""), "Expected HashSet.remove('') to return false for an empty HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_REMOVE_0400
function testRemoveValueFromEmptySet() {
    const hashSet = new HashSet<string>();
    arktest.assertFalse(hashSet.remove("a"), "Expected HashSet.remove('a') to return false for an empty HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_REMOVE_0500
function testRemoveExistingElement() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertTrue(hashSet.remove(1), "Expected HashSet.remove(1) to return true after adding 1 to the HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_REMOVE_0600
function testRemoveNonExistingElement() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    arktest.assertFalse(hashSet.remove(2), "Expected HashSet.remove(2) to return false when not added to the HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_REMOVE_0700
function testRemoveAfterRACSequence() {
    const hashSet = new HashSet<double>();
    hashSet.remove(1);
    arktest.assertFalse(hashSet.has(1),
        "Expected HashSet.has(1) to return false as 1 was not added or removed from an empty set");
    hashSet.add(1);
    arktest.assertTrue(hashSet.has(1), "Expected HashSet.has(1) to return true after adding 1");
    hashSet.clear();
    arktest.assertFalse(hashSet.remove(1), "Expected HashSet.remove(1) to return false after clearing the HashSet");
    arktest.assertFalse(hashSet.has(1), "Expected HashSet.has(1) to return false after clearing the HashSet");
}
