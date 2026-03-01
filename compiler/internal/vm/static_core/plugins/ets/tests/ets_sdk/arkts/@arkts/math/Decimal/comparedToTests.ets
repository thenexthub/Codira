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

const N = NaN;
const I = Infinity;

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal comparedTo API tests");
    suite.addTest("Decimal comparedTo with specific config", testComparedToWithSpecificConfig001);
    suite.addTest("Decimal comparedTo with specific config", testComparedToWithSpecificConfig002);
    suite.addTest("Decimal comparedTo with specific config", testComparedToWithSpecificConfig003);
    suite.addTest("Decimal comparedTo with specific config", testComparedToWithSpecificConfig004);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString001);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString002);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString003);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString004);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString005);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString006);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString007);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString008);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString009);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString010);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString011);
    suite.addTest("Decimal comparedTo with constructor form string", testComparedToWithStringAndString012);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal001);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal002);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal003);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal004);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal005);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal006);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal007);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal008);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal009);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal010);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal011);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal012);
    suite.addTest("Decimal comparedTo with decimal", testComparedToWithDecimal013);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber001);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber002);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber003);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber004);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber005);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber006);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber007);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber008);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber009);
    suite.addTest("Decimal comparedTo with exponential number", testComparedToWithExponentialNumber010);
    return suite.run();
}

function testComparedToWithSpecificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(1).comparedTo(0), 1);
    arktest.assertEQ(new Decimal(1).comparedTo(-0), 1);
    arktest.assertEQ(new Decimal(-1).comparedTo(0), -1);
    arktest.assertEQ(new Decimal(-1).comparedTo(-0), -1);
    arktest.assertEQ(new Decimal(0).comparedTo(1), -1);
    arktest.assertEQ(new Decimal(0).comparedTo(-1), 1);
    arktest.assertEQ(new Decimal(-0).comparedTo(1), -1);
    arktest.assertEQ(new Decimal(-0).comparedTo(-1), 1);
    arktest.assertEQ(new Decimal(0).comparedTo(0), 0);
    arktest.assertEQ(new Decimal(0).comparedTo(-0), 0);
    arktest.assertEQ(new Decimal(-0).comparedTo(0), 0);
    arktest.assertEQ(new Decimal(-0).comparedTo(-0), 0);
    arktest.assertEQ(new Decimal(0.04).comparedTo(0.079393068), -1);
    arktest.assertEQ(new Decimal(0.023).comparedTo(0.04840192819), -1);
    arktest.assertEQ(new Decimal(0.021879).comparedTo(0.02), 1);
    arktest.assertEQ(new Decimal(I).comparedTo(1), 1);
    arktest.assertEQ(new Decimal(I).comparedTo(-1), 1);
    arktest.assertEQ(new Decimal(-I).comparedTo(1), -1);
    arktest.assertEQ(new Decimal(-I).comparedTo(-1), -1);
    arktest.assertEQ(new Decimal(I).comparedTo(0), 1);
    arktest.assertEQ(new Decimal(I).comparedTo(-0), 1);
    arktest.assertEQ(new Decimal(-I).comparedTo(0), -1);
    arktest.assertEQ(new Decimal(-I).comparedTo(-0), -1);
}

function testComparedToWithSpecificConfig002() {
    arktest.assertEQ(new Decimal(I).comparedTo('354.345341'), 1);
    arktest.assertEQ(new Decimal(I).comparedTo('-999e999'), 1);
    arktest.assertEQ(new Decimal(I).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(-I).comparedTo(I), -1);
    arktest.assertEQ(new Decimal(-I).comparedTo(-I), 0);
    arktest.assertEQ(new Decimal(I).comparedTo(123), 1);
    arktest.assertEQ(new Decimal(3).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(1).comparedTo(I), -1);
    arktest.assertEQ(new Decimal(1).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(-1).comparedTo(I), -1);
    arktest.assertEQ(new Decimal(-1).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(0).comparedTo(I), -1);
    arktest.assertEQ(new Decimal(0).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(-0).comparedTo(I), -1);
    arktest.assertEQ(new Decimal(-0).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(1.21123e43).comparedTo(-I), 1);
    arktest.assertEQ(new Decimal('-999.0').comparedTo(I), -1);
    arktest.assertEQ(new Decimal('657.342e-45').comparedTo(-I), 1);
    arktest.assertEQ(new Decimal(-0).comparedTo(I), -1);
}

function testComparedToWithSpecificConfig003() {
    arktest.assertEQ(new Decimal(0).comparedTo('0.1'), -1);
    arktest.assertEQ(new Decimal(0).comparedTo('-0.1'), 1);
    arktest.assertEQ(new Decimal(-0).comparedTo('0.1'), -1);
    arktest.assertEQ(new Decimal(-0).comparedTo('-0.1'), 1);
    arktest.assertEQ(new Decimal('0.1').comparedTo(0), 1);
    arktest.assertEQ(new Decimal('0.1').comparedTo(-0), 1);
    arktest.assertEQ(new Decimal('-0.1').comparedTo(0), -1);
    arktest.assertEQ(new Decimal('-0.1').comparedTo(-0), -1);
    arktest.assertEQ(new Decimal(0).comparedTo('0.000000009'), -1);
    arktest.assertEQ(new Decimal(0).comparedTo('-0.000000009'), 1);
    arktest.assertEQ(new Decimal(-0).comparedTo('0.000000009'), -1);
    arktest.assertEQ(new Decimal(-0).comparedTo('-0.000000009'), 1);
    arktest.assertEQ(new Decimal('0.000000009').comparedTo(0), 1);
    arktest.assertEQ(new Decimal('0.000000009').comparedTo(-0), 1);
    arktest.assertEQ(new Decimal('-0.000000009').comparedTo(0), -1);
    arktest.assertEQ(new Decimal('-0.000000009').comparedTo(-0), -1);
    arktest.assertEQ(new Decimal(0).comparedTo('5.5'), -1);
    arktest.assertEQ(new Decimal(0).comparedTo('-5.5'), 1);
    arktest.assertEQ(new Decimal(-0).comparedTo('5.5'), -1);
    arktest.assertEQ(new Decimal(-0).comparedTo('-5.5'), 1);
    arktest.assertEQ(new Decimal('5.5').comparedTo(0), 1);
    arktest.assertEQ(new Decimal('5.5').comparedTo(-0), 1);
    arktest.assertEQ(new Decimal('-5.5').comparedTo(0), -1);
    arktest.assertEQ(new Decimal('-5.5').comparedTo(-0), -1);
}

