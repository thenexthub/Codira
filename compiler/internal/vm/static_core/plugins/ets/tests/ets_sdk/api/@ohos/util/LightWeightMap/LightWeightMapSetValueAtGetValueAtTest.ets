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

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightMap SetValueAt GetValueAt API tests")

    suite.addTest("LightWeightMap setValueAt getValueAt by index",
        testSetValueAtValidIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by boundary index",
        testSetValueAtBoundaryIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by left exceeded index",
        testSetValueAtLeftExceededIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by right exceeded index",
        testSetValueAtRightExceededIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by index of null lightWeightMap",
        testSetValueAtIndexOfEmptyMap)
    suite.addTest("LightWeightMap setValueAt getValueAt by left float index",
        testSetValueAtLeftFloatIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by right float index",
        testSetValueAtRightFloatIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt set large value and get by large index",
        testSetValueAtLargeValueAndIndex)

    return suite.run()
}

function testSetValueAtValidIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(1, 777);
    let result2 = lightWeightMap.getValueAt(1);
    arktest.assertEQ(result1, true, "The lightWeightMap should set the value at the specified index");
    arktest.assertEQ(result2, 777, "The lightWeightMap should return the correct value at the specified index");
}

function testSetValueAtBoundaryIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(2, 777);
    let result2 = lightWeightMap.getValueAt(2);
    arktest.assertEQ(result1, true, "The lightWeightMap should set the value at the boundary index");
    arktest.assertEQ(result2, 777, "The lightWeightMap should return the correct value at the boundary index");
}

function testSetValueAtLeftExceededIndex() {
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
    arktest.expectThrow(() => {lightWeightMap.setValueAt(-1, 777)}, exceptionCheck);
    arktest.expectThrow(() => {lightWeightMap.getValueAt(-1)}, exceptionCheck);
}

function testSetValueAtRightExceededIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 2. Received value is: 6" &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {lightWeightMap.setValueAt(6, 777)}, exceptionCheck);
    arktest.expectThrow(() => {lightWeightMap.getValueAt(6)}, exceptionCheck);
}

function testSetValueAtIndexOfEmptyMap() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == "Container is empty";
        }
        return false;
    }
    arktest.expectThrow(() => {lightWeightMap.setValueAt(6, 777)}, exceptionCheck);
    arktest.expectThrow(() => {lightWeightMap.getValueAt(6)}, exceptionCheck);
}

function testSetValueAtLeftFloatIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(Math.floor(1.2).toInt(), 777);
    let result2 = lightWeightMap.getValueAt(Math.floor(1.2).toInt());
    arktest.assertEQ(result1, true, "The lightWeightMap should set the value at the floored left float index");
    arktest.assertEQ(result2, 777, "The lightWeightMap should return the correct value at the floored left float index");
}

function testSetValueAtRightFloatIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(Math.floor(1.8).toInt(), 777);
    let result2 = lightWeightMap.getValueAt(Math.floor(1.8).toInt());
    arktest.assertEQ(result1, true, "The lightWeightMap should set the value at the floored right float index");
    arktest.assertEQ(result2, 777, "The lightWeightMap should return the correct value at the floored right float index");
}

function testSetValueAtLargeValueAndIndex() {
    let lightWeightMap: LightWeightMap<string, double> = new LightWeightMap<string, double>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(0, Number.MAX_VALUE);
    let result2 = lightWeightMap.getValueAt(0);
    arktest.assertEQ(result1, true, "The lightWeightMap should set the value at the floored right float index");
    arktest.assertEQ(result2, Number.MAX_VALUE, "The lightWeightMap should get the value at the key index");
}
