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
    const suite = new arktest.ArkTestsuite("HashSet Entries Iterator API tests")

    suite.addTest("Testing HashSet is empty get entries length", testEntriesEmptySet)
    suite.addTest("Testing HashSet has data check everyOne and length", testEntriesNonEmptySet)
    suite.addTest("Testing HashSet has data add and check length", testEntriesAddDuringIteration)
    suite.addTest("Testing HashSet has data clear and check length", testEntriesClearDuringIteration)
    suite.addTest("Testing $_iterator().next().done for an empty HashSet", testSymbolIteratorEmptySet)
    suite.addTest("Testing $_iterator() iteration, value, done, and length for a non-empty HashSet",
        testSymbolIteratorNonEmptySet)
    suite.addTest("Testing HashSet has data check custom value", testCustomObjectSetBasicOperations)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHSET_ENTRIES_0100
function testEntriesEmptySet() {
    const hashSet = new HashSet<double>();
    let entries = hashSet.entries();
    let temp = entries.next();
    arktest.assertTrue(temp.done, "Expected iterator to be done for an empty set");
}

//Test cases ported from ArkTS 1.0:HASHSET_ENTRIES_0200
function testEntriesNonEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.add(2);
    hashSet.add(3);
    let entries = hashSet.entries();
    let temp = entries.next();
    for (let index = 1; index <= 3; index++) {
        arktest.assertFalse(temp.done, `Iteration ${index}: done should be false`);
        arktest.assertEQ(temp.value![1], index);
        temp = entries.next();
    }
    arktest.assertTrue(temp.done, `After 3 iterations, done should be true`);
    arktest.assertEQ(hashSet.length, 3);
}

//Test cases ported from ArkTS 1.0:HASHSET_ENTRIES_0300
function testEntriesAddDuringIteration() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    let entries = hashSet.entries();
    let temp = entries.next();
    for (let index = 1; index <= 2; index++) {
        arktest.assertFalse(temp.done, `Iteration ${index}: done should be false`);
        hashSet.add(2);
        arktest.assertEQ(temp.value![1], index);
        temp = entries.next();
    }
    arktest.assertTrue(temp.done, `After 2 iterations, done should be true`);
    arktest.assertEQ(hashSet.length, 2);
}

//Test cases ported from ArkTS 1.0:HASHSET_ENTRIES_0400
function testEntriesClearDuringIteration() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    let entries = hashSet.entries();
    let temp = entries.next();
    arktest.assertFalse(temp.done, "Expected iterator to not be done before clearing");
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
    temp = entries.next();
    arktest.assertTrue(temp.done, "Expected iterator to be done after clearing the set and calling next()");
}

//Test cases ported from ArkTS 1.0:HASHSET_SYMBOL_ITERATOR_0100
function testSymbolIteratorEmptySet() {
    const hashSet = new HashSet<double>();
    let iter = hashSet.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    arktest.assertTrue(temp.done, "Expected iterator to be done for an empty set");
}

//Test cases ported from ArkTS 1.0:HASHSET_SYMBOL_ITERATOR_0200
function testSymbolIteratorNonEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.add(2);
    hashSet.add(3);
    let iter = hashSet.$_iterator();
    let temp: IteratorResult<double>;
    for (let index = 1; index <= 3; index++) {
        temp = iter.next();
        arktest.assertFalse(temp.done, `Iteration ${index}: done should be false`);
        arktest.assertEQ(temp.value, index);
    }
    temp = iter.next();
    arktest.assertTrue(temp.done, `After 3 iterations, done should be true`);
    arktest.assertEQ(hashSet.length, 3);
}

class Temp {
    age: number = 0
    name: string = ""
}

//Test cases ported from ArkTS 1.0:HASHSET_CUSTOM_0100
function testCustomObjectSetBasicOperations() {
    const hashSet = new HashSet<Temp>();
    let temp: Temp = {
        age: 18, name: "zhang san"
    };
    hashSet.add(temp)
    let temp1: Temp = {
        age: 19, name: "li si"
    };
    hashSet.add(temp1)
    let temp2: Temp = {
        age: 31, name: "wang wu"
    };
    hashSet.add(temp2);
    arktest.assertFalse(hashSet.isEmpty(), "Expected HashSet to not be empty after adding elements");
    let removeResult = hashSet.remove(temp);
    arktest.assertTrue(removeResult, "Expected HashSet.remove to return true for an existing element");
    arktest.assertFalse(hashSet.has(temp), "Expected HashSet to not contain the removed element");
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
}
