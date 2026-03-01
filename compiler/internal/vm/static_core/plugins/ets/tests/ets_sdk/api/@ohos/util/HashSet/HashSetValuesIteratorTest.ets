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
    const suite = new arktest.ArkTestsuite("HashSet Values Iterator API tests")

    suite.addTest("HashSet values return iterator by value", testValuesIterator)
    suite.addTest("HashSet values return iterator by value with null hashSet", testValuesIteratorWithEmptyHashSet)
    suite.addTest("HashSet $_iterator return iterator", testIterator)
    suite.addTest("HashSet $_iterator return iterator with null hashSet", testIteratorWithEmptyHashSet)

    return suite.run()
}

function testValuesIterator() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.add("sparrow");
    let iteratorSetValues = hashSet.values();
    arktest.assertEQ(iteratorSetValues.next().value, "squirrel", "The hashSet values iterator should return the first value");
    arktest.assertEQ(iteratorSetValues.next().value, "sparrow", "The hashSet values iterator should return the second value");
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashSet values iterator.done should true for no more values");
}

function testValuesIteratorWithEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let iteratorSetValues = hashSet.values();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashSet values iterator.done should true when the hashSet is empty");
}

function testIterator() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.add("sparrow");
    let iteratorSetValues = hashSet.$_iterator();
    arktest.assertEQ(iteratorSetValues.next().value, "squirrel", "The hashSet $_iterator should return the first value");
    arktest.assertEQ(iteratorSetValues.next().value, "sparrow", "The hashSet $_iterator should return the second value");
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashSet iterator.done should true for no more values");
}

function testIteratorWithEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let iteratorSetValues = hashSet.$_iterator();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashSet iterator.done should true when the hashSet is empty");
}
