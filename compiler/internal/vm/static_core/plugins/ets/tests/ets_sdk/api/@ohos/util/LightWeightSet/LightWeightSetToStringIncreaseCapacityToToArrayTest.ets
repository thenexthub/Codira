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
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite("LightWeightSet ToString IncreaseCapacityTo ToArray API tests")

    suite.addTest("LightWeightSet toString output data as a string",
        testToStringOutputData)
    suite.addTest("LightWeightSet toString output data as a string of the empty LightWeightSet",
        testToStringEmptySet)
    suite.addTest("LightWeightSet toString output large data as a string",
        testToStringLargeData)
    suite.addTest("LightWeightSet increaseCapacityTo the capacity",
        testIncreaseCapacityTo)
    suite.addTest("LightWeightSet increaseCapacityTo the capacity less than the length of lightWeightSet",
        testIncreaseCapacityToLessThanLength)
    suite.addTest("LightWeightSet increaseCapacityTo the left float capacity",
        testIncreaseCapacityToLeftFloat)
    suite.addTest("LightWeightSet increaseCapacityTo the right float capacity",
        testIncreaseCapacityToRightFloat)
    suite.addTest("LightWeightSet increaseCapacityTo the negative capacity", testIncreaseCapacityToNegativeNumber)
    suite.addTest("LightWeightSet toArray lightWeightSet",
        testToArray)

    return suite.run()
}

function testToStringOutputData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "aa,bb", "The lightWeightSet should return a string representation of the set");
    arktest.assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testToStringEmptySet() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, "", "The lightWeightSet should return a string representation of the set");
    arktest.assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testToStringLargeData() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    const MY_MAX = 1024000;
    lightWeightSet.add(MY_MAX);
    let result = lightWeightSet.toString();
    arktest.assertEQ(result, MY_MAX + '', "The lightWeightSet should return a string representation of the set");
    arktest.assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testIncreaseCapacityTo() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    arktest.expectNoThrow(() => { lightWeightSet.increaseCapacityTo(16); });
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.code == OutOfBoundsErrorCodeId && e.message ==
                'The value of \"minimumCapacity\" is out of range. It must be > 16. Received value is: 2';
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightSet.increaseCapacityTo(2) }, exceptionCheck);
}

function testIncreaseCapacityToLessThanLength() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.code == OutOfBoundsErrorCodeId && e.message ==
                'The value of \"minimumCapacity\" is out of range. It must be > 8. Received value is: 1';
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightSet.increaseCapacityTo(1) }, exceptionCheck);
}

function testIncreaseCapacityToLeftFloat() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.code == OutOfBoundsErrorCodeId && e.message ==
                'The value of \"minimumCapacity\" is out of range. It must be > 8. Received value is: 1';
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightSet.increaseCapacityTo(Math.floor(1.2).toInt()) }, exceptionCheck);
}

function testIncreaseCapacityToRightFloat() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.code == OutOfBoundsErrorCodeId && e.message ==
                'The value of \"minimumCapacity\" is out of range. It must be > 8. Received value is: 3';
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightSet.increaseCapacityTo(Math.floor(3.8).toInt()) }, exceptionCheck);
}

function testIncreaseCapacityToNegativeNumber() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.code == OutOfBoundsErrorCodeId && e.message ==
                'The value of \"minimumCapacity\" is out of range. It must be > 8. Received value is: -4';
        }
        return false;
    }
    arktest.expectThrow(() => { lightWeightSet.increaseCapacityTo(-4) }, exceptionCheck);
}

function testToArray() {
    let lightWeightSet: LightWeightSet<double | string> = new LightWeightSet<double | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let test1 = lightWeightSet.toArray()
    let test2 = new Array<string>(2)
    test2[0] = "aa"
    test2[1] = "bb"
    arktest.assertEQ(test1[0], test2[0], "The lightWeightSet fail at 1st judgement");
    arktest.assertEQ(test1[1], test2[1], "The lightWeightSet fail at 2nd judgement");
}
