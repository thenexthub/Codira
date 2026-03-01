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
    const suite = new arktest.ArkTestsuite("BitVector getBitCountByRange API tests");

    suite.addTest("Counts the occurrences of a bit element within a valid range.", BitVector_getBitCountByRange_Func_001);
    suite.addTest("Throws an error for invalid 'toIndex' range.", BitVector_getBitCountByRange_Func_002);
    suite.addTest("Throws an error for negative 'fromIndex' and 'toIndex'.", BitVector_getBitCountByRange_Func_004);
    suite.addTest("Throws an error for negative 'fromIndex' with valid 'toIndex'.", BitVector_getBitCountByRange_Func_005);
    suite.addTest("Counts the occurrences of a bit element in a large range.", BitVector_getBitCountByRange_Func_006);
    suite.addTest("Counts the occurrences of a bit element with an initial length.", BitVector_getBitCountByRange_Func_007);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_038
 * @tc.name      : BitVector_getBitCountByRange_Func_001
 * @tc.desc      : Counts the occurrences of a bit element within a valid range in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_getBitCountByRange_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    arktest.assertEQ(bitVector.getBitCountByRange(0, 0, 5), 3, "BitVector should contain 3 zeros in range 0-5");
    arktest.assertEQ(bitVector.getBitCountByRange(1, 0, 5), 2, "BitVector should contain 2 ones in range 0-5");
}

/**
 * @tc.number    : BITVECTOR_TEST_039
 * @tc.name      : BitVector_getBitCountByRange_Func_002
 * @tc.desc      : Throws an error when 'toIndex' is out of range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_getBitCountByRange_Func_002() {
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
    arktest.expectThrow(() => { bitVector.getBitCountByRange(0, 8, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_041
 * @tc.name      : BitVector_getBitCountByRange_Func_004
 * @tc.desc      : Throws an error when both 'fromIndex' and 'toIndex' are negative.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_getBitCountByRange_Func_004() {
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
    arktest.expectThrow(() => { bitVector.getBitCountByRange(0, -3, -3); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_042
 * @tc.name      : BitVector_getBitCountByRange_Func_005
 * @tc.desc      : Throws an error when 'fromIndex' is negative and 'toIndex' is valid.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_getBitCountByRange_Func_005() {
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
    arktest.expectThrow(() => { bitVector.getBitCountByRange(0, -3, 8); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_043
 * @tc.name      : BitVector_getBitCountByRange_Func_006
 * @tc.desc      : Counts the occurrences of a bit element in a large range.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_getBitCountByRange_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    arktest.assertEQ(bitVector.getBitCountByRange(0, 0, 64), 32, "BitVector should contain 32 zeros in range 0-64");
    arktest.assertEQ(bitVector.getBitCountByRange(1, 0, 64), 32, "BitVector should contain 32 ones in range 0-64");
}

/**
 * @tc.number    : BITVECTOR_TEST_044
 * @tc.name      : BitVector_getBitCountByRange_Func_007
 * @tc.desc      : Counts the occurrences of a bit element with an initial length.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_getBitCountByRange_Func_007() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i >= 32 ? 1 : 0);
        arktest.assertTrue(res, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    arktest.assertEQ(bitVector.getBitCountByRange(0, 10, bitVectorLength + 10), 32, "BitVector should contain 32 zeros in range 10-74");
    arktest.assertEQ(bitVector.getBitCountByRange(1, 10, bitVectorLength + 10), 32, "BitVector should contain 32 ones in range 10-74");
}
