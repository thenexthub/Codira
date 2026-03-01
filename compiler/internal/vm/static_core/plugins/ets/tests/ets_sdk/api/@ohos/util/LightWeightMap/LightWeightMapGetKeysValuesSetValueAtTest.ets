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

import LightWeightMap from "@ohos.util.LightWeightMap";

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightMap Get Keys Values SetValueAt API tests")

    suite.addTest("Testing LightWeightMap get large string value by key", testGetLargeStringValue)
    suite.addTest("Testing LightWeightMap get all key for a LightWeightMap contain large string key",
        testKeysWithLargeStringKey)
    suite.addTest("Testing LightWeightMap get all value for a LightWeightMap contain large string value",
        testValuesWithLargeStringValue)
    suite.addTest("Testing LightWeightMap set large string value by index", testSetValueAtLargeString)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GET_0600
function testGetLargeStringValue() {
    let value = '';
    for (let i: number = 0; i < 10000; i++) {
        value += i;
    }
    let lightWeightMap = new LightWeightMap<string, string>();
    lightWeightMap.set("squirrel", value);
    arktest.assertEQ(lightWeightMap.get("squirrel"), value);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_KEYS_0300
function testKeysWithLargeStringKey() {
    let key = '';
    for (let i: number = 0; i < 10000; i++) {
        key += i;
    }
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set(key, 123);
    let iter: IterableIterator<string> = lightWeightMap.keys();
    let temp: IteratorResult<string> = iter.next();
    arktest.assertFalse(temp.done,
        "Expected the iterator not to be done for a LightWeightMap with one large string key");
    arktest.assertEQ(temp.value, key);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_VALUES_0300
function testValuesWithLargeStringValue() {
    let value = '';
    for (let i: number = 0; i < 10000; i++) {
        value += i;
    }
    let lightWeightMap = new LightWeightMap<string, string>();
    lightWeightMap.set("squirrel", value);
    let iter: IterableIterator<string> = lightWeightMap.values();
    let temp: IteratorResult<string> = iter.next();
    arktest.assertFalse(temp.done,
        "Expected the iterator not to be done for a LightWeightMap with one large string value");
    arktest.assertEQ(temp.value, value);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SETVALUEAT_0600
function testSetValueAtLargeString() {
    let value = '';
    for (let i: number = 0; i < 10000; i++) {
        value += i;
    }
    let lightWeightMap = new LightWeightMap<string, string>();
    lightWeightMap.set("squirrel", "sparrow");
    arktest.assertTrue(lightWeightMap.setValueAt(0, value), "Expected setValueAt(0, largeStringValue) to return true");
    arktest.assertEQ(lightWeightMap.getValueAt(0), value);
}
