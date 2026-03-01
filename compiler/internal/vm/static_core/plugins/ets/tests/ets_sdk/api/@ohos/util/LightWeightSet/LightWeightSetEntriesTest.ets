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
    const suite = new arktest.ArkTestsuite("LightWeightSet Entries API tests")

    suite.addTest("LightWeightSet entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("LightWeightSet entries return iterator by key and value with null lightWeightSet",
        testEntriesReturnIteratorByKeyValueWithEmptySet)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let lightWeightSet: LightWeightSet<double | undefined> = new LightWeightSet<double | undefined>();
    lightWeightSet.add(123);
    lightWeightSet.add(456);
    lightWeightSet.add(333);
    lightWeightSet.add(222);
    let iterator = lightWeightSet.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "123,123,222,222,333,333,456,456";
    arktest.assertEQ(str, strAll, "The lightWeightSet entries iterator should return all key-value pairs");
}

function testEntriesReturnIteratorByKeyValueWithEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let it = lightWeightSet.entries();
    arktest.assertTrue(it.next().done, "The lightWeightSet entries iterator.done should true for an empty set");
}
