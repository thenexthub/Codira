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
    const suite = new arktest.ArkTestsuite("LightWeightMap IsEmpty API tests")

    suite.addTest("Testing LightWeightMap is empty", testIsEmptyOnNewMap)
    suite.addTest("Testing LightWeightMap has data", testIsEmptyAfterAdd)
    suite.addTest("Testing LightWeightMap has data and remove", testIsEmptyAfterAddRemove)
    suite.addTest("Testing LightWeightMap has large data and clear", testIsEmptyAfterAddClearLargeData)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ISEMPTY_0100
function testIsEmptyOnNewMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    arktest.assertTrue(lightWeightMap.isEmpty(), "Expected LightWeightMap to be empty after initialization");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ISEMPTY_0200
function testIsEmptyAfterAdd() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertFalse(lightWeightMap.isEmpty(), "Expected LightWeightMap to not be empty after adding an element");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ISEMPTY_0300
function testIsEmptyAfterAddRemove() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.remove("squirrel");
    arktest.assertTrue(lightWeightMap.isEmpty(), "Expected LightWeightMap to be empty after removing the only element");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_ISEMPTY_0500
function testIsEmptyAfterAddClearLargeData() {
    let lightWeightMap = new LightWeightMap<string, number>();
    for (let i = 0; i < 10000; i++) {
        lightWeightMap.set('key' + i, i);
    }
    lightWeightMap.clear();
    arktest.assertTrue(lightWeightMap.isEmpty(),
        "Expected LightWeightMap to be empty after clearing all 10000 elements");
}
