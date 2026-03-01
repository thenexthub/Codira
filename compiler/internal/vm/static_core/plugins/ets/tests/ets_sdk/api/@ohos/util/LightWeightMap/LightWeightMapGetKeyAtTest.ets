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
    const suite = new arktest.ArkTestsuite("LightWeightMap GetKeyAt API tests")

    suite.addTest("LightWeightMap getKeyAt key by index", testGetKeyAtValidIndex)
    suite.addTest("LightWeightMap getKeyAt key by boundary index", testGetKeyAtBoundaryIndex)
    suite.addTest("LightWeightMap getKeyAt key by left exceeded index", testGetKeyAtLeftExceededIndex)
    suite.addTest("LightWeightMap getKeyAt key by right exceeded index", testGetKeyAtRightExceededIndex)
    suite.addTest("LightWeightMap getKeyAt key by index of null lightWeightMap", testGetKeyAtIndexOfEmptyMap)
    suite.addTest("LightWeightMap getKeyAt key by left float index", testGetKeyAtLeftFloatIndex)
    suite.addTest("LightWeightMap getKeyAt key by right float index", testGetKeyAtRightFloatIndex)


    return suite.run()
}

function testGetKeyAtValidIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(1);
    arktest.assertEQ(result, "test", "The lightWeightMap should return the correct key at the specified index");
}

function testGetKeyAtBoundaryIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(2);
    arktest.assertEQ(result, "sparrow", "The lightWeightMap should return the correct key at the boundary index");
}

function testGetKeyAtLeftExceededIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 2. Received value is: -1" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightMap.getKeyAt(-1) }, exceptionCheck);
}

function testGetKeyAtRightExceededIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 2. Received value is: 5" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightMap.getKeyAt(5) }, exceptionCheck);
}

function testGetKeyAtIndexOfEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightMap.getKeyAt(5) }, exceptionCheck);
}

function testGetKeyAtLeftFloatIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(Math.floor(1.2).toInt());
    arktest.assertEQ(result, "test", "The lightWeightMap should return the correct key at the floored left float index");
}

function testGetKeyAtRightFloatIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(Math.floor(1.8).toInt());
    arktest.assertEQ(result, "test", "The lightWeightMap should return the correct key at the floored right float index");
}
