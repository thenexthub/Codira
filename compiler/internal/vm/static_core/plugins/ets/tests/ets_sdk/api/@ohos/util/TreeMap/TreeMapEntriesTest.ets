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
    const suite = new arktest.ArkTestsuite("TreeMap Entries API tests")

    suite.addTest("TreeMap entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("TreeMap entries return iterator by key and value with null treeMap",
        testEntriesReturnIteratorByKeyValueWithEmptyTreeMap)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    treeMap.set("cc", "789");
    treeMap.set("dd", "112");
    let iterator = treeMap.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "aa,123,bb,456,cc,789,dd,112";
    arktest.assertEQ(str, strAll, "The treeMap should return the correct entries");
}

function testEntriesReturnIteratorByKeyValueWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.entries();
    arktest.assertTrue(it.next().done, "The treeMap iterator.done should true for the next entry in an empty map");
}
