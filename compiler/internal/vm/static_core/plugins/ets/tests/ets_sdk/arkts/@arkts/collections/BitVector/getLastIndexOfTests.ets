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

import { collections } from "@arkts.collections"
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("BitVector getLastIndexOf API tests");

    suite.addTest("Locates the last occurrence of a bit value within a valid range.", BitVector_getLastIndexOf_Func_001);
    suite.addTest("Throws an error for invalid 'toIndex' range.", BitVector_getLastIndexOf_Func_002);
    suite.addTest("Throws an error for negative 'fromIndex' and 'toIndex'.", BitVector_getLastIndexOf_Func_003);
    suite.addTest("Throws an error for negative 'fromIndex' with valid 'toIndex'.", BitVector_getLastIndexOf_Func_004);
    suite.addTest("Locates the last occurrence of a bit value in a large BitVector.", BitVector_getLastIndexOf_Func_006);
    suite.addTest("Locates the last occurrence of a bit value in a BitVector with initial length.", BitVector_getLastIndexOf_Func_007);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: BITVECTOR_TEST_052
function BitVector_getLastIndexOf_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    arktest.assertEQ(bitVector.getLastIndexOf(0, 0, 5), 4, "Last index of 0 should be 4");
    arktest.assertEQ(bitVector.getLastIndexOf(1, 0, 5), 3, "Last index of 1 should be 3");
}

// Test cases ported from ArkTS 1.0: BITVECTOR_TEST_053
function BitVector_getLastIndexOf_Func_002() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code === OutOfBoundsErrorCodeId &&
                e.message === `The value of "toIndex" is out of range. It must be >= 0 && <= 8. Received value is: -3`;
        }
        return false;
    };
    arktest.expectThrow(() => { bitVector.getLastIndexOf(0, 8, -3); }, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: BITVECTOR_TEST_054
function BitVector_getLastIndexOf_Func_003() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code === OutOfBoundsErrorCodeId &&
                e.message === `The value of "toIndex" is out of range. It must be >= 0 && <= 8. Received value is: -3`;
        }
        return false;
    };
    arktest.expectThrow(() => { bitVector.getLastIndexOf(0, -3, -3); }, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: BITVECTOR_TEST_055
function BitVector_getLastIndexOf_Func_004() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code === OutOfBoundsErrorCodeId &&
                e.message === `The value of "fromIndex" is out of range. It must be >= 0 && <= 7. Received value is: -3`;
        }
        return false;
    };
    arktest.expectThrow(() => { bitVector.getLastIndexOf(0, -3, 8); }, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: BITVECTOR_TEST_057
function BitVector_getLastIndexOf_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    arktest.assertEQ(bitVector.getLastIndexOf(0, 0, 64), 31, "Last index of 0 should be 31");
    arktest.assertEQ(bitVector.getLastIndexOf(1, 0, 64), 63, "Last index of 1 should be 63");
}

// Test cases ported from ArkTS 1.0: BITVECTOR_TEST_058
function BitVector_getLastIndexOf_Func_007() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    arktest.assertEQ(bitVector.getLastIndexOf(0, 10, bitVectorLength + 10), 41, "Last index of 0 should be 41");
    arktest.assertEQ(bitVector.getLastIndexOf(1, 10, bitVectorLength + 10), 73, "Last index of 1 should be 73");
}
