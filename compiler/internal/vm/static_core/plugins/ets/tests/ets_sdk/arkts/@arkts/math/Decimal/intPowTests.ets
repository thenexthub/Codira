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
    suite.addTest("Decimal pow with specific config", testIntPowWithSpecificConfig001);
    suite.addTest("Decimal pow with specific config", testIntPowWithSpecificConfig002);
    suite.addTest("Decimal pow with specific config", testIntPowWithSpecificConfig003);
    suite.addTest("Decimal pow with specific config", testIntPowWithPrecisionIs600);
    return suite.run();
}

function testIntPowWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(2).pow(2).valueOf(), '4');
    arktest.assertEQ(new Decimal(2).pow(31).valueOf(), '2147483648');
    arktest.assertEQ(new Decimal(2).pow(-2).valueOf(), '0.25');
    arktest.assertEQ(new Decimal(2).pow(-4).valueOf(), '0.0625');
    arktest.assertEQ(new Decimal(1).pow(100).valueOf(), '1');
    arktest.assertEQ(new Decimal(0).pow(1000).valueOf(), '0');
    arktest.assertEQ(new Decimal(2).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(+0).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-0).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(Infinity).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-Infinity).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(+0).valueOf(), '1');
    arktest.assertEQ(new Decimal(NaN).pow(-0).valueOf(), '1');
    arktest.assertEQ(new Decimal(NaN).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(2.2).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(-1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(-2.2).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(Infinity).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).pow(-Infinity).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(1.1).pow(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-1.1).pow(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(2).pow(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-2).pow(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(2).pow(-Infinity).valueOf(), '0');
    arktest.assertEQ(new Decimal(-2).pow(-Infinity).valueOf(), '0');
    arktest.assertEQ(new Decimal(1/1.1).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(1/-1.1).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(1/2).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(1/-2).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(0.1).pow(Infinity).valueOf(), '0');
    arktest.assertEQ(new Decimal(-0.1).pow(Infinity).valueOf(), '0');
    arktest.assertEQ(new Decimal(0.999).pow(Infinity).valueOf(), '0');
    arktest.assertEQ(new Decimal(-0.999).pow(Infinity).valueOf(), '0');
    arktest.assertEQ(new Decimal(0.1).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-0.1).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(0.999).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-0.999).pow(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(Infinity).pow(2).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(1/Infinity).pow(-2).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).pow(2).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).pow(3).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(-Infinity).pow(13).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('-0').pow(-3).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(1/-Infinity).pow(-2).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(+0).pow(-1.1).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(+0).pow(-2).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-0).pow(-2).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-0.00001).pow(1.1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-0.00001).pow(-1.1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(2).pow(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-0).pow(-0.5).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-0).pow(-0.6).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(Infinity).pow(0.1).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(1/Infinity).pow(-0.1).valueOf(), 'Infinity');
}

function testIntPowWithSpecificConfig002() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(2).pow(1e6 + 1).valueOf(), '1.9801312458591796501e+301030');
    arktest.assertEQ(new Decimal(2).pow(-1e6 - 1).valueOf(), '5.0501702959901511235e-301031');
    arktest.assertEQ(new Decimal(-2).pow(1e6 + 1).valueOf(), '-1.9801312458591796501e+301030');
    arktest.assertEQ(new Decimal(-2).pow(-1e6 - 1).valueOf(), '-5.0501702959901511235e-301031');
    arktest.assertEQ(new Decimal(-0.2).pow(1e6 + 1).valueOf(), '-1.9801312458591796501e-698971');
    arktest.assertEQ(new Decimal(-0.2).pow(-1e6 - 1).valueOf(), '-5.0501702959901511235e+698970');
    arktest.assertEQ(new Decimal(0.2).pow(1e6 + 1).valueOf(), '1.9801312458591796501e-698971');
    arktest.assertEQ(new Decimal(0.2).pow(-1e6 - 1).valueOf(), '5.0501702959901511235e+698970');
}

