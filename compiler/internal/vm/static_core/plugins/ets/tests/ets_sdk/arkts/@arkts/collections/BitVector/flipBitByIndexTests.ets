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
    const suite = new arktest.ArkTestsuite("BitVector flipBitByIndex API tests");

    suite.addTest("Verify basic bit flipping functionality in a bit vector.", BitVector_flipBitByIndex_Func_001);
    suite.addTest("Test flipBitByIndex with out of range index value.", BitVector_flipBitByIndex_Func_002);
    suite.addTest("Test bit flipping operations on larger bit vector.", BitVector_flipBitByIndex_Func_004);
    suite.addTest("Test bit flipping operations on bit vector with initial size.", BitVector_flipBitByIndex_Func_005);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_059
 * @tc.name      : BitVector_flipBitByIndex_Func_001
 * @tc.desc      : Test flipping bits from 1 to 0 and from 0 to 1 in a bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_flipBitByIndex_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.flipBitByIndex(0);
    bitVector.flipBitByIndex(1);
    bitVector.flipBitByIndex(2);
    bitVector.flipBitByIndex(3);
    bitVector.flipBitByIndex(4);
    arktest.assertEQ(bitVector[0], 0, "Bit at index 0 should be flipped to 0");
    arktest.assertEQ(bitVector[1], 0, "Bit at index 1 should be flipped to 0");
    arktest.assertEQ(bitVector[2], 0, "Bit at index 2 should be flipped to 0");
    arktest.assertEQ(bitVector[3], 0, "Bit at index 3 should be flipped to 0");
    arktest.assertEQ(bitVector[4], 1, "Bit at index 4 should be flipped to 1");
}

/**
 * @tc.number    : BITVECTOR_TEST_060
 * @tc.name      : BitVector_flipBitByIndex_Func_002
 * @tc.desc      : Test flipBitByIndex with out of range index value.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_flipBitByIndex_Func_002() {
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
                e.message === `The value of "index" is out of range. It must be >= 0 && <= 7. Received value is: 20`;
        }
        return false;
    }
    arktest.expectThrow(() => { bitVector.flipBitByIndex(20); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_062
 * @tc.name      : BitVector_flipBitByIndex_Func_004
 * @tc.desc      : Test bit flipping operations on larger bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_flipBitByIndex_Func_004() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        if (i >= 32) {
            let res = bitVector.push(1);
            arktest.assertTrue(res, "Push should return true");
        } else {
            let res = bitVector.push(0);
            arktest.assertTrue(res, "Push should return true");
        }
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");

    for (let i = 0; i < bitVectorLength; i++) {
        bitVector.flipBitByIndex(i);
    }

    for (let i = 0; i < bitVectorLength; i++) {
        if (i >= 32) {
            arktest.assertEQ(bitVector[i], 0, "Bit at index " + i + " should be flipped to 0");
        } else {
            arktest.assertEQ(bitVector[i], 1, "Bit at index " + i + " should be flipped to 1");
        }
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_063
 * @tc.name      : BitVector_flipBitByIndex_Func_005
 * @tc.desc      : Test bit flipping operations on bit vector with initial size.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_flipBitByIndex_Func_005() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        if (i >= 32) {
            let res = bitVector.push(1);
            arktest.assertTrue(res, "Push should return true");
        } else {
            let res = bitVector.push(0);
            arktest.assertTrue(res, "Push should return true");
        }
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");

    for (let i = 10; i < bitVectorLength + 10; i++) {
        bitVector.flipBitByIndex(i);
    }

    for (let i = 10; i < bitVectorLength + 10; i++) {
        if (i >= 42) {
            arktest.assertEQ(bitVector[i], 0, "Bit at index " + i + " should be flipped to 0");
        } else {
            arktest.assertEQ(bitVector[i], 1, "Bit at index " + i + " should be flipped to 1");
        }
    }
}
