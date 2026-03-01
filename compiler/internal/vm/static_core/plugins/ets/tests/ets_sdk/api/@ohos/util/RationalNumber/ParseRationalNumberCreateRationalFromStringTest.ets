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
    const suite = new arktest.ArkTestsuite("CommonUtil RationalNumber ParseRationalNumber CreateRationalFromString API tests");

    suite.addTest("Parses two integers (2 and 1) into a RationalNumber and verifies its value.",
        testParseRationalNumber001)
    suite.addTest("Creates a RationalNumber from the string '-1:2' and verifies its value.",
        testCreateRationalFromString001)
    suite.addTest("Creates a RationalNumber from the string '+3/4' and verifies its value.",
        testCreateRationalFromString002)
    suite.addTest("Creates a RationalNumber from the string '+3:-4' and verifies its value.",
        testCreateRationalFromString003)
    suite.addTest("Creates a RationalNumber from the string '+2:4' and verifies its value.",
        testCreateRationalFromString004)
    suite.addTest("Creates a RationalNumber from the string '+2:-4' and verifies its value.",
        testCreateRationalFromString005)
    suite.addTest("Creates a RationalNumber from the string '-2:-4' and verifies its value.",
        testCreateRationalFromString006)
    suite.addTest("Verifies that creating a RationalNumber from the string '123' throws a BusinessError.",
        testCreateRationalFromString007)
    suite.addTest("Verifies that creating a RationalNumber from the string 'abc' throws a BusinessError.",
        testCreateRationalFromString008)
    suite.addTest("Verifies that creating a RationalNumber from the string '123abc' throws a BusinessError.",
        testCreateRationalFromString009)
    suite.addTest("Verifies that creating a RationalNumber from the string 'abc:' throws a BusinessError.",
        testCreateRationalFromString010)
    suite.addTest("Verifies that creating a RationalNumber from the string ':abc' throws a BusinessError.",
        testCreateRationalFromString011)
    suite.addTest("Verifies that creating a RationalNumber from the string 'abc/' throws a BusinessError.",
        testCreateRationalFromString012)
    suite.addTest("Verifies that creating a RationalNumber from the string '123:abc' throws a BusinessError.",
        testCreateRationalFromString013)
    suite.addTest("Verifies that creating a RationalNumber from the string 'abc/123' throws a BusinessError.",
        testCreateRationalFromString014)
    suite.addTest("Verifies that creating a RationalNumber from the string 'abc/abcd' throws a BusinessError.",
        testCreateRationalFromString015)
    suite.addTest("Verifies that creating a RationalNumber from an empty string throws a BusinessError.",
        testCreateRationalFromString016)

    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_PARSE_RATIONAL_NUMBER_001
 * @tc.name: testParseRationalNumber001
 * @tc.desc: Parses two integers (2 and 1) into a RationalNumber and verifies its value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testParseRationalNumber001() {
    let res = util.RationalNumber.parseRationalNumber(2, 1);
    let result = res.valueOf();
    arktest.assertEQ(result, 2, "The value of the RationalNumber should be 2");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_001
 * @tc.name: testCreateRationalFromString001
 * @tc.desc: Creates a RationalNumber from the string '-1:2' and verifies its value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString001() {
    let res = util.RationalNumber.createRationalFromString('-1:2');
    let result = res.valueOf();
    arktest.assertEQ(result, -0.5, "The RationalNumber value should be -0.5");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_002
 * @tc.name: testCreateRationalFromString002
 * @tc.desc: Creates a RationalNumber from the string '+3/4' and verifies its value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString002() {
    let res = util.RationalNumber.createRationalFromString('+3/4');
    let result = res.valueOf();
    arktest.assertEQ(result, 0.75, "The RationalNumber value should be 0.75");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_003
 * @tc.name: testCreateRationalFromString003
 * @tc.desc: Creates a RationalNumber from the string '+3:-4' and verifies its value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString003() {
    let res = util.RationalNumber.createRationalFromString('+3:-4');
    let result = res.valueOf();
    arktest.assertEQ(result, -0.75, "The RationalNumber value should be -0.75");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_004
 * @tc.name: testCreateRationalFromString004
 * @tc.desc: Creates a RationalNumber from the string '+2:4' and verifies its value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString004() {
    let res = util.RationalNumber.createRationalFromString('+2:4');
    let result = res.valueOf();
    arktest.assertEQ(result, 0.5, "The RationalNumber value should be 0.5");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_005
 * @tc.name: testCreateRationalFromString005
 * @tc.desc: Creates a RationalNumber from the string '+2:-4' and verifies its value.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString005() {
    let res = util.RationalNumber.createRationalFromString('+2:-4');
    let result = res.valueOf();
    arktest.assertEQ(result, -0.5, "The RationalNumber value should be -0.5");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_006
 * @tc.name: testCreateRationalFromString006
 * @tc.desc: Creates a RationalNumber from the string '-2:-4' and verifies its value.
 * @tc.size: MediumT
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString006() {
    let res = util.RationalNumber.createRationalFromString('-2:-4');
    let result1 = res.valueOf();
    arktest.assertEQ(result1, 0.5, "The RationalNumber value should be 0.5");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_007
 * @tc.name: testCreateRationalFromString007
 * @tc.desc: Verifies that creating a RationalNumber from the string '123' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString007() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of 123 must be effective string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('123') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_008
 * @tc.name: testCreateRationalFromString008
 * @tc.desc: Verifies that creating a RationalNumber from the string 'abc' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString008() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of abc must be effective string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('abc') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_009
 * @tc.name: testCreateRationalFromString009
 * @tc.desc: Verifies that creating a RationalNumber from the string '123abc' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString009() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of 123abc must be effective string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('123abc') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_010
 * @tc.name: testCreateRationalFromString010
 * @tc.desc: Verifies that creating a RationalNumber from the string 'abc:' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString010() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of abc: must be character string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('abc:') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_011
 * @tc.name: testCreateRationalFromString011
 * @tc.desc: Verifies that creating a RationalNumber from the string ':abc' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString011() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of :abc must be character string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString(':abc') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_012
 * @tc.name: testCreateRationalFromString012
 * @tc.desc: Verifies that creating a RationalNumber from the string 'abc/' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString012() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of abc/ must be character string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('abc/') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_013
 * @tc.name: testCreateRationalFromString013
 * @tc.desc: Verifies that creating a RationalNumber from the string '123:abc' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString013() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of 123:abc must be character string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('123:abc') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_014
 * @tc.name: testCreateRationalFromString014
 * @tc.desc: Verifies that creating a RationalNumber from the string 'abc/123' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString014() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of abc/123 must be character string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('abc/123') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_015
 * @tc.name: testCreateRationalFromString015
 * @tc.desc: Verifies that creating a RationalNumber from the string 'abc/abcd' throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString015() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of abc/abcd must be character string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('abc/abcd') }, exceptionCheck);
}

/**
 * @tc.number: SUB_COMMONLIBRARY_CREATE_RATIONAL_FROM_STRING_016
 * @tc.name: testCreateRationalFromString016
 * @tc.desc: Verifies that creating a RationalNumber from an empty string throws a BusinessError.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testCreateRationalFromString016() {
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                "Parameter error. The type of  must be effective string" && e.code == 401;
        }
        return false;
    }
    arktest.expectThrow(() => { let rational = util.RationalNumber.createRationalFromString('') }, exceptionCheck);
}
