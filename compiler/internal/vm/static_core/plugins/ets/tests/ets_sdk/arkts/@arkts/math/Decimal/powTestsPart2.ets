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
    suite.addTest("Decimal pow with different precision and rounding", testPowWithDiffPRandRM001);
    suite.addTest("Decimal pow with different precision and rounding", testPowWithDiffPRandRM002);
    suite.addTest("Decimal pow with different precision and rounding", testPowWithDiffPRandRM003);
    return suite.run();
}

function checkPowResult(base: string, exp: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd; 
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(base).pow(exp).valueOf(), expected);
}

function testPowWithDiffPRandRM001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkPowResult('48.9262695992662373981', '1.0', '48.926269599266237', 17, 1);
    checkPowResult('1.21', '0.5', '1.1', 2, 1);
    checkPowResult('12.96', '0.5', '3.6', 2, 3);
    checkPowResult('3.24', '0.5', '1.8', 2, 1);
    checkPowResult('70.56', '0.5', '8.4', 2, 3);
    checkPowResult('4.41', '6.5', '15447.2377739119461', 32, 3);
    checkPowResult('11.05', '2.00000000000000007', '122.103', 6, 4);
    checkPowResult('10.5', '3.000000000000000002', '1157.63', 6, 4);
    checkPowResult('1.00000000000000000003', '4.00000005', '1.000000000000000000120000002', 28, 4);
    checkPowResult('6.0000005', '1.00000000000000006', '6.000001', 7, 4);
}

function testPowWithDiffPRandRM002() {
    checkPowResult('1.0000000000000000000005', '49.0000000000000000000002', '1.000000000000000000025', 22, 4);
    checkPowResult('5344.87762641765349023882127126550721', '1.0625', '9139.7407411741874683083843738640173291', 38, 1);
    checkPowResult('28', '6.166675020000903537297764507632802193308677149', '839756321.64088511', 17, 0);
    checkPowResult('91.180153837', '0.5', '9.54882997214842023704943457512609', 33, 1);
    checkPowResult('16', '26.5', '81129638414606681695789005144064', 35, 4);
    checkPowResult('25', '13.5', '7450580596923828125', 39, 4);
    checkPowResult('4.3985903', '20.9956530307', '32120869378609.033520730996715368034448124619', 44, 2);
    checkPowResult('2.858368', '48.97', '21682301291468972839895.193017121528607658932', 44, 5);
    checkPowResult('91.145', '23.8479557348417627', '54402923894673605836306983589686900000000000000', 33, 1);
    checkPowResult('5.379973182', '2.65', '86.4', 3, 6);
}

function testPowWithDiffPRandRM003() {
    checkPowResult('625', '4.5', '3814697265625', 13, 0);
    checkPowResult('65536', '1.25', '1048576', 7, 5);
    checkPowResult('9', '1.5', '27', 2, 8);
    checkPowResult('256', '1.625', '8192', 4, 8);
    checkPowResult('65536', '1.875', '1073741824', 10, 6);
    checkPowResult('65536', '2.5', '1099511627776', 13, 1);
    checkPowResult('625', '5.25', '476837158203125', 15, 4);
    checkPowResult('0.16', '-0.9999999999999', '6.2', 2, 4);
    checkPowResult('3.6361', '-0.06', '0.92547', 5, 4);
    checkPowResult('8.7881541', '-0.00000006', '0.999999869595727123998', 22, 4);
    checkPowResult('5.812', '-0.99999', '0.17206083953928505581714758136682954', 35, 4);
    checkPowResult('6.06737421654397', '-0.000000000001', '0.99999999999819707407228698', 26, 4);
    checkPowResult('5.57197470953405387', '-0.9', '0.213', 3, 4);
}
