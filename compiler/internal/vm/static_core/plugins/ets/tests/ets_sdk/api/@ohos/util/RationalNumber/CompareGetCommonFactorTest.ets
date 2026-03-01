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
import {BusinessError} from "@ohos.base";

function main() {
    const suite = new arktest.ArkTestsuite("CommonUtil RationalNumber Compare GetCommonFactor API tests");

    suite.addTest("Compares two RationalNumber objects (2/1 and 3/4) and verifies the result.", testCompare001)
    suite.addTest("Compares two RationalNumber objects (2/1 and 0/0) and verifies the result.", testCompare002)
    suite.addTest("Compares two RationalNumber objects (2/1 and 8/3) and verifies the result.", testCompare003)
    suite.addTest("Compares two identical RationalNumber objects (2/1 and 2/1) and verifies the result.",
        testCompare004)
    suite.addTest("Compares two RationalNumber objects (0/0 and 2/1) and verifies the result.", testCompare005)
    suite.addTest("Calculates the greatest common divisor of 4 and 8.", testGetCommonFactor001)
    suite.addTest("Calculates the greatest common divisor of 10 and 15.", testGetCommonFactor002)
    suite.addTest("Calculates the greatest common divisor of 8 and 4.", testGetCommonFactor003)
    suite.addTest("Calculates the greatest common divisor of 8 and 16.", testGetCommonFactor004)
    suite.addTest("Calculates the greatest common divisor of 2 and 16.", testGetCommonFactor005)
    suite.addTest("Obtains the greatest common divisor of 4 and 6.", testGetCommonFactor006)
    suite.addTest("Obtains the greatest common divisor of 1 and 6.", testGetCommonFactor007)
    suite.addTest("Verifies that obtaining the greatest common divisor with a zero parameter throws a BusinessError.",
        testGetCommonFactor008)
    suite.addTest("Verifies that obtaining the greatest common divisor with both are zero throws a BusinessError.",
        testGetCommonFactor009)

    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_COMPARE_001
 * @tc.name: testCompare001
 * @tc.desc: Compares two RationalNumber objects (2/1 and 3/4) and verifies the result.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCompare001() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let proc = util.RationalNumber.parseRationalNumber(3, 4);
    let res = pro.compare(proc);
    arktest.assertEQ(res, 1, "The comparison result should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_COMPARE_002
 * @tc.name: testCompare002
 * @tc.desc: Compares two RationalNumber objects (2/1 and 0/0) and verifies the result.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCompare002() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let proc = util.RationalNumber.parseRationalNumber(0, 0);
    let res = pro.compare(proc);
    arktest.assertEQ(res, -1, "The comparison result should be -1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_COMPARE_003
 * @tc.name: testCompare003
 * @tc.desc: Compares two RationalNumber objects (2/1 and 8/3) and verifies the result.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCompare003() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let proc = util.RationalNumber.parseRationalNumber(8, 3);
    let res = pro.compare(proc);
    arktest.assertEQ(res, -1, "The comparison result should be -1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_COMPARE_004
 * @tc.name: testCompare004
 * @tc.desc: Compares two identical RationalNumber objects (2/1 and 2/1) and verifies the result.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCompare004() {
    let pro = util.RationalNumber.parseRationalNumber(2, 1);
    let proc = util.RationalNumber.parseRationalNumber(2, 1);
    let res = pro.compare(proc);
    arktest.assertEQ(res, 0, "The comparison result should be 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_COMPARE_005
 * @tc.name: testCompare005
 * @tc.desc: Compares two RationalNumber objects (0/0 and 2/1) and verifies the result.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCompare005() {
    let pro = util.RationalNumber.parseRationalNumber(0, 0);
    let proc = util.RationalNumber.parseRationalNumber(2, 1);
    let res = pro.compare(proc);
    arktest.assertEQ(res, 1, "The comparison result should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_001
 * @tc.name: testGetCommonFactor001
 * @tc.desc: Calculates the greatest common divisor of 4 and 8.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor001() {
    let res = util.RationalNumber.getCommonFactor(4, 8);
    arktest.assertEQ(res, 4, "The greatest common divisor of 4 and 8 should be 4");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_002
 * @tc.name: testGetCommonFactor002
 * @tc.desc: Calculates the greatest common divisor of 10 and 15.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor002() {
    let res = util.RationalNumber.getCommonFactor(10, 15);
    arktest.assertEQ(res, 5, "The greatest common divisor of 10 and 15 should be 5");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_003
 * @tc.name: testGetCommonFactor003
 * @tc.desc: Calculates the greatest common divisor of 8 and 4.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor003() {
    let res = util.RationalNumber.getCommonFactor(8, 4);
    arktest.assertEQ(res, 4, "The greatest common divisor of 8 and 4 should be 4");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_004
 * @tc.name: testGetCommonFactor004
 * @tc.desc: Calculates the greatest common divisor of 8 and 16.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor004() {
    let res = util.RationalNumber.getCommonFactor(8, 16);
    arktest.assertEQ(res, 8, "The greatest common divisor of 8 and 16 should be 8");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_005
 * @tc.name: testGetCommonFactor005
 * @tc.desc: Calculates the greatest common divisor of 2 and 16.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor005() {
    let res = util.RationalNumber.getCommonFactor(2, 16);
    arktest.assertEQ(res, 2, "The greatest common divisor of 2 and 16 should be 2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_006
 * @tc.name: testGetCommonFactor006
 * @tc.desc: Obtains the greatest common divisor of 4 and 6.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor006() {
    let result = util.RationalNumber.getCommonFactor(4, 6);
    arktest.assertEQ(result, 2, "The greatest common divisor of 4 and 6 should be 2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_007
 * @tc.name: testGetCommonFactor007
 * @tc.desc: Obtains the greatest common divisor of 1 and 6.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor007() {
    let result = util.RationalNumber.getCommonFactor(1, 6);
    arktest.assertEQ(result, 1, "The greatest common divisor of 1 and 6 should be 1");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_008
 * @tc.name: testGetCommonFactor008
 * @tc.desc: Verifies that obtaining the greatest common divisor with a zero parameter throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor008() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The Parameter cannot be zero" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let result = util.RationalNumber.getCommonFactor(0, 6) }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_GET_COMMON_FACTOR_009
 * @tc.name: testGetCommonFactor009
 * @tc.desc: Verifies that obtaining the greatest common divisor with both parameters as zero throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testGetCommonFactor009() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The Parameter cannot be zero" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let result = util.RationalNumber.getCommonFactor(0, 0) }, exceptionCheck);
}
