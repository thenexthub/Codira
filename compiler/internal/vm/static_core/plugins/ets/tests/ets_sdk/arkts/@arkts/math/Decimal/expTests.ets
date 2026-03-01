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
    const suite = new arktest.ArkTestsuite("Decimal exp API tests");
    suite.addTest("Decimal exp with different precision and rounding", testExpWithDiffPRandRM001);
    suite.addTest("Decimal exp with different precision and rounding", testExpWithDiffPRandRM002);
    suite.addTest("Decimal exp with different precision and rounding", testExpWithDiffPRandRM003);
    suite.addTest("Decimal exp with different precision and rounding", testExpWithDiffPRandRM004);
    suite.addTest("Decimal exp with different precision and rounding", testExpWithDiffPRandRM005);
    return suite.run();
}

function checkExpResult(n: string, expected: string, pr: double, rm: int) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).exp().valueOf(), expected);
}

function testExpWithDiffPRandRM001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkExpResult('0', '1', 40, 4);
    checkExpResult('0', String(Math.exp(0)), 40, 4);
    checkExpResult('-0', '1', 40, 4);
    checkExpResult('-0', String(Math.exp(-0)), 40, 4);
    checkExpResult('Infinity', 'Infinity', 40, 4);
    checkExpResult('Infinity', String(Math.exp(Infinity)), 40, 4);
    checkExpResult('-Infinity', '0', 40, 4);
    checkExpResult('-Infinity', String(Math.exp(-Infinity)), 40, 4);
    checkExpResult('NaN', 'NaN', 40, 4);
    checkExpResult('NaN', String(Math.exp(NaN)), 40, 4);
    checkExpResult('1', '2.718281828459045235360287471352662497757', 40, 4);
    checkExpResult('4.504575', '90.42990317191332252519829', 25, 5);
    checkExpResult('6.3936622751479561979', '598.04277120550571020949043340838952845520759628012', 50, 4);
    checkExpResult('0.000000000000000004', '1.000000000000000004000000000000000008001', 40, 2);
    checkExpResult('0.9', '2.45960311115694966380012656360247069542177230644', 48, 1);
    checkExpResult('-0.0000000000000005', '0.999999999999999500000000000000124', 33, 1);
    checkExpResult('-0.00000000000000000001', '0.99999999999999999999000000000000000000004999', 44, 3);
    checkExpResult('-0.000000000000004', '0.999999999999996000000000000008', 31, 0);
    checkExpResult('-0.0000000000000000006', '0.99999', 5, 3);
    checkExpResult('0.0000000000000000006', '1', 5, 3);
    checkExpResult('-0.0000003', '1', 1, 4);
}

function testExpWithDiffPRandRM002() {
    checkExpResult('20.72326583694641116', '1000000000.0000000038', 20, 1);
    checkExpResult('-27.6310211159285483', '0.000000000000999', 3, 1);
    checkExpResult('-0.9', '0.40656965974059911188345423964562', 32, 1);
    checkExpResult('-0.00000000000005', '0.999', 3, 3);
    checkExpResult('-0.9999999999999999', '0.367879441171442358383467887305694866395394004', 45, 4);
    checkExpResult('-0.99999', '0.36788311998424806939070532012638041', 35, 3);
    checkExpResult('-0.00000000001', '0.99999999999000000000004999999999983333333333375', 49, 2);
    checkExpResult('-0.9999999999999', '0.367879441171479109539640916233017625680100198337', 48, 0);
    checkExpResult('-0.999999999', '0.36787944153932176295090581241', 29, 0);
    checkExpResult('-0.0000000003', '0.9999999997000000001', 19, 2);
    checkExpResult('-0.0000001', '0.99999990000000499999983333333749999991667', 41, 2);
    checkExpResult('-0.0000000000000001', '0.9999999999999999', 26, 1);
    checkExpResult('-0.999999999999999', '0.36788', 5, 2);
    checkExpResult('-0.999999999', '0.367879441539321762951', 21, 4);
    checkExpResult('-0.000000000001', '0.9999999999990000000000005', 31, 0);
    checkExpResult('-0.1', '0.9048374180359595731642491', 25, 0);
    checkExpResult('-0.99999999', '0.36787944485', 12, 3);
    checkExpResult('-0.99999999', '0.36787944485023675170391910600205499737', 38, 0);
    checkExpResult('-0.1', '0.9048374180359595731642491', 25, 0);
    checkExpResult('-0.9', '0.4065696597', 10, 3);
    checkExpResult('-0.9999999999999', '0.367879441171479', 15, 3);
    checkExpResult('-0.99', '0.371576691022045690531524119908201386918028', 42, 3);
    checkExpResult('-0.999999999999999', '0.3678794411714426894749649417', 28, 0);
    checkExpResult('-0.9', '0.4', 2, 1);
}

