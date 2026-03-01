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

function main() {
    const suite = new arktest.ArkTestsuite("BitVector values and setAllBits API tests");

    suite.addTest("Verify if values method returns a correct iterator for BitVector with mixed bits.", BitVector_Values_Func_001);
    suite.addTest("Verify if values method returns a correct iterator for BitVector with 64 zeroes.", BitVector_Values_Func_002);
    suite.addTest("Verify if values method returns a correct iterator for BitVector with initial length plus 64 zeroes.", BitVector_Values_Func_003);
    suite.addTest("Verify if setAllBits method correctly sets all bits to 0 or 1 in a small BitVector.", BitVector_setAllBits_Func_001);
    suite.addTest("Verify if setAllBits method correctly sets all bits to 0 or 1 in a BitVector with 64 elements.", BitVector_setAllBits_Func_002);
    suite.addTest("Verify if setAllBits method correctly sets all bits to 0 or 1 in a BitVector with initial length plus 64 elements.", BitVector_setAllBits_Func_003);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_071
 * @tc.name      : BitVector_Values_Func_001
 * @tc.desc      : Verify if values method returns a correct iterator for BitVector with mixed bits.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_Values_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    let vec: Array<number> = new Array<number>();
    let a: number[] = [1, 1, 0, 1, 0];
    let itr = bitVector.values();
    let tmp: IteratorResult<int> = itr.next();
    while (!tmp.done) {
        vec.push(tmp.value!);
        tmp = itr.next();
    }
    arktest.assertEQ(vec.length, a.length, "Iterator should return the correct number of elements");
    for (let i = 0; i < a.length; i++) {
        arktest.assertEQ(vec[i], a[i], `Element at index ${i} should match expected value`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_072
 * @tc.name      : BitVector_Values_Func_002
 * @tc.desc      : Verify if values method returns a correct iterator for BitVector with 64 zeroes.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_Values_Func_002() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res0 = bitVector.push(0);
        arktest.assertTrue(res0, "Push operation should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    let vec: Array<number> = new Array<number>();
    let itr = bitVector.values();
    let tmp: IteratorResult<int> = itr.next();
    while (!tmp.done) {
        vec.push(tmp.value!);
        tmp = itr.next();
    }
    arktest.assertEQ(vec.length, bitVectorLength, "Iterator should return correct number of elements");
    for (let i = 0; i < bitVectorLength; i++) {
        arktest.assertEQ(vec[i], 0, `Element at index ${i} should be 0`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_073
 * @tc.name      : BitVector_Values_Func_003
 * @tc.desc      : Verify if values method returns a correct iterator for BitVector with initial length plus 64 zeroes.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_Values_Func_003() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res0 = bitVector.push(0);
        arktest.assertTrue(res0, "Push operation should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    let vec: Array<number> = new Array<number>();
    let itr = bitVector.values();
    let tmp: IteratorResult<int> = itr.next();
    while (!tmp.done) {
        vec.push(tmp.value!);
        tmp = itr.next();
    }
    arktest.assertEQ(vec.length, bitVectorLength + 10, "Iterator should return correct number of elements");
    for (let i = 10; i < bitVectorLength + 10; i++) {
        arktest.assertEQ(vec[i], 0, `Element at index ${i} should be 0`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_074
 * @tc.name      : BitVector_setAllBits_Func_001
 * @tc.desc      : Verify if setAllBits method correctly sets all bits to 0 or 1 in a small BitVector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_setAllBits_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.push(1);
    bitVector.setAllBits(0);
    arktest.assertEQ(bitVector[0], 0, "Bit at index 0 should be 0 after setAllBits(0)");
    arktest.assertEQ(bitVector[1], 0, "Bit at index 1 should be 0 after setAllBits(0)");
    arktest.assertEQ(bitVector[2], 0, "Bit at index 2 should be 0 after setAllBits(0)");
    arktest.assertEQ(bitVector[3], 0, "Bit at index 3 should be 0 after setAllBits(0)");
    arktest.assertEQ(bitVector[4], 0, "Bit at index 4 should be 0 after setAllBits(0)");
    bitVector.setAllBits(1);
    arktest.assertEQ(bitVector[0], 1, "Bit at index 0 should be 1 after setAllBits(1)");
    arktest.assertEQ(bitVector[1], 1, "Bit at index 1 should be 1 after setAllBits(1)");
    arktest.assertEQ(bitVector[2], 1, "Bit at index 2 should be 1 after setAllBits(1)");
    arktest.assertEQ(bitVector[3], 1, "Bit at index 3 should be 1 after setAllBits(1)");
    arktest.assertEQ(bitVector[4], 1, "Bit at index 4 should be 1 after setAllBits(1)");
}

/**
 * @tc.number    : BITVECTOR_TEST_075
 * @tc.name      : BitVector_setAllBits_Func_002
 * @tc.desc      : Verify if setAllBits method correctly sets all bits to 0 or 1 in a BitVector with 64 elements.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_setAllBits_Func_002() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res0 = bitVector.push(0);
        arktest.assertTrue(res0, "Push operation should return true");
    }
    arktest.assertEQ(bitVector.length, 64, "BitVector length should be 64");
    bitVector.setAllBits(1);
    for (let i = 0; i < bitVectorLength; i++) {
        arktest.assertEQ(bitVector[i], 1, `Bit at index ${i} should be 1 after setAllBits(1)`);
    }
    bitVector.setAllBits(0);
    for (let i = 0; i < bitVectorLength; i++) {
        arktest.assertEQ(bitVector[i], 0, `Bit at index ${i} should be 0 after setAllBits(0)`);
    }
}

/**
 * @tc.number    : BITVECTOR_TEST_076
 * @tc.name      : BitVector_setAllBits_Func_003
 * @tc.desc      : Verify if setAllBits method correctly sets all bits to 0 or 1 in a BitVector with initial length plus 64 elements.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_setAllBits_Func_003() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res0 = bitVector.push(0);
        arktest.assertTrue(res0, "Push operation should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
    bitVector.setAllBits(1);
    for (let i = 10; i < bitVectorLength + 10; i++) {
        arktest.assertEQ(bitVector[i], 1, `Bit at index ${i} should be 1 after setAllBits(1)`);
    }
    bitVector.setAllBits(0);
    for (let i = 10; i < bitVectorLength + 10; i++) {
        arktest.assertEQ(bitVector[i], 0, `Bit at index ${i} should be 0 after setAllBits(0)`);
    }
}
