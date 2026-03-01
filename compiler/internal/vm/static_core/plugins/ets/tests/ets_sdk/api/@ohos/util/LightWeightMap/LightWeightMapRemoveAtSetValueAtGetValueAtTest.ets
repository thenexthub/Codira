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
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightMap GetKeyAt RemoveAt SetValueAt GeyValueAt API tests")

    suite.addTest("Testing LightWeightMap remove data by index", testRemoveAtValidIndex)
    suite.addTest("Testing LightWeightMap remove data by negative number index", testRemoveAtNegativeIndex)
    suite.addTest("Testing LightWeightMap remove data by outside length index", testRemoveAtInvalidIndex)
    suite.addTest("Testing LightWeightMap set value by index", testSetValueAtValidIndex)
    suite.addTest("Testing LightWeightMap set value by negative number index", testSetValueAtNegativeIndex)
    suite.addTest("Testing LightWeightMap set large number value by index", testSetValueAtLargeNumber)
    suite.addTest("Testing LightWeightMap get value by index", testGetValueAtValidIndex)
    suite.addTest("Testing LightWeightMap get value by outside length index", testGetValueAtInvalidIndex)

    return suite.run()
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVEAT_0100
function testRemoveAtValidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let index = lightWeightMap.getIndexOfKey("squirrel");
    arktest.assertNE(index, -1);
    arktest.assertTrue(lightWeightMap.removeAt(index), "Expected removeAt(validIndex) to return true");
    arktest.assertFalse(lightWeightMap.hasKey("squirrel"), "Expected hasKey('squirrel') to return false after removal");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVEAT_0200
function testRemoveAtNegativeIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    arktest.assertFalse(lightWeightMap.removeAt(-1), "Expected removeAt(-1) with a negative index to return false");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_REMOVEAT_0300
function testRemoveAtInvalidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    arktest.assertFalse(lightWeightMap.removeAt(10),
        "Expected removeAt(10) with an out-of-bounds index to return false");
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SETVALUEAT_0100
function testSetValueAtValidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let index = lightWeightMap.getIndexOfKey("squirrel");
    arktest.assertNE(index, -1);
    arktest.assertTrue(lightWeightMap.setValueAt(index, 3546),
        "Expected setValueAt(validIndex, newValue) to return true");
    arktest.assertEQ(lightWeightMap.getValueAt(index), 3546);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SETVALUEAT_0300
function testSetValueAtNegativeIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 0. Received value is: -1" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightMap.setValueAt(-1, 356) }, exceptionCheck);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_SETVALUEAT_0500
function testSetValueAtLargeNumber() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    arktest.assertTrue(lightWeightMap.setValueAt(0, Number.MAX_VALUE),
        "Expected setValueAt(0, Number.MAX_VALUE) to return true");
    arktest.assertEQ(lightWeightMap.getValueAt(0), Number.MAX_VALUE);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETVALUEAT_0100
function testGetValueAtValidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let index = lightWeightMap.getIndexOfKey("squirrel");
    arktest.assertNE(index, -1);
    arktest.assertEQ(lightWeightMap.getValueAt(index), 123);
}

//Test cases ported from ArkTS 1.0:LIGHTWEIGHTMAP_GETVALUEAT_0300
function testGetValueAtInvalidIndex() {
    let lightWeightMap = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 1. Received value is: 10" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightMap.getValueAt(10) }, exceptionCheck);
}
