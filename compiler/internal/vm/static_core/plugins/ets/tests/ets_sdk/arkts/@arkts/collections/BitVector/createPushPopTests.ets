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

function main() {
    const suite = new arktest.ArkTestsuite("BitVector API tests");

    suite.addTest("A constructor used to create a BitVector object.", BitVector_constructor_Func_001);
    suite.addTest("Appends the bit element to the end of this bit vector.", BitVector_push_Func_001);
    suite.addTest("Appends multiple bit elements to the end of this bit vector in a loop.", BitVector_push_Func_002);
    suite.addTest("Appends multiple bit elements to the end of this bit vector, starting with an initial length.", BitVector_push_Func_003);
    suite.addTest("Retrieves and removes the bit element from the end of this bit vector.", BitVector_pop_Func_001);
    suite.addTest("Retrieves and removes the bit elements from the end of this bit vector in a loop.", BitVector_pop_Func_002);
    suite.addTest("Retrieves and removes the bit elements from the end of this bit vector, leaving the initial length intact.", BitVector_pop_Func_003);

    return suite.run();
}

/**
 * @tc.number    : BITVECTOR_TEST_001
 * @tc.name      : BitVector_constructor_Func_001
 * @tc.desc      : A constructor used to create a BitVector object.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_constructor_Func_001() {
    let bitVector = new collections.BitVector(10);
    arktest.assertEQ(bitVector.length, 10, "BitVector length should be 10");
}

/**
 * @tc.number    : BITVECTOR_TEST_002
 * @tc.name      : BitVector_push_Func_001
 * @tc.desc      : Appends the bit element to the end of this bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_push_Func_001() {
    let bitVector = new collections.BitVector(0);
    let res0 = bitVector.push(0);
    let res1 = bitVector.push(1);
    let res2 = bitVector.push(0);
    let res3 = bitVector.push(1);
    let res4 = bitVector.push(1);
    arktest.assertTrue(res0, "Push 0 should return true");
    arktest.assertTrue(res1, "Push 1 should return true");
    arktest.assertTrue(res2, "Push 0 should return true");
    arktest.assertTrue(res3, "Push 1 should return true");
    arktest.assertTrue(res4, "Push 1 should return true");
    arktest.assertEQ(bitVector.length, 5, "BitVector length should be 5");
}

/**
 * @tc.number    : BITVECTOR_TEST_003
 * @tc.name      : BitVector_push_Func_002
 * @tc.desc      : Appends multiple bit elements to the end of this bit vector in a loop.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_push_Func_002() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(1);
        arktest.assertEQ(res, true, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength, "BitVector length should be 64");
}

/**
 * @tc.number    : BITVECTOR_TEST_004
 * @tc.name      : BitVector_push_Func_003
 * @tc.desc      : Appends multiple bit elements to the end of this bit vector, starting with an initial length.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_push_Func_003() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res = bitVector.push(i % 2);
        arktest.assertEQ(res, true, "Push should return true");
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");
}

/**
 * @tc.number    : BITVECTOR_TEST_005
 * @tc.name      : BitVector_pop_Func_001
 * @tc.desc      : Retrieves and removes the bit element to the end of this bit vector.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_pop_Func_001() {
    let bitVector = new collections.BitVector(0);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(0);
    bitVector.push(1);
    bitVector.push(1);
    let res0 = bitVector.pop();
    let res1 = bitVector.pop();
    let res2 = bitVector.pop();
    let res3 = bitVector.pop();
    let res4 = bitVector.pop();
    arktest.assertEQ(res0, 1, "Pop should return 1");
    arktest.assertEQ(res1, 1, "Pop should return 1");
    arktest.assertEQ(res2, 0, "Pop should return 0");
    arktest.assertEQ(res3, 1, "Pop should return 1");
    arktest.assertEQ(res4, 0, "Pop should return 0");
    arktest.assertEQ(bitVector.length, 0, "BitVector length should be 0");
}

/**
 * @tc.number    : BITVECTOR_TEST_006
 * @tc.name      : BitVector_pop_Func_002
 * @tc.desc      : Retrieves and removes the bit elements from the end of this bit vector in a loop.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 0
 */
function BitVector_pop_Func_002() {
    let bitVector = new collections.BitVector(0);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res0 = bitVector.push(0);
    }
    arktest.assertEQ(bitVector.length, bitVectorLength, "BitVector length should be 64");

    for (let i = 0; i < bitVectorLength; i++) {
        let res1 = bitVector.pop();
        arktest.assertEQ(res1, 0, "Pop should return 0");
    }
    arktest.assertEQ(bitVector.length, 0, "BitVector length should be 0");
}

/**
 * @tc.number    : BITVECTOR_TEST_007
 * @tc.name      : BitVector_pop_Func_003
 * @tc.desc      : Retrieves and removes the bit elements from the end of this bit vector, leaving the initial length intact.
 * @tc.size      : MediumTest
 * @tc.type      : Function
 * @tc.level     : Level 1
 */
function BitVector_pop_Func_003() {
    let bitVector = new collections.BitVector(10);
    let bitVectorLength = 64;
    for (let i = 0; i < bitVectorLength; i++) {
        let res0 = bitVector.push(0);
    }
    arktest.assertEQ(bitVector.length, bitVectorLength + 10, "BitVector length should be 74");

    for (let i = 0; i < bitVectorLength; i++) {
        let res1 = bitVector.pop();
        arktest.assertEQ(res1, 0, "Pop should return 0");
    }
    arktest.assertEQ(bitVector.length, 10, "BitVector length should be 10");
}
