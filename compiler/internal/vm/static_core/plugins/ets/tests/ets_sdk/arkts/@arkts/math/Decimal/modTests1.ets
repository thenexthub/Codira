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
    const suite = new arktest.ArkTestsuite("Decimal mod API tests");
    suite.addTest("Decimal mod with specific set", testModWithSpecificConfig001);
    suite.addTest("Decimal mod with specific set", testModWithSpecificConfig002);
    suite.addTest("Decimal mod with specific set", testModWithSpecificConfig003);
    suite.addTest("Decimal mod with specific set", testModWithSpecificConfig004);
    suite.addTest("Decimal mod with specific set", testModWithSpecificConfig005);
    suite.addTest("Decimal mod with specific set", testModWithSpecificConfig006);
    suite.addTest("Decimal mod with specific set", testModWithMoreConfig001);
    suite.addTest("Decimal mod with specific set", testModWithMoreConfig002);
    suite.addTest("Decimal mod with specific set", testModWithMoreConfig003);
    suite.addTest("Decimal mod with specific set", testModWithMoreConfig004);
    suite.addTest("Decimal mod with different precision and rounding", testModWithDiffPRandRM);
    return suite.run();
}

function checkModResult(a: string, b: string, expected: string, sd: double, rm: int) {
    Decimal.set({ precision: sd, rounding: rm, toExpNeg: 0, toExpPos: 0, minE: -9e15, maxE: 9e15, modulo: 1 });
    arktest.assertEQ(new Decimal(a).mod(b).valueOf(), expected);
}

function testModWithSpecificConfig001() {
    Decimal.set({ precision: 400, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15, modulo: 1 });
    arktest.assertEQ(new Decimal(1).mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(1).mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-1).mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-1).mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(1).mod('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-1).mod('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(1).mod('Infinity').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('-Infinity').valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).mod('Infinity').valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1).mod('-Infinity').valueOf(), '-1');
    arktest.assertEQ(new Decimal(0).mod(1).valueOf(), '0');
    arktest.assertEQ(new Decimal(0).mod(-1).valueOf(), '0');
    arktest.assertEQ(new Decimal(0).mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(0).mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-0).mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-0).mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(0).mod(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-0).mod(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(0).mod(Infinity).valueOf(), '0');
}

