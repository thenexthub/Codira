/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0(the "License");
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

import { Decimal } from "@arkts.math.Decimal";

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal decimalPlaces and precision API tests");
    suite.addTest("Decimal decimalPlaces and precision with constructor from number", testDpSdWithConstructorFromNumber);
    suite.addTest("Decimal decimalPlaces and precision with constructor from string", testDpSdWithConstructorFromString);
    return suite.run();
}

function checkDpAndSdResult(n: double | string, dp: double, sd: double, zs?: int | boolean) {
    if (Number.isNaN(dp)) {
        arktest.assertTrue(Number.isNaN(new Decimal(n).decimalPlaces()));
        arktest.assertTrue(Number.isNaN(new Decimal(n).precision()));
    } else {
        arktest.assertEQ(new Decimal(n).decimalPlaces(), dp);
        if (zs == undefined) {
            arktest.assertEQ(new Decimal(n).precision(), sd);
        } else {
            arktest.assertEQ(new Decimal(n).precision(zs), sd);
        }
    }
}

function testDpSdWithConstructorFromNumber() {
    checkDpAndSdResult(0, 0, 1);
    checkDpAndSdResult(-0, 0, 1);
    checkDpAndSdResult(NaN, NaN, NaN);
    checkDpAndSdResult(Infinity, NaN, NaN);
    checkDpAndSdResult(-Infinity, NaN, NaN);
    checkDpAndSdResult(1, 0, 1);
    checkDpAndSdResult(-1, 0, 1);
    checkDpAndSdResult(100, 0, 1);
    checkDpAndSdResult(100, 0, 1, 0);
    checkDpAndSdResult(100, 0, 1, false);
    checkDpAndSdResult(100, 0, 3, 1);
    checkDpAndSdResult(100, 0, 3, true);
}

function testDpSdWithConstructorFromString() {
    checkDpAndSdResult('987654321000000.0012345689000001', 16, 31, 0);
    checkDpAndSdResult('987654321000000.0012345689000001', 16, 31, 1);
    checkDpAndSdResult('1e+123', 0, 1);
    checkDpAndSdResult('1e+123', 0, 124, 1);
    checkDpAndSdResult('1e-123', 123, 1);
    checkDpAndSdResult('1e-123', 123, 1, 1);
    checkDpAndSdResult('9.9999e+9000000000000000', 0, 5, false);
    checkDpAndSdResult('9.9999e+9000000000000000', 0, 9000000000000001, true);
    checkDpAndSdResult('-9.9999e+9000000000000000', 0, 5, false);
    checkDpAndSdResult('-9.9999e+9000000000000000', 0, 9000000000000001, true);
    checkDpAndSdResult('1e-9000000000000000', 9e15, 1, false);
    checkDpAndSdResult('1e-9000000000000000', 9e15, 1, true);
    checkDpAndSdResult('-1e-9000000000000000', 9e15, 1, false);
    checkDpAndSdResult('-1e-9000000000000000', 9e15, 1, true);
    checkDpAndSdResult('55325252050000000000000000000000.000000004534500000001', 21, 53);
}
