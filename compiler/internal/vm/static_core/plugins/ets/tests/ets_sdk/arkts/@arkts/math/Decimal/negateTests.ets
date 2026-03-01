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
    const suite = new arktest.ArkTestsuite("Decimal negate API tests");
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig001);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig002);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig003);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig004);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig005);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig006);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig007);
    suite.addTest("Decimal negate with default config", testNegatedWithDefaultConfig008);
    suite.addTest("Decimal negate with specific config", testNegatedWithSpecificConfig);
    return suite.run();
}

function testNegatedWithDefaultConfig001() {
    arktest.assertEQ(new Decimal(4).negate().valueOf(), '-4');
    arktest.assertEQ(new Decimal(2147483648).negate().valueOf(), '-2147483648');
    arktest.assertEQ(new Decimal(0.25).negate().valueOf(), '-0.25');
    arktest.assertEQ(new Decimal(0.0625).negate().valueOf(), '-0.0625');
    arktest.assertEQ(new Decimal(1).negate().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1).negate().valueOf(), '1');
    arktest.assertEQ(new Decimal(0).negate().valueOf(), '-0',);
    arktest.assertEQ(new Decimal('0').negate().valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').negate().valueOf(), '0');
    arktest.assertEQ(new Decimal(NaN).negate().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').negate().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(Infinity).negate().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(-Infinity).negate().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('Infinity').negate().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('-Infinity').negate().valueOf(), 'Infinity');
}

function testNegatedWithDefaultConfig002() {
    arktest.assertEQ(new Decimal('9.99e+9000000000000000').negate().valueOf(), '-9.99e+9000000000000000');
    arktest.assertEQ(new Decimal('-9.99e+9000000000000000').negate().valueOf(),'9.99e+9000000000000000');
    arktest.assertEQ(new Decimal('1e+9000000000000001').negate().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('-1e+9000000000000001').negate().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('1e-9000000000000000').negate().valueOf(), '-1e-9000000000000000');
    arktest.assertEQ(new Decimal('-1e-9000000000000000').negate().valueOf(), '1e-9000000000000000');
    arktest.assertEQ(new Decimal('-1e-9000000000000001').negate().valueOf(), '0');
    arktest.assertEQ(new Decimal('238').negate().valueOf(), '-238');
    arktest.assertEQ(new Decimal('-0.000000000013').negate().valueOf(), '1.3e-11');
    arktest.assertEQ(new Decimal('33.1').negate().valueOf(), '-33.1');
    arktest.assertEQ(new Decimal('-2.61').negate().valueOf(), '2.61');
    arktest.assertEQ(new Decimal('4.0').negate().valueOf(), '-4');
    arktest.assertEQ(new Decimal('5.8').negate().valueOf(), '-5.8');
    arktest.assertEQ(new Decimal('0.000000352').negate().valueOf(), '-3.52e-7');
}

function testNegatedWithDefaultConfig003() {
    arktest.assertEQ(new Decimal('-190').negate().valueOf(), '190');
    arktest.assertEQ(new Decimal('-4.47').negate().valueOf(), '4.47');
    arktest.assertEQ(new Decimal('-0.0000000000069525').negate().valueOf(), '6.9525e-12');
    arktest.assertEQ(new Decimal('-1.3').negate().valueOf(), '1.3');
    arktest.assertEQ(new Decimal('6.21').negate().valueOf(), '-6.21');
    arktest.assertEQ(new Decimal('-2').negate().valueOf(), '2');
    arktest.assertEQ(new Decimal('1').negate().valueOf(), '-1');
    arktest.assertEQ(new Decimal('-147.857').negate().valueOf(), '147.857');
    arktest.assertEQ(new Decimal('26.517').negate().valueOf(), '-26.517');
    arktest.assertEQ(new Decimal('3').negate().valueOf(), '-3');
    arktest.assertEQ(new Decimal('-5').negate().valueOf(), '5');
    arktest.assertEQ(new Decimal('-204').negate().valueOf(), '204');
    arktest.assertEQ(new Decimal('-0.000000021').negate().valueOf(), '2.1e-8');
    arktest.assertEQ(new Decimal('-0.00000037015').negate().valueOf(), '3.7015e-7');
    arktest.assertEQ(new Decimal('50.1839').negate().valueOf(), '-50.1839');
    arktest.assertEQ(new Decimal('-44768.1').negate().valueOf(), '44768.1');
    arktest.assertEQ(new Decimal('-0.0000000000000038').negate().valueOf(), '3.8e-15');
    arktest.assertEQ(new Decimal('7.4379').negate().valueOf(), '-7.4379');
    arktest.assertEQ(new Decimal('-1.5').negate().valueOf(), '1.5');
    arktest.assertEQ(new Decimal('-6.0399').negate().valueOf(), '6.0399');
    arktest.assertEQ(new Decimal('-109.070').negate().valueOf(), '109.07');
}

function testNegatedWithDefaultConfig004() {
    arktest.assertEQ(new Decimal('-1582').negate().valueOf(), '1582');
    arktest.assertEQ(new Decimal('772').negate().valueOf(), '-772');
    arktest.assertEQ(new Decimal('0.000000000000067824').negate().valueOf(), '-6.7824e-14');
    arktest.assertEQ(new Decimal('0.00000001819').negate().valueOf(), '-1.819e-8');
    arktest.assertEQ(new Decimal('0.0000000000000030').negate().valueOf(), '-3e-15');
    arktest.assertEQ(new Decimal('424120').negate().valueOf(), '-424120');
    arktest.assertEQ(new Decimal('1814.54').negate().valueOf(), '-1814.54');
    arktest.assertEQ(new Decimal('0.00000000000000004295').negate().valueOf(), '-4.295e-17');
    arktest.assertEQ(new Decimal('5').negate().valueOf(), '-5');
    arktest.assertEQ(new Decimal('-2152').negate().valueOf(), '2152');
    arktest.assertEQ(new Decimal('-4.6').negate().valueOf(), '4.6');
    arktest.assertEQ(new Decimal('-1.9').negate().valueOf(), '1.9');
    arktest.assertEQ(new Decimal('2.0').negate().valueOf(), '-2');
    arktest.assertEQ(new Decimal('0.00036').negate().valueOf(), '-0.00036');
    arktest.assertEQ(new Decimal('0.000006962').negate().valueOf(), '-0.000006962');
    arktest.assertEQ(new Decimal('-3.6').negate().valueOf(), '3.6');
    arktest.assertEQ(new Decimal('0.000000000000011495').negate().valueOf(), '-1.1495e-14');
    arktest.assertEQ(new Decimal('312.4').negate().valueOf(), '-312.4');
    arktest.assertEQ(new Decimal('-0.00000000043').negate().valueOf(), '4.3e-10');
    arktest.assertEQ(new Decimal('-5').negate().valueOf(), '5');
    arktest.assertEQ(new Decimal('0.000000018911').negate().valueOf(), '-1.8911e-8');
    arktest.assertEQ(new Decimal('-4963.53').negate().valueOf(), '4963.53');
    arktest.assertEQ(new Decimal('0.00000000043934').negate().valueOf(), '-4.3934e-10');
    arktest.assertEQ(new Decimal('1.30').negate().valueOf(), '-1.3');
}

function testNegatedWithDefaultConfig005() {
    arktest.assertEQ(new Decimal('1.0').negate().valueOf(), '-1');
    arktest.assertEQ(new Decimal('68.32').negate().valueOf(), '-68.32');
    arktest.assertEQ(new Decimal('-0.014836').negate().valueOf(), '0.014836');
    arktest.assertEQ(new Decimal('-8').negate().valueOf(), '8');
    arktest.assertEQ(new Decimal('-2.13510').negate().valueOf(), '2.1351');
    arktest.assertEQ(new Decimal('-162224').negate().valueOf(), '162224');
    arktest.assertEQ(new Decimal('-0.00000000000000000030').negate().valueOf(), '3e-19');
    arktest.assertEQ(new Decimal('-0.00004985').negate().valueOf(), '0.00004985');
    arktest.assertEQ(new Decimal('-28.9321').negate().valueOf(), '28.9321');
    arktest.assertEQ(new Decimal('2').negate().valueOf(), '-2');
    arktest.assertEQ(new Decimal('16688').negate().valueOf(), '-16688');
    arktest.assertEQ(new Decimal('1').negate().valueOf(), '-1');
    arktest.assertEQ(new Decimal('-5').negate().valueOf(), '5');
    arktest.assertEQ(new Decimal('20.0').negate().valueOf(), '-20');
    arktest.assertEQ(new Decimal('1.9').negate().valueOf(), '-1.9');
    arktest.assertEQ(new Decimal('-3').negate().valueOf(), '3');
    arktest.assertEQ(new Decimal('-185640').negate().valueOf(), '185640');
    arktest.assertEQ(new Decimal('0.0000058').negate().valueOf(), '-0.0000058');
    arktest.assertEQ(new Decimal('-0.000000000000967').negate().valueOf(), '9.67e-13');
    arktest.assertEQ(new Decimal('707.98').negate().valueOf(), '-707.98');
    arktest.assertEQ(new Decimal('-2.57917').negate().valueOf(), '2.57917');
}

function testNegatedWithDefaultConfig006() {
    arktest.assertEQ(new Decimal('1.3').negate().valueOf(), '-1.3');
    arktest.assertEQ(new Decimal('4.2655').negate().valueOf(), '-4.2655');
    arktest.assertEQ(new Decimal('149.6').negate().valueOf(), '-149.6');
    arktest.assertEQ(new Decimal('1.32383').negate().valueOf(), '-1.32383');
    arktest.assertEQ(new Decimal('26.925').negate().valueOf(), '-26.925');
    arktest.assertEQ(new Decimal('0.00013').negate().valueOf(), '-0.00013');
    arktest.assertEQ(new Decimal('6868').negate().valueOf(), '-6868');
    arktest.assertEQ(new Decimal('-7').negate().valueOf(), '7');
    arktest.assertEQ(new Decimal('0.0000000050').negate().valueOf(), '-5e-9');
    arktest.assertEQ(new Decimal('-0.00000000000000032555').negate().valueOf(), '3.2555e-16');
    arktest.assertEQ(new Decimal('-0.000000000000142768').negate().valueOf(), '1.42768e-13');
    arktest.assertEQ(new Decimal('-11.2962').negate().valueOf(), '11.2962');
    arktest.assertEQ(new Decimal('-3186.7').negate().valueOf(), '3186.7');
    arktest.assertEQ(new Decimal('6.9').negate().valueOf(), '-6.9');
    arktest.assertEQ(new Decimal('0.00000062618').negate().valueOf(), '-6.2618e-7');
    arktest.assertEQ(new Decimal('-8').negate().valueOf(), '8');
    arktest.assertEQ(new Decimal('8.04').negate().valueOf(), '-8.04');
    arktest.assertEQ(new Decimal('22').negate().valueOf(), '-22');
    arktest.assertEQ(new Decimal('750.6').negate().valueOf(), '-750.6');
    arktest.assertEQ(new Decimal('-12.803').negate().valueOf(), '12.803');
}

function testNegatedWithDefaultConfig007() {
    arktest.assertEQ(new Decimal('20513.4').negate().valueOf(), '-20513.4');
    arktest.assertEQ(new Decimal('-114781').negate().valueOf(), '114781');
    arktest.assertEQ(new Decimal('16.9046').negate().valueOf(), '-16.9046');
    arktest.assertEQ(new Decimal('-0.00000046').negate().valueOf(), '4.6e-7');
    arktest.assertEQ(new Decimal('31399').negate().valueOf(), '-31399');
    arktest.assertEQ(new Decimal('-1.04').negate().valueOf(), '1.04');
    arktest.assertEQ(new Decimal('51.2544').negate().valueOf(), '-51.2544');
    arktest.assertEQ(new Decimal('-0.000000000000001023').negate().valueOf(), '1.023e-15');
    arktest.assertEQ(new Decimal('-281').negate().valueOf(), '281');
    arktest.assertEQ(new Decimal('128315').negate().valueOf(), '-128315');
    arktest.assertEQ(new Decimal('-20.2').negate().valueOf(), '20.2');
    arktest.assertEQ(new Decimal('-9').negate().valueOf(), '9');
    arktest.assertEQ(new Decimal('10').negate().valueOf(), '-10');
    arktest.assertEQ(new Decimal('0.0000000000000000192262').negate().valueOf(), '-1.92262e-17');
    arktest.assertEQ(new Decimal('0.0023').negate().valueOf(), '-0.0023');
    arktest.assertEQ(new Decimal('-5').negate().valueOf(), '5');
    arktest.assertEQ(new Decimal('-7').negate().valueOf(), '7');
    arktest.assertEQ(new Decimal('-13.72').negate().valueOf(), '13.72');
    arktest.assertEQ(new Decimal('-98068').negate().valueOf(), '98068');
    arktest.assertEQ(new Decimal('-3.2').negate().valueOf(), '3.2');
    arktest.assertEQ(new Decimal('-1.1').negate().valueOf(), '1.1');
    arktest.assertEQ(new Decimal('0.000000000000000003970').negate().valueOf(), '-3.97e-18');
    arktest.assertEQ(new Decimal('-0.00334824').negate().valueOf(), '0.00334824');
    arktest.assertEQ(new Decimal('0.000000054892').negate().valueOf(), '-5.4892e-8');
    arktest.assertEQ(new Decimal('1.0').negate().valueOf(), '-1');
    arktest.assertEQ(new Decimal('0.000000028135').negate().valueOf(), '-2.8135e-8');
}

function testNegatedWithDefaultConfig008() {
    arktest.assertEQ(new Decimal('0.0000000000001816').negate().valueOf(), '-1.816e-13');
    arktest.assertEQ(new Decimal('-199724').negate().valueOf(), '199724');
    arktest.assertEQ(new Decimal('19.40').negate().valueOf(), '-19.4');
    arktest.assertEQ(new Decimal('12.74').negate().valueOf(), '-12.74');
    arktest.assertEQ(new Decimal('2171.8').negate().valueOf(), '-2171.8');
    arktest.assertEQ(new Decimal('2.7').negate().valueOf(), '-2.7');
    arktest.assertEQ(new Decimal('-1.0').negate().valueOf(), '1');
    arktest.assertEQ(new Decimal('-21779').negate().valueOf(), '21779');
    arktest.assertEQ(new Decimal('-0.0000000000089').negate().valueOf(), '8.9e-12');
    arktest.assertEQ(new Decimal('4.51').negate().valueOf(), '-4.51');
    arktest.assertEQ(new Decimal('-2.6').negate().valueOf(), '2.6');
    arktest.assertEQ(new Decimal('0.000160').negate().valueOf(), '-0.00016');
    arktest.assertEQ(new Decimal('-6').negate().valueOf(), '6');
    arktest.assertEQ(new Decimal('-50.566').negate().valueOf(), '50.566');
    arktest.assertEQ(new Decimal('16.2').negate().valueOf(), '-16.2');
    arktest.assertEQ(new Decimal('9444').negate().valueOf(), '-9444');
    arktest.assertEQ(new Decimal('-21.4').negate().valueOf(), '21.4');
    arktest.assertEQ(new Decimal('-2.5').negate().valueOf(), '2.5');
    arktest.assertEQ(new Decimal('-489311').negate().valueOf(), '489311');
    arktest.assertEQ(new Decimal('-6.8').negate().valueOf(), '6.8');
    arktest.assertEQ(new Decimal('-4.29').negate().valueOf(), '4.29');
    arktest.assertEQ(new Decimal('-23982.0').negate().valueOf(), '23982');
    arktest.assertEQ(new Decimal('0.0111781').negate().valueOf(), '-0.0111781');
    arktest.assertEQ(new Decimal('-0.0000000000000000000496').negate().valueOf(), '4.96e-20');
    arktest.assertEQ(new Decimal('40.5481').negate().valueOf(), '-40.5481');
    arktest.assertEQ(new Decimal('32.52').negate().valueOf(), '-32.52');
    arktest.assertEQ(new Decimal('7.4').negate().valueOf(), '-7.4');
    arktest.assertEQ(new Decimal('5.0').negate().valueOf(), '-5');
    arktest.assertEQ(new Decimal('2463.4').negate().valueOf(), '-2463.4');
    arktest.assertEQ(new Decimal('-7.363').negate().valueOf(), '7.363');
    arktest.assertEQ(new Decimal('-2.8').negate().valueOf(), '2.8');
    arktest.assertEQ(new Decimal('14498').negate().valueOf(), '-14498');
    arktest.assertEQ(new Decimal('-201').negate().valueOf(), '201');
}

function testNegatedWithSpecificConfig() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: 0, toExpPos: 0, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal('5.0600621890668482322956892808849303e+20').negate().valueOf(), '-5.0600621890668482322956892808849303e+20');
    arktest.assertEQ(new Decimal('-7e+0').negate().valueOf(), '7e+0');
    arktest.assertEQ(new Decimal('6.1095374220609e+13').negate().valueOf(), '-6.1095374220609e+13');
    arktest.assertEQ(new Decimal('-9.01e+2').negate().valueOf(), '9.01e+2');
    arktest.assertEQ(new Decimal('1.016984074247269470395836690098169093010136836967e+39').negate().valueOf(), '-1.016984074247269470395836690098169093010136836967e+39');
    arktest.assertEQ(new Decimal('1.497639134680472576e+18').negate().valueOf(), '-1.497639134680472576e+18');
    arktest.assertEQ(new Decimal('4.1717657571404248e+16').negate().valueOf(), '-4.1717657571404248e+16');
    arktest.assertEQ(new Decimal('-8.983272e+1').negate().valueOf(), '8.983272e+1');
    arktest.assertEQ(new Decimal('5.308416e+6').negate().valueOf(), '-5.308416e+6');
    arktest.assertEQ(new Decimal('2.09764e+3').negate().valueOf(), '-2.09764e+3');
    arktest.assertEQ(new Decimal('3.83432050166120236679168e+23').negate().valueOf(), '-3.83432050166120236679168e+23');
    arktest.assertEQ(new Decimal('4.096e+3').negate().valueOf(), '-4.096e+3');
    arktest.assertEQ(new Decimal('-2.679971527468745095582058350756311201706813294321409e+51').negate().valueOf(), '2.679971527468745095582058350756311201706813294321409e+51');
    arktest.assertEQ(new Decimal('5.067853299870089529116832768e+2').negate().valueOf(), '-5.067853299870089529116832768e+2');
    arktest.assertEQ(new Decimal('3.48822062687911109850066182676769e+32').negate().valueOf(), '-3.48822062687911109850066182676769e+32');
    arktest.assertEQ(new Decimal('1e+0').negate().valueOf(), '-1e+0');
    arktest.assertEQ(new Decimal('-4.2773e+0').negate().valueOf(), '4.2773e+0');
    arktest.assertEQ(new Decimal('-5.8169306081172252508071119604378757744768e+12').negate().valueOf(), '5.8169306081172252508071119604378757744768e+12');
    arktest.assertEQ(new Decimal('1e+0').negate().valueOf(), '-1e+0');
    arktest.assertEQ(new Decimal('-1.51655708279450944384385164853883404204414169862685507e+46').negate().valueOf(), '1.51655708279450944384385164853883404204414169862685507e+46');
    arktest.assertEQ(new Decimal('8.1e+1').negate().valueOf(), '-8.1e+1');
    arktest.assertEQ(new Decimal('1.296e+3').negate().valueOf(), '-1.296e+3');
    arktest.assertEQ(new Decimal('2.9e+0').negate().valueOf(), '-2.9e+0');
    arktest.assertEQ(new Decimal('1.764e+3').negate().valueOf(), '-1.764e+3');
    arktest.assertEQ(new Decimal('-9.3418332730097368870513138581415704704611459349313e+49').negate().valueOf(), '9.3418332730097368870513138581415704704611459349313e+49');
}
