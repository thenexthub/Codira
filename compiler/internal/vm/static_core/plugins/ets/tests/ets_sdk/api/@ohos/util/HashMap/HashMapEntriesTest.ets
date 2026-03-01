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

import HashMap from "@ohos.util.HashMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("HashMap Entries API tests")

    suite.addTest("HashMap entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("HashMap entries return iterator by key and value with null hashMap",
        testEntriesReturnIteratorByKeyValueWithEmptyHashMap)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("sparrow", "123");
    hashMap.set("gull", "357");
    hashMap.set("test", "111");
    hashMap.set("apply", "222");
    let iterator = hashMap.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "sparrow,123,gull,357,test,111,apply,222";
    arktest.assertEQ(str, strAll, "The hashMap entries iterator should return all key-value pairs");
}

function testEntriesReturnIteratorByKeyValueWithEmptyHashMap() {
    let hashMap: HashMap<string, double> = new HashMap<string, double>();
    let iteratorSetValues = hashMap.entries();
    arktest.assertTrue(iteratorSetValues.next().done,
        "The hashMap entries iterator.done should true when the hashMap is empty");
}