function testExpWithDiffPRandRM003() {
    checkExpResult('-0.00000000009', '0.99999', 5, 1);
    checkExpResult('0.9', '2.45960311115694966380012656360247069542177230644', 48, 1);
    checkExpResult('40.95984262795251', '614658133673303019.41715', 23, 1);
    checkExpResult('50.57728', '9234930123395249855007.64784227728909958776637', 45, 0);
    checkExpResult('-9.295952106254287693', '0.00009179505707794839996147521992', 28, 3);
    checkExpResult('24.429', '40679902037.5', 12, 1);
    checkExpResult('3.085347', '21.875056169741656067', 20, 2);
    checkExpResult('6.079163', '436.663554324263441178', 21, 0);
    checkExpResult('0.89588138', '2.4494937731', 11, 5);
    checkExpResult('3.06', '21.3', 3, 4);
    checkExpResult('0.828620743', '2.2901578446832146', 17, 6);
    checkExpResult('0.8747', '2.39815573', 9, 1);
    checkExpResult('4', '54.5', 3, 3);
    checkExpResult('1.74023', '5.698653962365493026791', 22, 3);
    checkExpResult('0.3178134', '1.37411982654', 12, 5);
    checkExpResult('1.0212228', '2.77658790066265475', 18, 0);
    checkExpResult('2.8', '16.444646771097049871498016', 26, 6);
    checkExpResult('2', '7.389', 5, 1);
    checkExpResult('2.13349', '8.44428600324102919', 19, 1);
    checkExpResult('1.0306766', '2.8029617', 8, 4);
    checkExpResult('1.38629371', '3.99999739553', 12, 0);
    checkExpResult('2.140864956', '8.5', 2, 4);
}

function testExpWithDiffPRandRM004() {
    checkExpResult('1', '2.7182818284590452353602874713', 29, 1);
    checkExpResult('2.8', '16.4446467711', 13, 4);
    checkExpResult('1.7923271', '6.0034067514286690274238254973', 29, 0);
    checkExpResult('2', '7.38905609893065', 15, 4);
    checkExpResult('1.839758663', '6.2950188567239', 14, 1);
    checkExpResult('3.1541', '23.4319388536798', 15, 3);
    checkExpResult('6.23103', '508.27874', 8, 1);
    checkExpResult('0.15', '1.16183424272828312261663', 24, 0);
    checkExpResult('3.6454', '38.298089', 8, 4);
    checkExpResult('2.8086602', '16.5877', 6, 2);
    checkExpResult('1', '2.71828182845904523536', 22, 4);
    checkExpResult('3.712', '40.9355959021562903', 19, 2);
    checkExpResult('1.742336005', '5.71066800248', 12, 5);
    checkExpResult('67.0234090932359557332', '129000000000000000000000000000', 3, 2);
    checkExpResult('6.4350484439574', '623.3127778698531510658792212713024749828103299', 46, 3);
    checkExpResult('-90.6147801309103528', '0.0000000000000000000000000000000000000004430992452392223286671364132586', 31, 0);
    checkExpResult('52.6735295600', '75131702520984694212520.839', 26, 5);
    checkExpResult('4.91754742409', '136.667015585278752656929641054712859399847337855456678258883', 60, 0);
    checkExpResult('-8.291786018917236430647515856', '0.0002505', 4, 3);
}

function testExpWithDiffPRandRM005() {
    Decimal.toExpNeg = Decimal.toExpPos = 0;
    checkExpResult('20723265836946413', '6.3207512951460243608e+9000000000000000', 20, 4);
    checkExpResult('-20723265836946411', '1.1690154783664756563e-9000000000000000', 20, 4);
    checkExpResult('2.08E+16', 'Infinity', 10, 1);
    checkExpResult('9.99999999e+9000000000000000', 'Infinity', 100, 4);
    checkExpResult('-2.08E+16', '0e+0', 10, 1);
    checkExpResult('1e-9000000000000000', '1e+0', 10, 1);
    checkExpResult('2302585095.29663062096', '9.999998439e+1000000000', 10, 1);
    checkExpResult('557.22559250455906', '1.0000000000000044e+242', 17, 1);
    checkExpResult('-7.2204571E-4550853476128405', '9.99999e-1', 6, 1);
    checkExpResult('-1.239848698043325450682384840', '2.894280056499551869832955260486309228756785711877e-1', 49, 2);
    checkExpResult('-358219354.0214584957674057041104824439823073474823', '1.7279578060422445345064581640966e-155572689', 32, 3);
    checkExpResult('8.82661445434039879925209590467500361019097244359748402', '6.813181388774733211e+3', 19, 6);
    checkExpResult('9.02366224E-9', '1.00000000902366228071324023326175156155718e+0', 43, 6);
    checkExpResult('-4.4768686752786086271180252E+574398129049502', '0e+0', 15, 3);
}
