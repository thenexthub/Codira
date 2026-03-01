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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightSet ForEach Values Iterator API tests")

    suite.addTest("LightWeightSet forEach the ightWeightSet with no reference",
        testForEachNoReference)
    suite.addTest("LightWeightSet forEach the ightWeightSet with reference is value",
        testForEachValueReference)
    suite.addTest("LightWeightSet forEach the ightWeightSet with reference are value and key",
        testForEachValueKeyReference)
    suite.addTest("LightWeightSet forEach the ightWeightSet with three references",
        testForEachThreeReferences)
    suite.addTest("LightWeightSet values return iterator by value",
        testValuesIterator)
    suite.addTest("LightWeightSet values return iterator by value with null lightWeightSet",
        testValuesIteratorEmptySet)
    suite.addTest("LightWeightSet $_iterator return iterator",
        testIterator)
    suite.addTest("LightWeightSet $_iterator return iterator with null lightWeightSet",
        testIteratorEmptySet)

    return suite.run()
}

function testForEachNoReference() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let count: int = 0;
    lightWeightSet.forEach(() => {
        count++;
    });
    arktest.assertEQ(count, 2, "The lightWeightSet forEach should iterate over all elements");
}

function testForEachValueReference() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let flag = false;
    lightWeightSet.forEach((value: double | string) => {
        flag = lightWeightSet.getIndexOf("aa") == 0 && lightWeightSet.getIndexOf("bb") == 1;
        arktest.assertEQ(flag, true, "The lightWeightSet forEach should iterate over all values");
    });
}

function testForEachValueKeyReference() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let flag = false;
    lightWeightSet.forEach((value: double | string, key: double | string) => {
        flag = lightWeightSet.getIndexOf("aa") == 0 && lightWeightSet.getIndexOf("bb") == 1;
        arktest.assertEQ(flag, true, "The lightWeightSet forEach should iterate over all values and keys");
    });
}

function testForEachThreeReferences() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let flag = false;
    lightWeightSet.forEach((value: double | string, key: double | string,
        lightWeightSet: LightWeightSet<double | string>) => {
        flag = lightWeightSet.getIndexOf("aa") == 0 && lightWeightSet.getIndexOf("bb") == 1;
        arktest.assertEQ(flag, true, "The lightWeightSet forEach should iterate over all values, keys, and the set itself");
    });
}

function testValuesIterator() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let it = lightWeightSet.values();
    arktest.assertEQ(it.next().value, "aa", "The lightWeightSet values iterator should return the first value");
    arktest.assertEQ(it.next().value, "bb", "The lightWeightSet values iterator should return the second value");
    arktest.assertTrue(it.next().done, "The lightWeightSet values iterator.done should true for no more values");
}

function testValuesIteratorEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let it = lightWeightSet.values();
    arktest.assertTrue(it.next().done, "The lightWeightSet values iterator.done should true for an empty set");
}

function testIterator() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let it = lightWeightSet.$_iterator();
    arktest.assertEQ(it.next().value, "aa", "The lightWeightSet $_iterator should return the first value");
    arktest.assertEQ(it.next().value, "bb", "The lightWeightSet $_iterator should return the second value");
    arktest.assertTrue(it.next().done, "The lightWeightSet iterator.done should true for no more values");
}

function testIteratorEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let it = lightWeightSet.$_iterator();
    arktest.assertTrue(it.next().done, "The lightWeightSet iterator.done should true for an empty set");
}
