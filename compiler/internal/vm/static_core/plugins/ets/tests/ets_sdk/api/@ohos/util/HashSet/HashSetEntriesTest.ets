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
    const suite = new arktest.ArkTestsuite("HashSet Entries API tests")

    suite.addTest("HashSet entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("HashSet entries return iterator by key and value with null hashSet",
        testEntriesReturnIteratorByKeyValueWithEmptyHashSet)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let newHashSet: HashSet<double | undefined> = new HashSet<double | undefined>();
    newHashSet.add(1);
    newHashSet.add(2);
    newHashSet.add(3);
    newHashSet.add(4);
    let iterator = newHashSet.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "1,1,2,2,3,3,4,4";
    arktest.assertEQ(str, strAll, "The hashSet entries iterator should return all key-value pairs");
}

function testEntriesReturnIteratorByKeyValueWithEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let iteratorSetValues = hashSet.entries();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashSet entries iterator.done should true when the hashSet is empty");
}
