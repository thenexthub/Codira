/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { collections } from "@arkts.collections"
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("BitVector has API tests");

    suite.addTest("Verify if bit vector correctly identifies bit values within a specified range.", BitVector_has_Func_001);
    suite.addTest("Verify that bit vector throws an error when toIndex is negative.", BitVector_has_Func_002);
    suite.addTest("Verify that bit vector throws an error when both fromIndex and toIndex are negative.", BitVector_has_Func_004);
    suite.addTest("Verify that bit vector throws an error when fromIndex is negative while toIndex is valid.", BitVector_has_Func_005);
    suite.addTest("Verify if bit vector correctly identifies bit values in a large range spanning multiple buffer elements.", BitVector_has_Func_006);
    suite.addTest("Verify if bit vector with initial length correctly identifies bit values across its full range.", BitVector_has_Func_007);
    suite.addTest("Verify that bit vector throws an error when fromIndex is greater than toIndex.", BitVector_has_Func_008);
    suite.addTest("Verify if bit vector correctly identifies bit values in a single-element range.", BitVector_has_Func_010);
    suite.addTest("Verify if bit vector correctly identifies bit values in ranges that span across buffer boundaries.", BitVector_has_Func_011);
    suite.addTest("Verify if bit vector correctly identifies bit values when checking the entire vector.", BitVector_has_Func_012);
    suite.addTest("Verify if bit vector correctly identifies presence of bit values in specific portions of the vector.", BitVector_has_Func_013);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_008
 * @tc.name      : BitVector_has_Func_001
 * @tc.desc      : Check if bit vector contains a particular bit element within a valid range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_has_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    arktest.assertEQ(bitVector.has(1, 3, 6), true, "BitVector should contain 1 in range 3-6");
    arktest.assertEQ(bitVector.has(1, 0, 2), false, "BitVector should not contain 1 in range 0-2");
    arktest.assertEQ(bitVector.has(0, 3, 5), false, "BitVector should not contain 0 in range 3-5");
    arktest.assertEQ(bitVector.has(0, 0, 3), true, "BitVector should contain 0 in range 0-3");
}

/**
 * @tc.number    : BITVECTOR_TEST_009
 * @tc.name      : BitVector_has_Func_002
 * @tc.desc      : Check if bit vector throws an error for invalid 'toIndex' range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_has_Func_002() {
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
    }
    arktest.expectThrow(() => { bitVector.has(0, 8, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_011
 * @tc.name      : BitVector_has_Func_004
 * @tc.desc      : Check if bit vector throws an error for negative 'fromIndex' and 'toIndex'.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_has_Func_004() {
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
    }
    arktest.expectThrow(() => { bitVector.has(0, -3, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_012
 * @tc.name      : BitVector_has_Func_005
 * @tc.desc      : Check if bit vector throws an error for negative 'fromIndex' with valid 'toIndex'.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_has_Func_005() {
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
    }
    arktest.expectThrow(() => { bitVector.has(0, -3, 8); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_013
 * @tc.name      : BitVector_has_Func_006
 * @tc.desc      : Check if bit vector contains a particular bit element in a large range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_has_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    arktest.assertEQ(bitVector.has(1, 0, 31), false, "BitVector should not contain 1 in range 0-31");
    arktest.assertEQ(bitVector.has(1, 32, 64), true, "BitVector should contain 1 in range 32-64");
    arktest.assertEQ(bitVector.has(0, 0, 32), true, "BitVector should contain 0 in range 0-32");
    arktest.assertEQ(bitVector.has(0, 32, 63), false, "BitVector should not contain 0 in range 32-63");
}

/**
 * @tc.number    : BITVECTOR_TEST_014
 * @tc.name      : BitVector_has_Func_007
 * @tc.desc      : Check if bit vector contains a particular bit element with an initial length.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_has_Func_007() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    arktest.assertEQ(bitVector.has(1, 10, 41), false, "BitVector should not contain 1 in range 10-41");
    arktest.assertEQ(bitVector.has(1, 42, bitVectorLength + 10), true, "BitVector should contain 1 in range 42-74");
    arktest.assertEQ(bitVector.has(0, 10, 42), true, "BitVector should contain 0 in range 10-42");
    arktest.assertEQ(bitVector.has(0, 42, bitVectorLength + 9), false, "BitVector should not contain 0 in range 42-73");
}

/**
 * @tc.number    : BITVECTOR_TEST_015
 * @tc.name      : BitVector_has_Func_008
 * @tc.desc      : Check if bit vector throws an error for an empty range (fromIndex >= toIndex).
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_has_Func_008() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code === OutOfBoundsErrorCodeId &&
                e.message === `The value of "fromIndex" is out of range. It must be >= 0 && <= 1. Received value is: 2`;
        }
        return false;
    };
    arktest.expectThrow(() => { bitVector.has(1, 2, 2); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_017
 * @tc.name      : BitVector_has_Func_010
 * @tc.desc      : Check if bit vector correctly identifies a bit element in a single-element range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_has_Func_010() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    arktest.assertEQ(bitVector.has(1, 0, 1), true, "BitVector should contain 1 in range 0-1");
    arktest.assertEQ(bitVector.has(0, 0, 1), false, "BitVector should not contain 0 in range 0-1");
}

/**
 * @tc.number    : BITVECTOR_TEST_018
 * @tc.name      : BitVector_has_Func_011
 * @tc.desc      : Check if bit vector correctly identifies a bit element across multiple buffer elements.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_has_Func_011() {
    let bitVector = new collections.BitVector(0);
    for (let i = 0; i < 64; i++) {
        bitVector.push(i < 32 ? 0 : 1);
    }
    arktest.assertEQ(bitVector.has(1, 31, 33), true, "BitVector should contain 1 in range 31-33");
    arktest.assertEQ(bitVector.has(0, 30, 32), true, "BitVector should contain 0 in range 30-32");
    arktest.assertEQ(bitVector.has(1, 0, 31), false, "BitVector should not contain 1 in range 0-31");
}

/**
 * @tc.number    : BITVECTOR_TEST_019
 * @tc.name      : BitVector_has_Func_012
 * @tc.desc      : Check if bit vector correctly identifies a bit element when range spans the entire buffer.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_has_Func_012() {
    let bitVector = new collections.BitVector(0);
    for (let i = 0; i < 64; i++) {
        bitVector.push(i % 2);
    }
    arktest.assertEQ(bitVector.has(1, 0, 64), true, "BitVector should contain 1 in range 0-64");
    arktest.assertEQ(bitVector.has(0, 0, 64), true, "BitVector should contain 0 in range 0-64");
}

/**
     * @tc.number    : BITVECTOR_TEST_077
     * @tc.name      : BitVector_has_Func_008
     * @tc.desc      : Check if bit vector contains a particular bit element.
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
function BitVector_has_Func_013() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 20;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 10 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, 20, "BitVector length should be 20");
    arktest.assertEQ(bitVector.has(1, 0, 10), false, "BitVector should contain 1 in range 0-10");
    arktest.assertEQ(bitVector.has(1, 0, 9), false, "BitVector should not contain 1 in range 0-9");
    arktest.assertEQ(bitVector.has(0, 10, 19), false, "BitVector should not contain 0 in range 10-19");
    arktest.assertEQ(bitVector.has(0, 0, 19), true, "BitVector should contain 0 in range 0-19");
};
