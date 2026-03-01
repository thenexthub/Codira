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
    const suite = new arktest.ArkTestsuite("HashSet Clear Values ForEach API tests")

    suite.addTest("Testing HashSet clear on an empty set", testClearEmptySet)
    suite.addTest("Testing HashSet clear on a non-empty set", testClearNonEmptySet)
    suite.addTest("Testing HashSet clear then add an element", testClearThenAdd)
    suite.addTest("Testing HashSet values on an empty set, iterator.done is true", testValuesEmptySet)
    suite.addTest("Testing HashSet values on a non-empty set, check iterated elements", testValuesNonEmptySet)
    suite.addTest("Testing HashSet forEach on an empty set", testForEachEmptySet)
    suite.addTest("Testing HashSet forEach on a non-empty set, check elements and length", testForEachNonEmptySet)
    suite.addTest("Testing HashSet add an element inside forEach callback, check length", testForEachAddInsideCallback)
    suite.addTest("Testing HashSet clear the set inside forEach callback, check length", testForEachClearInsideCallback)
    suite.addTest("Testing HashSet clear the set inside forEach callback, ensure another set is unchanged",
        testForEachClearSetInsideCallbackAnotherSetUnchanged)
    suite.addTest("Testing HashSet forEach with all callback parameters and clear set inside",
        testForEachAllParamsClearInside)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:HASHSET_CLEAR_0100
function testClearEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHSET_CLEAR_0200
function testClearNonEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.clear();
    arktest.assertEQ(hashSet.length, 0);
}

//Test cases ported from ArkTS 1.0:HASHSET_CLEAR_0300
function testClearThenAdd() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.clear();
    hashSet.add(1);
    arktest.assertEQ(hashSet.length, 1);
}

//Test cases ported from ArkTS 1.0:HASHSET_VALUES_0100
function testValuesEmptySet() {
    const hashSet = new HashSet<double>();
    let values = hashSet.values();
    let temp = values.next();
    arktest.assertTrue(temp.done, "Expected iterator to be done for an empty HashSet");
}

//Test cases ported from ArkTS 1.0:HASHSET_VALUES_0200
function testValuesNonEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.add(2);
    hashSet.add(3);
    let values = hashSet.values();
    let temp = values.next();
    for (let index = 1; index <= 3; index++) {
        arktest.assertFalse(temp.done, `Iteration ${index}: done should be false`);
        arktest.assertEQ(temp.value, index);
        temp = values.next();
    }
    arktest.assertTrue(temp.done, `After 3 iterations, done should be true`);
    arktest.assertEQ(hashSet.length, 3);
}

//Test cases ported from ArkTS 1.0:HASHSET_FOREACH_0100
function testForEachEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.forEach((value?: double, key?: double) => {
        arktest.assertEQ(hashSet.length, 0);
    });
}

//Test cases ported from ArkTS 1.0:HASHSET_FOREACH_0200
function testForEachNonEmptySet() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.add(2);
    hashSet.add(3);
    let index = 1;
    hashSet.forEach((value?: double) => {
        arktest.assertEQ(value, index);
        index++;
    });
    arktest.assertEQ(hashSet.length, 3);
}

//Test cases ported from ArkTS 1.0:HASHSET_FOREACH_0300
function testForEachAddInsideCallback() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.forEach((value?: double) => {
        hashSet.add(2);
        arktest.assertEQ(hashSet.length, 2);
    });
}

//Test cases ported from ArkTS 1.0:HASHSET_FOREACH_0400
function testForEachClearInsideCallback() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.forEach((value?: double) => {
        hashSet.clear();
        arktest.assertEQ(hashSet.length, 0);
    });
}

//Test cases ported from ArkTS 1.0:HASHSET_FOREACH_0500
function testForEachClearSetInsideCallbackAnotherSetUnchanged() {
    const hashSet = new HashSet<double>();
    const hashSetNew = new HashSet<double>();
    hashSetNew.add(1);
    hashSet.add(1);
    hashSet.forEach((value?: double) => {
        hashSet.clear();
        arktest.assertEQ(hashSet.length, 0);
    });
    arktest.assertEQ(hashSetNew.length, 1);
    arktest.assertTrue(hashSetNew.has(1), "Expected hashSetNew to contain the value 1");
}

//Test cases ported from ArkTS 1.0:HASHSET_FOREACH_0600
function testForEachAllParamsClearInside() {
    const hashSet = new HashSet<double>();
    hashSet.add(1);
    hashSet.forEach((value: double, index, hashSet: HashSet<double>) => {
        hashSet.clear();
        arktest.assertEQ(hashSet.length, 0);
    })
}
