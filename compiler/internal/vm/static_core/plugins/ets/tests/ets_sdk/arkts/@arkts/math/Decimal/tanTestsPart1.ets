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
    const suite = new arktest.ArkTestsuite("Decimal tan API tests");
    suite.addTest("Decimal tan with specific config", testTanWithSpecificConfig001);
    suite.addTest("Decimal tan with specific config", testTanWithSpecificConfig002);
    return suite.run();
}

function checkTanResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).tan().valueOf(), expected);
}

function testTanWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkTanResult('NaN', 40, 4, 'NaN');
    checkTanResult('0', 20, 4, '0');
    checkTanResult('-0', 20, 4, '-0');
    checkTanResult('Infinity', 40, 4, 'NaN');
    checkTanResult('-Infinity', 40, 4, 'NaN');

    checkTanResult('3', 5, 5, '-0.14255');
    checkTanResult('0.8', 7, 6, '1.029639');
    checkTanResult('539998661123466984649191689138494900416386817996524', 7, 6, '32.04695');
    checkTanResult('0.5172293543584894071801901902', 5, 1, '0.56888');
    checkTanResult('5', 10, 5, '-3.380515006');
    checkTanResult('0.5283979979897015', 1, 4, '0.6');
    checkTanResult('0.032294', 4, 5, '0.03231');
    checkTanResult('6339441327243052114957865586706124503.386166832963015135918043343179', 7, 0, '-0.06666567');
    checkTanResult('0.9483926380646355642560348839879277', 4, 6, '1.394');
    checkTanResult('0.4989999', 7, 6, '0.5450046');
    checkTanResult('80', 8, 6, '9.0036549');
    checkTanResult('9', 2, 0, '-0.46');
    checkTanResult('710.0571640039', 2, 2, '0.058');
    checkTanResult('2485', 2, 0, '0.00022');
    checkTanResult('1', 8, 1, '1.5574077');
    checkTanResult('1683930704126586032365374681825182443446390958574678366195884605194996193', 6, 1, '-0.540063');
    checkTanResult('4', 7, 2, '1.157822');
    checkTanResult('488888888888999.88888888406189896488888888888', 9, 0, '5.10971582');
    checkTanResult('0.76322415', 7, 0, '0.9566071');
    checkTanResult('0.005', 4, 2, '0.005001');
    checkTanResult('0.313', 9, 6, '0.323638526');
    checkTanResult('9798000000100000070000000000010007300', 5, 0, '1.1308');
    checkTanResult('0.0000002', 2, 0, '0.00000021');
    checkTanResult('7.9548', 4, 1, '-9.885');
    checkTanResult('7', 5, 1, '0.87144');
    checkTanResult('50080000', 10, 3, '0.3252122977');
    checkTanResult('1687619439340503284670339680.931590414877', 6, 6, '-4.69126');
}

function testTanWithSpecificConfig002() {
    checkTanResult('60970535.5', 3, 1, '0.804');
    checkTanResult('0.17', 3, 2, '0.172');
    checkTanResult('0.5', 5, 6, '0.5463');
    checkTanResult('976.8', 2, 0, '-0.24');
    checkTanResult('5957.69', 6, 0, '2.82277');
    checkTanResult('3859393443331162786655368.87986161531415848272022672234461227844', 3, 6, '0.47');
    checkTanResult('0.5', 1, 5, '0.5');
    checkTanResult('468455006548968895675682202443647176252098177665257898933', 5, 5, '-1.4111');
    checkTanResult('26588816756666', 1, 2, '0.07');
    checkTanResult('0.239424045403994683949187909216', 5, 2, '0.24411');
    checkTanResult('810209.109', 6, 6, '-4.52238');
    checkTanResult('799063368.11558288974072', 1, 2, '-0.9');
    checkTanResult('72.3640503', 7, 2, '0.1078344');
    checkTanResult('843003462755848631687717888604', 5, 5, '-1.1699');
    checkTanResult('90706.61', 3, 2, '-0.676');
    checkTanResult('0.1', 7, 5, '0.1003347');
    checkTanResult('29555.88888888888885555555', 9, 5, '-0.218161582');
    checkTanResult('3079999999999999999999.999949999999999999999999999999999999999999999', 2, 5, '1.6');
    checkTanResult('6', 3, 4, '-0.291');
    checkTanResult('9.648589238187519', 7, 0, '0.2276248');
    checkTanResult('590.89', 7, 1, '0.2773838');
    checkTanResult('807876335', 9, 2, '0.965588602');
    checkTanResult('32099561.989', 8, 4, '-0.24584079');
    checkTanResult('7743398777777777777777733.77', 3, 2, '-0.526');
    checkTanResult('45.52', 5, 2, '30.207');
    checkTanResult('399999999999999999999999999999999999999999999999909999999999.99999999999999', 4, 4, '1.742');
    checkTanResult('32400', 3, 2, '0.941');
    checkTanResult('4152914444444444444444444544444444444444444444444444444.944444444440444442444494444444444', 9, 0, '-0.101470716');
    checkTanResult('90000000000508319.117628510842726222', 5, 0, '5.1435');
    checkTanResult('0.0074072', 8, 3, '0.0074073354');
    checkTanResult('0.413272', 4, 5, '0.4385');
    checkTanResult('0.8633', 10, 3, '1.169338115');
    checkTanResult('33333333333333333333333333333.33333333333333333333', 8, 2, '0.5813679');
}
