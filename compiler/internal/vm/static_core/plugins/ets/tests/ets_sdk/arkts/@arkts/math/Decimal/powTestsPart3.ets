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
    suite.addTest("Decimal pow with different precision and rounding", testPowWithDiffPRandRM004);
    return suite.run();
}

function checkPowResult(base: string, exp: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd; 
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(base).pow(exp).valueOf(), expected);
}

function testPowWithDiffPRandRM001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkPowResult('8.4297580531324', '-0.000000000000002', '0.99999999999999573646385819', 26, 4);
    checkPowResult('1.746122696164', '-0.9', '0.605526', 6, 4);
    checkPowResult('5.74274073282643192871', '-0.000000000000004', '0.999999999999993008253696156596264156', 36, 4);
    checkPowResult('9.66306878602393217324', '-0.999', '0.1037217997755957147', 19, 4);
    checkPowResult('5', '-0.9999999', '0.200000032188760838972540436', 28, 4);
    checkPowResult('21.8005326254960840089', '14.99999999999999999999', '119400615273418803650.1362563340821916898208389', 46, 4);
    checkPowResult('46.80102307015', '4.000001', '4797589.19437982876031', 21, 4);
    checkPowResult('29.255206217375', '9.9999999999999999999999999', '459231465846284.22207', 20, 4);
    checkPowResult('0.72591761772', '6.999999999999999999999', '0.106221237503302998', 18, 4);
    checkPowResult('0.3928066161887', '32.0002', '0.00000000000010319062643292561810866879769121849802', 38, 4);
    checkPowResult('24.798046085018648753453', '5.9999999999999999', '232543806.207', 12, 4);
}

function testPowWithDiffPRandRM002() {
    checkPowResult('20.485568584242', '18.99999999999999999999999', '8270131718672851271097903.87621818958353436851582', 48, 4);
    checkPowResult('969.0', '-1', '0.0010319917440660474716202', 23, 4);
    checkPowResult('8.97', '-1', '0.111482720178', 12, 3);
    checkPowResult('61766796871807246.3278075', '-1', '0.00000000000000001618993', 7, 0);
    checkPowResult('-1', '101', '-1', 100, 1);
    checkPowResult('-1', '9999999999999999999999999999999999999999999999999999999999999999999999999', '-1', 100, 1);
    checkPowResult('-1', '1e307', '1', 100, 1);
    checkPowResult('-1', '1e309', '1', 100, 1);
}

function testPowWithDiffPRandRM003() {
    Decimal.toExpNeg = Decimal.toExpPos = 0;
    checkPowResult('9.9999999999999', '2220.75', '5.623413251778e+2220', 13, 1);
    checkPowResult('0.9999999999999999991999999999019999949909999999', '2220.75', '9.999999999999982233999997e-1', 25, 1);
    checkPowResult('987504387560932846509387650789.49807365', '981459.4903857', '9.876e+29438424', 4, 4);
    checkPowResult('-2', '1001', '-2.1430172143725346418e+301', 20, 1);
    checkPowResult('-2', 1e6, '9.9006562292958982506979236164e+301029', 29, 0);
    checkPowResult('5.0771598579583468811E-101844', '7064449.87442997380369702938801116641723585825702571602', '3.907934864857193219594361275098983e-719466848189', 34, 1);
    checkPowResult('5.80246472674775E+21125581', '0.00077726506294426495082193497633668602085', '1.5018938138904125617523547e+16420', 26, 3);
    checkPowResult('1.66630944E+74', '6980757669.9081156729942256', '3.74152e+518124090060', 6, 5);
    checkPowResult('5.9E+6', '3456.7700', '1.8971788927235700943477592799711063194e+23405', 38, 0);
}

function testPowWithDiffPRandRM004() {
    checkPowResult('93720986.7819907489497420190553708041564963922285117', '39.580', '3.3e+315', 2, 1);
    checkPowResult('908948247.896330216349750387912923575076135766138', '11.38907521122213262858256836', '1.0702278292293091784680297675223031e+102', 35, 3);
    checkPowResult('4.485925762349120387154391E+47', '1677945.16766265206929939', '8.53959030215133943e+79957194', 18, 5);
    checkPowResult('2.8448989811706207675566E+89', '2.368592228588521845032068137267440272102614', '7.58940197453762187722508511706932e+211', 33, 5);
    checkPowResult('0.9999999999999999', '-1e+30', '1.530863912e+43429448190325', 10, 1);
    checkPowResult('0.9999999999999999999999999999999999999999999999999', '-1e+32', '1.00000000000000001000000000000000005e+0', 36, 1);
    checkPowResult('0.9999999999999999', '-1e+50', 'Infinity', 40, 1);
    checkPowResult('0.9999999999999999999999999999999899999999999999994403269002375809806554775739676251993670310626872684', '-1.49181945463118148622657269735650603014891811120124843379694396257337810020127409048127397077199569e+271', 'Infinity', 100, 1);
}
