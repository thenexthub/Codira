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
    const suite = new arktest.ArkTestsuite("BitVector getBitsByRange API tests");

    suite.addTest("Returns the bit values in a valid range of indices.", BitVector_getBitsByRange_Func_001);
    suite.addTest("Throws an error for invalid 'toIndex' range.", BitVector_getBitsByRange_Func_002);
    suite.addTest("Throws an error for negative 'fromIndex' and 'toIndex'.", BitVector_getBitsByRange_Func_004);
    suite.addTest("Throws an error for negative 'fromIndex' with valid 'toIndex'.", BitVector_getBitsByRange_Func_005);
    suite.addTest("Returns the bit values in a large range.", BitVector_getBitsByRange_Func_006);
    suite.addTest("Returns the bit values in a range with an initial length.", BitVector_getBitsByRange_Func_007);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_022
 * @tc.name      : BitVector_getBitsByRange_Func_001
 * @tc.desc      : Returns the bit values in a valid range of indices in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_getBitsByRange_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(1);
    let result = bitVector.getBitsByRange(0, 3);
    arktest.assertEQ(result[0], 1, "Bit at index 0 should be 1");
    arktest.assertEQ(result[1], 1, "Bit at index 1 should be 1");
    arktest.assertEQ(result[2], 0, "Bit at index 2 should be 0");
}

/**
 * @tc.number    : BITVECTOR_TEST_023
 * @tc.name      : BitVector_getBitsByRange_Func_002
 * @tc.desc      : Throws an error for invalid 'toIndex' range in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_getBitsByRange_Func_002() {
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
    arktest.expectThrow(() => { bitVector.getBitsByRange(8, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_025
 * @tc.name      : BitVector_getBitsByRange_Func_004
 * @tc.desc      : Throws an error for negative 'fromIndex' and 'toIndex' in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_getBitsByRange_Func_004() {
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
    arktest.expectThrow(() => { bitVector.getBitsByRange(-3, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_026
 * @tc.name      : BitVector_getBitsByRange_Func_005
 * @tc.desc      : Throws an error for negative 'fromIndex' with valid 'toIndex' in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_getBitsByRange_Func_005() {
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
    arktest.expectThrow(() => { bitVector.getBitsByRange(-3, 8); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_027
 * @tc.name      : BitVector_getBitsByRange_Func_006
 * @tc.desc      : Returns the bit values in a large range of indices in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_getBitsByRange_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    let result = bitVector.getBitsByRange(0, 64);
    for (let i = 0; i < bitVectorLength; i++) {
        arktest.assertEQ(result[i], 0, `Bit at index ${i} should be 0`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_028
 * @tc.name      : BitVector_getBitsByRange_Func_007
 * @tc.desc      : Returns the bit values in a range with an initial length in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_getBitsByRange_Func_007() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    let result = bitVector.getBitsByRange(0, bitVectorLength + 10);
    for (let i = 10; i < bitVectorLength + 10; i++) {
        arktest.assertEQ(result[i], 0, `Bit at index ${i} should be 0`);
    }
}
