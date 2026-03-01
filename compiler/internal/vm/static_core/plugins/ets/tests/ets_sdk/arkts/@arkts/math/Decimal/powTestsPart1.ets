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
    const suite = new arktest.ArkTestsuite("Decimal pow API tests");
    suite.addTest("Decimal pow with different precision and rounding", testPowWithDiffPRandRM);
    return suite.run();
}

function checkPowResult(base: string, exp: string, expected: string, sd: double, rm: int) {
    Decimal.set({ precision: sd, rounding: rm, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(base).pow(exp).valueOf(), expected);
}

function testPowWithDiffPRandRM() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkPowResult('9', '0.5', '3', 7, 4);
    checkPowResult('9', '0.5', '3', 26, 4);
    checkPowResult('0.9999999999', '6', '0.999999999400000000149999999980000000001', 39, 4);
    checkPowResult('2.56', '6.5', '450.3599627370496', 16, 1);
    checkPowResult('1.96', '1.5', '2.744', 15, 1);
    checkPowResult('2.25', '9.5', '2216.8378200531005859375', 23, 1);
    checkPowResult('11.05', '2.00000000000000007', '122.103', 6, 4);
    checkPowResult('10.5', '3.000000000000000002', '1157.63', 6, 4);
    checkPowResult('1.00000000000000000003', '4.00000005', '1.000000000000000000120000002', 28, 4);
    checkPowResult('6.0000005', '1.00000000000000006', '6.000001', 7, 4);
    checkPowResult('1.0000000000000000000005', '49.0000000000000000000002', '1.000000000000000000025', 22, 4);
    checkPowResult('15.333333333333333333', '28.33333333333333', '3917746643938779840069598486694964.98308568625045', 49, 4);
    checkPowResult('7.537714', '7.9', '8515169.08260507715975', 21, 4);
    checkPowResult('6.951', '9.225', '58598464.57', 10, 4);
    checkPowResult('6.01093', '9.8911', '50651225.3819968681522216250662534915', 36, 4);
    checkPowResult('8.7587', '4.23', '9694.37298592397372', 18, 4);
    checkPowResult('5.1749', '7.7267995', '328229.2815443039852', 19, 4);
    checkPowResult('0.16', '-0.9999999999999', '6.2', 2, 4);
    checkPowResult('0.4', '-20', '90949470.1772928237915039063', 27, 4);
    checkPowResult('0.5', '22', '0.000000238418579101563', 15, 4);
    checkPowResult('32', '0.4', '4', 1, 4);
    checkPowResult('4', '2.5', '32', 11, 4);
    checkPowResult('4', '5.5', '2048', 27, 4);
    checkPowResult('16', '23.5', '19807040628566084398385987584', 29, 4);
    checkPowResult('16', '26.5', '81129638414606681695789005144064', 35, 4);
    checkPowResult('25', '13.5', '7450580596923828125', 39, 4);
    checkPowResult('32', '28.2', '2787593149816327892691964784081045188247552', 43, 4);
    checkPowResult('32', '3.6', '262144', 35, 4);
    checkPowResult('25', '21.5', '1136868377216160297393798828125', 31, 4);
    checkPowResult('9', '8.5', '129140163', 19, 4);
    checkPowResult('4', '7.5', '32768', 13, 4);
    checkPowResult('4', '6.5', '8192', 10, 4);
    checkPowResult('6.034', '0.25964', '2', 1, 4);
    checkPowResult('9', '4.5', '19683', 16, 4);
    checkPowResult('9', '1.5', '27', 5, 4);
    checkPowResult('9.61', '3.5', '2751.2614111', 12, 4);
    checkPowResult('4', '6.5', '8192', 8, 4);
    checkPowResult('4', '7.5', '32768', 11, 4);
    checkPowResult('9', '4.5', '19683', 5, 4);
}
