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
    const suite = new arktest.ArkTestsuite("HashSet Constructor IsEmpty API tests")

    suite.addTest("HashSet construct create default empty hashSet", testConstructorCreatesDefaultEmptyHashSet)
    suite.addTest("HashSet construct create hashSet add custom type data", testConstructorWithCustomTypeData)
    suite.addTest("HashSet isEmpty is true", testIsEmptyTrue)
    suite.addTest("HashSet isEmpty is false", testIsEmptyFalse)
    suite.addTest("HashSet isEmpty is true after remove data", testIsEmptyTrueAfterRemoveData)
    suite.addTest("HashSet isEmpty is true after clear data", testIsEmptyTrueAfterClearData)

    return suite.run()
}

function testConstructorCreatesDefaultEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    arktest.assertTrue(hashSet instanceof HashSet, "The hashSet should be of the same type as the HashSet");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

class Tmp {
    name: number = 0
}
function testConstructorWithCustomTypeData() {
    let hashSet: HashSet<Tmp> = new HashSet<Tmp>();
    let param: Tmp = {
        name: 1
    };
    hashSet.add(param);
    let result = hashSet.has(param);
    arktest.assertTrue(hashSet instanceof HashSet, "The hashSet should be of the same type as the HashSet");
    arktest.assertEQ(result, true, "The hashSet should contain the specified value");
}

function testIsEmptyTrue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let result = hashSet.isEmpty();
    arktest.assertEQ(result, true, "The hashSet should be empty");
    arktest.assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testIsEmptyFalse() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.isEmpty();
    arktest.assertEQ(result, false, "The hashSet should not be empty");
    arktest.assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testIsEmptyTrueAfterRemoveData() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result1 = hashSet.isEmpty();
    hashSet.remove("squirrel");
    let result2 = hashSet.isEmpty();
    arktest.assertEQ(result1, false, "The hashSet should not be empty");
    arktest.assertEQ(result2, true, "The hashSet should be empty");
    arktest.assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testIsEmptyTrueAfterClearData() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result1 = hashSet.isEmpty();
    hashSet.clear();
    let result2 = hashSet.isEmpty();
    arktest.assertEQ(result1, false, "The hashSet should not be empty");
    arktest.assertEQ(result2, true, "The hashSet should be empty");
    arktest.assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}
