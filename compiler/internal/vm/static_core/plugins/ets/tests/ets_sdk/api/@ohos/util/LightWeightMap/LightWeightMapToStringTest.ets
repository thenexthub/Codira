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
    const suite = new arktest.ArkTestsuite("LightWeightMap ToString API tests")

    suite.addTest("Testing LightWeightMap convert to string", testToStringNonEmptyMap)
    suite.addTest("Testing LightWeightMap convert to string for an empty LightWeightMap", testToStringEmptyMap)
    suite.addTest("Testing LightWeightMap convert to string for a LightWeightMap contain duplicate value",
        testToStringWithDuplicateValues)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_TOSTRING_0100
function testToStringNonEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    arktest.assertEQ(lightWeightMap.toString(), 'sparrow:356,squirrel:123');
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_TOSTRING_0200
function testToStringEmptyMap() {
    let lightWeightMap = new LightWeightMap<string, number>();
    arktest.assertEQ(lightWeightMap.toString(), '');
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_TOSTRING_0300
function testToStringWithDuplicateValues() {
    const VALUE = 'VALUE';
    let lightWeightMap = new LightWeightMap<number, string>();
    for (let i = 0; i < 5; i++) {
        lightWeightMap.set(i, VALUE);
    }
    arktest.assertEQ(lightWeightMap.toString(), '0:VALUE,1:VALUE,2:VALUE,3:VALUE,4:VALUE');
}
