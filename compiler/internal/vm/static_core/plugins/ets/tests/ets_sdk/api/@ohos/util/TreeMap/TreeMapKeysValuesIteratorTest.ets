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

import TreeMap from "@ohos.util.TreeMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeMap Keys Values Iterator API tests")

    suite.addTest("TreeMap keys return iterator by key",
        testKeysReturnIteratorByKey)
    suite.addTest("TreeMap keys return iterator by key with null treeMap",
        testKeysReturnIteratorByKeyWithEmptyTreeMap)
    suite.addTest("TreeMap values return iterator by value",
        testValuesReturnIteratorByValue)
    suite.addTest("TreeMap values return iterator by value with null treeMap",
        testValuesReturnIteratorByValueWithEmptyTreeMap)
    suite.addTest("TreeMap $_iterator return iterator",
        testIteratorReturnIterator)
    suite.addTest("TreeMap $_iterator return iterator with null TreeMap",
        testIteratorReturnIteratorWithEmptyTreeMap)

    return suite.run()
}

function testKeysReturnIteratorByKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let it = treeMap.keys();
    arktest.assertEQ(it.next().value, "aa", "The treeMap should return the correct key");
    arktest.assertEQ(it.next().value, "bb", "The treeMap should return the correct key");
    arktest.assertTrue(it.next().done, "The treeMap iterator.done should true for the next key in an empty map");
}

function testKeysReturnIteratorByKeyWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.keys();
    arktest.assertTrue(it.next().done, "The treeMap iterator.done should true for the key in an empty map");
}

function testValuesReturnIteratorByValue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let it = treeMap.values();
    arktest.assertEQ(it.next().value, "123", "The treeMap should return the correct value");
    arktest.assertEQ(it.next().value, "456", "The treeMap should return the correct value");
    arktest.assertTrue(it.next().done, "The treeMap iterator.done should true for the next value in an empty map");
}

function testValuesReturnIteratorByValueWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.values();
    arktest.assertTrue(it.next().done, "The treeMap iterator.done should true for the next value in an empty map");
}

function testIteratorReturnIterator() {
    let treeMap: TreeMap<string, double | undefined> =
        new TreeMap<string, double | undefined>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", 123);
    treeMap.set("bb", 456);
    treeMap.set("cc", 789);
    treeMap.set("dd", 112);
    let iterator = treeMap.$_iterator();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "aa,123,bb,456,cc,789,dd,112";
    arktest.assertEQ(str, strAll, "The treeMap should return the correct iterator");
}

function testIteratorReturnIteratorWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.$_iterator();
    arktest.assertTrue(it.next().done, "The treeMap iterator.done should true for the next iterator in an empty map");
}
