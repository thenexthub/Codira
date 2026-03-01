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
    const suite = new arktest.ArkTestsuite("HashSet Has API tests")

    suite.addTest("HashSet has value of the hashSet", testHasValue)
    suite.addTest("HashSet has not set value of the hashSet", testHasNotSetValue)
    suite.addTest("HashSet has value of the null hashSet", testHasValueOfEmptyHashSet)
    suite.addTest("HashSet has null value of the hashSet", testHasNullValue)
    suite.addTest("HashSet has not set null value of the hashSet", testHasNotSetNullValue)
    suite.addTest("HashSet has value after clear data", testHasValueAfterClear)
    suite.addTest("HashSet has empty string", testHasEmptyString)

    return suite.run()
}

function testHasValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has("squirrel");
    arktest.assertEQ(result, true, "The hashSet should contain the specified value");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasNotSetValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has("test");
    arktest.assertEQ(result, false, "The hashSet should not contain the specified value that is not set");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasValueOfEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let result = hashSet.has("test");
    arktest.assertEQ(result, false, "The hashSet should not contain the specified value when the hashSet is empty");
    arktest.assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testHasNullValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add(null);
    let result = hashSet.has(null);
    arktest.assertEQ(result, true, "The hashSet should contain the null value");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasNotSetNullValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has(null);
    arktest.assertEQ(result, false, "The hashSet should not contain the specified value that is not set");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasValueAfterClear() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result1 = hashSet.isEmpty();
    hashSet.clear();
    let result2 = hashSet.has("squirrel");
    arktest.assertEQ(result1, false, "The hashSet should not be empty");
    arktest.assertEQ(result2, false, "The hashSet should contain the specified value that is not set");
    arktest.assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testHasEmptyString() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("");
    let result = hashSet.has("");
    arktest.assertEQ(result, true, "The hashSet should contain the specified value");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}
