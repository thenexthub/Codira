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
    const suite = new arktest.ArkTestsuite("Decimal sin API tests");
    suite.addTest("Decimal sin with specific config", testSinWithSpecificConfigPart001);
    suite.addTest("Decimal sin with specific config", testSinWithSpecificConfigPart002);
    return suite.run();
}

function checkSinResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).sin().valueOf(), expected);
}

function testSinWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkSinResult('NaN', 40, 4, 'NaN');
    checkSinResult('0', 20, 4, '0');
    checkSinResult('-0', 20, 4, '-0');
    checkSinResult('Infinity', 40, 4, 'NaN');
    checkSinResult('-Infinity', 40, 4, 'NaN');

    checkSinResult('93332358216', 48, 5, '0.000816283943127306029649893248669685836695194145277');
    checkSinResult('766737078569022420173292205068953186848344673451240027655054631094318050', 18, 6, '0.00111135055272494455');

    checkSinResult('0.359136383962093189473162700700590970518456985109181875257764726415814563424613193774806585890478208084275468196568294033', 38, 1, '0.35146584650000000000000000000000000006');
    checkSinResult('3202222222227222222222222222222224222222222222222222222222.222222222222222222222222222', 7, 0, '0.5499568');
    checkSinResult('8', 5, 6, '0.98936');
    checkSinResult('0.15865555', 6, 0, '0.157991');
    checkSinResult('4.752', 5, 5, '-0.99922');
    checkSinResult('82315980453524767298085693995239044972', 5, 6, '0.18971');
    checkSinResult('9975666666', 10, 4, '-0.1932244457');
    checkSinResult('8111111111111111222222222222222.1118', 8, 4, '0.93383768');
    checkSinResult('1582389099446045988.444444444164077777777777777777', 3, 5, '0.145');
    checkSinResult('42.29499991999999919999999919', 2, 5, '-0.99');
    checkSinResult('2', 3, 1, '0.909');
    checkSinResult('225204831071708162873414430801935442203', 3, 4, '-0.367');
    checkSinResult('70387777777777777777', 6, 6, '-0.903662');
    checkSinResult('0.000009960105486531367489163480390414384530151948706557615588133070855712559603', 5, 3, '0.0000099601');
    checkSinResult('2.3', 1, 5, '0.7');
    checkSinResult('0.0369897655941305860082440964', 1, 0, '0.04');
    checkSinResult('93333333333.1111111113333335607254506018497853948241554833333333', 1, 4, '1');
    checkSinResult('0.81475726', 10, 2, '0.7275590895');
}

function testSinWithSpecificConfigPart002() {
    checkSinResult('0.0020719', 5, 2, '0.0020719');
    checkSinResult('0.822309454290117043180146453820530722', 3, 2, '0.733');
    checkSinResult('811801481366184919659088.161679696898591', 8, 0, '-0.33739172');
    checkSinResult('82222222239', 6, 1, '0.85357');
    checkSinResult('0.671515352458', 4, 1, '0.6221');
    checkSinResult('0.1', 10, 2, '0.09983341665');
    checkSinResult('30.4507', 5, 0, '-0.82218');
    checkSinResult('82539', 1, 6, '0.06');
    checkSinResult('7904297312438003184', 8, 1, '0.88548151');
    checkSinResult('1555591677261255189013625216076301609467761280', 4, 4, '0.6012');
    checkSinResult('0.206', 5, 0, '0.20455');
    checkSinResult('47.85', 4, 3, '-0.664');
    checkSinResult('354522', 6, 0, '-0.432959');
    checkSinResult('57111.555555555551111', 9, 1, '-0.51645502');
    checkSinResult('5084117035723528870966332457.91693481159599044107', 10, 0, '0.5456651665');
    checkSinResult('66', 3, 2, '-0.0265');
    checkSinResult('4415133931290705273393593393689.69701970681754754956578240920056', 3, 4, '0.651');
    checkSinResult('4641', 4, 1, '-0.7628');
    checkSinResult('60782.222222', 3, 0, '-0.967');
    checkSinResult('9295.7', 4, 1, '0.2692');
    checkSinResult('660060.8', 5, 6, '-0.3736');
    checkSinResult('86', 7, 0, '-0.9234585');
    checkSinResult('0.867224365334893476015547423881141062327495194226274166759019810427983958472019', 7, 0, '0.7625362');
    checkSinResult('35', 9, 4, '-0.428182669');
    checkSinResult('6.4522', 4, 2, '0.1683');
}
