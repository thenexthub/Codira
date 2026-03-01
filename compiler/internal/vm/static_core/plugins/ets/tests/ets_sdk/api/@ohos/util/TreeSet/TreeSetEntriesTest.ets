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

import {TreeSet} from "@ohos.util.TreeSet";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeSet Entries API tests")

    suite.addTest("TreeSet entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("TreeSet entries return iterator by key and value with null treeSet",
        testEntriesReturnIteratorByKeyValueWithEmptyTreeSet)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let treeSet: TreeSet<double | undefined> = new TreeSet<double | undefined>(
        (firstValue: double | undefined, secondValue: double | undefined): double => {
            return secondValue! - firstValue!
        });
    treeSet.add(123);
    treeSet.add(456);
    treeSet.add(222);
    treeSet.add(333);
    let iterator = treeSet.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "123,123,222,222,333,333,456,456";
    arktest.assertEQ(str, strAll, "The treeSet entries iterator should return the first value");
}

function testEntriesReturnIteratorByKeyValueWithEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    let it = treeSet.entries();
    arktest.assertTrue(it.next().done, "The treeSet entries iterator.done should true when treeSet is empty");
}