function testModWithSpecificConfig002() {
    arktest.assertEQ(new Decimal('NaN').mod(1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod(-1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod('Infinity').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod('-Infinity').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod(1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod(-1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod(1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod(-1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod('Infinity').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod('-Infinity').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod('Infinity').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod('-Infinity').valueOf(), 'NaN');
}

function testModWithSpecificConfig003() {
    arktest.assertEQ(new Decimal(1).mod('0').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(1).mod('1').valueOf(), '0');
    arktest.assertEQ(new Decimal(1).mod('-45').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('22').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(0o144).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('0144').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('6.1915').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('-1.02').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('-1.02').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(2).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(-2).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).mod(2).valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1).mod(-2).valueOf(), '-1');
    arktest.assertEQ(new Decimal( 1).mod(-0.1).valueOf(), '0');
    arktest.assertEQ(new Decimal(-1).mod(-0.1).valueOf(), '0');
    arktest.assertEQ(new Decimal( 2).mod(-0.1).valueOf(), '0');
    arktest.assertEQ(new Decimal(-2).mod(-0.1).valueOf(), '0');
    arktest.assertEQ(new Decimal( 2).mod(-0.5).valueOf(), '0');
    arktest.assertEQ(new Decimal(-2).mod(-0.5).valueOf(), '0');
    arktest.assertEQ(new Decimal(999.99).mod(-0.99).valueOf(), '0.09');
    arktest.assertEQ(new Decimal(-999.99).mod(-0.99).valueOf(), '-0.09');
    arktest.assertEQ(new Decimal(999.99).mod('3.01').valueOf(), '0.67');
    arktest.assertEQ(new Decimal(-999.99).mod('3.01').valueOf(), '-0.67');
}

function testModWithSpecificConfig004() {
    arktest.assertEQ(new Decimal(1).mod('0.09').valueOf(), '0.01');
    arktest.assertEQ(new Decimal(1).mod('-0.0001').valueOf(), '0');
    arktest.assertEQ(new Decimal('-0').mod('-1').valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').mod(Infinity).valueOf(), '-0');
    arktest.assertEQ(new Decimal(1).mod('1e-14').valueOf(), '0');
    arktest.assertEQ(new Decimal(1).mod('8e5').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('9E12').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('3.345E-9').valueOf(), '6.1e-10');
    arktest.assertEQ(new Decimal(1).mod('-345.43e+4').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod('-94.12E+0').valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(Number.POSITIVE_INFINITY).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(Number.NEGATIVE_INFINITY).valueOf(), '1');
    arktest.assertEQ(new Decimal('0').mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('0').mod('0').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(3).mod(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(9.654).mod(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(0).mod('0.001').valueOf(), '0');
    arktest.assertEQ(new Decimal(0).mod('111.1111111110000').valueOf(), '0');
    arktest.assertEQ(new Decimal('NaN').mod('0').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-1).mod(1).valueOf(), '0');
    arktest.assertEQ(new Decimal(-0.01).mod(0.01).valueOf(), '0');
    arktest.assertEQ(new Decimal(54).mod(-54).valueOf(), '0');
    arktest.assertEQ(new Decimal(9.99).mod('-9.99').valueOf(), '0');
    arktest.assertEQ(new Decimal('0.0000023432495704937').mod('-0.0000023432495704937').valueOf(), '0');
}

function testModWithSpecificConfig005() {
    arktest.assertEQ(new Decimal(NaN).mod(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).mod('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod(4).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod('4534534.45435435').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').mod(99999.999).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(Infinity).mod('354.345341').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(3).mod('-Infinity').valueOf(), '3');
    arktest.assertEQ(new Decimal(-Infinity).mod('-Infinity').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-Infinity').mod(-Infinity).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').mod('-999e999').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('1.21123e43').mod('-Infinity').valueOf(), '1.21123e+43');
    arktest.assertEQ(new Decimal('-999.0').mod('Infinity').valueOf(), '-999');
    arktest.assertEQ(new Decimal('657.342e-45').mod('-Infinity').valueOf(), '6.57342e-43');
    arktest.assertEQ(new Decimal('Infinity').mod(123).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(100).mod(100).valueOf(), '0');
    arktest.assertEQ(new Decimal('03.333').mod(-4).valueOf(), '3.333');
    arktest.assertEQ(new Decimal(43534.5435).mod('0.054645').valueOf(), '0.019545');
    arktest.assertEQ(new Decimal('99999').mod('1').valueOf(), '0');
    arktest.assertEQ(new Decimal('-0.111').mod('21').valueOf(), '-0.111');
    arktest.assertEQ(new Decimal('0.000621').mod('-1').valueOf(), '0.000621');
    arktest.assertEQ(new Decimal('6.41').mod('11').valueOf(), '6.41');
    arktest.assertEQ(new Decimal('21').mod('11').valueOf(), '10');
    arktest.assertEQ(new Decimal('1531').mod('-11').valueOf(), '2');
    arktest.assertEQ(new Decimal('21').mod('-0.0161').valueOf(), '0.0056');
    arktest.assertEQ(new Decimal('-41').mod('-21').valueOf(), '-20');
    arktest.assertEQ(new Decimal('31').mod('-1').valueOf(), '0');
}

function testModWithSpecificConfig006() {
    arktest.assertEQ(new Decimal('31').mod('2.21').valueOf(), '0.06');
    arktest.assertEQ(new Decimal('-842658.56').mod('-19.52').valueOf(), '-19.2');
    arktest.assertEQ(new Decimal('-34.871').mod('-7325043345').valueOf(), '-34.871');
    arktest.assertEQ(new Decimal('78691958').mod('49006.018397').valueOf(), '37298.472815');
    arktest.assertEQ(new Decimal('-11618324684').mod('621.3').valueOf(), '-394.1');
    arktest.assertEQ(new Decimal('0').mod('69.07362125').valueOf(), '0');
    arktest.assertEQ(new Decimal('0.27474921216').mod('0.00000000002460017868').valueOf(), '2.224673016e-11');
    arktest.assertEQ(new Decimal('0').mod('59137483772').valueOf(), '0');
    arktest.assertEQ(new Decimal('-0.000000000000000000014634493').mod('-4.2').valueOf(), '-1.4634493e-20');
    arktest.assertEQ(new Decimal('-5.470').mod('-1.8989').valueOf(), '-1.6722');
    arktest.assertEQ(new Decimal('-95519.5319132').mod('4.1').valueOf(), '-1.8319132');
    arktest.assertEQ(new Decimal('43.257').mod('-4704608304').valueOf(), '43.257');
    arktest.assertEQ(new Decimal('-32128.403').mod('68263.11').valueOf(), '-32128.403');
    arktest.assertEQ(new Decimal('85.4005884').mod('-0.000000000000000010953667').valueOf(), '4.132633e-18');
    arktest.assertEQ(new Decimal('3197.909').mod('-10738864.39').valueOf(), '3197.909');
    arktest.assertEQ(new Decimal('1.03436722695').mod('2.6').valueOf(), '1.03436722695');
    arktest.assertEQ(new Decimal('-4').mod('-56.36464628').valueOf(), '-4');
    arktest.assertEQ(new Decimal('92.0618').mod('-454.866').valueOf(), '92.0618');
    arktest.assertEQ(new Decimal('-6.147123243').mod('-53259.9').valueOf(), '-6.147123243');
    arktest.assertEQ(new Decimal('-3').mod('-0.4911').valueOf(), '-0.0534');
    arktest.assertEQ(new Decimal('0').mod('-12.3').valueOf(), '0');
    arktest.assertEQ(new Decimal('-0.001713264153').mod('0.00000000000000000001843605').valueOf(), '-1.36779e-20');
    arktest.assertEQ(new Decimal('2').mod('-9325.1665961').valueOf(), '2');
    arktest.assertEQ(new Decimal('42016273.6500').mod('-122.861496017').valueOf(), '99.24210634');
    arktest.assertEQ(new Decimal('35.9').mod('3867744417').valueOf(), '35.9');
    arktest.assertEQ(new Decimal('0.000000000000000479760268839').mod('-209975.082').valueOf(), '4.79760268839e-16');
    arktest.assertEQ(new Decimal('0.0000000000000000000428713848').mod('3').valueOf(), '4.28713848e-20');
    arktest.assertEQ(new Decimal('0.000000000000000104698757876').mod('-15902.7654').valueOf(), '1.04698757876e-16');
}

function testModWithMoreConfig001() {
    Decimal.set({ precision: 400, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15, modulo: 1 });
    arktest.assertEQ(new Decimal(8).mod(3).valueOf(), '2');
    arktest.assertEQ(new Decimal(8).mod(-3).valueOf(), '2');
    arktest.assertEQ(new Decimal(-8).mod(3).valueOf(), '-2');
    arktest.assertEQ(new Decimal(-8).mod(-3).valueOf(), '-2');
    arktest.assertEQ(new Decimal(1).mod(2).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(-2).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).mod(2).valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1).mod(-2).valueOf(), '-1');
    arktest.assertEQ(new Decimal(34).mod(5).valueOf(), '4');
    arktest.assertEQ(new Decimal(34).mod(-5).valueOf(), '4');
    arktest.assertEQ(new Decimal(-34).mod(5).valueOf(), '-4');
    arktest.assertEQ(new Decimal(-34).mod(-5).valueOf(), '-4');
}

function testModWithMoreConfig002() {
    Decimal.set({ precision: 400, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15, modulo: 3 });
    arktest.assertEQ(new Decimal(8).mod(3).valueOf(), '2');
    arktest.assertEQ(new Decimal(8).mod(-3).valueOf(), '-1');
    arktest.assertEQ(new Decimal(-8).mod(3).valueOf(), '1');
    arktest.assertEQ(new Decimal(-8).mod(-3).valueOf(), '-2');
    arktest.assertEQ(new Decimal(1).mod(2).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(-2).valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1).mod(2).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).mod(-2).valueOf(), '-1');
    arktest.assertEQ(new Decimal(34).mod(5).valueOf(), '4');
    arktest.assertEQ(new Decimal(34).mod(-5).valueOf(), '-1');
    arktest.assertEQ(new Decimal(-34).mod(5).valueOf(), '1');
    arktest.assertEQ(new Decimal(-34).mod(-5).valueOf(), '-4');
}

function testModWithMoreConfig003() {
    Decimal.set({ precision: 400, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15, modulo: 9 });
    arktest.assertEQ(new Decimal(8).mod(3).valueOf(), '2');
    arktest.assertEQ(new Decimal(8).mod(-3).valueOf(), '2');
    arktest.assertEQ(new Decimal(-8).mod(3).valueOf(), '1');
    arktest.assertEQ(new Decimal(-8).mod(-3).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(2).valueOf(), '1');
    arktest.assertEQ(new Decimal(1).mod(-2).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).mod(2).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).mod(-2).valueOf(), '1');
    arktest.assertEQ(new Decimal(34).mod(5).valueOf(), '4');
    arktest.assertEQ(new Decimal(34).mod(-5).valueOf(), '4');
    arktest.assertEQ(new Decimal(-34).mod(5).valueOf(), '1');
    arktest.assertEQ(new Decimal(-34).mod(-5).valueOf(), '1');
}

