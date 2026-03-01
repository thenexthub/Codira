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
    const suite = new arktest.ArkTestsuite("Decimal atan2 API tests");
    suite.addTest("Decimal atan2 with specific config", testAtan2WithSpecificConfigPart001);
    suite.addTest("Decimal atan2 with specific config", testAtan2WithSpecificConfigPart002);
    suite.addTest("Decimal atan2 with specific config", testAtan2WithSpecificConfigPart003);
    return suite.run();
}

function checkAtan2Result(y: string, x: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(Decimal.atan2(y, x).valueOf(), expected);
}

function testAtan2WithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkAtan2Result('NaN', '1', 20, 4, 'NaN');
    checkAtan2Result('NaN', 'Infinity', 20, 4, 'NaN');
    checkAtan2Result('0', 'NaN', 20, 4, 'NaN');
    checkAtan2Result('-Infinity', 'NaN', 20, 4, 'NaN');
    checkAtan2Result('NaN', 'NaN', 20, 4, 'NaN');
    checkAtan2Result('1', '0', 20, 4, '1.5707963267948966192');
    checkAtan2Result('1', '-0', 20, 4, '1.5707963267948966192');
    checkAtan2Result('0', '1', 20, 4, '0');
    checkAtan2Result('0', '0', 20, 4, '0');
    checkAtan2Result('0', '-0', 20, 4, '3.1415926535897932385');
    checkAtan2Result('0', '-1', 20, 4, '3.1415926535897932385');
    checkAtan2Result('-0', '1', 20, 4, '-0');
    checkAtan2Result('-0', '0', 20, 4, '-0');
    checkAtan2Result('-0', '-0', 20, 4, '-3.1415926535897932385');
    checkAtan2Result('-0', '-1', 20, 4, '-3.1415926535897932385');
    checkAtan2Result('-1', '0', 20, 4, '-1.5707963267948966192');
    checkAtan2Result('-1', '-0', 20, 4, '-1.5707963267948966192');
    checkAtan2Result('1', 'Infinity', 20, 4, '0');
    checkAtan2Result('1', '-Infinity', 20, 4, '3.1415926535897932385');
    checkAtan2Result('-1', 'Infinity', 20, 4, '-0');
    checkAtan2Result('-1', '-Infinity', 20, 4, '-3.1415926535897932385');
    checkAtan2Result('Infinity', '1', 20, 4, '1.5707963267948966192');
    checkAtan2Result('Infinity', '-1', 20, 4, '1.5707963267948966192');
    checkAtan2Result('-Infinity', '1', 20, 4, '-1.5707963267948966192');
    checkAtan2Result('-Infinity', '-1', 20, 4, '-1.5707963267948966192');
    checkAtan2Result('Infinity', 'Infinity', 20, 4, '0.78539816339744830962');
    checkAtan2Result('Infinity', '-Infinity', 20, 4, '2.3561944901923449288');
    checkAtan2Result('-Infinity', 'Infinity', 20, 4, '-0.78539816339744830962');
    checkAtan2Result('-Infinity', '-Infinity', 20, 4, '-2.3561944901923449288');
}

