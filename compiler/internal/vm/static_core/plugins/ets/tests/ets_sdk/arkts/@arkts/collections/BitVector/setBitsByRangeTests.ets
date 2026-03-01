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
    const suite = new arktest.ArkTestsuite("BitVector setBitsByRange API tests");

    suite.addTest("Verify setting a range of bits updates the bit vector correctly.", BitVector_setBitsByRange_Func_001);
    suite.addTest("Verify error is thrown when 'toIndex' is out of range.", BitVector_setBitsByRange_Func_002);
    suite.addTest("Verify error is thrown when both 'fromIndex' and 'toIndex' are negative.", BitVector_setBitsByRange_Func_004);
    suite.addTest("Verify error is thrown when 'fromIndex' is negative and 'toIndex' is valid.", BitVector_setBitsByRange_Func_005);
    suite.addTest("Verify setting a large range of bits updates the bit vector correctly.", BitVector_setBitsByRange_Func_006);
    suite.addTest("Verify setting a range of bits with an initial offset updates the bit vector correctly.", BitVector_setBitsByRange_Func_007);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_015
 * @tc.name      : BitVector_setBitsByRange_Func_001
 * @tc.desc      : Verify setting a range of bits to 0 and 1 updates the bit vector correctly.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_setBitsByRange_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.setBitsByRange(0, 1, 4);
    arktest.assertEQ(bitVector[1], 0, "Bit at index 1 should be 0");
    arktest.assertEQ(bitVector[2], 0, "Bit at index 2 should be 0");
    arktest.assertEQ(bitVector[3], 0, "Bit at index 3 should be 0");
    bitVector.setBitsByRange(1, 1, 4);
    arktest.assertEQ(bitVector[1], 1, "Bit at index 1 should be 1");
    arktest.assertEQ(bitVector[2], 1, "Bit at index 2 should be 1");
    arktest.assertEQ(bitVector[3], 1, "Bit at index 3 should be 1");
}

/**
 * @tc.number    : BITVECTOR_TEST_016
 * @tc.name      : BitVector_setBitsByRange_Func_002
 * @tc.desc      : Verify error is thrown when 'toIndex' is out of range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_setBitsByRange_Func_002() {
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
    arktest.expectThrow(() => { bitVector.setBitsByRange(0, 8, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_018
 * @tc.name      : BitVector_setBitsByRange_Func_004
 * @tc.desc      : Verify error is thrown when both 'fromIndex' and 'toIndex' are negative.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_setBitsByRange_Func_004() {
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
    arktest.expectThrow(() => { bitVector.setBitsByRange(0, -3, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_019
 * @tc.name      : BitVector_setBitsByRange_Func_005
 * @tc.desc      : Verify error is thrown when 'fromIndex' is negative and 'toIndex' is valid.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_setBitsByRange_Func_005() {
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
    arktest.expectThrow(() => { bitVector.setBitsByRange(0, -3, 8); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_020
 * @tc.name      : BitVector_setBitsByRange_Func_006
 * @tc.desc      : Verify setting a large range of bits to 1 and 0 updates the bit vector correctly.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_setBitsByRange_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    bitVector.setBitsByRange(1, 0, 32);
    for (let i = 0; i < 32; i++) {
        arktest.assertEQ(bitVector[i], 1, `Bit at index ${i} should be 1`);
    }
    bitVector.setBitsByRange(0, 32, 64);
    for (let i = 32; i < 64; i++) {
        arktest.assertEQ(bitVector[i], 0, `Bit at index ${i} should be 0`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_021
 * @tc.name      : BitVector_setBitsByRange_Func_007
 * @tc.desc      : Verify setting a range of bits with an initial offset updates the bit vector correctly.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_setBitsByRange_Func_007() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    bitVector.setBitsByRange(1, 10, 42);
    for (let i = 10; i < 42; i++) {
        arktest.assertEQ(bitVector[i], 1, `Bit at index ${i} should be 1`);
    }
    bitVector.setBitsByRange(0, 42, 74);
    for (let i = 42; i < 74; i++) {
        arktest.assertEQ(bitVector[i], 0, `Bit at index ${i} should be 0`);
    }
}