function testModWithMoreConfig004() {
    Decimal.set({ precision: 400, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15, modulo: 6 });
    arktest.assertEQ(new Decimal(3).mod(2).valueOf(), '-1');
    arktest.assertEQ(new Decimal(4).mod(2).valueOf(), '0');
    arktest.assertEQ(new Decimal(10).mod(3).valueOf(), '1');
    arktest.assertEQ(new Decimal(11).mod(3).valueOf(), '-1');
    arktest.assertEQ(new Decimal(27).mod(4).valueOf(), '-1');
    arktest.assertEQ(new Decimal(28).mod(5).valueOf(), '-2');
    arktest.assertEQ(new Decimal(17.8).mod(4).valueOf(), '1.8');
    arktest.assertEQ(new Decimal(17.8).mod(4.1).valueOf(), '1.4');
    arktest.assertEQ(new Decimal(-16.3).mod(4.1).valueOf(), '0.1');
    arktest.assertEQ(new Decimal(17.8).mod(-4.1).valueOf(), '1.4');
    arktest.assertEQ(new Decimal(-17.8).mod(-4.1).valueOf(), '-1.4');
}

function testModWithDiffPRandRM() {
    checkModResult('-8.943550875826899174004147098160947209117401e+112', '8795986955015178649130170399228023.855728306', '-8.254602310669191899712579731892356266536504e+33', 57, 3);
    checkModResult('-2.736538371967866031325075682600169801042080643176906e+67', '-798.10121', '-5.1093943e+2', 15, 6);
    checkModResult('8.89244411964474967575e+995', '-1.22396282949406136970691396606e+143', '2.1076810292497291230174828448e+142', 37, 5);
    checkModResult('-4.133853260833105676637834626304395288e+331', '-4151623665.00637323381006132759572375', '-1.953e+9', 4, 0);
    checkModResult('-4.98716329433101e+63', '5.2324163e+989', '-4.98716329433101e+63', 42, 3);
    checkModResult('-1.21401788871257659e+26', '7606.68304751112951636636391962372647031362684321', '-3.8282851383973410080190582059122127821019964721e+3', 47, 4);
    checkModResult('5.098420853880029521477e+9004', '7.160182680398e+769', '6.109129834846e+769', 93, 4);
    checkModResult('38972.7', '8.884699481786844829e+413', '3.89727e+4', 31, 3);
    checkModResult('6114.6135258543884127880811426762677209603105075061581615', '3.0606472374614422625444168825327291936017622710500e+275', '6.1146135258543884127880811426762677209603105075061581615e+3', 98, 4);
    checkModResult('9.44899733642716929280554042413664887e+67', '3365420322.33466499588376037066560633919203780807168', '3.14018804773158888457522038228739693507536858699776e+9', 54, 4);
    checkModResult('4.649378943629298120865999754037420765850004956622433023174e+987', '-8.06433839e+21', '7.10669696e+21', 44, 4);
    checkModResult('-6.99689242488633036641737497911e+337', '-8.62087725552e+511', '-6.99689242488633036641737497911e+337', 39, 6);
    checkModResult('2.68888209327808354367202351104495e+405', '-8819404.2382061121909378683556', '1.1144088192608904221647532924e+6', 34, 0);
    checkModResult('95.999733394918432190664281885062439232831974613845764432', '260016173581199901254971273211250760375409527201652153.674', '9.5999733394918432190664281885062439232831974613845764432e+1', 79, 2);
    checkModResult('-65183.1712462898879902640839124139318475', '6.256224403276153e+18', '-6.518317124628988799026408391241393185e+4', 37, 3);
    checkModResult('6.73109e+5632', '6.2e+5', '5.4e+5', 62, 4);
    checkModResult('5.02028141167363618090367152620443275143849821e+65', '-5.1423741815608532540616250447301058274773391073e+397', '5.020281411673636180903672e+65', 25, 0);
    checkModResult('-5.92043551e+148', '-4.28952447049490303828404027555378e+6513', '-5.92043551e+148', 52, 3);
    checkModResult('-7.776298593803313194202564e+91', '-8.65584352736057897977696414850809690748461947332009869e+121', '-7.776298593803313194202564e+91', 58, 5);
    checkModResult('6.0468446816579233067128095e+47', '-9.25703241665044447133e+232', '6.0468446816579233067128095e+47', 61, 6);
    checkModResult('-2.8596931954822549177171791e+65', '-2842388394.3157048684464', '-2.23925487e+9', 9, 5);
    checkModResult('6.049098770167682103e+61', '6.62504441855149114634843111447292655983275898565177681129e+549', '6.049098770167682103e+61', 77, 5);
    checkModResult('-3.1766317506888756886345e+6581', '-1.02395581038358273020004954944197227e+8632', '-3e+6581', 1, 5);
    checkModResult('-6.494537598665515278622730e+407', '-3.693271952093896608738805402952058e+593', '-6.49453759866551527862273e+407', 55, 4);
    checkModResult('-54570662.9152087959960116', '3628345183.695935530586261340756849', '-5.4570662915209e+7', 14, 5);
    checkModResult('4.0937805605767803276628345776693834523568082109865317580e+7653', '4129.795194087170', '9.9197199795059e+2', 78, 0);
    checkModResult('19716777.12985866079587414603535351339087353731317', '4.15268790332934991803928616e+612', '1.971677713e+7', 10, 5);
    checkModResult('-653510.8843942609262130842269762944441504401', '-2.027688666442884256244726305622087828395505875189979280284e+936', '-6.535108843943e+5', 13, 3);
    checkModResult('-5.1464720662639698186031461962218221108584708551990576e+426', '-3.753738703787238056897308105489740e+5947', '-5.1464720662639698186031461962218221108584708551990576e+426', 77, 0);
    checkModResult('-185083.95424764097664278043736816641562291', '6.88483981698920539e+549', '-1.8508395424764097664278043736816641562291e+5', 52, 3);
    checkModResult('-8.4526212586667859362683394505144580845548023e+92', '8.56475540823390109934430835325058371670270846338329e+93', '-8.4526212586667859362683394505144580845548023e+92', 90, 5);
    checkModResult('-945636271.5759891', '-2.17432808397e+594', '-9.456362715759891e+8', 89, 0);
    checkModResult('3.548871234875504632967049284731417963316430094e+938', '8.58647841598924619155917147408449004293104046547183e+8404', '3.548871234875504632967049284731417963316430094e+938', 56, 6);
    checkModResult('-1.1430431170771599354075836985722188465803720293752e+2890', '8.77e+542', '-5.33e+542', 27, 3);
    checkModResult('-9.04312894420050284873550010294637483269334834464174461210e+871', '-4.224918240e+77', '-1.03574112e+77', 25, 6);
}
