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

import {util} from "@ohos.util";

function main() {
    const suite = new arktest.ArkTestsuite("CommonUtil RationalNumber Equals ValueOf GetDenominator GetNumerator API tests");

    suite.addTest("Verifies that two RationalNumber objects (2/1 and 3/4) are not equal.", testEquals001)
    suite.addTest("Verifies that two RationalNumber objects (2/1 and 4/2) are equal.", testEquals002)
    suite.addTest("Verifies that two RationalNumber objects (0/1 and 0/2) are equal.", testEquals003)
    suite.addTest("Verifies that two RationalNumber objects (0/0 and 0/2) are not equal.", testEquals004)
    suite.addTest("Verifies that two RationalNumber objects (-2/0 and 2/0) are not equal.", testEquals005)
    suite.addTest("Verifies that the value of the RationalNumber object (2/1).", testValueOf001)
    suite.addTest("Verifies that the value of the RationalNumber object (2/10).", testValueOf002)
    suite.addTest("Verifies that the value of the RationalNumber object (1/2).", testValueOf003)
    suite.addTest("Verifies that the value of the RationalNumber object (4/2).", testValueOf004)
    suite.addTest("Verifies that the value of the RationalNumber object (4/1).", testValueOf005)
    suite.addTest("Verifies that the denominator of the RationalNumber object (2/1) is 1.", testGetDenominator001)
    suite.addTest("Verifies that the denominator of the RationalNumber object (2/3) is 3.", testGetDenominator002)
    suite.addTest("Verifies that the denominator of the RationalNumber object (2/0) is 0.", testGetDenominator003)
    suite.addTest("Verifies that the denominator of the RationalNumber object (10/5) is 1 after simplification.",
        testGetDenominator004)
    suite.addTest("Verifies that the denominator of the RationalNumber object (6/3) is 1 after simplification.",
        testGetDenominator005)
    suite.addTest("Verifies that the denominator of the RationalNumber object (6/-3) is 1 after simplification.",
        testGetDenominator006)
    suite.addTest("Verifies that the numerator of the RationalNumber object (-2/1) is -2.", testGetNumerator001)
    suite.addTest("Verifies that the numerator of the RationalNumber object (0/3) is 0.", testGetNumerator002)
    suite.addTest("Verifies that the numerator of the RationalNumber object (2/4) is 1 after simplification.",
        testGetNumerator003)
    suite.addTest("Verifies that the numerator of the RationalNumber object (3/6) is 1 after simplification.",
        testGetNumerator004)
    suite.addTest("Verifies that the numerator of the RationalNumber object (10/5) is 2 after simplification.",
        testGetNumerator005)

    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_EQUALS_001
 * @tc.name: testEquals001
 * @tc.desc: Verifies that two RationalNumber objects (2/1 and 3/4) are not equal.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testEquals001() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let proc = util.RationalNumber.parseRationalNumber(3, 4);
    let res = pro.equals(proc);
    arktest.assertEQ(res, false, "The two RationalNumber objects (2/1 and 3/4) should not be equal");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_EQUALS_002
 * @tc.name: testEquals002
 * @tc.desc: Verifies that two RationalNumber objects (2/1 and 4/2) are equal.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testEquals002() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let proc = util.RationalNumber.parseRationalNumber(4, 2);
    let res = pro.equals(proc);
    arktest.assertEQ(res, true, "The two RationalNumber objects (2/1 and 4/2) should be equal");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_EQUALS_003
 * @tc.name: testEquals003
 * @tc.desc: Verifies that two RationalNumber objects (0/1 and 0/2) are equal.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testEquals003() {
    let pro = util.RationalNumber.parseRationalNumber(0, 1);
    let proc = util.RationalNumber.parseRationalNumber(0, 2);
    let res = pro.equals(proc);
    arktest.assertEQ(res, true, "The two RationalNumber objects (0/1 and 0/2) should be equal");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_EQUALS_004
 * @tc.name: testEquals004
 * @tc.desc: Verifies that two RationalNumber objects (0/0 and 0/2) are not equal.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testEquals004() {
    let pro = util.RationalNumber.parseRationalNumber(0, 0);
    let proc = util.RationalNumber.parseRationalNumber(0, 2);
    let res = pro.equals(proc);
    arktest.assertEQ(res, false, "The two RationalNumber objects (0/0 and 0/2) should not be equal");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_EQUALS_005
 * @tc.name: testEquals005
 * @tc.desc: Verifies that two RationalNumber objects (-2/0 and 2/0) are not equal.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testEquals005() {
    let pro = util.RationalNumber.parseRationalNumber(-2, 0);
    let proc = util.RationalNumber.parseRationalNumber(2, 0);
    let res = pro.equals(proc);
    arktest.assertEQ(res, false, "The two RationalNumber objects (-2/0 and 2/0) should not be equal");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_VALUEOF_001
 * @tc.name: testValueOf001
 * @tc.desc: Verifies that the value of the RationalNumber object (2/1).
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testValueOf001() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let res = pro.valueOf();
    arktest.assertEQ(res, 2, "The value of the RationalNumber should be 2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_VALUEOF_002
 * @tc.name: testValueOf002
 * @tc.desc: Verifies that the value of the RationalNumber object (2/10).
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testValueOf002() {
    let pro = util.RationalNumber.parseRationalNumber(2, 10);
    let res = pro.valueOf();
    arktest.assertEQ(res, 0.2, "The value of the RationalNumber should be 0.2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_VALUEOF_003
 * @tc.name: testValueOf003
 * @tc.desc: Verifies that the value of the RationalNumber object (1/2).
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testValueOf003() {
    let pro = util.RationalNumber.parseRationalNumber(1, 2);
    let res = pro.valueOf();
    arktest.assertEQ(res, 0.5, "The value of the RationalNumber should be 0.5");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_VALUEOF_004
 * @tc.name: testValueOf004
 * @tc.desc: Verifies that the value of the RationalNumber object (4/2).
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testValueOf004() {
    let pro = util.RationalNumber.parseRationalNumber(4, 2);
    let res = pro.valueOf();
    arktest.assertEQ(res, 2, "The value of the RationalNumber should be 2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_VALUEOF_005
 * @tc.name: testValueOf005
 * @tc.desc: Verifies that the value of the RationalNumber object (4/1).
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testValueOf005() {
    let pro = util.RationalNumber.parseRationalNumber(4, 1)
    let res = pro.valueOf()
    arktest.assertEQ(res, 4, "The value of the RationalNumber should be 4")
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_DENOMINATOR_001
 * @tc.name: testGetDenominator001
 * @tc.desc: Verifies that the denominator of the RationalNumber object (2/1) is 1.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetDenominator001() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let res = pro.getDenominator();
    arktest.assertEQ(res, 1, "The denominator of the RationalNumber (2/1) should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_DENOMINATOR_002
 * @tc.name: testGetDenominator002
 * @tc.desc: Verifies that the denominator of the RationalNumber object (2/3) is 3.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetDenominator002() {
    let pro = util.RationalNumber.parseRationalNumber(2, 3);
    let res = pro.getDenominator();
    arktest.assertEQ(res, 3, "The denominator of the RationalNumber (2/3) should be 3");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_DENOMINATOR_003
 * @tc.name: testGetDenominator003
 * @tc.desc: Verifies that the denominator of the RationalNumber object (2/0) is 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetDenominator003() {
    let pro = util.RationalNumber.parseRationalNumber(2, 0);
    let res = pro.getDenominator();
    arktest.assertEQ(res, 0, "The denominator of the RationalNumber (2/0) should be 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_DENOMINATOR_004
 * @tc.name: testGetDenominator004
 * @tc.desc: Verifies that the denominator of the RationalNumber object (10/5) is 1 after simplification.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetDenominator004() {
    let pro = util.RationalNumber.parseRationalNumber(10, 5);
    let res = pro.getDenominator();
    arktest.assertEQ(res, 1, "The denominator of the RationalNumber (10/5) after simplification should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_DENOMINATOR_005
 * @tc.name: testGetDenominator005
 * @tc.desc: Verifies that the denominator of the RationalNumber object (6/3) is 1 after simplification.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetDenominator005() {
    let pro = util.RationalNumber.parseRationalNumber(6, 3);
    let res = pro.getDenominator();
    arktest.assertEQ(res, 1, "The denominator of the RationalNumber (6/3) after simplification should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_DENOMINATOR_006
 * @tc.name: testGetDenominator006
 * @tc.desc: Verifies that the denominator of the RationalNumber object (6/-3) is 1 after simplification.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetDenominator006() {
    let pro = util.RationalNumber.parseRationalNumber(6, -3);
    let res = pro.getDenominator();
    arktest.assertEQ(res, 1, "The denominator of the RationalNumber (6/-3) after simplification should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_NUMERATOR_001
 * @tc.name: testGetNumerator001
 * @tc.desc: Verifies that the numerator of the RationalNumber object (-2/1) is -2.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetNumerator001() {
    let pro = util.RationalNumber.parseRationalNumber(-2, 1);
    let res = pro.getNumerator();
    arktest.assertEQ(res, -2, "The numerator of the RationalNumber (-2/1) should be -2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_NUMERATOR_002
 * @tc.name: testGetNumerator002
 * @tc.desc: Verifies that the numerator of the RationalNumber object (0/3) is 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetNumerator002() {
    let pro = util.RationalNumber.parseRationalNumber(0, 3);
    let res = pro.getNumerator();
    arktest.assertEQ(res, 0, "The numerator of the RationalNumber (0/3) should be 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_NUMERATOR_003
 * @tc.name: testGetNumerator003
 * @tc.desc: Verifies that the numerator of the RationalNumber object (2/4) is 1 after simplification.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetNumerator003() {
    let pro = util.RationalNumber.parseRationalNumber(2, 4);
    let res = pro.getNumerator();
    arktest.assertEQ(res, 1, "The numerator of the RationalNumber (2/4) after simplification should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_NUMERATOR_004
 * @tc.name: testGetNumerator004
 * @tc.desc: Verifies that the numerator of the RationalNumber object (3/6) is 1 after simplification.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetNumerator004() {
    let pro = util.RationalNumber.parseRationalNumber(3, 6);
    let res = pro.getNumerator();
    arktest.assertEQ(res, 1, "The numerator of the RationalNumber (3/6) after simplification should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_NUMERATOR_005
 * @tc.name: testGetNumerator005
 * @tc.desc: Verifies that the numerator of the RationalNumber object (10/5) is 2 after simplification.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetNumerator005() {
    let pro = util.RationalNumber.parseRationalNumber(10, 5);
    let res = pro.getNumerator();
    arktest.assertEQ(res, 2, "The numerator of the RationalNumber (10/5) after simplification should be 2");
}