function testComparedToWithSpecificConfig004() {
    arktest.assertEQ(new Decimal(1).comparedTo('0'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('1'), 0);
    arktest.assertEQ(new Decimal(1).comparedTo('-45'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('22'), -1);
    arktest.assertEQ(new Decimal(1).comparedTo(0o144), -1);
    arktest.assertEQ(new Decimal(1).comparedTo('0144'), -1);
    arktest.assertEQ(new Decimal(1).comparedTo('6.1915'), -1);
    arktest.assertEQ(new Decimal(1).comparedTo('-1.02'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('0.09'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('-0.0001'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('8e5'), -1);
    arktest.assertEQ(new Decimal(1).comparedTo('9E12'), -1);
    arktest.assertEQ(new Decimal(1).comparedTo('1e-14'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('3.345E-9'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('-345.43e+4'), 1);
    arktest.assertEQ(new Decimal(1).comparedTo('-94.12E+0'), 1);
    arktest.assertEQ(new Decimal(3).comparedTo(-0), 1);
    arktest.assertEQ(new Decimal(9.654).comparedTo(0), 1);
    arktest.assertEQ(new Decimal(0).comparedTo('111.1111111110000'), -1);
    arktest.assertEQ(new Decimal(-1).comparedTo(1), -1);
    arktest.assertEQ(new Decimal(-0.01).comparedTo(0.01), -1);
    arktest.assertEQ(new Decimal(54).comparedTo(-54), 1);
    arktest.assertEQ(new Decimal(9.99).comparedTo('-9.99'), 1);
    arktest.assertEQ(new Decimal(100).comparedTo(100), 0);
    arktest.assertEQ(new Decimal(-999.99).comparedTo('0.01'), -1);
    arktest.assertEQ(new Decimal('03.333').comparedTo(-4), 1);
    arktest.assertEQ(new Decimal(-1).comparedTo(-0.1), -1);
    arktest.assertEQ(new Decimal(43534.5435).comparedTo('0.054645'), 1);
}

function testComparedToWithStringAndString001() {
    arktest.assertEQ(new Decimal('0').comparedTo('0'), 0);
    arktest.assertEQ(new Decimal('0.0000023432495704937').comparedTo('-0.0000023432495704937'), 1);
    arktest.assertEQ(new Decimal('99999').comparedTo('1'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-7502'), 1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-0.000000000000000000295176'), -1);
    arktest.assertEQ(new Decimal('2.9').comparedTo('3.23360'), -1);
    arktest.assertEQ(new Decimal('1.14').comparedTo('4289'), -1);
    arktest.assertEQ(new Decimal('-3').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('0.00000000000000551').comparedTo('-3958.43'), 1);
    arktest.assertEQ(new Decimal('-13').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-23.382').comparedTo('-16.1'), -1);
    arktest.assertEQ(new Decimal('-176.85').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('1.38497').comparedTo('-0.0000000561'), 1);
    arktest.assertEQ(new Decimal('3.90').comparedTo('18.124'), -1);
    arktest.assertEQ(new Decimal('-239.52').comparedTo('-0.00000000000066862'), -1);
    arktest.assertEQ(new Decimal('5.7').comparedTo('2248.28'), -1);
    arktest.assertEQ(new Decimal('-0.00000000012').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('29').comparedTo('-5'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-54'), 1);
    arktest.assertEQ(new Decimal('3').comparedTo('-1.1'), 1);
    arktest.assertEQ(new Decimal('-461.9').comparedTo('-13803'), 1);
}

function testComparedToWithStringAndString002() {
    arktest.assertEQ(new Decimal('-1').comparedTo('0.00054428'), -1);
    arktest.assertEQ(new Decimal('-0.000000000000000683').comparedTo('7'), -1);
    arktest.assertEQ(new Decimal('2').comparedTo('1'), 1);
    arktest.assertEQ(new Decimal('118.608').comparedTo('-4.8'), 1);
    arktest.assertEQ(new Decimal('3').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('83.5').comparedTo('44.9'), 1);
    arktest.assertEQ(new Decimal('-3').comparedTo('-2.86'), -1);
    arktest.assertEQ(new Decimal('1138.06').comparedTo('47.08'), 1);
    arktest.assertEQ(new Decimal('-2.3').comparedTo('3'), -1);
    arktest.assertEQ(new Decimal('-848').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('5').comparedTo('2'), 1);
    arktest.assertEQ(new Decimal('11.3').comparedTo('-176.3'), 1);
    arktest.assertEQ(new Decimal('-302142').comparedTo('3.8'), -1);
    arktest.assertEQ(new Decimal('-9').comparedTo('-38'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('6161').comparedTo('-3'), 1);
    arktest.assertEQ(new Decimal('8').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-1334').comparedTo('-14'), -1);
    arktest.assertEQ(new Decimal('3.593').comparedTo('0.000000000000059975'), 1);
    arktest.assertEQ(new Decimal('0.0000000000000000847').comparedTo('-1186.11'), 1);
    arktest.assertEQ(new Decimal('4.62').comparedTo('14.47'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-319'), 1);
    arktest.assertEQ(new Decimal('23168').comparedTo('8.3485'), 1);
}

function testComparedToWithStringAndString003() {
    arktest.assertEQ(new Decimal('-16').comparedTo('-1.6'), -1);
    arktest.assertEQ(new Decimal('64.3').comparedTo('2108.4'), -1);
    arktest.assertEQ(new Decimal('4').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('-0.50346').comparedTo('-319.6'), 1);
    arktest.assertEQ(new Decimal('10910.0').comparedTo('-0.00000000000000000314208'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000552').comparedTo('0.00000000015'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('16'), -1);
    arktest.assertEQ(new Decimal('38.4').comparedTo('35.83'), 1);
    arktest.assertEQ(new Decimal('-1.1').comparedTo('-0.000002850'), -1);
    arktest.assertEQ(new Decimal('0.000000000000011').comparedTo('1.2'), -1);
    arktest.assertEQ(new Decimal('-50.66').comparedTo('2.2'), -1);
    arktest.assertEQ(new Decimal('-1187').comparedTo('5'), -1);
    arktest.assertEQ(new Decimal('174').comparedTo('-1.8175'), 1);
    arktest.assertEQ(new Decimal('651.1').comparedTo('-11.332'), 1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-0.000000000000000011'), -1);
    arktest.assertEQ(new Decimal('7156').comparedTo('4.84'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('3'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('77487'), -1);
    arktest.assertEQ(new Decimal('51.7181').comparedTo('17'), 1);
    arktest.assertEQ(new Decimal('13.132').comparedTo('0.000000000000029446'), 1);
    arktest.assertEQ(new Decimal('5.4').comparedTo('0.000000000000000006325'), 1);
}

function testComparedToWithStringAndString004() {
    arktest.assertEQ(new Decimal('4.1').comparedTo('-2.42'), 1);
    arktest.assertEQ(new Decimal('-5.2').comparedTo('-44.1'), 1);
    arktest.assertEQ(new Decimal('-6').comparedTo('15.018'), -1);
    arktest.assertEQ(new Decimal('-27.949').comparedTo('-1170.9'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('0.0000000000000000120'), -1);
    arktest.assertEQ(new Decimal('-3.7').comparedTo('-52.1'), 1);
    arktest.assertEQ(new Decimal('-17.43').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-1.4').comparedTo('-11737'), 1);
    arktest.assertEQ(new Decimal('-7.17').comparedTo('615.40'), -1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-0.000000000000000389'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-11.7'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-0.0000039894'), 1);
    arktest.assertEQ(new Decimal('-14.15').comparedTo('-0.0000011139'), -1);
    arktest.assertEQ(new Decimal('-16260').comparedTo('-3.97'), -1);
    arktest.assertEQ(new Decimal('-0.00000066').comparedTo('-1.3'), 1);
    arktest.assertEQ(new Decimal('-818').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-3').comparedTo('-30020.5'), 1);
    arktest.assertEQ(new Decimal('-3.328').comparedTo('-28.16'), 1);
    arktest.assertEQ(new Decimal('-74').comparedTo('2'), -1);
    arktest.assertEQ(new Decimal('-30332').comparedTo('-1.61'), -1);
    arktest.assertEQ(new Decimal('-0.00000000000000494').comparedTo('0'), -1);
}

function testComparedToWithStringAndString005() {
    arktest.assertEQ(new Decimal('-2.1').comparedTo('11.6037'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000013762').comparedTo('-5273'), 1);
    arktest.assertEQ(new Decimal('-0.00000000000000000002353').comparedTo('1188'), -1);
    arktest.assertEQ(new Decimal('-0.00000000000000033619').comparedTo('-4'), 1);
    arktest.assertEQ(new Decimal('3.2').comparedTo('-597.81'), 1);
    arktest.assertEQ(new Decimal('14265').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-18.8703').comparedTo('1'), -1);
    arktest.assertEQ(new Decimal('-4.5').comparedTo('-146.5'), 1);
    arktest.assertEQ(new Decimal('-259.7').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-72.57').comparedTo('4.3'), -1);
    arktest.assertEQ(new Decimal('0.0000015').comparedTo('9.06'), -1);
    arktest.assertEQ(new Decimal('490815').comparedTo('0.0000000000000000000385'), 1);
    arktest.assertEQ(new Decimal('0.614').comparedTo('-0.000000000000000001154'), 1);
    arktest.assertEQ(new Decimal('1.2').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-1.8').comparedTo('-436269'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-7'), 1);
    arktest.assertEQ(new Decimal('-3.18290').comparedTo('-453.85'), 1);
    arktest.assertEQ(new Decimal('5.645').comparedTo('1'), 1);
    arktest.assertEQ(new Decimal('8.426').comparedTo('1.0'), 1);
    arktest.assertEQ(new Decimal('-6702').comparedTo('-0.000003053'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-0.0000000472'), 1);
    arktest.assertEQ(new Decimal('-13.789').comparedTo('0.12167'), -1);
}

function testComparedToWithStringAndString006() {
    arktest.assertEQ(new Decimal('0').comparedTo('-22472'), 1);
    arktest.assertEQ(new Decimal('769.0').comparedTo('0.00000019970'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-145.1'), 1);
    arktest.assertEQ(new Decimal('-307').comparedTo('-1'), -1);
    arktest.assertEQ(new Decimal('1').comparedTo('-0.000015'), 1);
    arktest.assertEQ(new Decimal('3').comparedTo('-4'), 1);
    arktest.assertEQ(new Decimal('7').comparedTo('3.791'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000021').comparedTo('0.00000414'), -1);
    arktest.assertEQ(new Decimal('-2').comparedTo('0.0486'), -1);
    arktest.assertEQ(new Decimal('-4').comparedTo('-0.00268'), -1);
    arktest.assertEQ(new Decimal('0.0000000000791').comparedTo('-4.97'), 1);
    arktest.assertEQ(new Decimal('-13795').comparedTo('3'), -1);
    arktest.assertEQ(new Decimal('-13.197').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-4.7'), 1);
    arktest.assertEQ(new Decimal('3.7').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('10.3925').comparedTo('7.312'), 1);
    arktest.assertEQ(new Decimal('2.263').comparedTo('7.388'), -1);
    arktest.assertEQ(new Decimal('-6.0').comparedTo('-2'), -1);
    arktest.assertEQ(new Decimal('1').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('1.987').comparedTo('32454'), -1);
    arktest.assertEQ(new Decimal('-0.361').comparedTo('-200.6'), 1);
    arktest.assertEQ(new Decimal('5.0').comparedTo('-7'), 1);
    arktest.assertEQ(new Decimal('-6.2').comparedTo('-0.0056'), -1);
    arktest.assertEQ(new Decimal('-3.8').comparedTo('1'), -1);
    arktest.assertEQ(new Decimal('-9.3').comparedTo('-7.64'), -1);
}

function testComparedToWithStringAndString007() {
    arktest.assertEQ(new Decimal('38364').comparedTo('-127.311'), 1);
    arktest.assertEQ(new Decimal('-1.6').comparedTo('-3.06'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('-2'), 1);
    arktest.assertEQ(new Decimal('3.952').comparedTo('5'), -1);
    arktest.assertEQ(new Decimal('566').comparedTo('125.359'), 1);
    arktest.assertEQ(new Decimal('1078.97').comparedTo('496'), 1);
    arktest.assertEQ(new Decimal('0.000000000097586').comparedTo('-0.000000000000000000027457'), 1);
    arktest.assertEQ(new Decimal('-37.5').comparedTo('-28572'), 1);
    arktest.assertEQ(new Decimal('412').comparedTo('-0.0000000000000001572'), 1);
    arktest.assertEQ(new Decimal('0.00000024221').comparedTo('-35.8881'), 1);
    arktest.assertEQ(new Decimal('25.8').comparedTo('0.0000000000000000000242'), 1);
    arktest.assertEQ(new Decimal('0.30').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-26.440').comparedTo('-3.64'), -1);
    arktest.assertEQ(new Decimal('1.15').comparedTo('-3229'), 1);
    arktest.assertEQ(new Decimal('-137.1').comparedTo('2'), -1);
    arktest.assertEQ(new Decimal('0.00000000077').comparedTo('53558'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('2.6'), -1);
    arktest.assertEQ(new Decimal('-3').comparedTo('-6'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000000139').comparedTo('-0.0005289'), 1);
    arktest.assertEQ(new Decimal('-0.00000000000000000697').comparedTo('-1.309'), 1);
    arktest.assertEQ(new Decimal('-0.0000045').comparedTo('-8.3'), 1);
    arktest.assertEQ(new Decimal('-2').comparedTo('-1.3'), -1);
    arktest.assertEQ(new Decimal('-1.42209').comparedTo('-1.74'), 1);
    arktest.assertEQ(new Decimal('-608.88').comparedTo('25.3'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-1'), 1);
}

function testComparedToWithStringAndString008() {
    arktest.assertEQ(new Decimal('5.6').comparedTo('-4'), 1);
    arktest.assertEQ(new Decimal('7.17').comparedTo('-4'), 1);
    arktest.assertEQ(new Decimal('56.6').comparedTo('8218.4'), -1);
    arktest.assertEQ(new Decimal('68.33').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('5.0').comparedTo('1'), 1);
    arktest.assertEQ(new Decimal('32.734').comparedTo('-9'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('20837.0'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000000013950').comparedTo('-3364.72'), 1);
    arktest.assertEQ(new Decimal('3.66').comparedTo('-0.0000000000000000536'), 1);
    arktest.assertEQ(new Decimal('-1471.14').comparedTo('6'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-0.0000013'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('-5'), 1);
    arktest.assertEQ(new Decimal('-10449.8').comparedTo('-58.424'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('0.0000000000000000017122'), -1);
    arktest.assertEQ(new Decimal('0.000000000000026585').comparedTo('4.1'), -1);
    arktest.assertEQ(new Decimal('-7').comparedTo('0.00000000000000000920'), -1);
    arktest.assertEQ(new Decimal('0.00000000489').comparedTo('0.00000000000000000036325'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('104674'), -1);
    arktest.assertEQ(new Decimal('-1.30063').comparedTo('0.000000004051'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('2'), -1);
    arktest.assertEQ(new Decimal('-3').comparedTo('0.00150'), -1);
    arktest.assertEQ(new Decimal('-407.95').comparedTo('495.290'), -1);
    arktest.assertEQ(new Decimal('24533.8').comparedTo('-2539'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-43.4').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-1399').comparedTo('16.89'), -1);
    arktest.assertEQ(new Decimal('-232480').comparedTo('0'), -1);
}

function testComparedToWithStringAndString009() {
    arktest.assertEQ(new Decimal('3').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('6.2').comparedTo('12.2'), -1);
    arktest.assertEQ(new Decimal('-23.38').comparedTo('0.0853'), -1);
    arktest.assertEQ(new Decimal('-18.2').comparedTo('1'), -1);
    arktest.assertEQ(new Decimal('0.002118').comparedTo('-16.9821'), 1);
    arktest.assertEQ(new Decimal('8819.2').comparedTo('28.744'), 1);
    arktest.assertEQ(new Decimal('-2').comparedTo('-5'), 1);
    arktest.assertEQ(new Decimal('-2.10').comparedTo('9.2620'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-5'), 1);
    arktest.assertEQ(new Decimal('23963.9').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-0.00000045').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('2').comparedTo('2'), 0);
    arktest.assertEQ(new Decimal('-3.444').comparedTo('3'), -1);
    arktest.assertEQ(new Decimal('0.0000016140').comparedTo('1.5132'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-7'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('-869.7'), 1);
    arktest.assertEQ(new Decimal('0.000000000000016').comparedTo('-1609.8'), 1);
    arktest.assertEQ(new Decimal('651.3').comparedTo('2.923'), 1);
    arktest.assertEQ(new Decimal('270.901').comparedTo('5.2'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-2.6'), 1);
    arktest.assertEQ(new Decimal('0.000000000020617').comparedTo('0.0000000096018'), -1);
    arktest.assertEQ(new Decimal('4.32').comparedTo('-7'), 1);
    arktest.assertEQ(new Decimal('-1.5').comparedTo('-23.7'), 1);
    arktest.assertEQ(new Decimal('1.2497').comparedTo('-58.42'), 1);
}

function testComparedToWithStringAndString010() {
    arktest.assertEQ(new Decimal('-6.4').comparedTo('2.24'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('1002.3'), -1);
    arktest.assertEQ(new Decimal('265.64').comparedTo('26.7'), 1);
    arktest.assertEQ(new Decimal('387.4').comparedTo('757.85'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('6.525'), -1);
    arktest.assertEQ(new Decimal('1120.24').comparedTo('-0.000000000000000000232098'), 1);
    arktest.assertEQ(new Decimal('67.22').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('67').comparedTo('-3.1'), 1);
    arktest.assertEQ(new Decimal('-1.01').comparedTo('0.00059917'), -1);
    arktest.assertEQ(new Decimal('-0.000000000000091884').comparedTo('2.7'), -1);
    arktest.assertEQ(new Decimal('171').comparedTo('4'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-367347'), 1);
    arktest.assertEQ(new Decimal('95.1').comparedTo('-0.000000114267'), 1);
    arktest.assertEQ(new Decimal('62.4').comparedTo('-5'), 1);
    arktest.assertEQ(new Decimal('11627.8').comparedTo('9.844'), 1);
    arktest.assertEQ(new Decimal('-1').comparedTo('2.33'), -1);
    arktest.assertEQ(new Decimal('-14').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-55').comparedTo('7'), -1);
    arktest.assertEQ(new Decimal('-22.460').comparedTo('872.15'), -1);
    arktest.assertEQ(new Decimal('-0.00000000027029').comparedTo('2.05'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000004846').comparedTo('17.73'), -1);
    arktest.assertEQ(new Decimal('3.6').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-3.233').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-3.878').comparedTo('-383'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000174').comparedTo('20.7'), -1);
    arktest.assertEQ(new Decimal('0.0000000000000000000126').comparedTo('-2.229'), 1);
    arktest.assertEQ(new Decimal('-2.307').comparedTo('14.8000'), -1);
}

function testComparedToWithStringAndString011() {
    arktest.assertEQ(new Decimal('-563.79').comparedTo('0.000000000000000069934'), -1);
    arktest.assertEQ(new Decimal('-315.71').comparedTo('-0.00832'), -1);
    arktest.assertEQ(new Decimal('7').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('188.9').comparedTo('-2.883'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('14.5720'), -1);
    arktest.assertEQ(new Decimal('2').comparedTo('5389.3'), -1);
    arktest.assertEQ(new Decimal('-2413.1').comparedTo('-28'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-98.587'), 1);
    arktest.assertEQ(new Decimal('3').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('17930.7').comparedTo('1880.31'), 1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-0.00000000000000000272'), -1);
    arktest.assertEQ(new Decimal('0.00000000000000000002288').comparedTo('51.0841'), -1);
    arktest.assertEQ(new Decimal('-1.004').comparedTo('-14799'), 1);
    arktest.assertEQ(new Decimal('-0.0024973').comparedTo('-6'), 1);
    arktest.assertEQ(new Decimal('-0.00000000000054').comparedTo('6'), -1);
    arktest.assertEQ(new Decimal('0.0009288').comparedTo('-5.2'), 1);
    arktest.assertEQ(new Decimal('-45.4').comparedTo('-0.000042102'), -1);
    arktest.assertEQ(new Decimal('-56.1').comparedTo('1793.86'), -1);
    arktest.assertEQ(new Decimal('-68.425').comparedTo('0.0000000000000068415'), -1);
    arktest.assertEQ(new Decimal('-2.20').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('136.51').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('-5.9773').comparedTo('45'), -1);
}

function testComparedToWithStringAndString012() {
    arktest.assertEQ(new Decimal('-6996').comparedTo('60.60'), -1);
    arktest.assertEQ(new Decimal('0.00000000773').comparedTo('-2.6'), 1);
    arktest.assertEQ(new Decimal('-3827').comparedTo('-42.150'), -1);
    arktest.assertEQ(new Decimal('-4').comparedTo('3277'), -1);
    arktest.assertEQ(new Decimal('0.0000000000000201').comparedTo('-0.00000000000000094'), 1);
    arktest.assertEQ(new Decimal('7497.2').comparedTo('-45738'), 1);
    arktest.assertEQ(new Decimal('5.20').comparedTo('2'), 1);
    arktest.assertEQ(new Decimal('139673').comparedTo('0.000013'), 1);
    arktest.assertEQ(new Decimal('258285').comparedTo('0.00000000000338'), 1);
    arktest.assertEQ(new Decimal('20.6').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('5').comparedTo('45.3'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('0.00000000000000000010937'), -1);
    arktest.assertEQ(new Decimal('2').comparedTo('-392.465'), 1);
}

function testComparedToWithDecimal001() {
    arktest.assertEQ(new Decimal('-0.00000000000004475220262613').comparedTo('390046.9220288808'), -1);
    arktest.assertEQ(new Decimal('-52').comparedTo('-7'), -1);
    arktest.assertEQ(new Decimal('-7137400653786386').comparedTo('-1220454'), -1);
    arktest.assertEQ(new Decimal('19.76074721670306').comparedTo('39433637.5'), -1);
    arktest.assertEQ(new Decimal('495810.901671347').comparedTo('-201199464.42718987'), 1);
    arktest.assertEQ(new Decimal('2180987378927.828').comparedTo('28150984.022786039'), 1);
    arktest.assertEQ(new Decimal('364.263').comparedTo('1334'), -1);
    arktest.assertEQ(new Decimal('-4.541827796913').comparedTo('0.000000000720283699415669'), -1);
    arktest.assertEQ(new Decimal('0.000000000000533031300116917190').comparedTo('-733584.206'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000019761199').comparedTo('-6724.9979821490'), 1);
    arktest.assertEQ(new Decimal('0.00000000012335234299864').comparedTo('56.021423640836509'), -1);
    arktest.assertEQ(new Decimal('17752.992435030').comparedTo('513735.52129558557'), -1);
    arktest.assertEQ(new Decimal('-341665592090.67519').comparedTo('895755.23903'), -1);
    arktest.assertEQ(new Decimal('-0.01174948256069749').comparedTo('38841.427732372'), -1);
    arktest.assertEQ(new Decimal('2067269955.10633').comparedTo('-215483633'), 1);
    arktest.assertEQ(new Decimal('58373').comparedTo('-0.000000000000000208432238083784077'), 1);
    arktest.assertEQ(new Decimal('2821.0921360930698').comparedTo('-0.0019370832810573843'), 1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-6987341307332550'), 1);
    arktest.assertEQ(new Decimal('-0.000000000000000135940520450').comparedTo('-7'), 1);
    arktest.assertEQ(new Decimal('368879186519841.03').comparedTo('3.86407044811851'), 1);
    arktest.assertEQ(new Decimal('54.8552188726690').comparedTo('-17618.8325331'), 1);
    arktest.assertEQ(new Decimal('0.00000000719285162').comparedTo('3.0'), -1);
    arktest.assertEQ(new Decimal('-24541949382.5').comparedTo('12000388993272705.7'), -1);
    arktest.assertEQ(new Decimal('-0.00000000000000016019271145').comparedTo('-5010014644.83398'), 1);
    arktest.assertEQ(new Decimal('0.00000342615147619').comparedTo('38.8937553143'), -1);
}

function testComparedToWithDecimal002() {
    arktest.assertEQ(new Decimal('0.000000000000030').comparedTo('698637'), -1);
    arktest.assertEQ(new Decimal('-940562183996').comparedTo('-9538.76857523965'), -1);
    arktest.assertEQ(new Decimal('0.7992283723').comparedTo('-301.642'), 1);
    arktest.assertEQ(new Decimal('-943394092028892').comparedTo('0.00000000000249028944'), -1);
    arktest.assertEQ(new Decimal('0.0000000065').comparedTo('19.8'), -1);
    arktest.assertEQ(new Decimal('-0.00000024892').comparedTo('1.081095088'), -1);
    arktest.assertEQ(new Decimal('4693767.27').comparedTo('0'), 1);
    arktest.assertEQ(new Decimal('241.51552').comparedTo('-0.000000000029998424146'), 1);
    arktest.assertEQ(new Decimal('4.70').comparedTo('-3889836.808618'), 1);
    arktest.assertEQ(new Decimal('-31').comparedTo('22.1677904'), -1);
    arktest.assertEQ(new Decimal('-8').comparedTo('0.0000000000000018564862859'), -1);
    arktest.assertEQ(new Decimal('-2.82').comparedTo('-0.1308117625'), -1);
    arktest.assertEQ(new Decimal('-0.000000000000000490813322').comparedTo('9628302045.4'), -1);
    arktest.assertEQ(new Decimal('14').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('-21665490839.0').comparedTo('-308.104922674'), -1);
    arktest.assertEQ(new Decimal('-5').comparedTo('-6.6'), 1);
    arktest.assertEQ(new Decimal('20.356245251').comparedTo('-2629.4181'), 1);
    arktest.assertEQ(new Decimal('-1.39').comparedTo('24102492360.03791'), -1);
    arktest.assertEQ(new Decimal('26.56').comparedTo('-81.84491435616642'), 1);
    arktest.assertEQ(new Decimal('12733804882').comparedTo('3.16734'), 1);
    arktest.assertEQ(new Decimal('0.0000000000001331522614064615').comparedTo('-62.298429591'), 1);
    arktest.assertEQ(new Decimal('4.5417').comparedTo('-0.0000000000000000000906964635852953'), 1);
    arktest.assertEQ(new Decimal('5.0').comparedTo('3.99206'), 1);
    arktest.assertEQ(new Decimal('28282932454425').comparedTo('20112043684.32'), 1);
    arktest.assertEQ(new Decimal('-119.144720881').comparedTo('-129.74436439545'), 1);
    arktest.assertEQ(new Decimal('740279.3975').comparedTo('18685368847070828'), -1);
}

function testComparedToWithDecimal003() {
    arktest.assertEQ(new Decimal('-194303.7').comparedTo('0.000000000000006199756267389375'), -1);
    arktest.assertEQ(new Decimal('0.000000000000000002405090').comparedTo('-0.00000000979604845'), 1);
    arktest.assertEQ(new Decimal('-0.0001603124275271').comparedTo('0.000000006060544102736781'), -1);
    arktest.assertEQ(new Decimal('-179.52').comparedTo('-76.6'), -1);
    arktest.assertEQ(new Decimal('9736.4996007851800').comparedTo('99.6'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000000000126060214789203821').comparedTo('6748323899.81'), -1);
    arktest.assertEQ(new Decimal('-0.0000076569074016550').comparedTo('0.000000000525779058'), -1);
    arktest.assertEQ(new Decimal('0.0000000000000000047090003506832525').comparedTo('0.00000000000000011759'), -1);
    arktest.assertEQ(new Decimal('-286334697213252').comparedTo('8557782748357'), -1);
    arktest.assertEQ(new Decimal('-0.000000025274118612780187').comparedTo('-127000540'), 1);
    arktest.assertEQ(new Decimal('0.00000000610055019436568').comparedTo('-87.3'), 1);
    arktest.assertEQ(new Decimal('-1560822.78').comparedTo('360857.435'), -1);
    arktest.assertEQ(new Decimal('243209397988565176').comparedTo('-2171.0'), 1);
    arktest.assertEQ(new Decimal('1089364.8').comparedTo('-7'), 1);
    arktest.assertEQ(new Decimal('5').comparedTo('-195.43311519499808'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-12.6'), 1);
    arktest.assertEQ(new Decimal('-8917436.7369').comparedTo('-2412090'), -1);
    arktest.assertEQ(new Decimal('0.0000000000258').comparedTo('-0.000000000000000001137187844109'), 1);
    arktest.assertEQ(new Decimal('65611').comparedTo('-0.00000000000527362056737'), 1);
    arktest.assertEQ(new Decimal('0.0000000000000000000113412508151534071').comparedTo('-4.9964'), 1);
    arktest.assertEQ(new Decimal('20156417.0').comparedTo('0.0000000000000253800552009'), 1);
    arktest.assertEQ(new Decimal('32.967316').comparedTo('-176195213.7000883'), 1);
    arktest.assertEQ(new Decimal('-542212737079077800').comparedTo('-593.88569405080'), -1);
    arktest.assertEQ(new Decimal('-0.000000000000000004199278271388794').comparedTo('57186245414.80793'), -1);
    arktest.assertEQ(new Decimal('0.00000004507194567013278').comparedTo('1.0'), -1);
    arktest.assertEQ(new Decimal('43551631176247').comparedTo('-2195'), 1);
    arktest.assertEQ(new Decimal('-4.6').comparedTo('0.00008016'), -1);
    arktest.assertEQ(new Decimal('-0.000013384535').comparedTo('3437784.58'), -1);
    arktest.assertEQ(new Decimal('0.00000000000000002007911523').comparedTo('-1.34'), 1);
}

function testComparedToWithDecimal004() {
    arktest.assertEQ(new Decimal('2.5').comparedTo('-39.06'), 1);
    arktest.assertEQ(new Decimal('0.0000000000000000051').comparedTo('2'), -1);
    arktest.assertEQ(new Decimal('0.00012813936789257785').comparedTo('1021.0303931'), -1);
    arktest.assertEQ(new Decimal('0.00000000000000000132229').comparedTo('47.1'), -1);
    arktest.assertEQ(new Decimal('-0.00002120044').comparedTo('0.0000000000000000064902789323'), -1);
    arktest.assertEQ(new Decimal('9').comparedTo('379472788024622.729'), -1);
    arktest.assertEQ(new Decimal('20.510820935587124').comparedTo('0.0000000000028179781'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000000000336381763977').comparedTo('1.17539'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-335.0'), 1);
    arktest.assertEQ(new Decimal('2.4').comparedTo('-0.00000000000023'), 1);
    arktest.assertEQ(new Decimal('379.690496326').comparedTo('-0.000000000000000003740124432080006'), 1);
    arktest.assertEQ(new Decimal('9612.15057327').comparedTo('7'), 1);
    arktest.assertEQ(new Decimal('64.55819568').comparedTo('103'), -1);
    arktest.assertEQ(new Decimal('306554.22').comparedTo('-21168.82471'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('0.00000000000000378671778'), -1);
    arktest.assertEQ(new Decimal('-4869994212.02').comparedTo('-82.3249644'), -1);
    arktest.assertEQ(new Decimal('-28281314').comparedTo('0.00023'), -1);
    arktest.assertEQ(new Decimal('-272266413010184382').comparedTo('-21443895.1'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('256.5'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000000059504').comparedTo('-393'), 1);
    arktest.assertEQ(new Decimal('-85483356.741520').comparedTo('-0.135106007572'), -1);
    arktest.assertEQ(new Decimal('1256398743581').comparedTo('-0.00000000000000113'), 1);
    arktest.assertEQ(new Decimal('9265.8054085790753').comparedTo('1101.60'), 1);
    arktest.assertEQ(new Decimal('21755297.7418484298').comparedTo('-1961304'), 1);
    arktest.assertEQ(new Decimal('-4.24').comparedTo('-20364.5241763'), 1);
    arktest.assertEQ(new Decimal('-0.00000163743621').comparedTo('-15716423.93'), 1);
    arktest.assertEQ(new Decimal('0.0000000020933557160').comparedTo('-8448055.64162653'), 1);
    arktest.assertEQ(new Decimal('193').comparedTo('1919.381696195'), -1);
    arktest.assertEQ(new Decimal('-39007.0872890').comparedTo('58659197064.3566'), -1);
    arktest.assertEQ(new Decimal('1.224').comparedTo('-82293358.63'), 1);
}

function testComparedToWithDecimal005() {
    arktest.assertEQ(new Decimal('0.0000000000000001573421585953').comparedTo('0.00000000159648'), -1);
    arktest.assertEQ(new Decimal('0').comparedTo('-91.479627934421'), 1);
    arktest.assertEQ(new Decimal('-186.247902631').comparedTo('-0.00000000000000008879'), -1);
    arktest.assertEQ(new Decimal('34342532998345').comparedTo('0.000000000000000001681734691290559'), 1);
    arktest.assertEQ(new Decimal('-4020.8878086173').comparedTo('1.08'), -1);
    arktest.assertEQ(new Decimal('-0.1630447778870097').comparedTo('8176.334'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000000582491728505043').comparedTo('0.00000000000000808314'), -1);
    arktest.assertEQ(new Decimal('-0.00000031').comparedTo('3137213.71057'), -1);
    arktest.assertEQ(new Decimal('-28.21073575152').comparedTo('8530334210.39291'), -1);
    arktest.assertEQ(new Decimal('-0.00000041292').comparedTo('-66921.7723433'), 1);
    arktest.assertEQ(new Decimal('-261247.289569419').comparedTo('-0.0000022547224'), -1);
    arktest.assertEQ(new Decimal('-4.07').comparedTo('-6.1'), 1);
    arktest.assertEQ(new Decimal('688.144527235').comparedTo('45986414948'), -1);
    arktest.assertEQ(new Decimal('-1').comparedTo('-0.0231932902995'), -1);
    arktest.assertEQ(new Decimal('0.0000000000000016241364087636498').comparedTo('3'), -1);
    arktest.assertEQ(new Decimal('64634.877977').comparedTo('-39803483'), 1);
    arktest.assertEQ(new Decimal('93.85').comparedTo('-27.012'), 1);
    arktest.assertEQ(new Decimal('-1804.8479').comparedTo('-2720034'), 1);
    arktest.assertEQ(new Decimal('-31207.70845').comparedTo('-0.05392720639355'), -1);
    arktest.assertEQ(new Decimal('-7479301046098815').comparedTo('37.3'), -1);
    arktest.assertEQ(new Decimal('50515.653686861597').comparedTo('-0.00000000000000204399360'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000000055012280316826870').comparedTo('149.91618262'), -1);
    arktest.assertEQ(new Decimal('2216863.9954067').comparedTo('0.000000000000000775121457372458'), 1);
    arktest.assertEQ(new Decimal('-3.1').comparedTo('2'), -1);
    arktest.assertEQ(new Decimal('-4.67').comparedTo('0.000000018732162086'), -1);
    arktest.assertEQ(new Decimal('-0.00002189591').comparedTo('-0.000000000255830068339'), -1);
    arktest.assertEQ(new Decimal('410611332258005.0').comparedTo('-2840.2819'), 1);
    arktest.assertEQ(new Decimal('-7397921366979').comparedTo('-0.00000000000053'), -1);
    arktest.assertEQ(new Decimal('-0.12413104038561').comparedTo('-0.00000000000255343979'), -1);
    arktest.assertEQ(new Decimal('10521370.1').comparedTo('-55871447.8'), 1);
}

function testComparedToWithDecimal006() {
    arktest.assertEQ(new Decimal('129500.264553199').comparedTo('-3811409369958.95'), 1);
    arktest.assertEQ(new Decimal('0.00000240712277094094').comparedTo('1354108422.5'), -1);
    arktest.assertEQ(new Decimal('-0.000000000000000005719028368733990').comparedTo('540474.19019760'), -1);
    arktest.assertEQ(new Decimal('977162.7422').comparedTo('-0.000000000001402550'), 1);
    arktest.assertEQ(new Decimal('1.54339295432').comparedTo('-0.000001596789'), 1);
    arktest.assertEQ(new Decimal('53798507.25').comparedTo('-0.00129745384800'), 1);
    arktest.assertEQ(new Decimal('-0.00418268966509248').comparedTo('0.0000324984627'), -1);
    arktest.assertEQ(new Decimal('118092.9308827').comparedTo('-0.0000000000091'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('-93.6'), 1);
    arktest.assertEQ(new Decimal('0').comparedTo('152.55335'), -1);
    arktest.assertEQ(new Decimal('-3405109.0892217').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('445424.679402140').comparedTo('85.3554'), 1);
    arktest.assertEQ(new Decimal('5036.98').comparedTo('22371.6657'), -1);
    arktest.assertEQ(new Decimal('-1.9').comparedTo('0.000000102345283335839'), -1);
    arktest.assertEQ(new Decimal('-42.2659').comparedTo('3.721'), -1);
    arktest.assertEQ(new Decimal('0.0000016709189598').comparedTo('6.170585'), -1);
    arktest.assertEQ(new Decimal('-3').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-0.0000000061').comparedTo('5497334338493.62'), -1);
    arktest.assertEQ(new Decimal('-124932073873228').comparedTo('0.000000000370'), -1);
    arktest.assertEQ(new Decimal('1675530.7719').comparedTo('-1065461.6469931248'), 1);
    arktest.assertEQ(new Decimal('1053.3').comparedTo('10097922.4048112434'), -1);
    arktest.assertEQ(new Decimal('-92523142643508').comparedTo('-4'), -1);
    arktest.assertEQ(new Decimal('200.8').comparedTo('-36.675816237421759'), 1);
    arktest.assertEQ(new Decimal('2.0').comparedTo('34282969.24101'), -1);
    arktest.assertEQ(new Decimal('-0.120192312259366').comparedTo('-99423384.4155'), 1);
    arktest.assertEQ(new Decimal('0.00000000000000000746964').comparedTo('-4.50907217'), 1);
    arktest.assertEQ(new Decimal('2521.848164').comparedTo('-16771705.710'), 1);
}

function testComparedToWithDecimal007() {
    arktest.assertEQ(new Decimal('0.00002756245400').comparedTo('0.0032449553261'), -1);
    arktest.assertEQ(new Decimal('-0.00000001332527923').comparedTo('574.55219962424'), -1);
    arktest.assertEQ(new Decimal('-0.000000000000000179221403668').comparedTo('57235.797'), -1);
    arktest.assertEQ(new Decimal('0.000000057866988749115').comparedTo('0.00000000014'), 1);
    arktest.assertEQ(new Decimal('3').comparedTo('1.2'), 1);
    arktest.assertEQ(new Decimal('56.30058910190568').comparedTo('-1680170716816089'), 1);
    arktest.assertEQ(new Decimal('-2').comparedTo('-24381767.5'), 1);
    arktest.assertEQ(new Decimal('11799.81119').comparedTo('-0.0000092161582'), 1);
    arktest.assertEQ(new Decimal('1.13').comparedTo('-1'), 1);
    arktest.assertEQ(new Decimal('1').comparedTo('-802.5013810814'), 1);
    arktest.assertEQ(new Decimal('54624549209148.7').comparedTo('-0.000002846161'), 1);
    arktest.assertEQ(new Decimal('138301402').comparedTo('-0.00011'), 1);
    arktest.assertEQ(new Decimal('5').comparedTo('-2.4'), 1);
    arktest.assertEQ(new Decimal('-4604424').comparedTo('1.28478944'), -1);
    arktest.assertEQ(new Decimal('-146100841048.0').comparedTo('92.57'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000000020786339367465').comparedTo('0.0000000517784775175150122'), -1);
    arktest.assertEQ(new Decimal('94366206024220.8').comparedTo('-147.3818299'), 1);
    arktest.assertEQ(new Decimal('-3.30342757').comparedTo('-2.8'), -1);
    arktest.assertEQ(new Decimal('1458645603.7314952').comparedTo('-0.00000000000000071'), 1);
    arktest.assertEQ(new Decimal('-22352269.3857581').comparedTo('1.133'), -1);
    arktest.assertEQ(new Decimal('0.00000000000203016463277398240').comparedTo('11.54042'), -1);
    arktest.assertEQ(new Decimal('119.31').comparedTo('0.0000000000005419744970291'), 1);
    arktest.assertEQ(new Decimal('-479.458846').comparedTo('-61007894'), 1);
    arktest.assertEQ(new Decimal('-12.748').comparedTo('-42.6'), 1);
    arktest.assertEQ(new Decimal('-4').comparedTo('-2102161'), 1);
    arktest.assertEQ(new Decimal('-0.0000892951').comparedTo('-0.00000000700082036664'), -1);
}

function testComparedToWithDecimal008() {
    arktest.assertEQ(new Decimal('-78249536473266824330849.66777355094653206196949807187125').comparedTo('-0.0000000000000003657205364898565'), -1);
    arktest.assertEQ(new Decimal('-794146002335392753453.706827563775514677616').comparedTo('-0.000000000000002003697772718678199294529887857336117611280517515976'), -1);
    arktest.assertEQ(new Decimal('-415').comparedTo('392495171560354735.54'), -1);
    arktest.assertEQ(new Decimal('-0.00000000003641527559883410541221').comparedTo('118893973463454526683808886319100407749'), -1);
    arktest.assertEQ(new Decimal('-246230199620.751986').comparedTo('-60621558.218545419613692429476293547415222080017643997'), -1);
    arktest.assertEQ(new Decimal('1135.2381823').comparedTo('-7025657983677893287919132587754823076851573889.74685'), 1);
    arktest.assertEQ(new Decimal('-1225393854304.583672953386').comparedTo('3712805076014395323562.4542406246'), -1);
    arktest.assertEQ(new Decimal('0.0000000000002805023524281798').comparedTo('-1195739.55713460927706968721363067'), 1);
    arktest.assertEQ(new Decimal('386896730804748668115534580.29201499').comparedTo('737595739534741234.44386299'), 1);
    arktest.assertEQ(new Decimal('-160941276095561769012497288743700757276588036').comparedTo('194.232174807028239974132449968549146279604'), -1);
    arktest.assertEQ(new Decimal('0.0000000000000009760').comparedTo('-112424977227983.369213445871607530779963339'), 1);
    arktest.assertEQ(new Decimal('-120493.3535480159461699138868088193226718499846079').comparedTo('-963938726344338862591656029356573.2356831292145'), 1);
    arktest.assertEQ(new Decimal('1.731842697869349182975403693663977472678249978539').comparedTo('-0.0000000000018342058226450839410208095980857226'), 1);
    arktest.assertEQ(new Decimal('0.000000000000000000186044372917454007190404393463337172211654339256226').comparedTo('38603.328332936580263680889352090136846643070031691'), -1);
    arktest.assertEQ(new Decimal('2.24833678840107152900324').comparedTo('-0.000000000000535013632333188'), 1);
    arktest.assertEQ(new Decimal('-0.000000000001014445297159531').comparedTo('-182225871.4'), 1);
    arktest.assertEQ(new Decimal('-110739').comparedTo('39245367522721683497924493692756130948391370303714834'), -1);
    arktest.assertEQ(new Decimal('0.000000000000000000011109996302235').comparedTo('0.00000021747567330836261997878389805423396667433455'), -1);
    arktest.assertEQ(new Decimal('913805.7272137617667300113969454560').comparedTo('12015557861856523.82185'), -1);
    arktest.assertEQ(new Decimal('4.44').comparedTo('17327761911331833939511.3229350'), -1);
    arktest.assertEQ(new Decimal('3343815205898708797845323710139050310101815211790183').comparedTo('0.000002944097410246031171887416698431193651848888'), 1);
    arktest.assertEQ(new Decimal('32760398270494573303330410.50750229445655494486793110334706').comparedTo('-9510914125.45789497493920290055'), 1);
    arktest.assertEQ(new Decimal('0.00000031352646').comparedTo('2982438266661362962075700689156408.44636939397'), -1);
    arktest.assertEQ(new Decimal('77053023622996.111620117040477945').comparedTo('-594'), 1);
    arktest.assertEQ(new Decimal('28.651584435').comparedTo('0.0000000000000517018230'), 1);
    arktest.assertEQ(new Decimal('69454425323857571.298955').comparedTo('0.00000003183'), 1);
    arktest.assertEQ(new Decimal('8.585707061691').comparedTo('27579495297994019790817050261359'), -1);
}

function testComparedToWithDecimal009() {
    arktest.assertEQ(new Decimal('0.0000245072088767920273').comparedTo('597422085664.8946200175189947965015981804000953540032149'), -1);
    arktest.assertEQ(new Decimal('6395952035600311.145296483306709937475').comparedTo('-7839682836938441873082358214760503680131094656590381429'), 1);
    arktest.assertEQ(new Decimal('-878403860673101771781740794629505317750999165.91704111').comparedTo('23574346417573958371843475532943.5863825875041348813273'), -1);
    arktest.assertEQ(new Decimal('0.0000767302828677966418632441811470540764577').comparedTo('-1276636.42385918072718338162162974143519652320393232805716411'), 1);
    arktest.assertEQ(new Decimal('-15930445184474597597265919925606183581356435.4578944').comparedTo('-0.00000000000114252044623272018125497556095987129012517542089248'), -1);
    arktest.assertEQ(new Decimal('-48665285374684448074343674590620067.7').comparedTo('1'), -1);
    arktest.assertEQ(new Decimal('105930.422').comparedTo('11113072169.652252071060557105162'), -1);
    arktest.assertEQ(new Decimal('26564763').comparedTo('-3'), 1);
    arktest.assertEQ(new Decimal('4811244033166.822319661770849918367233504118713435759512811').comparedTo('4.7'), 1);
    arktest.assertEQ(new Decimal('-56.8').comparedTo('4443507757632459.195553180343008573591954189736210531834'), -1);
    arktest.assertEQ(new Decimal('612.72379387256827500110').comparedTo('-452.196955446440438357'), 1);
    arktest.assertEQ(new Decimal('-494127587184494770597966464398').comparedTo('0.000000000000000505582034979983270144354985077988823279257757358354'), -1);
    arktest.assertEQ(new Decimal('151593171537726736029059210').comparedTo('3280207153334176964'), 1);
    arktest.assertEQ(new Decimal('-0.0000000727423791233275798').comparedTo('-33.983207203823'), 1);
    arktest.assertEQ(new Decimal('-0.00000000000000000014088202399161').comparedTo('967411649025173.3'), -1);
    arktest.assertEQ(new Decimal('2.4').comparedTo('-0.000000000001348198764946041705529604570289570284'), 1);
    arktest.assertEQ(new Decimal('-1458694234293.393').comparedTo('-0.0000000277889669157893917'), -1);
    arktest.assertEQ(new Decimal('-1').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-295498271141109248639464345535.73829013835409617653972').comparedTo('-239733948637940687.4210093379196529044279897'), -1);
    arktest.assertEQ(new Decimal('-114.668973584958627612325023154590').comparedTo('0.00000000000000000204'), -1);
    arktest.assertEQ(new Decimal('11086296933075691303').comparedTo('61846609604939708809351700530350279982'), -1);
    arktest.assertEQ(new Decimal('-199290.9539286389610').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-0.00008470578761402').comparedTo('77227954881772040530.19249809987494428807714982'), -1);
    arktest.assertEQ(new Decimal('1675393978448').comparedTo('-218061932.345257837058901'), 1);
    arktest.assertEQ(new Decimal('-148234164778648.90057404374761057399794220096395194121763477').comparedTo('-1807153054383052.16291022958635252906429'), 1);
    arktest.assertEQ(new Decimal('-3504475807101437909894549201613284414285').comparedTo('0.00000180507680656754700593791722943439535831913116826463442277'), -1);
    arktest.assertEQ(new Decimal('137188855.912293557682373').comparedTo('-84390113782876.78301756825303169680317158020741023212586877'), 1);
    arktest.assertEQ(new Decimal('-8.04').comparedTo('48917396714819601257978143.0'), -1);
    arktest.assertEQ(new Decimal('19441.23908').comparedTo('-3621875953293954379164.80898'), 1);
    arktest.assertEQ(new Decimal('-488683915776821.54128571136').comparedTo('-2764.655'), -1);
    arktest.assertEQ(new Decimal('-497.39163093').comparedTo('456272554.910083158313064'), -1);
}

function testComparedToWithDecimal010() {
    arktest.assertEQ(new Decimal('3888553456.187038805').comparedTo('1.79351971'), 1);
    arktest.assertEQ(new Decimal('-0.09665505758418475963567867363097339048321797943170880174').comparedTo('42.26560746275599'), -1);
    arktest.assertEQ(new Decimal('18902797728391145440494082949463571569467603044').comparedTo('-160840513303121014911814965287069764.329850142'), 1);
    arktest.assertEQ(new Decimal('-0.00000000000363783296932').comparedTo('-229510691003150718410894.798513190'), 1);
    arktest.assertEQ(new Decimal('-0.019578513996').comparedTo('-0.0000068012586581839285561'), -1);
    arktest.assertEQ(new Decimal('-0.00000000000000003337257672564450192788237207780262977502390656214').comparedTo('-576.230537914969490642'), 1);
    arktest.assertEQ(new Decimal('0.0002071041857705517964418488527910814283948842252102023').comparedTo('4965'), -1);
    arktest.assertEQ(new Decimal('-762578569061554379901567768164.24708334695367').comparedTo('-2902150555266343625933468.2'), -1);
    arktest.assertEQ(new Decimal('366547461068.82113690').comparedTo('473567955790856323671.7615303868255685632986068926885'), -1);
    arktest.assertEQ(new Decimal('2212.60505074').comparedTo('54.4399284220426917738'), 1);
    arktest.assertEQ(new Decimal('24229481820388119475112679.2869744056638142529094915709331146').comparedTo('-39663594276.8672343304'), 1);
    arktest.assertEQ(new Decimal('204581808810007220059753337566285703.0558403249497').comparedTo('0.14724245060761654341919'), 1);
    arktest.assertEQ(new Decimal('186.26481926730311740748275435747257').comparedTo('3169339033932711835137078410077027045.6405'), -1);
    arktest.assertEQ(new Decimal('147438798802.4222641736117').comparedTo('-30953.68649'), 1);
    arktest.assertEQ(new Decimal('0.0000000000000000036665066712498278880448755082419497041867983779348').comparedTo('49969977335002848872793397'), -1);
    arktest.assertEQ(new Decimal('-15').comparedTo('-1610335165763678418286838402407661955'), 1);
    arktest.assertEQ(new Decimal('2689653843679875122027095562540366.2198041476017623').comparedTo('1680374149507467199865152885259.7294203852546004613867'), 1);
    arktest.assertEQ(new Decimal('-0.11').comparedTo('-0.00000000000000000043896646420939540095966'), -1);
    arktest.assertEQ(new Decimal('2080671699743.511238240714127').comparedTo('-0.000036842318404340179848907'), 1);
    arktest.assertEQ(new Decimal('568000304').comparedTo('-90592884500253731464906623327632577358591186.946837858889'), 1);
    arktest.assertEQ(new Decimal('-0.000000000000002107518141285').comparedTo('-3224876873708851754101647.818256157564'), 1);
    arktest.assertEQ(new Decimal('2265098771').comparedTo('0.00303612511166914586857128974318819782'), 1);
    arktest.assertEQ(new Decimal('-5664007822465.98050400197979616740857').comparedTo('4318641.351415884425470426852575673963981647030782942'), -1);
    arktest.assertEQ(new Decimal('-2548615100934958409898695557521334.971165846219072').comparedTo('-554798.735556810907'), -1);
    arktest.assertEQ(new Decimal('-0.000008820826897431443').comparedTo('-1685288868236928715034057139645.0670918901230'), 1);
    arktest.assertEQ(new Decimal('0.2773641914039966806939077833931526594995617517107827').comparedTo('-155510002669979906258854608426737730.80732922'), 1);
    arktest.assertEQ(new Decimal('0.00000028631297293946002594331990323448420').comparedTo('12.530358985140'), -1);
    arktest.assertEQ(new Decimal('65.74').comparedTo('0.000000180226526307592'), 1);
}

function testComparedToWithDecimal011() {
    arktest.assertEQ(new Decimal('35366830254177.323').comparedTo('-195533735162357391160682221146179916675364691505329.522594'), 1);
    arktest.assertEQ(new Decimal('623190599276699090316221455590041049805679995441489316.522').comparedTo('6153742899048073987312064712008701512451.12942'), 1);
    arktest.assertEQ(new Decimal('0.001389654081957090114281716350682554539119289983031175').comparedTo('0.0000000000000000004172245036048453978505043356031'), 1);
    arktest.assertEQ(new Decimal('-0.000000000000065125596165975329468700878377195017853327025922232').comparedTo('5010.27'), -1);
    arktest.assertEQ(new Decimal('99858764586308080.815938383').comparedTo('6906.98277'), 1);
    arktest.assertEQ(new Decimal('-0.0000002120213841110365850583078206489619527666932442761000523841').comparedTo('1560226.624828'), -1);
    arktest.assertEQ(new Decimal('-0.00000011209616970611').comparedTo('437120468783339721.8270432960'), -1);
    arktest.assertEQ(new Decimal('39633522.28696180408828').comparedTo('404609951713'), -1);
    arktest.assertEQ(new Decimal('0.00000439899975727851857364404628011452819').comparedTo('-295058799.554060774457769229721869795967861'), 1);
    arktest.assertEQ(new Decimal('-0.00000000000000042342244818730713908050671583102154').comparedTo('549754666663906751820116493890514432554613796763091771'), -1);
    arktest.assertEQ(new Decimal('271354103.271534143867908347243952035').comparedTo('-3.4348029165'), 1);
    arktest.assertEQ(new Decimal('-18391137696458176.709403414422273535748').comparedTo('-135526005724.36'), -1);
    arktest.assertEQ(new Decimal('-16789648705436253610328466585451657189').comparedTo('87790015500203097.67217621'), -1);
    arktest.assertEQ(new Decimal('-0.1934527980285924234435139814').comparedTo('0.000000044283070912014290453388249787033315196663317475885890'), -1);
    arktest.assertEQ(new Decimal('175727618.7').comparedTo('-4712524302.9633361502507795469472010873256880899968244213838'), 1);
    arktest.assertEQ(new Decimal('86148708.7').comparedTo('0.000000000043067763982519792325233598208451198'), 1);
    arktest.assertEQ(new Decimal('-1676718.814').comparedTo('-126027512979.7359979901775996'), 1);
    arktest.assertEQ(new Decimal('204654162545474.954541730442039127578542771572577634608702').comparedTo('1278262396819307952614391540746000135708145121661'), -1);
    arktest.assertEQ(new Decimal('20599411052765018472280152629960269.784440835643761573').comparedTo('382871.68'), 1);
    arktest.assertEQ(new Decimal('9.1').comparedTo('-0.000009868296529724402614472149967657'), 1);
    arktest.assertEQ(new Decimal('-51.7').comparedTo('-0.00004138549340599259177969843213431710067044689658507'), -1);
    arktest.assertEQ(new Decimal('-0.00000000065519676351299476540767970').comparedTo('-159950069281804382298.104882147944419487737768'), 1);
    arktest.assertEQ(new Decimal('132937997125408.196564521').comparedTo('0.0000000000000416671095818013123525013575750'), 1);
    arktest.assertEQ(new Decimal('-1313393782.747301').comparedTo('135189314585073603701567044403904503'), -1);
    arktest.assertEQ(new Decimal('0.26345218007386345905841').comparedTo('-0.0000000000000000131698402541203188119609481297941368650'), 1);
    arktest.assertEQ(new Decimal('-639051.389850068275663978184079330600736417788030565545').comparedTo('-233130008509087611163354733950079542005785536290.28'), 1);
    arktest.assertEQ(new Decimal('-2.5').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('-48').comparedTo('-1.30483838443'), -1);
}

function testComparedToWithDecimal012() {
    arktest.assertEQ(new Decimal('-0.0000007957592304983915492296920988493287302223429001').comparedTo('222281838853.55743543126434998666514248954378468481403010'), -1);
    arktest.assertEQ(new Decimal('-256924767625.92102529834889997062393414595239208331582').comparedTo('-138645955.7309701679'), -1);
    arktest.assertEQ(new Decimal('5825.6936168480383901079880').comparedTo('-15454778381.1635817748532891601'), 1);
    arktest.assertEQ(new Decimal('322234158068890727047790331925494003913674775559').comparedTo('0.0000000000000001440104086097707578083604'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000284163905693832757026').comparedTo('735321.796040692391355698638806984647971670552494930079886'), -1);
    arktest.assertEQ(new Decimal('13.51354661049663').comparedTo('60174586.03966439546'), -1);
    arktest.assertEQ(new Decimal('-0.0000000563225262474012816830760397217').comparedTo('-19259915510222671.51018'), 1);
    arktest.assertEQ(new Decimal('58260242951538386451799849576838755846723438641458498.45').comparedTo('81583418293233481447477355060230.5984042555029388'), 1);
    arktest.assertEQ(new Decimal('15676086441613294586793.0600578779086373855887').comparedTo('553785093898805279609967970.6227004730416155843290795860965'), -1);
    arktest.assertEQ(new Decimal('-529215908').comparedTo('0.000000000000001261696290245850445593226'), -1);
    arktest.assertEQ(new Decimal('0.2765770305056755810402655395163548457323921993').comparedTo('2485048778003226295.0487979661044'), -1);
    arktest.assertEQ(new Decimal('3.106357').comparedTo('-196188865060.96789451729283131151802962'), 1);
    arktest.assertEQ(new Decimal('-3812721623392532238628.52058836531583261604602809206126').comparedTo('0.000000000129949612553432181149516674241'), -1);
    arktest.assertEQ(new Decimal('-725747.78308445614294711222169380088709733492626141').comparedTo('0'), -1);
    arktest.assertEQ(new Decimal('0.00005978057030838911749503353205688236750513118711').comparedTo('-2301374753039922.5'), 1);
    arktest.assertEQ(new Decimal('-21135.6').comparedTo('0.0000000000000677319833766662844296'), -1);
    arktest.assertEQ(new Decimal('-18592472738146.165033667092366').comparedTo('-0.245177658870876212432221320582908107465202878031116163978799'), -1);
    arktest.assertEQ(new Decimal('2.0853471559342294701189635489971894524307297590789015254659').comparedTo('1820710163171110437504131672398834715.796647407'), -1);
    arktest.assertEQ(new Decimal('3017.7628349701943687591214').comparedTo('-2123676032175089021.800850462329'), 1);
    arktest.assertEQ(new Decimal('2245516696370652079582811421413778879168242699149').comparedTo('-0.000026865223'), 1);
    arktest.assertEQ(new Decimal('396591381033578245290535623.59522101830916124323912171677').comparedTo('-399632909396.27251691705488670319'), 1);
    arktest.assertEQ(new Decimal('25071395481877740931640606160951694567581843991.91250540').comparedTo('0.000000129577260075209456379795979548520957596680350786352443111'), 1);
    arktest.assertEQ(new Decimal('-10071840824462229810456841986062182478').comparedTo('-0.000000000000036'), -1);
    arktest.assertEQ(new Decimal('-378.382656390921442297').comparedTo('2995.111135910'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000011003920232607040692705').comparedTo('0.00000000000000002355157685511431284976273511723771203290674587231328782173'), -1);
    arktest.assertEQ(new Decimal('894.8158182999506884919347710660910').comparedTo('-0.00000000000000000096323615121853367670285903274497581'), 1);
    arktest.assertEQ(new Decimal('2319037.233322076813874258').comparedTo('-32416.903557641'), 1);
    arktest.assertEQ(new Decimal('23396460119.155847859729080972376305599746464844501598782869').comparedTo('-23349855891702694333726330.5788991'), 1);
    arktest.assertEQ(new Decimal('90907619612776583630058641475331.6').comparedTo('834081.31401857878701092663973283388246447251015444'), 1);
    arktest.assertEQ(new Decimal('-0.0000000000000000081501364451042879423837094013790187022253').comparedTo('-0.00000001399490096609828204621353982853023593'), 1);
    arktest.assertEQ(new Decimal('-420605.932784654921007625945921605929').comparedTo('-0.001003986559391891122940439549512675'), -1);
    arktest.assertEQ(new Decimal('29941819100256.1010903984648918366033290553065670117884535').comparedTo('-1399273749558314.35414647141708136502314'), 1);
    arktest.assertEQ(new Decimal('9.87428322502844153862497406397966790245714773688').comparedTo('282378323960341447353633555973.60816609111820'), -1);
    arktest.assertEQ(new Decimal('0.000000011571').comparedTo('56.055239680'), -1);
}

function testComparedToWithDecimal013() {
    arktest.assertEQ(new Decimal('57847056020509197.81010326363252948483107773').comparedTo('-1.4444'), 1);
    arktest.assertEQ(new Decimal('-51735520880178.3819992386025068250767281052153771374742233700').comparedTo('-0.000000000000000001995850558903648471134005565207297729809954251013177868'), -1);
    arktest.assertEQ(new Decimal('-252689605402394270719155.41734356951305987480624879').comparedTo('-14.7'), -1);
    arktest.assertEQ(new Decimal('-0.00000000001151229725364301025243181548204622943024492441630358').comparedTo('26395005645801030069538169698242312.433651971508113804'), -1);
    arktest.assertEQ(new Decimal('-0.00000000000091483130556462719').comparedTo('2'), -1);
    arktest.assertEQ(new Decimal('208.9827699741542594700570327748394160582183088044349').comparedTo('-0.00000000000052759011609237039384483268124939035873403'), 1);
    arktest.assertEQ(new Decimal('12412.9657547118').comparedTo('4'), 1);
    arktest.assertEQ(new Decimal('-2').comparedTo('91580728825224739577263849484410121433204621.500780779690'), -1);
    arktest.assertEQ(new Decimal('459526644.772925035397599290137226147632521530511862018').comparedTo('-0.000000005846169451429981275734796656'), 1);
    arktest.assertEQ(new Decimal('671926163021026323133951152902929830282.91041796493').comparedTo('548498210972677.51442460018844632588617668956139310271612'), 1);
    arktest.assertEQ(new Decimal('2422.5875291768881507425406922241585652688246').comparedTo('19145241007530705409.9615104357443736565548963'), -1);
    arktest.assertEQ(new Decimal('-0.00000078875216605898054012159').comparedTo('-3150.0'), 1);
    arktest.assertEQ(new Decimal('6244534680058004259589682789034535443037697').comparedTo('-25351939981079.604012'), 1);
    arktest.assertEQ(new Decimal('-180.203958489450').comparedTo('39025034688578315815199.297787915576434930764'), -1);
    arktest.assertEQ(new Decimal('-1361080637840651575481575485233.15286726682268248409468726').comparedTo('-0.0000000000000039297884367828635174097881955958806085'), -1);
    arktest.assertEQ(new Decimal('180278536333055.7355327506162313565598247413').comparedTo('0.000000000005454620478830996984164588529474180472398020083723230733100'), 1);
    arktest.assertEQ(new Decimal('-12521213.94690').comparedTo('213.36136'), -1);
    arktest.assertEQ(new Decimal('-0.0000000000000076966493627693824010017610386791').comparedTo('-2.378265167488246191074961795033182605013037407884326'), 1);
    arktest.assertEQ(new Decimal('-253.6948').comparedTo('126006206'), -1);
}

function testComparedToWithExponentialNumber001() {
    arktest.assertEQ(new Decimal('-5.5239e+3').comparedTo('-2.2807805e+1'), -1);
    arktest.assertEQ(new Decimal('8.554e+2').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('-2.1176736939e+5').comparedTo('2.13526332e+2'), -1);
    arktest.assertEQ(new Decimal('2.528e-2').comparedTo('-1.5e-1'), 1);
    arktest.assertEQ(new Decimal('-2.1536554e-2').comparedTo('-3.93e-6'), -1);
    arktest.assertEQ(new Decimal('4.853430802e-5').comparedTo('-4.9e-11'), 1);
    arktest.assertEQ(new Decimal('-6.9e-3').comparedTo('1.9596e+1'), -1);
    arktest.assertEQ(new Decimal('-9.93182949e+1').comparedTo('-7.9983535342e-20'), -1);
    arktest.assertEQ(new Decimal('1.629795e+5').comparedTo('1.58117535423e+5'), 1);
    arktest.assertEQ(new Decimal('-2.934333546e-15').comparedTo('-2.093237829e-14'), 1);
    arktest.assertEQ(new Decimal('2.2916931e+1').comparedTo('7.88e+2'), -1);
    arktest.assertEQ(new Decimal('7.656115e+0').comparedTo('5.5885422e-10'), 1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('-3.3e+0'), 1);
    arktest.assertEQ(new Decimal('-2.4327703616e+3').comparedTo('1.1e+1'), -1);
    arktest.assertEQ(new Decimal('1.61903621e+6').comparedTo('-2.3444006423e+2'), 1);
    arktest.assertEQ(new Decimal('4.5332e+4').comparedTo('3.281123867e+1'), 1);
    arktest.assertEQ(new Decimal('-7.9e-4').comparedTo('0e+0'), -1);
    arktest.assertEQ(new Decimal('-2.6e-19').comparedTo('-5.4489933e+6'), 1);
    arktest.assertEQ(new Decimal('5.877152e+1').comparedTo('1.5125e-2'), 1);
    arktest.assertEQ(new Decimal('6.293000381e-1').comparedTo('7.2e+1'), -1);
    arktest.assertEQ(new Decimal('1.072062222225e+10').comparedTo('7.521052067e+0'), 1);
    arktest.assertEQ(new Decimal('-2.9744557e+7').comparedTo('0e+0'), -1);
    arktest.assertEQ(new Decimal('-1.0235e-13').comparedTo('-3.671886e+0'), 1);
    arktest.assertEQ(new Decimal('-2.06364e-4').comparedTo('6.2113337e-3'), -1);
}

function testComparedToWithExponentialNumber002() {
    arktest.assertEQ(new Decimal('-7.02e+0').comparedTo('-1.1348644434e+3'), 1);
    arktest.assertEQ(new Decimal('-3.57688146e+3').comparedTo('3.81547e+2'), -1);
    arktest.assertEQ(new Decimal('2.9527098536e+3').comparedTo('1.32080504447e-4'), 1);
    arktest.assertEQ(new Decimal('-2.6286e+1').comparedTo('3.8e+0'), -1);
    arktest.assertEQ(new Decimal('-3.45054603e+2').comparedTo('-9.88229e-15'), -1);
    arktest.assertEQ(new Decimal('-2.469e+0').comparedTo('-3e+0'), 1);
    arktest.assertEQ(new Decimal('-8.369800004e-4').comparedTo('1.4e-17'), -1);
    arktest.assertEQ(new Decimal('-1e+0').comparedTo('4e+0'), -1);
    arktest.assertEQ(new Decimal('-3.0829216081e+2').comparedTo('-3.08e+1'), -1);
    arktest.assertEQ(new Decimal('-1e+0').comparedTo('-5.756e+1'), 1);
    arktest.assertEQ(new Decimal('-8.511e+0').comparedTo('3.126324e+6'), -1);
    arktest.assertEQ(new Decimal('2.4474011949e+4').comparedTo('1.12649e+2'), 1);
    arktest.assertEQ(new Decimal('-2.57868e+4').comparedTo('-1.03178e+1'), -1);
    arktest.assertEQ(new Decimal('1e+0').comparedTo('9.5177e+1'), -1);
    arktest.assertEQ(new Decimal('-9.8069663157e-6').comparedTo('1.4532689e+5'), -1);
    arktest.assertEQ(new Decimal('3.721e+1').comparedTo('-2e+0'), 1);
    arktest.assertEQ(new Decimal('-9.7657879751e+10').comparedTo('-5.67e+2'), -1);
    arktest.assertEQ(new Decimal('-4.8657905e+6').comparedTo('-2.17381281e+1'), -1);
    arktest.assertEQ(new Decimal('-4.20131e+0').comparedTo('-1.3179640655e-8'), -1);
    arktest.assertEQ(new Decimal('1.6e+0').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('5.16068325659e-4').comparedTo('1.269e+1'), -1);
    arktest.assertEQ(new Decimal('9.91969e+5').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('2.32e+0').comparedTo('3.69e+1'), -1);
    arktest.assertEQ(new Decimal('5.685149e+2').comparedTo('3.694e-13'), 1);
    arktest.assertEQ(new Decimal('-9.0148e-5').comparedTo('0e+0'), -1);
    arktest.assertEQ(new Decimal('-3e+0').comparedTo('-8.7855e-7'), -1);
    arktest.assertEQ(new Decimal('1.3669489e+3').comparedTo('1.33866e-4'), 1);
    arktest.assertEQ(new Decimal('-2.6e+1').comparedTo('1.36976797894e+3'), -1);
    arktest.assertEQ(new Decimal('4.6e+0').comparedTo('0e+0'), 1);
}

function testComparedToWithExponentialNumber003() {
    arktest.assertEQ(new Decimal('-7.2e-8').comparedTo('1.971355e+2'), -1);
    arktest.assertEQ(new Decimal('1.19755e-5').comparedTo('7e+0'), -1);
    arktest.assertEQ(new Decimal('-3.46e+0').comparedTo('-3.7715361e-11'), -1);
    arktest.assertEQ(new Decimal('3.25762245146e+6').comparedTo('-4.02e+0'), 1);
    arktest.assertEQ(new Decimal('1.3174225699e+7').comparedTo('9.6222695655e+10'), -1);
    arktest.assertEQ(new Decimal('6.6e-13').comparedTo('-5.23193346205e+4'), 1);
    arktest.assertEQ(new Decimal('-3.873e+3').comparedTo('-1.23681175e+3'), -1);
    arktest.assertEQ(new Decimal('-1.19869e+5').comparedTo('-8e+2'), -1);
    arktest.assertEQ(new Decimal('2.027381656e+3').comparedTo('1.47629351439e+11'), -1);
    arktest.assertEQ(new Decimal('-1.467418e-17').comparedTo('9.0073930856e+5'), -1);
    arktest.assertEQ(new Decimal('7.1e+0').comparedTo('6.116301582e+2'), -1);
    arktest.assertEQ(new Decimal('-1.7883916e+5').comparedTo('3.533e-17'), -1);
    arktest.assertEQ(new Decimal('8.8e-11').comparedTo('-5.9707897e-1'), 1);
    arktest.assertEQ(new Decimal('-5.337e+3').comparedTo('4.6614e-13'), -1);
    arktest.assertEQ(new Decimal('1e+0').comparedTo('-3.1e+0'), 1);
    arktest.assertEQ(new Decimal('-3.05920196e-6').comparedTo('-4e+0'), 1);
    arktest.assertEQ(new Decimal('-1.692375e-10').comparedTo('2.29981330096e+6'), -1);
    arktest.assertEQ(new Decimal('5.19341813e-1').comparedTo('6.1e+0'), -1);
    arktest.assertEQ(new Decimal('5.796e+3').comparedTo('-9.386773281e-8'), 1);
    arktest.assertEQ(new Decimal('9.117497e+0').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('6.962934e-8').comparedTo('1.413982e+2'), -1);
    arktest.assertEQ(new Decimal('2e+0').comparedTo('1.4e+1'), -1);
    arktest.assertEQ(new Decimal('-2.04e+0').comparedTo('-1.30300025e+9'), 1);
    arktest.assertEQ(new Decimal('1.57647e+1').comparedTo('3.0862783894e+0'), 1);
    arktest.assertEQ(new Decimal('-4e+0').comparedTo('7.1e+1'), -1);
    arktest.assertEQ(new Decimal('1.21e-6').comparedTo('-2.109584154e+10'), 1);
    arktest.assertEQ(new Decimal('6.55669245e+3').comparedTo('-8.15e-8'), 1);
    arktest.assertEQ(new Decimal('-8.3307e+2').comparedTo('-9.1476874e+7'), 1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('-3.64676e+5'), 1);
}

function testComparedToWithExponentialNumber004() {
    arktest.assertEQ(new Decimal('-1.2803e+2').comparedTo('7.2908749e+3'), -1);
    arktest.assertEQ(new Decimal('3.2263e+3').comparedTo('-1.43355e+1'), 1);
    arktest.assertEQ(new Decimal('4.92751482244e-18').comparedTo('1.3747248256e+3'), -1);
    arktest.assertEQ(new Decimal('8.20054755e-10').comparedTo('6.2348e+2'), -1);
    arktest.assertEQ(new Decimal('-1.06e+1').comparedTo('-7.2569084631e+4'), 1);
    arktest.assertEQ(new Decimal('1.86e+2').comparedTo('3.76613527e-5'), 1);
    arktest.assertEQ(new Decimal('3.33652e+3').comparedTo('5.1e+1'), 1);
    arktest.assertEQ(new Decimal('3.665307673e+8').comparedTo('3.31344e+5'), 1);
    arktest.assertEQ(new Decimal('-7.341261161e+2').comparedTo('-2.98910916e+7'), 1);
    arktest.assertEQ(new Decimal('6.24638573245e+5').comparedTo('4.47893448327e+2'), 1);
    arktest.assertEQ(new Decimal('1.23e-8').comparedTo('-5e+0'), 1);
    arktest.assertEQ(new Decimal('4.469176e+5').comparedTo('-2.09863e+3'), 1);
    arktest.assertEQ(new Decimal('-2.72624816161e+3').comparedTo('0e+0'), -1);
    arktest.assertEQ(new Decimal('-6.78251e-20').comparedTo('-6.837e-1'), 1);
    arktest.assertEQ(new Decimal('6.2456922e-6').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('6.2e-4').comparedTo('4.40516e+4'), -1);
    arktest.assertEQ(new Decimal('-7.624e+2').comparedTo('-3e+0'), -1);
    arktest.assertEQ(new Decimal('-5.22549619458e+0').comparedTo('-4.0866914233e+9'), 1);
    arktest.assertEQ(new Decimal('-9.51999532e+7').comparedTo('-2e+0'), -1);
    arktest.assertEQ(new Decimal('-1.07321e+2').comparedTo('-1.3e+0'), -1);
    arktest.assertEQ(new Decimal('-5.405555671e-6').comparedTo('8.0306e+1'), -1);
    arktest.assertEQ(new Decimal('2.2e+0').comparedTo('6.49e-17'), 1);
    arktest.assertEQ(new Decimal('3e+0').comparedTo('2.2e+0'), 1);
    arktest.assertEQ(new Decimal('-1.12e+2').comparedTo('3.4165375e+1'), -1);
    arktest.assertEQ(new Decimal('-2.36978379e+4').comparedTo('-4.2e+0'), -1);
    arktest.assertEQ(new Decimal('-8e+0').comparedTo('-6.42e+0'), -1);
    arktest.assertEQ(new Decimal('-3.2e+1').comparedTo('6.17231707e-19'), -1);
}

function testComparedToWithExponentialNumber005() {
    arktest.assertEQ(new Decimal('-4.8454741051e+1').comparedTo('4.3339e-10'), -1);
    arktest.assertEQ(new Decimal('4.29e+0').comparedTo('3.28376306e+4'), -1);
    arktest.assertEQ(new Decimal('-6.33089e+1').comparedTo('-5.7065758354e-19'), -1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('-3.31e-7'), 1);
    arktest.assertEQ(new Decimal('-2.18973e+2').comparedTo('1.88333703659e+4'), -1);
    arktest.assertEQ(new Decimal('-1.67e+2').comparedTo('3e+0'), -1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('9.7605e-12'), -1);
    arktest.assertEQ(new Decimal('1.294433816e+0').comparedTo('-1.1e+1'), 1);
    arktest.assertEQ(new Decimal('-1.4246177e+3').comparedTo('-1.043532436e+8'), 1);
    arktest.assertEQ(new Decimal('-1.0531e+1').comparedTo('2.2268899e-6'), -1);
    arktest.assertEQ(new Decimal('-1.5611e+3').comparedTo('1.59757e+0'), -1);
    arktest.assertEQ(new Decimal('3.3638160556e+11').comparedTo('6.29136402e+6'), 1);
    arktest.assertEQ(new Decimal('-7.7326244542e+2').comparedTo('-9.6e+0'), -1);
    arktest.assertEQ(new Decimal('-3.593606997e+8').comparedTo('-4e+0'), -1);
    arktest.assertEQ(new Decimal('9.4433e+2').comparedTo('-1e+0'), 1);
    arktest.assertEQ(new Decimal('-1.0131e-9').comparedTo('-4.551632556e+9'), 1);
    arktest.assertEQ(new Decimal('1.61735e+5').comparedTo('-6.550194286e+9'), 1);
    arktest.assertEQ(new Decimal('3e+0').comparedTo('-8.14e+3'), 1);
    arktest.assertEQ(new Decimal('-3.492e+0').comparedTo('8.85393125e+2'), -1);
    arktest.assertEQ(new Decimal('-9.3678169131e-19').comparedTo('-1.98802069e+4'), 1);
    arktest.assertEQ(new Decimal('-4.39338019e+7').comparedTo('1.07850051e-15'), -1);
    arktest.assertEQ(new Decimal('-1.1677e+0').comparedTo('2.22825374e+8'), -1);
    arktest.assertEQ(new Decimal('-3.476e+0').comparedTo('6e+0'), -1);
    arktest.assertEQ(new Decimal('1.453e+0').comparedTo('-6.34599e-7'), 1);
    arktest.assertEQ(new Decimal('1.789e-9').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('-2.92933e+4').comparedTo('5.802095e+4'), -1);
    arktest.assertEQ(new Decimal('9e+0').comparedTo('2.63702e-7'), 1);
}

function testComparedToWithExponentialNumber006() {
    arktest.assertEQ(new Decimal('-6.0696e+4').comparedTo('9.46475e+5'), -1);
    arktest.assertEQ(new Decimal('-1.508052e+6').comparedTo('2.45600810354e+0'), -1);
    arktest.assertEQ(new Decimal('6.83354e+1').comparedTo('1.93e-12'), 1);
    arktest.assertEQ(new Decimal('-1.6e+0').comparedTo('-2e+0'), 1);
    arktest.assertEQ(new Decimal('-7.4275644e+5').comparedTo('-6.1587e-14'), -1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('3.540869872e-9'), -1);
    arktest.assertEQ(new Decimal('-7.86e+0').comparedTo('8.041734e+6'), -1);
    arktest.assertEQ(new Decimal('-8e+0').comparedTo('1.569e+1'), -1);
    arktest.assertEQ(new Decimal('2.0839e-18').comparedTo('1.2848955034e-14'), -1);
    arktest.assertEQ(new Decimal('6.84909e+3').comparedTo('2.19e+1'), 1);
    arktest.assertEQ(new Decimal('5.556062e+3').comparedTo('1.037e-12'), 1);
    arktest.assertEQ(new Decimal('-1.011918e-9').comparedTo('3.52315476249e+0'), -1);
    arktest.assertEQ(new Decimal('-1.2e+2').comparedTo('-5.8677085e+1'), -1);
    arktest.assertEQ(new Decimal('1.5e+0').comparedTo('-8.2951e-8'), 1);
    arktest.assertEQ(new Decimal('5.254e+0').comparedTo('-2.08338e+3'), 1);
    arktest.assertEQ(new Decimal('8.272e+0').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('-2.3496e+3').comparedTo('-1.74217696625e-1'), -1);
    arktest.assertEQ(new Decimal('2.33e+0').comparedTo('-6e+0'), 1);
    arktest.assertEQ(new Decimal('8.138135061e-6').comparedTo('1.52e+1'), -1);
    arktest.assertEQ(new Decimal('-2.691357e+5').comparedTo('1e+0'), -1);
    arktest.assertEQ(new Decimal('4.277011e-13').comparedTo('2.1978e+4'), -1);
    arktest.assertEQ(new Decimal('-4.164225e+0').comparedTo('3.94601989492e-6'), -1);
    arktest.assertEQ(new Decimal('-1.32e-18').comparedTo('-3.94e-3'), 1);
    arktest.assertEQ(new Decimal('-2.443211e-11').comparedTo('2.02e-8'), -1);
    arktest.assertEQ(new Decimal('-1.2e+0').comparedTo('-3.2e+1'), 1);
}

function testComparedToWithExponentialNumber007() {
    arktest.assertEQ(new Decimal('4.7652754554e+7').comparedTo('-2.1e-11'), 1);
    arktest.assertEQ(new Decimal('2.02136500541e+9').comparedTo('-2.77170882e+2'), 1);
    arktest.assertEQ(new Decimal('-1.65e-2').comparedTo('-2.4039e+3'), 1);
    arktest.assertEQ(new Decimal('1.8172e+1').comparedTo('-6.3e-5'), 1);
    arktest.assertEQ(new Decimal('1.8703827209e-10').comparedTo('-1.46184e+4'), 1);
    arktest.assertEQ(new Decimal('-1.11606903e+6').comparedTo('-2.8e-6'), -1);
    arktest.assertEQ(new Decimal('-7.84180535e+0').comparedTo('-6.9715e+2'), 1);
    arktest.assertEQ(new Decimal('-1.6095371e-5').comparedTo('-4.007e+2'), 1);
    arktest.assertEQ(new Decimal('2e+0').comparedTo('2.7730114619e+0'), -1);
    arktest.assertEQ(new Decimal('1.3694403e+7').comparedTo('-3.14014263471e-3'), 1);
    arktest.assertEQ(new Decimal('3e+0').comparedTo('2.4553237278e+3'), -1);
    arktest.assertEQ(new Decimal('-3e+0').comparedTo('-2.78756184842e+11'), 1);
    arktest.assertEQ(new Decimal('-1.2522e+2').comparedTo('-7.7e+1'), -1);
    arktest.assertEQ(new Decimal('4.491e-18').comparedTo('-1.196e+3'), 1);
    arktest.assertEQ(new Decimal('-3e+0').comparedTo('-2.78811358e+4'), 1);
    arktest.assertEQ(new Decimal('-3.5481789e+3').comparedTo('1.6004994927e+5'), -1);
    arktest.assertEQ(new Decimal('1.9963146e+6').comparedTo('-6.923e-1'), 1);
    arktest.assertEQ(new Decimal('-1.37573e-12').comparedTo('2.072e+2'), -1);
    arktest.assertEQ(new Decimal('6.570067306e-1').comparedTo('3.73554490286e-16'), 1);
    arktest.assertEQ(new Decimal('3.0659071736e-15').comparedTo('6.946525e+1'), -1);
    arktest.assertEQ(new Decimal('-5.346e+0').comparedTo('1e+0'), -1);
    arktest.assertEQ(new Decimal('8.55e-9').comparedTo('7.9e-11'), 1);
    arktest.assertEQ(new Decimal('-7.5096624e+2').comparedTo('3.40622424e+5'), -1);
    arktest.assertEQ(new Decimal('2.219404561e-14').comparedTo('5.47749e+0'), -1);
    arktest.assertEQ(new Decimal('3.000476e+2').comparedTo('-1.754092582e+1'), 1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('-1.69774e+3'), 1);
    arktest.assertEQ(new Decimal('2.993e+0').comparedTo('-1e+0'), 1);
}

function testComparedToWithExponentialNumber008() {
    arktest.assertEQ(new Decimal('2.7528038851e+1').comparedTo('-3.945799123e+4'), 1);
    arktest.assertEQ(new Decimal('-6.6266e+2').comparedTo('-4.5e-19'), -1);
    arktest.assertEQ(new Decimal('9.204609e-14').comparedTo('1.25862e+5'), -1);
    arktest.assertEQ(new Decimal('-7.0237408e+5').comparedTo('-4.56413483775e+3'), -1);
    arktest.assertEQ(new Decimal('4.8481576e+2').comparedTo('-6.53713e+3'), 1);
    arktest.assertEQ(new Decimal('1e+0').comparedTo('1.103e+2'), -1);
    arktest.assertEQ(new Decimal('6.57977e+2').comparedTo('-3.852e-16'), 1);
    arktest.assertEQ(new Decimal('3.861e+2').comparedTo('-6.2072117462e+5'), 1);
    arktest.assertEQ(new Decimal('-6e+0').comparedTo('1.3494901111e+6'), -1);
    arktest.assertEQ(new Decimal('-5.870696e-1').comparedTo('-3.801808e+1'), 1);
    arktest.assertEQ(new Decimal('-3.83761721e+8').comparedTo('8.081107501e-5'), -1);
    arktest.assertEQ(new Decimal('6.50057174e+5').comparedTo('2.0206519e-12'), 1);
    arktest.assertEQ(new Decimal('5.5207e+4').comparedTo('-2.48e+2'), 1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('1.05116516384e+11'), -1);
    arktest.assertEQ(new Decimal('-1.75279284e+4').comparedTo('3.899762267e-1'), -1);
    arktest.assertEQ(new Decimal('-1.6197e+0').comparedTo('4.3274e+4'), -1);
    arktest.assertEQ(new Decimal('1.256294e+6').comparedTo('-5.58921e-15'), 1);
    arktest.assertEQ(new Decimal('3e+0').comparedTo('2.0586937916e-17'), 1);
    arktest.assertEQ(new Decimal('-3.28553e-13').comparedTo('-1.18e+1'), 1);
    arktest.assertEQ(new Decimal('-2.81418e+5').comparedTo('1.72529458e-12'), -1);
    arktest.assertEQ(new Decimal('7.8666421e+3').comparedTo('5.9153e+0'), 1);
    arktest.assertEQ(new Decimal('4.84347589e-8').comparedTo('2e+0'), -1);
    arktest.assertEQ(new Decimal('3e+0').comparedTo('-2.6655512e+2'), 1);
    arktest.assertEQ(new Decimal('6.2696e-10').comparedTo('-2.054382974e+1'), 1);
    arktest.assertEQ(new Decimal('6.1413274e-2').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('-2.9095e-18').comparedTo('-6.9975956783e+8'), 1);
    arktest.assertEQ(new Decimal('4.01e-20').comparedTo('-7.5588e-6'), 1);
}

function testComparedToWithExponentialNumber009() {
    arktest.assertEQ(new Decimal('9.490622e-18').comparedTo('-2.77e+0'), 1);
    arktest.assertEQ(new Decimal('-1.0267e-14').comparedTo('-8.790091e-9'), 1);
    arktest.assertEQ(new Decimal('-4.077591e-14').comparedTo('-3.0259558246e+0'), 1);
    arktest.assertEQ(new Decimal('-2.8261e+2').comparedTo('-4.99857e+0'), -1);
    arktest.assertEQ(new Decimal('9.899113e+1').comparedTo('1e+0'), 1);
    arktest.assertEQ(new Decimal('5.5e+0').comparedTo('-7.0265e+0'), 1);
    arktest.assertEQ(new Decimal('5.286e-7').comparedTo('-4.3972682e+2'), 1);
    arktest.assertEQ(new Decimal('1.5520187e+6').comparedTo('1.150018e+2'), 1);
    arktest.assertEQ(new Decimal('-2.04e-10').comparedTo('-4.0474e+2'), 1);
    arktest.assertEQ(new Decimal('-2.32245517e+7').comparedTo('-4.96976e+4'), -1);
    arktest.assertEQ(new Decimal('-3.6487688611e-10').comparedTo('-1.1763503e+2'), 1);
    arktest.assertEQ(new Decimal('8.3414625e+0').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('9.905071e+6').comparedTo('-5.9541e-4'), 1);
    arktest.assertEQ(new Decimal('1.984307e+4').comparedTo('-2.4291e-7'), 1);
    arktest.assertEQ(new Decimal('-1e+0').comparedTo('-1e+0'), 0);
    arktest.assertEQ(new Decimal('2.060415969e+9').comparedTo('-2.03455e+2'), 1);
    arktest.assertEQ(new Decimal('3.1982e-19').comparedTo('-1.993945e+3'), 1);
    arktest.assertEQ(new Decimal('3.093739e+1').comparedTo('-1.99128676855e+0'), 1);
    arktest.assertEQ(new Decimal('1.881e+1').comparedTo('-1.09028754e+5'), 1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('-4.7092316e-14'), 1);
    arktest.assertEQ(new Decimal('-9.455374693e-10').comparedTo('-9.45812534991e+6'), 1);
    arktest.assertEQ(new Decimal('-1.67881e+4').comparedTo('-2.1683630866e+3'), -1);
    arktest.assertEQ(new Decimal('6.3514013642e-17').comparedTo('-4.5e+0'), 1);
    arktest.assertEQ(new Decimal('1.20179765e-20').comparedTo('4.9098e+3'), -1);
    arktest.assertEQ(new Decimal('0e+0').comparedTo('-7.476983768e+1'), 1);
    arktest.assertEQ(new Decimal('-8.5797e+2').comparedTo('2.1e+0'), -1);
}

function testComparedToWithExponentialNumber010() {
    arktest.assertEQ(new Decimal('-2.75e+1').comparedTo('-1.749e-18'), -1);
    arktest.assertEQ(new Decimal('-1e+0').comparedTo('8.4e+1'), -1);
    arktest.assertEQ(new Decimal('7.110682e+4').comparedTo('8.5646556e+2'), 1);
    arktest.assertEQ(new Decimal('-3.1875423048e+6').comparedTo('7.216e+1'), -1);
    arktest.assertEQ(new Decimal('-1.41969e+0').comparedTo('0e+0'), -1);
    arktest.assertEQ(new Decimal('-3.707432e+6').comparedTo('1.5048071e+2'), -1);
    arktest.assertEQ(new Decimal('-2.65447780189e+11').comparedTo('2e+0'), -1);
    arktest.assertEQ(new Decimal('3.808865e+6').comparedTo('-7.22140745e-10'), 1);
    arktest.assertEQ(new Decimal('2.972544e+3').comparedTo('0e+0'), 1);
    arktest.assertEQ(new Decimal('1.142002886e-2').comparedTo('-3.063e+0'), 1);
    arktest.assertEQ(new Decimal('-3.292e-15').comparedTo('-2.2e-1'), 1);
    arktest.assertEQ(new Decimal('-9.29480350675e-6').comparedTo('9.4830355594e+4'), -1);
    arktest.assertEQ(new Decimal('-2.49267401e-19').comparedTo('3.3296905e+4'), -1);
}
