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
const TypeErrorCodeId: int = 401;

function main() {
    const suite = new arktest.ArkTestsuite("BitVector resize API tests");

    suite.addTest("Resize the bitVector to a smaller length.", BitVector_resize_Func_001);
    suite.addTest("Resize the bitVector to the same length.", BitVector_resize_Func_002);
    suite.addTest("Resize the bitVector to a larger length with default values.", BitVector_resize_Func_003);
    suite.addTest("Check if resizing to a negative length throws an error.", BitVector_resize_Func_004);
    suite.addTest("Resize the bitVector to a smaller length and verify.", BitVector_resize_Func_006);
    suite.addTest("Resize the bitVector to the same length and verify.", BitVector_resize_Func_007);
    suite.addTest("Resize the bitVector to a larger length and verify default values.", BitVector_resize_Func_008);
    suite.addTest("Resize the bitVector with an initial length and verify.", BitVector_resize_Func_009);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_029
 * @tc.name      : BitVector_resize_Func_001
 * @tc.desc      : Resize the bitVector to a smaller length and verify the new length.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_resize_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.resize(4);
    arktest.assertEQ(bitVector.length, 4, "BitVector length should be 4 after resizing.");
}

/**
 * @tc.number    : BITVECTOR_TEST_030
 * @tc.name      : BitVector_resize_Func_002
 * @tc.desc      : Resize the bitVector to the same length and verify elements remain unchanged.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_resize_Func_002() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.resize(5);
    arktest.assertEQ(bitVector[0], 1, "Element at index 0 should be 1.");
    arktest.assertEQ(bitVector[1], 1, "Element at index 1 should be 1.");
    arktest.assertEQ(bitVector[2], 1, "Element at index 2 should be 1.");
    arktest.assertEQ(bitVector[3], 1, "Element at index 3 should be 1.");
    arktest.assertEQ(bitVector[4], 1, "Element at index 4 should be 1.");
}

/**
 * @tc.number    : BITVECTOR_TEST_031
 * @tc.name      : BitVector_resize_Func_003
 * @tc.desc      : Resize the bitVector to a larger length and verify new elements are default values.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_resize_Func_003() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.resize(10);
    arktest.assertEQ(bitVector[5], 0, "Element at index 5 should be 0.");
    arktest.assertEQ(bitVector[6], 0, "Element at index 6 should be 0.");
    arktest.assertEQ(bitVector[7], 0, "Element at index 7 should be 0.");
    arktest.assertEQ(bitVector[8], 0, "Element at index 8 should be 0.");
    arktest.assertEQ(bitVector[9], 0, "Element at index 9 should be 0.");
}

/**
 * @tc.number    : BITVECTOR_TEST_032
 * @tc.name      : BitVector_resize_Func_004
 * @tc.desc      : Check if resizing to a negative length throws an error.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 2
 */
function BitVector_resize_Func_004() {
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
            return e.name == "BusinessError" &&
                e.message === `The value of "length" is out of range. It must be >= 0. Received value is: -4` &&
                e.code === OutOfBoundsErrorCodeId;
        }
        return false;
    };
    arktest.expectThrow(() => { bitVector.resize(-4); }, exceptionCheck);
}

/**
 * @tc.number    : BITVECTOR_TEST_034
 * @tc.name      : BitVector_resize_Func_006
 * @tc.desc      : Resize the bitVector to a smaller length and verify the new length.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_resize_Func_006() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        bitVector.push(0);
    }
    bitVector.resize(63);
    arktest.assertEQ(bitVector.length, 63, "BitVector length should be 63 after resizing.");
}

/**
 * @tc.number    : BITVECTOR_TEST_035
 * @tc.name      : BitVector_resize_Func_007
 * @tc.desc      : Resize the bitVector to the same length and verify the length remains unchanged.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_resize_Func_007() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        bitVector.push(0);
    }
    bitVector.resize(64);
    arktest.assertEQ(bitVector.length, 64, "BitVector length should remain 64 after resizing.");
}

/**
 * @tc.number    : BITVECTOR_TEST_036
 * @tc.name      : BitVector_resize_Func_008
 * @tc.desc      : Resize the bitVector to a larger length and verify new elements are default values.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_resize_Func_008() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        bitVector.push(1);
    }
    bitVector.resize(100);
    for (let i = bitVectorLength; i < 100; i++) {
        arktest.assertEQ(bitVector[i], 0, `Element at index ${i} should be 0.`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_037
 * @tc.name      : BitVector_resize_Func_009
 * @tc.desc      : Resize the bitVector with an initial length and verify the new length.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_resize_Func_009() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        bitVector.push(0);
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74 initially.");
    bitVector.resize(64);
    arktest.assertEQ(bitVector.length, bitVectorLength, "BitVector length should be 64 after resizing.");
}
