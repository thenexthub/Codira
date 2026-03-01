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

//Test cases ported from ArkTS 1.0:container_hashset.js
function main() {
    const suite = new arktest.ArkTestsuite("HashSet API tests");

    suite.addTest("Test HashSet: add, length, has, values, for...in, forEach, isEmpty, remove, clear",
        testBasicOperations);
    suite.addTest("Test HashSet: add numeric values and sum using forEach", testForEachSumNumericValues);
    suite.addTest("Test HashSet: entries() iterator and for...of iteration", testEntriesIteratorAndForOf);
    suite.addTest("Test HashSet: entries() iterator, verifying key equals value", testEntriesIteratorKeyEqualsValue);

    return suite.run();
}

// Verifies fundamental HashSet operations including add, has, values, for...in, forEach, isEmpty, remove, and clear.
function testBasicOperations() {
    let hashSet = new HashSet<string>();
    hashSet.add("aa");
    hashSet.add("bb");
    arktest.assertEQ(hashSet.length, 2);
    arktest.assertTrue(hashSet.has("aa"), "Expected HashSet to contain 'aa'");
    arktest.assertTrue(hashSet.has("bb"), "Expected HashSet to contain 'bb'");
    arktest.assertFalse(hashSet.has("cc"), "Expected HashSet to not contain 'cc'");

    hashSet.add("cc");
    let iteratorSetValues = hashSet.values();
    arktest.assertEQ(iteratorSetValues.next().value, "aa");
    arktest.assertEQ(iteratorSetValues.next().value, "bb");
    arktest.assertEQ(iteratorSetValues.next().value, "cc");
    arktest.assertEQ(iteratorSetValues.next().value, undefined);

    let itemArr = ["aa", "bb", "cc"]
    let item: string;
    let i: int = 0;
    for (item of hashSet) {
        arktest.assertEQ(item, itemArr[i]);
        i++;
    }

    hashSet.forEach((i, d) => { });
    arktest.assertFalse(hashSet.isEmpty(), "Expected HashSet to not be empty after adding elements");

    hashSet.add("ee");
    hashSet.add("dd");
    arktest.assertTrue(hashSet.remove("bb"), "Expected remove('bb') to return true");
    arktest.assertFalse(hashSet.has("bb"), "Expected HashSet to not contain 'bb' after removal");
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
    arktest.assertFalse(hashSet.has("cc"), "Expected HashSet to not contain 'cc' after clear");
    arktest.assertTrue(hashSet.isEmpty(), "Expected HashSet to be empty after clear");
}

// Tests `forEach` by adding doubles to a `HashSet` and summing them, verifying callback values and the total sum.
function testForEachSumNumericValues() {
    let collisionSet = new HashSet<double>();
    let count = 0;

    collisionSet.add(1224);
    collisionSet.add(1288);
    collisionSet.add(1464);
    collisionSet.add(4312);
    collisionSet.add(5128);
    collisionSet.add(5896);
    collisionSet.add(6600);
    collisionSet.add(6776);
    collisionSet.add(8424);
    collisionSet.add(9400);
    collisionSet.forEach((value, key) => {
        count += value;
    });
    arktest.assertEQ(count, 50512);
}

// Verifies `entries()` iterator behavior and `for...of` loop iteration for a HashSet of strings.
function testEntriesIteratorAndForOf() {
    let hashSet = new HashSet<string>();
    hashSet.add("aa");
    hashSet.add("bb");
    hashSet.add("cc");

    let iteratorSetEntries = hashSet.entries();
    arktest.assertEQ(iteratorSetEntries.next().value![0], "aa");
    arktest.assertEQ(iteratorSetEntries.next().value![0], "bb");
    arktest.assertEQ(iteratorSetEntries.next().value![0], "cc");
    arktest.assertEQ(iteratorSetEntries.next().value, undefined);
    arktest.assertTrue(iteratorSetEntries.next().done, "Expected iterator to be done after iterating all elements");

    let arr = ["aa", "bb", "cc"];
    let i: int = 0;
    for (const item of hashSet) {
        arktest.assertEQ(item, arr[i]);
        i++;
    }
    arktest.assertEQ(i, hashSet.length);
}

// Tests `entries()` iterator for doubles, iterator's `done` status, and final set length.
function testEntriesIteratorKeyEqualsValue() {
    let seten = new HashSet<double>();
    seten.add(1);
    seten.add(2);
    seten.add(3);
    seten.add(4);
    seten.add(5);
    let iter = seten.entries();
    let temp = iter.next();
    for (let count = 0; count <= 4; count++) {
        arktest.assertFalse(temp.done, `Iteration ${count}: done should be false`);
        arktest.assertEQ(temp.value![0], temp.value![1]);
        temp = iter.next();
    }
    arktest.assertTrue(temp.done, `After 5 iterations, done should be true`);
    arktest.assertEQ(seten.length, 5);
}
