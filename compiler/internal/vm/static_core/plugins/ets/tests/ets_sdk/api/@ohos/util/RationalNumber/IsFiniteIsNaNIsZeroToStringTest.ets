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
    const suite = new arktest.ArkTestsuite("CommonUtil RationalNumber IsFinite IsNaN IsZero ToString API tests");

    suite.addTest("Verifies that the RationalNumber object (-2/1) represents a finite value.", testIsFinite001)
    suite.addTest("Verifies that the RationalNumber object (0/3) represents a finite value.", testIsFinite002)
    suite.addTest("Verifies that the RationalNumber object (2/0) does not represent a finite value.", testIsFinite003)
    suite.addTest("Verifies that the RationalNumber object (1/3) represents a finite value.", testIsFinite004)
    suite.addTest("Verifies that the RationalNumber object (10/5) represents a finite value.", testIsFinite005)
    suite.addTest("Verifies that the RationalNumber object (-2/1) is not NaN.", testIsNaN001)
    suite.addTest("Verifies that the RationalNumber object (0/3) is not NaN.", testIsNaN002)
    suite.addTest("Verifies that the RationalNumber object (0/0) is NaN.", testIsNaN003)
    suite.addTest("Verifies that the RationalNumber object (10/0) is not NaN.", testIsNaN004)
    suite.addTest("Verifies that the RationalNumber object (10/1) is not NaN.", testIsNaN005)
    suite.addTest("Verifies that the RationalNumber object (-2/1) does not represent the value 0.", testIsZero001)
    suite.addTest("Verifies that the RationalNumber object (0/3) represents the value 0.", testIsZero002)
    suite.addTest("Verifies that the RationalNumber object (0/0) does not represent the value 0.", testIsZero003)
    suite.addTest("Verifies that the RationalNumber object (10/2) does not represent the value 0.", testIsZero004)
    suite.addTest("Verifies that the RationalNumber object (1/1) does not represent the value 0.", testIsZero005)
    suite.addTest("Verifies that the string representation of the RationalNumber object (-2/1) is '-2/1'.",
        testToString001)
    suite.addTest("Verifies that the string representation of the RationalNumber object (0/0) is 'NaN'.",
        testToString002)
    suite.addTest("Verifies that the string representation of the RationalNumber object (3/0) is 'Infinity'.",
        testToString003)
    suite.addTest("Verifies that the string representation of the RationalNumber object (-3/0) is '-Infinity'.",
        testToString004)
    suite.addTest("Verifies that the string representation of the RationalNumber object (2/3) is '2/3'.",
        testToString005)

    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISFINITE_001
 * @tc.name: testIsFinite001
 * @tc.desc: Verifies that the RationalNumber object (-2/1) represents a finite value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFinite001() {
    let pro = util.RationalNumber.parseRationalNumber(-2, 1);
    let res = pro.isFinite();
    arktest.assertEQ(res, true, "The RationalNumber (-2/1) should represent a finite value");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISFINITE_002
 * @tc.name: testIsFinite002
 * @tc.desc: Verifies that the RationalNumber object (0/3) represents a finite value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFinite002() {
    let pro = util.RationalNumber.parseRationalNumber(0, 3);
    let res = pro.isFinite();
    arktest.assertEQ(res, true, "The RationalNumber (0/3) should represent a finite value");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISFINITE_003
 * @tc.name: testIsFinite003
 * @tc.desc: Verifies that the RationalNumber object (2/0) does not represent a finite value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFinite003() {
    let pro = util.RationalNumber.parseRationalNumber(2, 0);
    let res = pro.isFinite();
    arktest.assertEQ(res, false, "The RationalNumber (2/0) should not represent a finite value");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISFINITE_004
 * @tc.name: testIsFinite004
 * @tc.desc: Verifies that the RationalNumber object (1/3) represents a finite value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFinite004() {
    let pro = util.RationalNumber.parseRationalNumber(1, 3);
    let res = pro.isFinite();
    arktest.assertEQ(res, true, "The RationalNumber (1/3) should represent a finite value");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISFINITE_005
 * @tc.name: testIsFinite005
 * @tc.desc: Verifies that the RationalNumber object (10/5) represents a finite value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFinite005() {
    let pro = util.RationalNumber.parseRationalNumber(10, 5);
    let res = pro.isFinite();
    arktest.assertEQ(res, true, "The RationalNumber (10/5) should represent a finite value");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISNAN_001
 * @tc.name: testIsNaN001
 * @tc.desc: Verifies that the RationalNumber object (-2/1) is not NaN.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNaN001() {
    let pro = util.RationalNumber.parseRationalNumber(-2, 1);
    let res = pro.isNaN();
    arktest.assertEQ(res, false, "The RationalNumber (-2/1) should not be NaN");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISNAN_002
 * @tc.name: testIsNaN002
 * @tc.desc: Verifies that the RationalNumber object (0/3) is not NaN.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNaN002() {
    let pro = util.RationalNumber.parseRationalNumber(0, 3);
    let res = pro.isNaN();
    arktest.assertEQ(res, false, "The RationalNumber (0/3) should not be NaN");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISNAN_003
 * @tc.name: testIsNaN003
 * @tc.desc: Verifies that the RationalNumber object (0/0) is NaN.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNaN003() {
    let pro = util.RationalNumber.parseRationalNumber(0, 0);
    let res = pro.isNaN();
    arktest.assertEQ(res, true, "The RationalNumber (0/0) should be NaN");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISNAN_004
 * @tc.name: testIsNaN004
 * @tc.desc: Verifies that the RationalNumber object (10/0) is not NaN.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNaN004() {
    let pro = util.RationalNumber.parseRationalNumber(10, 0);
    let res = pro.isNaN();
    arktest.assertEQ(res, false, "The RationalNumber (10/0) should not be NaN");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISNAN_005
 * @tc.name: testIsNaN005
 * @tc.desc: Verifies that the RationalNumber object (10/1) is not NaN.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNaN005() {
    let pro = util.RationalNumber.parseRationalNumber(10, 1);
    let res = pro.isNaN();
    arktest.assertEQ(res, false, "The RationalNumber (10/1) should not be NaN");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISZERO_001
 * @tc.name: testIsZero001
 * @tc.desc: Verifies that the RationalNumber object (-2/1) does not represent the value 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsZero001() {
    let pro = util.RationalNumber.parseRationalNumber(-2, 1);
    let res = pro.isZero();
    arktest.assertEQ(res, false, "The RationalNumber (-2/1) should not represent the value 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISZERO_002
 * @tc.name: testIsZero002
 * @tc.desc: Verifies that the RationalNumber object (0/3) represents the value 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsZero002() {
    let pro = util.RationalNumber.parseRationalNumber(0, 3);
    let res = pro.isZero();
    arktest.assertEQ(res, true, "The RationalNumber (0/3) should represent the value 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISZERO_003
 * @tc.name: testIsZero003
 * @tc.desc: Verifies that the RationalNumber object (0/0) does not represent the value 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsZero003() {
    let pro = util.RationalNumber.parseRationalNumber(0, 0);
    let res = pro.isZero();
    arktest.assertEQ(res, false, "The RationalNumber (0/0) should not represent the value 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISZERO_004
 * @tc.name: testIsZero004
 * @tc.desc: Verifies that the RationalNumber object (10/2) does not represent the value 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsZero004() {
    let pro = util.RationalNumber.parseRationalNumber(10, 2);
    let res = pro.isZero();
    arktest.assertEQ(res, false, "The RationalNumber (10/2) should not represent the value 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_ISZERO_005
 * @tc.name: testIsZero005
 * @tc.desc: Verifies that the RationalNumber object (1/1) does not represent the value 0.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsZero005() {
    let pro = util.RationalNumber.parseRationalNumber(1, 1);
    let res = pro.isZero();
    arktest.assertEQ(res, false, "The RationalNumber (1/1) should not represent the value 0");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_TOSTRING_001
 * @tc.name: testToString001
 * @tc.desc: Verifies that the string representation of the RationalNumber object (-2/1) is '-2/1'.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testToString001() {
    let pro = util.RationalNumber.parseRationalNumber(-2, 1);
    let res = pro.toString();
    arktest.assertEQ(res, "-2/1", "The string representation of the RationalNumber should be '-2/1'");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_TOSTRING_002
 * @tc.name: testToString002
 * @tc.desc: Verifies that the string representation of the RationalNumber object (0/0) is 'NaN'.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testToString002() {
    let pro = util.RationalNumber.parseRationalNumber(0, 0);
    let res = pro.toString();
    arktest.assertEQ(res, "NaN", "The string representation of the RationalNumber should be 'NaN'");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_TOSTRING_003
 * @tc.name: testToString003
 * @tc.desc: Verifies that the string representation of the RationalNumber object (3/0) is 'Infinity'.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testToString003() {
    let pro = util.RationalNumber.parseRationalNumber(3, 0);
    let res = pro.toString();
    arktest.assertEQ(res, "Infinity", "The string representation of the RationalNumber should be 'Infinity'");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_TOSTRING_004
 * @tc.name: testToString004
 * @tc.desc: Verifies that the string representation of the RationalNumber object (-3/0) is '-Infinity'.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testToString004() {
    let pro = util.RationalNumber.parseRationalNumber(-3, 0);
    let res = pro.toString();
    arktest.assertEQ(res, "-Infinity", "The string representation of the RationalNumber should be '-Infinity'");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_RATIONALNUMBER_TOSTRING_005
 * @tc.name: testToString005
 * @tc.desc: Verifies that the string representation of the RationalNumber object (2/3) is '2/3'.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testToString005() {
    let pro = util.RationalNumber.parseRationalNumber(2, 3);
    let res = pro.toString();
    arktest.assertEQ(res, "2/3", "The string representation of the RationalNumber should be '2/3'");
}
