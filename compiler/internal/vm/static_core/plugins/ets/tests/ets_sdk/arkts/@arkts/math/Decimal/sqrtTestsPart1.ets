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
    const suite = new arktest.ArkTestsuite("Decimal sqrt API tests");
    suite.addTest("Decimal clamp with specific config", testSqrtWithDiffPRandRM001);
    suite.addTest("Decimal clamp with specific config", testSqrtWithDiffPRandRM002);
    suite.addTest("Decimal clamp with specific config", testSqrtWithDiffPRandRM003);
    suite.addTest("Decimal clamp with specific config", testSqrtWithDiffPRandRM004);
    return suite.run();
}

function checkSqrtResult(expected: string, n: string | double, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).sqrt().valueOf(), expected);
}

function testSqrtWithDiffPRandRM001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkSqrtResult('NaN', NaN, 20, 4);
    checkSqrtResult('2', 4, 20, 4);
    checkSqrtResult('0.1', 0.01, 20, 4);
    checkSqrtResult('0', 0, 20, 4);
    checkSqrtResult('0', '0', 20, 4);
    checkSqrtResult('-0', '-0', 20, 4);
    checkSqrtResult('Infinity', Infinity, 20, 4);
    checkSqrtResult('NaN', -Infinity, 20, 4);
    checkSqrtResult('NaN', -1, 20, 4);
    checkSqrtResult('NaN', -35.999, 20, 4);
    checkSqrtResult('NaN', '-0.00000000000001', 20, 4);
}

function testSqrtWithDiffPRandRM002() {
    for (let i = 0; i < 500; i++) {
        let j = Math.floor(Math.random() * Math.pow(2, Math.floor(Math.random() * 26) + 1));
        j *= j;
        checkSqrtResult(new String(Math.sqrt(j)), new String(j), 20, 4);
    }
}

function testSqrtWithDiffPRandRM003() {
    checkSqrtResult('11', '101', 2, 0);
    checkSqrtResult('11', '111', 2, 0);
    checkSqrtResult('1000', 999000.25, 3, 0);
    checkSqrtResult('10', '101', 2, 1);
    checkSqrtResult('10', '111', 2, 1);
    checkSqrtResult('999', 999000.25, 3, 1);
    checkSqrtResult('11', '101', 2, 2);
    checkSqrtResult('11', '111', 2, 2);
    checkSqrtResult('1000', 999000.25, 3, 2);
    checkSqrtResult('10', '101', 2, 3);
    checkSqrtResult('10', '111', 2, 3);
    checkSqrtResult('999', 999000.25, 3, 3);
    checkSqrtResult('10', '101', 2, 4);
    checkSqrtResult('11', '111', 2, 4);
    checkSqrtResult('1000', 999000.25, 3, 4);
    checkSqrtResult('10', '101', 2, 5);
    checkSqrtResult('11', '111', 2, 5);
    checkSqrtResult('999', 999000.25, 3, 5);
    checkSqrtResult('10', '101', 2, 6);
    checkSqrtResult('11', '111', 2, 6);
    checkSqrtResult('1000', 999000.25, 3, 6);
    checkSqrtResult('10', '101', 2, 7);
    checkSqrtResult('11', '111', 2, 7);
    checkSqrtResult('1000', 999000.25, 2, 7);
    checkSqrtResult('10', '101', 2, 8);
    checkSqrtResult('11', '111', 2, 8);
    checkSqrtResult('999', 999000.25, 3, 8);
}

function testSqrtWithDiffPRandRM004() {
    checkSqrtResult('3.7', '13.69', 2, 6);
    checkSqrtResult('14.8', '219.04', 3, 6);
    checkSqrtResult('3.578708384438441', '12.80715370085', 16, 6);
    checkSqrtResult('49.56954417823', '2457.139710037', 13, 6);
    checkSqrtResult('4.9808503290100979', '24.80887', 17, 6);
    checkSqrtResult('54108953.7650348268963935650889893674872', '2927778877546676.5690784769302', 39, 6);
    checkSqrtResult('1.0000000000001', '1.000000000000000001402', 14, 2);
    checkSqrtResult('18.000001', '324.0000000000000000000008467', 8, 2);
    checkSqrtResult('11.00000000000000001', '121.000000000000000000000000001944', 19, 0);
    checkSqrtResult('2.00000000000001', '4.0000000000000000000000000000000005485', 15, 0);
    checkSqrtResult('7.1', '49.00000000000000000000007338', 2, 0);
    checkSqrtResult('19.00000000000001', '361.0000000000000000000000005645', 16, 2);
    checkSqrtResult('1.0000001', '1.00000000000000000000000000000006986', 8, 0);
    checkSqrtResult('30771277.0274560007853901874282412', '946871489900441.411712749999999999999999999999999999999999999999', 33, 6);
    checkSqrtResult('668.093253221434649746', '446348.594999999999999999999999999999', 21, 5);
    checkSqrtResult('0.9999999999999999999999999999999999999999', '0.9999999999999999999999999999999999999998', 40, 5);
    checkSqrtResult('0.9999999999999999999997187097975391240474677563824', '0.9999999999999999999994374195950782480949355918890297218761543222489261', 49, 3);
    checkSqrtResult('0.0000000000000298840312673111495324166498096924391126062701', '8.9305532478563043E-28', 45, 5);
    checkSqrtResult('0.0000000000000000000000000000000000000000025385115498645', '6.4440408887956349899441566682289191111111111111111111E-84', 14, 5);
    checkSqrtResult('0.0000000000000000000000000000000000000000000000000000140471526414353053458592377645343274655348630930173397', '1.97322497331782876158645511100784258994087355048761111111111111E-106', 54, 5);
    checkSqrtResult('0.00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002598902022257132166703490111187879281161770014282895', '6.7542917212922111E-232', 52, 5);
    checkSqrtResult('0.00999999999999999999999999999949', '0.00009999999999999999999999999999', 30, 1);
}