function testIntPowWithSpecificConfig003() {
    Decimal.set({ precision: 600, rounding: 4, toExpNeg: 0, toExpPos: 0, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(2).pow('1.0').valueOf(), '2e+0');
    arktest.assertEQ(new Decimal(2).pow('4.00000000').valueOf(), '1.6e+1');
    arktest.assertEQ(new Decimal(2).pow(-4).valueOf(), '6.25e-2');
    arktest.assertEQ(new Decimal('907.27').pow(7).valueOf(), '5.0600621890668482322956892808849303e+20');
    arktest.assertEQ(new Decimal('-7').pow(1).valueOf(), '-7e+0');
    arktest.assertEQ(new Decimal('-901').pow(1).valueOf(), '-9.01e+2');
    arktest.assertEQ(new Decimal('21584.7').pow(9).valueOf(), '1.016984074247269470395836690098169093010136836967e+39');
    arktest.assertEQ(new Decimal('-89.83272').pow(1).valueOf(), '-8.983272e+1');
    arktest.assertEQ(new Decimal('-48').pow(4).valueOf(), '5.308416e+6');
    arktest.assertEQ(new Decimal('52088').pow(5).valueOf(), '3.83432050166120236679168e+23');
    arktest.assertEQ(new Decimal('-517889').pow(9).valueOf(), '-2.679971527468745095582058350756311201706813294321409e+51');
    arktest.assertEQ(new Decimal('3.47508').pow(5).valueOf(), '5.067853299870089529116832768e+2');
    arktest.assertEQ(new Decimal('4129').pow(9).valueOf(), '3.48822062687911109850066182676769e+32');
    arktest.assertEQ(new Decimal('-429.32321').pow(0).valueOf(), '1e+0');
    arktest.assertEQ(new Decimal('-4.2773').pow(1).valueOf(), '-4.2773e+0');
    arktest.assertEQ(new Decimal('-66.6082').pow(7).valueOf(), '-5.8169306081172252508071119604378757744768e+12');
    arktest.assertEQ(new Decimal('-7.0654').pow(0).valueOf(), '1e+0');
    arktest.assertEQ(new Decimal('-3956084.3').pow(7).valueOf(), '-1.51655708279450944384385164853883404204414169862685507e+46');
    arktest.assertEQ(new Decimal('-3').pow(4).valueOf(), '8.1e+1');
    arktest.assertEQ(new Decimal('-6').pow(4).valueOf(), '1.296e+3');
    arktest.assertEQ(new Decimal('2.9').pow(1).valueOf(), '2.9e+0');
    arktest.assertEQ(new Decimal('-42').pow(2).valueOf(), '1.764e+3');
    arktest.assertEQ(new Decimal('-356673').pow(9).valueOf(), '-9.3418332730097368870513138581415704704611459349313e+49');
    arktest.assertEQ(new Decimal('79').pow(8).valueOf(), '1.517108809906561e+15');
}

function testIntPowWithPrecisionIs600() {
    Decimal.set({ precision: 600, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal('8').pow(4).valueOf(), '4096');
    arktest.assertEQ(new Decimal('-1.1').pow(3).valueOf(), '-1.331');
    arktest.assertEQ(new Decimal('-2.264').pow(2).valueOf(), '5.125696');
    arktest.assertEQ(new Decimal('6.7266E-7').pow(1).valueOf(), '6.7266e-7');
    arktest.assertEQ(new Decimal('21.06').pow(5).valueOf(), '4142779.6499215776');
    arktest.assertEQ(new Decimal('-5.2').pow(4).valueOf(), '731.1616');
    arktest.assertEQ(new Decimal('61818').pow(0).valueOf(), '1');
    arktest.assertEQ(new Decimal('3.2').pow(1).valueOf(), '3.2');
    arktest.assertEQ(new Decimal('-1.945E-7').pow(6).valueOf(), '5.4139923025768140625e-41');
    arktest.assertEQ(new Decimal('5.8').pow(8).valueOf(), '1280630.81718016');
    arktest.assertEQ(new Decimal('15.828').pow(3).valueOf(), '3965.318943552');
    arktest.assertEQ(new Decimal('53.721').pow(1).valueOf(), '53.721');
    arktest.assertEQ(new Decimal('-1.9').pow(0).valueOf(), '1');
    arktest.assertEQ(new Decimal('-0.0000161306').pow(8).valueOf(), '4.58357323731267363492522744606954913014016e-39');
    arktest.assertEQ(new Decimal('-96.94').pow(5).valueOf(), '-8560814308.6108448224');
    arktest.assertEQ(new Decimal('-6.77E-10').pow(8).valueOf(), '4.4127502627834341562081e-74');
    arktest.assertEQ(new Decimal('23.0371').pow(8).valueOf(), '79327261142.56790234534719652175488087744161');
    arktest.assertEQ(new Decimal('1.761E-19').pow(2).valueOf(), '3.101121e-38');
    arktest.assertEQ(new Decimal('-1').pow(4).valueOf(), '1');
    arktest.assertEQ(new Decimal('-1.58E-7').pow(4).valueOf(), '6.23201296e-28');
    arktest.assertEQ(new Decimal('-9.476E-7').pow(3).valueOf(), '-8.50893402176e-19');
    arktest.assertEQ(new Decimal('4').pow(2).valueOf(), '16');
    arktest.assertEQ(new Decimal('-97.5').pow(4).valueOf(), '90368789.0625');
    arktest.assertEQ(new Decimal('-112.8').pow(0).valueOf(), '1');
    arktest.assertEQ(new Decimal('40042.5').pow(6).valueOf(), '4.122181458338334221291398681640625e+27');
    arktest.assertEQ(new Decimal('1290.0').pow(7).valueOf(), '5.94467302491009e+21');
    arktest.assertEQ(new Decimal('141437').pow(5).valueOf(), '5.6599836943004175019970957e+25');
}
