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

import { Decimal } from '@arkts.math.Decimal';

function main(): int {
    const suite = new arktest.ArkTestsuite('Decimal static div API tests');
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });

    suite.addTest('Decimal div for result is Infinity or -Infinity', testDivForInfinity);
    suite.addTest('Decimal div for result is NaN', testDivForNaN);
    suite.addTest('Decimal div with different precision and rounding', testAddWithDiffConfig001);
    suite.addTest('Decimal div with different precision and rounding', testAddWithDiffConfig002);
    suite.addTest('Decimal div with different precision and rounding', testAddWithDiffConfig003);
    suite.addTest('Decimal div with different precision and rounding', testAddWithDiffConfig004);
    return suite.run();
}

function testDivForInfinity() {
    arktest.assertEQ(Decimal.div(1, 0).valueOf(), 'Infinity');
    arktest.assertEQ(Decimal.div(-1, 0).valueOf(), '-Infinity');
    arktest.assertEQ(Decimal.div(Infinity, 1).valueOf(), 'Infinity');
    arktest.assertEQ(Decimal.div(Infinity, -1).valueOf(), '-Infinity');
    arktest.assertEQ(Decimal.div(-Infinity, 1).valueOf(), '-Infinity');
    arktest.assertEQ(Decimal.div(-Infinity, -1).valueOf(), 'Infinity');
    arktest.assertEQ(Decimal.div(Infinity, 0).valueOf(), 'Infinity');
    arktest.assertEQ(Decimal.div(-Infinity, 0).valueOf(), '-Infinity');
}

function testDivForNaN() {
    arktest.assertEQ(Decimal.div(1, NaN).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(0, 0).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(0, NaN).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(NaN, 1).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(NaN, 0).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(NaN, NaN).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(NaN, Infinity).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(NaN, -Infinity).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(Infinity, NaN).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(-Infinity, NaN).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(Infinity, Infinity).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(Infinity, -Infinity).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(-Infinity, Infinity).valueOf(), 'NaN');
    arktest.assertEQ(Decimal.div(-Infinity, -Infinity).valueOf(), 'NaN');
}

function checkDivResult(dividend: string, divisor: double | string, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(Decimal.div(dividend, divisor).valueOf(), expected);
}

function testAddWithDiffConfig001() {
    checkDivResult('999.5', 1, '1000', 3, 0);
    checkDivResult('-999.5', 1, '-1000', 3, 0);
    checkDivResult('999.5', 1, '999', 3, 1);
    checkDivResult('-999.5', 1, '-999', 3, 1);
    checkDivResult('999.5', 1, '1000', 3, 2);
    checkDivResult('-999.5', 1, '-999', 3, 2);
    checkDivResult('999.5', 1, '999', 3, 3);
    checkDivResult('-999.5', 1, '-1000', 3, 3);
    checkDivResult('999.5', 1, '1000', 3, 4);
}

function testAddWithDiffConfig002() {
    checkDivResult('-999.5', 1, '-1000', 3, 4);
    checkDivResult('999.5', 1, '999', 3, 5);
    checkDivResult('-999.5', 1, '-999', 3, 5);
    checkDivResult('999.5', 1, '1000', 3, 6);
    checkDivResult('-999.5', 1, '-1000', 3, 6);
    checkDivResult('999.4', 1, '999', 3, 6);
    checkDivResult('-999.4', 1, '-999', 3, 6);
    checkDivResult('999.500001', 1, '1000', 3, 6);
    checkDivResult('-999.500001', 1, '-1000', 3, 6);
}

function testAddWithDiffConfig003() {
    checkDivResult('999.5', 1, '1000', 3, 7);
    checkDivResult('-999.5', 1, '-999', 3, 7);
    checkDivResult('999.5', 1, '999', 3, 8);
    checkDivResult('-999.5', 1, '-1000', 3, 8);
    checkDivResult('-2.8', '3', '-1', 1, 3);
    checkDivResult('-3', '-0.0047', '638.297872341', 12, 0);
    checkDivResult('15', '-3', '-5', 1, 0);
    checkDivResult('3', '0.38', '7.894736842106', 13, 2);
    checkDivResult('-2.9', '-2.2', '1.3181818181818182', 17, 4);
    checkDivResult('-1.20', '0.00035', '-3428.571428571', 13, 4);
}

function testAddWithDiffConfig004() {
    Decimal.toExpNeg = Decimal.toExpPos = 0;
    checkDivResult('-9.0201582007891419174890e+493625', '-5.50622769083719199346231074389e-8', '1.638173847369118854685833779858335057671769579218908745636679e+493633', 61, 4);
    checkDivResult('5.8729543161021068260124718708021832883057695897e+49274', '-8.708403405379587884587853336e+41461', '-6.7440081065538470526693072631940999905631492068918496007e+7812', 56, 3);
    checkDivResult('-1.11303135570566757735452613975119540626335099312445395511e+909913', '-4.9428912981238120390359176251627374079558e-50436', '2.25178198057510227e+960348', 18, 1);
    checkDivResult('9.34964786072062858037053820543908584e+34427', '-5.5139174707961944758881146494671658725524731327174937382473e+727', '-1.69564523049899134866471123135148425607621655195972485244321739609622142643026524021192529472e+33700', 93, 4);
    checkDivResult('-8.95495805776908286477460208890137140530903524050e+3546735', '-514449923650777303707976731752571405.60601209', '1.740686050494577121230344265740919289154954933388084539546267265149437887923633011167e+3546700', 85, 0);
    checkDivResult('-112977.156416947803174690696837418982360160807579018', '-6.7793027141628576643753135768314185053384986437525e+32928', '1.666501131169782697443349597254128378095825062301415158287415940806830463694e-32924', 76, 5);
    checkDivResult('-7.519050808224891132821326159698e-555', '0.0417050002083', '-1.80291e-553', 6, 1);
}