function testAtan2WithSpecificConfigPart002() {
    checkAtan2Result('17', '14', 4, 2, '0.8819');
    checkAtan2Result('0.9481218', '0.94118', 6, 6, '0.789072');
    checkAtan2Result('-7445', '20099977666211456789846696402535482054490525592', 5, 3, '-0.0000000000000000000000000000000000000000003704');
    checkAtan2Result('35739959929999990.99999999995999999999999999999999992959', '0.955', 8, 6, '1.5707963');
    checkAtan2Result('1.1', '6524806825029162376.20457337656513698810928', 5, 1, '0.00000000000000000016858');
    checkAtan2Result('5211111.101111111177', '606497413079598703117552754486732', 6, 1, '0.00000000000000000000000000859214');
    checkAtan2Result('-2913.07599', '986666666666666.8888888888873058007', 6, 6, '-0.00000000000295244');
    checkAtan2Result('2.66213924447211273', '0.344623945616634', 3, 0, '1.45');
    checkAtan2Result('312631627154243240137182349906197', '9999999555555555.88888888889999999999', 10, 1, '1.570796326');
    checkAtan2Result('899.99', '26.9', 3, 4, '1.54');
    checkAtan2Result('-2', '-0.0000006306201002089224863559916332763966685755123769189', 3, 5, '-1.57');
    checkAtan2Result('0.21', '8', 4, 2, '0.02625');
    checkAtan2Result('0.9481218', '0.94118', 6, 6, '0.789072');
    checkAtan2Result('2', '9333333333333333397407226578881017498733601111111111111', 9, 1, '0.000000000000000000000000000000000000000000000000000000214285714');
    checkAtan2Result('95726', '-2693596537831566265', 9, 3, '3.14159265');
    checkAtan2Result('3', '136424614399.47534873339', 6, 2, '0.0000000000219902');
    checkAtan2Result('724388888888888888888', '452240666.2222266666', 3, 0, '1.58');
    checkAtan2Result('0.04129', '4.7459', 6, 3, '0.00869992');
    checkAtan2Result('-0.000000003593', '831', 9, 3, '-0.00000000000432370638');
    checkAtan2Result('3', '652', 1, 3, '0.004');
    checkAtan2Result('53', '0.00021', 5, 2, '1.5708');
}

function testAtan2WithSpecificConfigPart003() {
    checkAtan2Result('583', '0.5', 2, 3, '1.5');
    checkAtan2Result('0.88956212348302', '-3613.47', 4, 1, '3.141');
    checkAtan2Result('499999999999999999999999999999999999999999899999999999999999999999999999999999999999999999999999.99999', '0.397', 2, 2, '1.6');
    checkAtan2Result('670.7246136257055836748897015621', '8', 9, 1, '1.55886949');
    checkAtan2Result('40000', '6809492801384871383672604464351', 5, 5, '0.0000000000000000000000000058742');
    checkAtan2Result('6000000002222222.00014266', '-0.450903', 1, 6, '2');
    checkAtan2Result('3', '4', 10, 3, '0.6435011087');
    checkAtan2Result('2666641666666666', '59999999999999999999999999999999999999999.99999999999999999999999999925', 1, 6, '0.00000000000000000000000004');
    checkAtan2Result('5', '0.65', 7, 6, '1.441521');
    checkAtan2Result('43.65', '8568924402932018888.66666666666666666666135181', 9, 0, '0.00000000000000000509398823');
    checkAtan2Result('4891999999999999999999999999999999999999999999999999991999999799999.999999999639999999992999', '710000000000000000.0000000000005700000000000060000000000033', 10, 0, '1.570796327');
    checkAtan2Result('4.73001', '455555555555559399859117776418166', 8, 0, '0.000000000000000000000000000000010382949');
    checkAtan2Result('40068.2342', '1', 2, 2, '1.6');
    checkAtan2Result('0.6', '511111111112433397604060000000000000000000', 1, 4, '0.000000000000000000000000000000000000000001');
    checkAtan2Result('530000000000000.286', '7', 7, 1, '1.570796');
    checkAtan2Result('9251010716.51554394500003489103488443', '727127846893147834657170633071486391409', 2, 6, '0.000000000000000000000000000013');
    checkAtan2Result('2156', '6', 5, 0, '1.5681');
    checkAtan2Result('-0.007700837496370833875888297147521851009315187908640623592495789607215971', '-0.60609673305145265303411222292406724609400618553386938', 8, 4, '-3.1288877');
    checkAtan2Result('0.631666343041280486605909338251618534368378514270566344350649949184', '0.1', 9, 0, '1.41378793');
    checkAtan2Result('0.81895579436335688497005428053140270015707', '34.88388', 10, 4, '0.02347231408');
    checkAtan2Result('7819', '8111111111111114050153374108607899100080038117344269814600726014856', 6, 2, '0.000000000000000000000000000000000000000000000000000000000000000963987');
    checkAtan2Result('78230794955', '-6', 10, 2, '1.570796327');
    checkAtan2Result('444444444444444444444', '0.7', 8, 3, '1.5707963');
    checkAtan2Result('0.9793', '0.7', 1, 1, '0.9');
    checkAtan2Result('5244107.0628867', '2999954.9965', 7, 3, '1.051172');
}
