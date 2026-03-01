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
    const suite = new arktest.ArkTestsuite("Decimal toFixed API tests");
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    suite.addTest("Decimal toFixed with specific config", testToFixedWithSpecificConfig001);
    suite.addTest("Decimal toFixed with specific config", testToFixedWithSpecificConfig002);
    suite.addTest("Decimal toFixed with specific config", testToFixedWithSpecificConfig003);
    suite.addTest("Decimal toFixed with specific config", testToFixedWithSpecificConfig004);
    suite.addTest("Decimal toFixed with rounding is 0", testToFixedWithRMIsZero001);
    suite.addTest("Decimal toFixed with rounding is 0", testToFixedWithRMIsZero002);
    suite.addTest("Decimal toFixed with rounding is 1", testToFixedWithRMIsOne001);
    suite.addTest("Decimal toFixed with rounding is 1", testToFixedWithRMIsOne002);
    suite.addTest("Decimal toFixed with rounding is 2", testToFixedWithRMIsTwo001);
    suite.addTest("Decimal toFixed with rounding is 2", testToFixedWithRMIsTwo002);
    suite.addTest("Decimal toFixed with rounding is 3", testToFixedWithRMIsThree001);
    suite.addTest("Decimal toFixed with rounding is 3", testToFixedWithRMIsThree002);
    suite.addTest("Decimal toFixed with rounding is 4", testToFixedWithRMIsFour001);
    suite.addTest("Decimal toFixed with rounding is 4", testToFixedWithRMIsFour002);
    suite.addTest("Decimal toFixed with rounding is 4", testToFixedWithRMIsFour003);
    suite.addTest("Decimal toFixed with rounding is 5", testToFixedWithRMIsFive001);
    suite.addTest("Decimal toFixed with rounding is 5", testToFixedWithRMIsFive002);
    suite.addTest("Decimal toFixed with rounding is 6", testToFixedWithRMIsSix001);
    suite.addTest("Decimal toFixed with rounding is 6", testToFixedWithRMIsSix002);
    suite.addTest("Decimal toFixed with rounding is 7", testToFixedWithRMIsSeven);
    suite.addTest("Decimal toFixed with rounding is 8", testToFixedWithRMIsEight);
    return suite.run();
}

function testToFixedWithSpecificConfig001() {
    arktest.assertEQ(new Decimal(99.9512986).toFixed(1), '100.0');
    arktest.assertEQ(new Decimal(9.95036).toFixed(1), '10.0');
    arktest.assertEQ(new Decimal(0.99).toFixed(1), '1.0');
    arktest.assertEQ(new Decimal(0.09906).toFixed(2), '0.10');
    arktest.assertEQ(new Decimal(0.0098034).toFixed(3), '0.010');
    arktest.assertEQ(new Decimal(NaN).toFixed(2), 'NaN');
    arktest.assertEQ(new Decimal(Infinity).toFixed(2), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).toFixed(2), '-Infinity');
}

function testToFixedWithSpecificConfig002() {
    arktest.assertEQ(new Decimal('1111111111111111111111').toFixed(8), '1111111111111111111111.00000000');
    arktest.assertEQ(new Decimal(0.1).toFixed(1), '0.1');
    arktest.assertEQ(new Decimal(0.1).toFixed(2), '0.10');
    arktest.assertEQ(new Decimal(0.1).toFixed(3), '0.100');
    arktest.assertEQ(new Decimal(0.01).toFixed(2), '0.01');
    arktest.assertEQ(new Decimal(0.01).toFixed(3), '0.010');
    arktest.assertEQ(new Decimal(0.01).toFixed(4), '0.0100');
    arktest.assertEQ(new Decimal(0.001).toFixed(2), '0.00');
    arktest.assertEQ(new Decimal(0.001).toFixed(3), '0.001');
    arktest.assertEQ(new Decimal(0.001).toFixed(4), '0.0010');
    arktest.assertEQ(new Decimal(1).toFixed(4), '1.0000');
    arktest.assertEQ(new Decimal(1).toFixed(1), '1.0');
    arktest.assertEQ(new Decimal(1).toFixed(0), '1');
    arktest.assertEQ(new Decimal(12).toFixed(0), '12');
    arktest.assertEQ(new Decimal(1.1).toFixed(0), '1');
    arktest.assertEQ(new Decimal(12.1).toFixed(0), '12');
    arktest.assertEQ(new Decimal(1.12).toFixed(0), '1');
    arktest.assertEQ(new Decimal(12.12).toFixed(0), '12');
    arktest.assertEQ(new Decimal(0.0000006).toFixed(7), '0.0000006');
    arktest.assertEQ(new Decimal(0.00000006).toFixed(8), '0.00000006');
    arktest.assertEQ(new Decimal(0.00000006).toFixed(9), '0.000000060');
    arktest.assertEQ(new Decimal(0.00000006).toFixed(10), '0.0000000600');
    arktest.assertEQ(new Decimal(0).toFixed(0), '0');
    arktest.assertEQ(new Decimal(0).toFixed(1), '0.0');
    arktest.assertEQ(new Decimal(0).toFixed(2), '0.00');
}

function testToFixedWithSpecificConfig003() {
    arktest.assertEQ(new Decimal('-1111111111111111111111').toFixed(8), '-1111111111111111111111.00000000');
    arktest.assertEQ(new Decimal(-0.1).toFixed(1), '-0.1');
    arktest.assertEQ(new Decimal(-0.1).toFixed(2), '-0.10');
    arktest.assertEQ(new Decimal(-0.1).toFixed(3), '-0.100');
    arktest.assertEQ(new Decimal(-0.01).toFixed(2), '-0.01');
    arktest.assertEQ(new Decimal(-0.01).toFixed(3), '-0.010');
    arktest.assertEQ(new Decimal(-0.01).toFixed(4), '-0.0100');
    arktest.assertEQ(new Decimal(-0.001).toFixed(2), '-0.00');
    arktest.assertEQ(new Decimal(-0.001).toFixed(3), '-0.001');
    arktest.assertEQ(new Decimal(-0.001).toFixed(4), '-0.0010');
    arktest.assertEQ(new Decimal(-1).toFixed(4), '-1.0000');
    arktest.assertEQ(new Decimal(-1).toFixed(1), '-1.0');
    arktest.assertEQ(new Decimal(-1).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-1.1).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-12.1).toFixed(0), '-12');
    arktest.assertEQ(new Decimal(-1.12).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-12.12).toFixed(0), '-12');
    arktest.assertEQ(new Decimal(-0.0000006).toFixed(5), '-0.00000');
    arktest.assertEQ(new Decimal(-0.0000006).toFixed(7), '-0.0000006');
    arktest.assertEQ(new Decimal(-0.00000006).toFixed(8), '-0.00000006');
    arktest.assertEQ(new Decimal(-0.00000006).toFixed(9), '-0.000000060');
    arktest.assertEQ(new Decimal(-0.00000006).toFixed(10), '-0.0000000600');
    arktest.assertEQ(new Decimal(0).toFixed(0), '0');
    arktest.assertEQ(new Decimal(-0).toFixed(0), '0');
    arktest.assertEQ(new Decimal(-0).toFixed(1), '0.0');
    arktest.assertEQ(new Decimal(-0).toFixed(2), '0.00');
    arktest.assertEQ(new Decimal('-0.0').toFixed(2), '0.00');
    arktest.assertEQ(new Decimal('-0.0000').toFixed(2), '0.00');
    arktest.assertEQ(new Decimal(-0).toFixed(4), '0.0000');
}

function testToFixedWithSpecificConfig004() {
    arktest.assertEQ(new Decimal(1000).toFixed(), '1000');
    arktest.assertEQ(new Decimal(0.00001).toFixed(), '0.00001');
    arktest.assertEQ(new Decimal(0.00001).toFixed(5), '0.00001');
    arktest.assertEQ(new Decimal('0.0000000000000000001').toFixed(20), '0.00000000000000000010');
    arktest.assertEQ(new Decimal(0.00001).toFixed(17), '0.00001000000000000');
    arktest.assertEQ(new Decimal(1).toFixed(17), '1.00000000000000000');
    arktest.assertEQ(new Decimal('1000000000000000128').toFixed(), '1000000000000000128');
    arktest.assertEQ(new Decimal('100000000000000128').toFixed(1), '100000000000000128.0');
    arktest.assertEQ(new Decimal('10000000000000128').toFixed(2), '10000000000000128.00');
    arktest.assertEQ(new Decimal('10000000000000128').toFixed(20), '10000000000000128.00000000000000000000');
    arktest.assertEQ(new Decimal(0).toFixed(), '0');
    arktest.assertEQ(new Decimal(-42).toFixed(3), '-42.000');
    arktest.assertEQ(new Decimal('-1000000000000000128').toFixed(), '-1000000000000000128');
    arktest.assertEQ(new Decimal('-0.0000000000000000001').toFixed(20), '-0.00000000000000000010');
    arktest.assertEQ(new Decimal('0.123123123123123').toFixed(20), '0.12312312312312300000');
    arktest.assertEQ(new Decimal(0.5).toFixed(0), '1');
    arktest.assertEQ(new Decimal(-0.5).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(1.25).toFixed(1), '1.3');
    arktest.assertEQ(new Decimal(234.20405).toFixed(4), '234.2041');
    arktest.assertEQ(new Decimal('234.204050000000000000000000000000006').toFixed(4), '234.2041');
}

function testToFixedWithRMIsZero001() {
    Decimal.rounding = 0;
    arktest.assertEQ(new Decimal('0.000001').toFixed(6), '0.000001');
    arktest.assertEQ(new Decimal('6552606716539719300271040797809220.237838405').toFixed(1), '6552606716539719300271040797809220.3');
    arktest.assertEQ(new Decimal('25605410.260045950231370974066').toFixed(15), '25605410.260045950231371');
    arktest.assertEQ(new Decimal('-65593283').toFixed(4), '-65593283.0000');
    arktest.assertEQ(new Decimal('-2238743875407788208860272230040067273281097722822075126.88').toFixed(11), '-2238743875407788208860272230040067273281097722822075126.88000000000');
    arktest.assertEQ(new Decimal('-1714042659419404211').toFixed(7), '-1714042659419404211.0000000');
    arktest.assertEQ(new Decimal('580861301').toFixed(3), '580861301.000');
    arktest.assertEQ(new Decimal('-495746734').toFixed(3), '-495746734.000');
    arktest.assertEQ(new Decimal('-909962202').toFixed(5), '-909962202.00000');
    arktest.assertEQ(new Decimal('-6588').toFixed(2), '-6588.00');
    arktest.assertEQ(new Decimal('-89235424125324').toFixed(6), '-89235424125324.000000');
    arktest.assertEQ(new Decimal('-0.0000000000177').toFixed(4), '-0.0001');
    arktest.assertEQ(new Decimal('-0.000000000000007').toFixed(1), '-0.1');
    arktest.assertEQ(new Decimal('7086047552021418140460170703933026989776558656.864197').toFixed(15), '7086047552021418140460170703933026989776558656.864197000000000');
    arktest.assertEQ(new Decimal('-0.0000000000000000007707').toFixed(10), '-0.0000000001');
    arktest.assertEQ(new Decimal('6613833500.133407255727325566781771').toFixed(12), '6613833500.133407255728');
    arktest.assertEQ(new Decimal('-0.6').toFixed(2), '-0.60');
    arktest.assertEQ(new Decimal('-5800577761301861465745732694236894834.34719979045921439394').toFixed(21), '-5800577761301861465745732694236894834.347199790459214393940');
    arktest.assertEQ(new Decimal('65495133621931312001260.89870016055223846451749943336585697').toFixed(2), '65495133621931312001260.90');
    arktest.assertEQ(new Decimal('-0.0000000000021727264987520611266069544793244453321336').toFixed(7), '-0.0000001');
}

function testToFixedWithRMIsZero002() {
    arktest.assertEQ(new Decimal('5138338134667513058267845351654.007282').toFixed(4), '5138338134667513058267845351654.0073');
    arktest.assertEQ(new Decimal('-419591612781413402.16060946510832813452').toFixed(16), '-419591612781413402.1606094651083282');
    arktest.assertEQ(new Decimal('-842984708').toFixed(5), '-842984708.00000');
    arktest.assertEQ(new Decimal('6094679992788973717749530934167375519948195193196').toFixed(11), '6094679992788973717749530934167375519948195193196.00000000000');
    arktest.assertEQ(new Decimal('3070926').toFixed(2), '3070926.00');
    arktest.assertEQ(new Decimal('-0.00000000000000042733957907641417564294').toFixed(19), '-0.0000000000000004274');
    arktest.assertEQ(new Decimal('-307434').toFixed(1), '-307434.0');
    arktest.assertEQ(new Decimal('-7155930111672774753753.845014828710226').toFixed(14), '-7155930111672774753753.84501482871023');
    arktest.assertEQ(new Decimal('-204638').toFixed(1), '-204638.0');
    arktest.assertEQ(new Decimal('67354.801797').toFixed(2), '67354.81');
    arktest.assertEQ(new Decimal('0.000042911674899533228107357062').toFixed(12), '0.000042911675');
    arktest.assertEQ(new Decimal('-57865060735863.590334711').toFixed(10), '-57865060735863.5903347110');
    arktest.assertEQ(new Decimal('-0.0088857').toFixed(5), '-0.00889');
    arktest.assertEQ(new Decimal('655593535442553089245040020344044').toFixed(12), '655593535442553089245040020344044.000000000000');
    arktest.assertEQ(new Decimal('-2077237944012133216588518213.9401561759783653376').toFixed(4), '-2077237944012133216588518213.9402');
    arktest.assertEQ(new Decimal('3354.49314543').toFixed(2), '3354.50');
    arktest.assertEQ(new Decimal('-0.000000000000091204118667391473401958802734362777499449233').toFixed(5), '-0.00001');
    arktest.assertEQ(new Decimal('-2817626898661648124242602088971648489124280903').toFixed(23), '-2817626898661648124242602088971648489124280903.00000000000000000000000');
    arktest.assertEQ(new Decimal('-0.1137406345875745478881089475981775971743').toFixed(4), '-0.1138');
    arktest.assertEQ(new Decimal('372625.836933301').toFixed(2), '372625.84');
    arktest.assertEQ(new Decimal('-0.0000000000030295').toFixed(4), '-0.0001');
}

function testToFixedWithRMIsOne001() {
    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('0.3').toFixed(1), '0.3');
    arktest.assertEQ(new Decimal('-200258348374.3595802551014614089785610548492885372322083789').toFixed(1), '-200258348374.3');
    arktest.assertEQ(new Decimal('-8996550690041157178188143247448286757711.55808574133329').toFixed(10), '-8996550690041157178188143247448286757711.5580857413');
    arktest.assertEQ(new Decimal('-3172413669280032477').toFixed(2), '-3172413669280032477.00');
    arktest.assertEQ(new Decimal('6547946357820.750067107731812021675351468709784004').toFixed(21), '6547946357820.750067107731812021675');
    arktest.assertEQ(new Decimal('24188393190716631282750407915133817062572333100239').toFixed(19), '24188393190716631282750407915133817062572333100239.0000000000000000000');
    arktest.assertEQ(new Decimal('539982361589798324286450574560330534901309503.82').toFixed(8), '539982361589798324286450574560330534901309503.82000000');
    arktest.assertEQ(new Decimal('829898800701640360552652').toFixed(4), '829898800701640360552652.0000');
    arktest.assertEQ(new Decimal('-1585782773394.44501382110847').toFixed(1), '-1585782773394.4');
    arktest.assertEQ(new Decimal('-7604844176594943774211951887242195107.399576743428669618164').toFixed(3), '-7604844176594943774211951887242195107.399');
    arktest.assertEQ(new Decimal('-0.0000000000000005153003506839').toFixed(2), '-0.00');
    arktest.assertEQ(new Decimal('0.00003').toFixed(3), '0.000');
    arktest.assertEQ(new Decimal('-5454249481540317712.56029385901373130215526009738012974642338').toFixed(18), '-5454249481540317712.560293859013731302');
    arktest.assertEQ(new Decimal('15352607654972198961429295651185206861818141054385638').toFixed(17), '15352607654972198961429295651185206861818141054385638.00000000000000000');
    arktest.assertEQ(new Decimal('91494535411039025233040.29224903220309368507011').toFixed(3), '91494535411039025233040.292');
    arktest.assertEQ(new Decimal('2043369316').toFixed(1), '2043369316.0');
    arktest.assertEQ(new Decimal('-0.0000000782350618457630647420312027682238301541350414').toFixed(19), '-0.0000000782350618457');
    arktest.assertEQ(new Decimal('2122652').toFixed(1), '2122652.0');
    arktest.assertEQ(new Decimal('-0.00841365099301981489219310202029642').toFixed(8), '-0.00841365');
    arktest.assertEQ(new Decimal('0.0007035').toFixed(1), '0.0');
    arktest.assertEQ(new Decimal('-0.0000000000000037491685778894015479084539735777088').toFixed(20), '-0.00000000000000374916');
    arktest.assertEQ(new Decimal('534883638').toFixed(2), '534883638.00');
}

function testToFixedWithRMIsOne002() {
    arktest.assertEQ(new Decimal('-2').toFixed(1), '-2.0');
    arktest.assertEQ(new Decimal('-5634442247266825358399629799939027370665').toFixed(19), '-5634442247266825358399629799939027370665.0000000000000000000');
    arktest.assertEQ(new Decimal('3331187169219186569122').toFixed(9), '3331187169219186569122.000000000');
    arktest.assertEQ(new Decimal('0.0000000000006604395609805032330367635527').toFixed(10), '0.0000000000');
    arktest.assertEQ(new Decimal('-0.00000000000005769013292086168690493327').toFixed(19), '-0.0000000000000576901');
    arktest.assertEQ(new Decimal('-661.828596629053201916486').toFixed(1), '-661.8');
    arktest.assertEQ(new Decimal('-6073555395665254434249128854999349235744174928042756.1153').toFixed(13), '-6073555395665254434249128854999349235744174928042756.1153000000000');
    arktest.assertEQ(new Decimal('-5013086846966').toFixed(3), '-5013086846966.000');
    arktest.assertEQ(new Decimal('0.00401523226833').toFixed(6), '0.004015');
    arktest.assertEQ(new Decimal('3140295374107374579919222510.1462722819395532').toFixed(7), '3140295374107374579919222510.1462722');
    arktest.assertEQ(new Decimal('-0.00000000000000799876460379334679831886').toFixed(6), '-0.000000');
    arktest.assertEQ(new Decimal('-0.0003').toFixed(2), '-0.00');
    arktest.assertEQ(new Decimal('-0.026760766726884267750393307117624838556001925491').toFixed(18), '-0.026760766726884267');
    arktest.assertEQ(new Decimal('-20821740502968847857923433558797.08997487843745').toFixed(4), '-20821740502968847857923433558797.0899');
    arktest.assertEQ(new Decimal('0.000000000020346524414696573703092255317751132892').toFixed(11), '0.00000000002');
    arktest.assertEQ(new Decimal('45492073832204366341299301624').toFixed(9), '45492073832204366341299301624.000000000');
}

function testToFixedWithRMIsTwo001() {
    Decimal.rounding = 2;
    arktest.assertEQ(new Decimal('7810971819938620325901899057970512790433').toFixed(16), '7810971819938620325901899057970512790433.0000000000000000');
    arktest.assertEQ(new Decimal('-623395').toFixed(1), '-623395.0');
    arktest.assertEQ(new Decimal('81334094079413997693749807').toFixed(12), '81334094079413997693749807.000000000000');
    arktest.assertEQ(new Decimal('790143966.406169480041929356421').toFixed(1), '790143966.5');
    arktest.assertEQ(new Decimal('-0.0004851099615478').toFixed(6), '-0.000485');
    arktest.assertEQ(new Decimal('843581583868277359263.97212648230399373761717').toFixed(7), '843581583868277359263.9721265');
    arktest.assertEQ(new Decimal('-921038771017147427324121032').toFixed(6), '-921038771017147427324121032.000000');
    arktest.assertEQ(new Decimal('-823').toFixed(1), '-823.0');
    arktest.assertEQ(new Decimal('505').toFixed(1), '505.0');
    arktest.assertEQ(new Decimal('-866703339332502.8636').toFixed(7), '-866703339332502.8636000');
    arktest.assertEQ(new Decimal('46').toFixed(1), '46.0');
    arktest.assertEQ(new Decimal('60042202798790129685').toFixed(7), '60042202798790129685.0000000');
    arktest.assertEQ(new Decimal('-1847392').toFixed(4), '-1847392.0000');
    arktest.assertEQ(new Decimal('-4580251343840510165820631684073999841789879').toFixed(22), '-4580251343840510165820631684073999841789879.0000000000000000000000');
    arktest.assertEQ(new Decimal('3501378778000.5390637488272654464266281832626835121535730779').toFixed(18), '3501378778000.539063748827265447');
    arktest.assertEQ(new Decimal('0.000000004685387489140232').toFixed(12), '0.000000004686');
    arktest.assertEQ(new Decimal('-0.0006').toFixed(1), '-0.0');
    arktest.assertEQ(new Decimal('-445677508').toFixed(2), '-445677508.00');
    arktest.assertEQ(new Decimal('0.00015099822561012723').toFixed(4), '0.0002');
    arktest.assertEQ(new Decimal('-2992505698074499692367794831646886').toFixed(16), '-2992505698074499692367794831646886.0000000000000000');
    arktest.assertEQ(new Decimal('5297873.5869265978056497873793730268').toFixed(2), '5297873.59');
    arktest.assertEQ(new Decimal('72978439324039191443182').toFixed(11), '72978439324039191443182.00000000000');
    arktest.assertEQ(new Decimal('0.660283869505570207917431271006277981914716620876').toFixed(8), '0.66028387');
    arktest.assertEQ(new Decimal('-232008389442485').toFixed(1), '-232008389442485.0');
}

function testToFixedWithRMIsTwo002() {
    arktest.assertEQ(new Decimal('-2.37156855939356279513952542004820909').toFixed(6), '-2.371568');
    arktest.assertEQ(new Decimal('731916154123696.193488323779184681349292529227446').toFixed(4), '731916154123696.1935');
    arktest.assertEQ(new Decimal('-793941153937799250384382615553258074').toFixed(5), '-793941153937799250384382615553258074.00000');
    arktest.assertEQ(new Decimal('651002').toFixed(2), '651002.00');
    arktest.assertEQ(new Decimal('-61317874871').toFixed(2), '-61317874871.00');
    arktest.assertEQ(new Decimal('8673').toFixed(2), '8673.00');
    arktest.assertEQ(new Decimal('-14490519736189597435728386282.0306558561264348956674').toFixed(6), '-14490519736189597435728386282.030655');
    arktest.assertEQ(new Decimal('0.00000000000002').toFixed(3), '0.001');
    arktest.assertEQ(new Decimal('0.000008920984342').toFixed(6), '0.000009');
    arktest.assertEQ(new Decimal('0.000092095708639597367987279557138375172816422475').toFixed(25), '0.0000920957086395973679873');
    arktest.assertEQ(new Decimal('28125849913667924088615.2540702331773399078402875044891').toFixed(14), '28125849913667924088615.25407023317734');
    arktest.assertEQ(new Decimal('87.55538596435839691343147').toFixed(8), '87.55538597');
    arktest.assertEQ(new Decimal('939705902083425775202905619379').toFixed(7), '939705902083425775202905619379.0000000');
    arktest.assertEQ(new Decimal('7303936681469130614.8345').toFixed(7), '7303936681469130614.8345000');
    arktest.assertEQ(new Decimal('-5831852886782.6801').toFixed(8), '-5831852886782.68010000');
    arktest.assertEQ(new Decimal('-0.0000000213769530812').toFixed(7), '-0.0000000');
}

function testToFixedWithRMIsThree001() {
    Decimal.rounding = 3;
    arktest.assertEQ(new Decimal('0.00000000000584').toFixed(4), '0.0000');
    arktest.assertEQ(new Decimal('-79055907138737329232.1656779990925523674').toFixed(18), '-79055907138737329232.165677999092552368');
    arktest.assertEQ(new Decimal('47206303530045536472421888342339').toFixed(8), '47206303530045536472421888342339.00000000');
    arktest.assertEQ(new Decimal('3974').toFixed(1), '3974.0');
    arktest.assertEQ(new Decimal('1296297430771.391670729445980444292').toFixed(2), '1296297430771.39');
    arktest.assertEQ(new Decimal('0.00001686928031').toFixed(6), '0.000016');
    arktest.assertEQ(new Decimal('-0.0000000000008').toFixed(5), '-0.00001');
    arktest.assertEQ(new Decimal('4883938724383').toFixed(3), '4883938724383.000');
    arktest.assertEQ(new Decimal('-5.411562230628539329345282639155988044655000352687981880842').toFixed(10), '-5.4115622307');
    arktest.assertEQ(new Decimal('-0.0000000000155280115429218804426906540524').toFixed(2), '-0.01');
    arktest.assertEQ(new Decimal('-2730930776035874709906').toFixed(5), '-2730930776035874709906.00000');
    arktest.assertEQ(new Decimal('8419336352549424092369623').toFixed(5), '8419336352549424092369623.00000');
    arktest.assertEQ(new Decimal('52897.4').toFixed(1), '52897.4');
    arktest.assertEQ(new Decimal('-783171007682720320053252521365465473997085').toFixed(1), '-783171007682720320053252521365465473997085.0');
    arktest.assertEQ(new Decimal('0.8').toFixed(1), '0.8');
    arktest.assertEQ(new Decimal('-0.000000000852796916216780032747621063045628213284').toFixed(14), '-0.00000000085280');
    arktest.assertEQ(new Decimal('809486781065644418.939').toFixed(1), '809486781065644418.9');
    arktest.assertEQ(new Decimal('-9').toFixed(1), '-9.0');
    arktest.assertEQ(new Decimal('2399395').toFixed(2), '2399395.00');
    arktest.assertEQ(new Decimal('-243707862514768528758611224374').toFixed(13), '-243707862514768528758611224374.0000000000000');
    arktest.assertEQ(new Decimal('-87906').toFixed(2), '-87906.00');
}

function testToFixedWithRMIsThree002() {
    arktest.assertEQ(new Decimal('-36206954271055061276640398631031143034683624940748.997').toFixed(22), '-36206954271055061276640398631031143034683624940748.9970000000000000000000');
    arktest.assertEQ(new Decimal('0.0000000000565885579252765288683939564608114096713').toFixed(2), '0.00');
    arktest.assertEQ(new Decimal('-8391656174313988740342135972218668561645437').toFixed(1), '-8391656174313988740342135972218668561645437.0');
    arktest.assertEQ(new Decimal('2798706581453003.17723911167031').toFixed(9), '2798706581453003.177239111');
    arktest.assertEQ(new Decimal('607781292230803273523112111.09172672114').toFixed(12), '607781292230803273523112111.091726721140');
    arktest.assertEQ(new Decimal('-0.3').toFixed(1), '-0.3');
    arktest.assertEQ(new Decimal('3008522535782692059736852844809784198247000184').toFixed(1), '3008522535782692059736852844809784198247000184.0');
    arktest.assertEQ(new Decimal('14227').toFixed(2), '14227.00');
    arktest.assertEQ(new Decimal('7841760307718014116576857993957').toFixed(4), '7841760307718014116576857993957.0000');
    arktest.assertEQ(new Decimal('-0.00000000000002596').toFixed(5), '-0.00001');
    arktest.assertEQ(new Decimal('5247199732982111073').toFixed(9), '5247199732982111073.000000000');
    arktest.assertEQ(new Decimal('-12').toFixed(1), '-12.0');
    arktest.assertEQ(new Decimal('0.00005').toFixed(3), '0.000');
    arktest.assertEQ(new Decimal('-1383703062').toFixed(4), '-1383703062.0000');
    arktest.assertEQ(new Decimal('0.00510192453051698060366202631871161193').toFixed(16), '0.0051019245305169');
    arktest.assertEQ(new Decimal('-45565092550117861497905653558246322559799').toFixed(1), '-45565092550117861497905653558246322559799.0');
    arktest.assertEQ(new Decimal('-0.284').toFixed(3), '-0.284');
    arktest.assertEQ(new Decimal('-8962').toFixed(2), '-8962.00');
    arktest.assertEQ(new Decimal('49335989571.217302304037').toFixed(7), '49335989571.2173023');
}

function testToFixedWithRMIsFour001() {
    Decimal.rounding = 4;
    Decimal.toExpNeg = 0;
    Decimal.toExpPos = 0;
    arktest.assertEQ(new Decimal('733744593401073823825766410831877679446').toFixed(19), '733744593401073823825766410831877679446.0000000000000000000');
    arktest.assertEQ(new Decimal('-64.6849458687691227978').toFixed(7), '-64.6849459');
    arktest.assertEQ(new Decimal('-0.00000000009').toFixed(6), '-0.000000');
    arktest.assertEQ(new Decimal('-62537287527837589298857228059657673223234916.9592326543').toFixed(17), '-62537287527837589298857228059657673223234916.95923265430000000');
    arktest.assertEQ(new Decimal('3393668096256773847245721315080265089731').toFixed(6), '3393668096256773847245721315080265089731.000000');
    arktest.assertEQ(new Decimal('0.0000000000000056674956638008432348702401392').toFixed(1), '0.0');
    arktest.assertEQ(new Decimal('72516372734.6447').toFixed(1), '72516372734.6');
    arktest.assertEQ(new Decimal('-418.2800731793741351').toFixed(2), '-418.28');
    arktest.assertEQ(new Decimal('0.001').toFixed(2), '0.00');
    arktest.assertEQ(new Decimal('8366217346845756726').toFixed(8), '8366217346845756726.00000000');
    arktest.assertEQ(new Decimal('-0.0000000000000092034548636370987112234384736726').toFixed(6), '-0.000000');
    arktest.assertEQ(new Decimal('0.35474830751442135112334772517193392').toFixed(2), '0.35');
    arktest.assertEQ(new Decimal('64703289793894.58296866').toFixed(4), '64703289793894.5830');
    arktest.assertEQ(new Decimal('-0.000000000000000036461242408590182363418943891').toFixed(18), '-0.000000000000000036');
    arktest.assertEQ(new Decimal('5494508405056449117588.631948458033233759999').toFixed(6), '5494508405056449117588.631948');
    arktest.assertEQ(new Decimal('-0.00393971618499838726739122333520030506235698').toFixed(1), '-0.0');
    arktest.assertEQ(new Decimal('375581290738585826632').toFixed(8), '375581290738585826632.00000000');
    arktest.assertEQ(new Decimal('254.96635275802300886544776010389418575738792480979736').toFixed(17), '254.96635275802300887');
    arktest.assertEQ(new Decimal('21492347.6946757139149862444482880595559468').toFixed(20), '21492347.69467571391498624445');
    arktest.assertEQ(new Decimal('313576441324233').toFixed(7), '313576441324233.0000000');
    arktest.assertEQ(new Decimal('-534460490015293367127173277346694900936058').toFixed(4), '-534460490015293367127173277346694900936058.0000');
    arktest.assertEQ(new Decimal('182707431911537249021116759327712693311898345504618668.43327').toFixed(23), '182707431911537249021116759327712693311898345504618668.43327000000000000000000');
    arktest.assertEQ(new Decimal('210005324615278.458683969004596332103244549279').toFixed(22), '210005324615278.4586839690045963321032');
    arktest.assertEQ(new Decimal('779837001772884165637922377221951347134695.6448338').toFixed(6), '779837001772884165637922377221951347134695.644834');
    arktest.assertEQ(new Decimal('-0.00000064188301390033596845335767993853284632527964514979079').toFixed(6), '-0.000001');
    arktest.assertEQ(new Decimal('13').toFixed(1), '13.0');
    arktest.assertEQ(new Decimal('0.0000001269060795648365813491128357427111184222').toFixed(10), '0.0000001269');
    arktest.assertEQ(new Decimal('18446632248354').toFixed(2), '18446632248354.00');
}

function testToFixedWithRMIsFour002() {
    arktest.assertEQ(new Decimal('-1229249.7897249259').toFixed(2), '-1229249.79');
    arktest.assertEQ(new Decimal('49082').toFixed(1), '49082.0');
    arktest.assertEQ(new Decimal('-61').toFixed(1), '-61.0');
    arktest.assertEQ(new Decimal('-893').toFixed(1), '-893.0');
    arktest.assertEQ(new Decimal('5002282278.569748776900664839184116538222902').toFixed(17), '5002282278.56974877690066484');
    arktest.assertEQ(new Decimal('41372').toFixed(2), '41372.00');
    arktest.assertEQ(new Decimal('-4732022445962399687294885123498809.7625585825095').toFixed(13), '-4732022445962399687294885123498809.7625585825095');
    arktest.assertEQ(new Decimal('-55484242.036895641').toFixed(9), '-55484242.036895641');
    arktest.assertEQ(new Decimal('-41427133134.525833234279076632683393992706825').toFixed(23), '-41427133134.52583323427907663268339');
    arktest.assertEQ(new Decimal('0.00004300614085218825243480119971669264977421').toFixed(1), '0.0');
    arktest.assertEQ(new Decimal('-472025754597316278339412186866.701065978877597089729906019843').toFixed(10), '-472025754597316278339412186866.7010659789');
    arktest.assertEQ(new Decimal('-535564000').toFixed(2), '-535564000.00');
    arktest.assertEQ(new Decimal('-80000000').toFixed(3), '-80000000.000');
    arktest.assertEQ(new Decimal('-240').toFixed(1), '-240.0');
    arktest.assertEQ(new Decimal('-7520000000').toFixed(1), '-7520000000.0');
    arktest.assertEQ(new Decimal('306550').toFixed(3), '306550.000');
    arktest.assertEQ(new Decimal('800000000').toFixed(1), '800000000.0');
    arktest.assertEQ(new Decimal('-454000000').toFixed(5), '-454000000.00000');
    arktest.assertEQ(new Decimal('60').toFixed(1), '60.0');
    arktest.assertEQ(new Decimal('-4700000').toFixed(3), '-4700000.000');
    arktest.assertEQ(new Decimal('25000').toFixed(2), '25000.00');
    arktest.assertEQ(new Decimal('6847').toFixed(1), '6847.0');
    arktest.assertEQ(new Decimal('780').toFixed(1), '780.0');
    arktest.assertEQ(new Decimal('-50').toFixed(1), '-50.0');
    arktest.assertEQ(new Decimal('700').toFixed(1), '700.0');
    arktest.assertEQ(new Decimal('412598000').toFixed(4), '412598000.0000');
    arktest.assertEQ(new Decimal('-84581600').toFixed(1), '-84581600.0');
    arktest.assertEQ(new Decimal('5590000').toFixed(2), '5590000.00');
    arktest.assertEQ(new Decimal('-66000').toFixed(1), '-66000.0');
    arktest.assertEQ(new Decimal('600').toFixed(1), '600.0');
    arktest.assertEQ(new Decimal('-513335000').toFixed(3), '-513335000.000');
    arktest.assertEQ(new Decimal('-40').toFixed(1), '-40.0');
}

function testToFixedWithRMIsFour003() {
    arktest.assertEQ(new Decimal('-5.253620252713891931933498708496518913773517016037224E10').toFixed(), '-52536202527.13891931933498708496518913773517016037224');
    arktest.assertEQ(new Decimal('8.761383898703999300297605784533730922E-16').toFixed(), '0.0000000000000008761383898703999300297605784533730922');
    arktest.assertEQ(new Decimal('-4E-37').toFixed(), '-0.0000000000000000000000000000000000004');
    arktest.assertEQ(new Decimal('2.83204419494047654015529986948208E12').toFixed(), '2832044194940.47654015529986948208');
    arktest.assertEQ(new Decimal('6.4668155275864837E-25').toFixed(), '0.00000000000000000000000064668155275864837');
    arktest.assertEQ(new Decimal('4.34128088694E0').toFixed(), '4.34128088694');
    arktest.assertEQ(new Decimal('-8.519576572615611436542E5').toFixed(),'-851957.6572615611436542');
    arktest.assertEQ(new Decimal('-8.90974682862561077879598798580253771544265682053E10').toFixed(), '-89097468286.2561077879598798580253771544265682053');
    arktest.assertEQ(new Decimal('-3.0409767E-47').toFixed(), '-0.000000000000000000000000000000000000000000000030409767');
    arktest.assertEQ(new Decimal('4.295112484112357722747956581254E-21').toFixed(), '0.000000000000000000004295112484112357722747956581254');
    arktest.assertEQ(new Decimal('-8.392032074756516542808578E14').toFixed(), '-839203207475651.6542808578');
    arktest.assertEQ(new Decimal('-7.8216E-19').toFixed(), '-0.00000000000000000078216');
    arktest.assertEQ(new Decimal('-1.78229027493570124773421262413032385E21').toFixed(), '-1782290274935701247734.21262413032385');
    arktest.assertEQ(new Decimal('3.166945110414480133707643345718511076E28').toFixed(), '31669451104144801337076433457.18511076');
    arktest.assertEQ(new Decimal('4.72351852761E-37').toFixed(), '0.000000000000000000000000000000000000472351852761');
    arktest.assertEQ(new Decimal('-5.015522021752356890108362378842580174913602672593731E25').toFixed(), '-50155220217523568901083623.78842580174913602672593731');
    arktest.assertEQ(new Decimal('-2.9137E2').toFixed(), '-291.37');
    arktest.assertEQ(new Decimal('-5.24746119364562058860271953529612123832279441143E37').toFixed(), '-52474611936456205886027195352961212383.2279441143');
    arktest.assertEQ(new Decimal('-3.269376463292543E2').toFixed(), '-326.9376463292543');
    arktest.assertEQ(new Decimal('4.4E-13').toFixed(), '0.00000000000044');
    arktest.assertEQ(new Decimal('8.7792449E-8').toFixed(), '0.000000087792449');
    arktest.assertEQ(new Decimal('4.97835793870987132484033938845920610565887398960253648E23').toFixed(), '497835793870987132484033.938845920610565887398960253648');
    arktest.assertEQ(new Decimal('-3.396043944230277005874863962E19').toFixed(), '-33960439442302770058.74863962');
    arktest.assertEQ(new Decimal('2.0694696587293782E-40').toFixed(), '0.00000000000000000000000000000000000000020694696587293782');
    arktest.assertEQ(new Decimal('123.45').toFixed(), '123.45');
    arktest.assertEQ(new Decimal('123.45').toFixed(0), '123');
    arktest.assertEQ(new Decimal('123.45').toFixed(-0), '123');
}

function testToFixedWithRMIsFive001() {
    Decimal.rounding = 5;
    arktest.assertEQ(new Decimal('0.00000000000000070362').toFixed(2), '0.00');
    arktest.assertEQ(new Decimal('682377946933004839').toFixed(7), '682377946933004839.0000000');
    arktest.assertEQ(new Decimal('0.00000000000000301722508588270616971784651380892069087869575').toFixed(9), '0.000000000');
    arktest.assertEQ(new Decimal('-356330174906.737986270704').toFixed(1), '-356330174906.7');
    arktest.assertEQ(new Decimal('771875969530483104163034745359742192923504').toFixed(15), '771875969530483104163034745359742192923504.000000000000000');
    arktest.assertEQ(new Decimal('-85.576121333642541652128540737082').toFixed(11), '-85.57612133364');
    arktest.assertEQ(new Decimal('-796870619982006783').toFixed(8), '-796870619982006783.00000000');
    arktest.assertEQ(new Decimal('985819').toFixed(1), '985819.0');
    arktest.assertEQ(new Decimal('0.00000000000000000007093034243958').toFixed(6), '0.000000');
    arktest.assertEQ(new Decimal('-0.0000000006400178083107075310177579449').toFixed(4), '-0.0000');
    arktest.assertEQ(new Decimal('-105570971372984').toFixed(1), '-105570971372984.0');
    arktest.assertEQ(new Decimal('4387745564661852845308.9422981508391799473309882').toFixed(14), '4387745564661852845308.94229815083918');
    arktest.assertEQ(new Decimal('-3978043797116373.159631').toFixed(1), '-3978043797116373.2');
    arktest.assertEQ(new Decimal('5.8961281424853232622594846548715356650909645537495738043051').toFixed(22), '5.8961281424853232622595');
    arktest.assertEQ(new Decimal('-0.000008153997055187438449482684680130340977472796176926').toFixed(25), '-0.0000081539970551874384495');
    arktest.assertEQ(new Decimal('-65').toFixed(1), '-65.0');
    arktest.assertEQ(new Decimal('-24012940205869').toFixed(3), '-24012940205869.000');
    arktest.assertEQ(new Decimal('0.000000000000000066985234999507396').toFixed(6), '0.000000');
    arktest.assertEQ(new Decimal('2935194283452.357389511578322069071242729133894').toFixed(23), '2935194283452.35738951157832206907124');
    arktest.assertEQ(new Decimal('542060.77713479586821060591807145320317').toFixed(4), '542060.7771');
    arktest.assertEQ(new Decimal('890815036528241639534803534813.59258860883716842586502472472').toFixed(24), '890815036528241639534803534813.592588608837168425865025');
    arktest.assertEQ(new Decimal('685877749980391051.1715').toFixed(2), '685877749980391051.17');
    arktest.assertEQ(new Decimal('47820177163147405231185068730213.89429254869826585947429821').toFixed(6), '47820177163147405231185068730213.894293');
    arktest.assertEQ(new Decimal('1536364305').toFixed(2), '1536364305.00');
    arktest.assertEQ(new Decimal('-85825125.5810188298').toFixed(7), '-85825125.5810188');
}

function testToFixedWithRMIsFive002() {
    arktest.assertEQ(new Decimal('-4983288214158634188.4572988600705359104').toFixed(7), '-4983288214158634188.4572989');
    arktest.assertEQ(new Decimal('0.0000000000000000017929073').toFixed(10), '0.0000000000');
    arktest.assertEQ(new Decimal('-1684136337600384671').toFixed(8), '-1684136337600384671.00000000');
    arktest.assertEQ(new Decimal('567733137127609543').toFixed(6), '567733137127609543.000000');
    arktest.assertEQ(new Decimal('607675679770387').toFixed(3), '607675679770387.000');
    arktest.assertEQ(new Decimal('43727409067319').toFixed(1), '43727409067319.0');
    arktest.assertEQ(new Decimal('-77274559773372606').toFixed(2), '-77274559773372606.00');
    arktest.assertEQ(new Decimal('-28855643486070486857836').toFixed(5), '-28855643486070486857836.00000');
    arktest.assertEQ(new Decimal('-915356570870.00041832252262829437239').toFixed(2), '-915356570870.00');
    arktest.assertEQ(new Decimal('0.00000000000004843552301428650828').toFixed(1), '0.0');
}

function testToFixedWithRMIsSix001() {
    Decimal.rounding = 6;
    arktest.assertEQ(new Decimal('-6614662975368684488885953285955838893900074215956').toFixed(2), '-6614662975368684488885953285955838893900074215956.00');
    arktest.assertEQ(new Decimal('8642393776898').toFixed(3), '8642393776898.000');
    arktest.assertEQ(new Decimal('3612207878528.1106344698085142865438924419').toFixed(19), '3612207878528.1106344698085142865');
    arktest.assertEQ(new Decimal('977910386.9343537').toFixed(2), '977910386.93');
    arktest.assertEQ(new Decimal('-8937937492688941768401276851642629965923372').toFixed(1), '-8937937492688941768401276851642629965923372.0');
    arktest.assertEQ(new Decimal('-8327876772303865517454752161.43761127529219047742910886').toFixed(19), '-8327876772303865517454752161.4376112752921904774');
    arktest.assertEQ(new Decimal('-27707').toFixed(2), '-27707.00');
    arktest.assertEQ(new Decimal('-7287595765894242586375.927004354837723619581861634319').toFixed(11), '-7287595765894242586375.92700435484');
    arktest.assertEQ(new Decimal('-0.000000000000023670858459165486137218740358207859').toFixed(5), '-0.00000');
    arktest.assertEQ(new Decimal('574676836802209077.641563875647945277493356873895').toFixed(5), '574676836802209077.64156');
    arktest.assertEQ(new Decimal('-3041038455237222898218053653661.87952947815').toFixed(11), '-3041038455237222898218053653661.87952947815');
    arktest.assertEQ(new Decimal('-628244132307').toFixed(3), '-628244132307.000');
    arktest.assertEQ(new Decimal('316566935167341670725238').toFixed(3), '316566935167341670725238.000');
    arktest.assertEQ(new Decimal('-77953301569468294124105528').toFixed(1), '-77953301569468294124105528.0');
    arktest.assertEQ(new Decimal('0.00460227833968584').toFixed(4), '0.0046');
    arktest.assertEQ(new Decimal('4323265763616518980379929221104').toFixed(1), '4323265763616518980379929221104.0');
    arktest.assertEQ(new Decimal('1674500565014237781637959673471730398.1120032995511774').toFixed(7), '1674500565014237781637959673471730398.1120033');
    arktest.assertEQ(new Decimal('872559441430499650732600166').toFixed(5), '872559441430499650732600166.00000');
    arktest.assertEQ(new Decimal('-87858304').toFixed(4), '-87858304.0000');
    arktest.assertEQ(new Decimal('-4158788').toFixed(3), '-4158788.000');
    arktest.assertEQ(new Decimal('3983930437416823631395.90934402583657').toFixed(4), '3983930437416823631395.9093');
    arktest.assertEQ(new Decimal('-14.5531936852106573016020290135814233645752955297443336').toFixed(7), '-14.5531937');
    arktest.assertEQ(new Decimal('0.00000000002200503795474854372849141363413146996972').toFixed(14), '0.00000000002201');
    arktest.assertEQ(new Decimal('0.00000000000000000894955653982033503846831364474746320232').toFixed(13), '0.0000000000000');
    arktest.assertEQ(new Decimal('61564779').toFixed(2), '61564779.00');
    arktest.assertEQ(new Decimal('-7').toFixed(1), '-7.0');
}

function testToFixedWithRMIsSix002() {
    arktest.assertEQ(new Decimal('-0.00000000000002845597622815028653703372125435938812845106').toFixed(24), '-0.000000000000028455976228');
    arktest.assertEQ(new Decimal('728982423193598397582409707715766595902844').toFixed(7), '728982423193598397582409707715766595902844.0000000');
    arktest.assertEQ(new Decimal('14538075860529.269793748').toFixed(10), '14538075860529.2697937480');
    arktest.assertEQ(new Decimal('0.00000000000000000007021566096').toFixed(3), '0.000');
    arktest.assertEQ(new Decimal('-5136066535080.86830591678842264063462546262988980471309228558').toFixed(29), '-5136066535080.86830591678842264063462546263');
    arktest.assertEQ(new Decimal('-3026751112367460839746524832112404665').toFixed(15), '-3026751112367460839746524832112404665.000000000000000');
    arktest.assertEQ(new Decimal('0.5').toFixed(), '0.5');
    arktest.assertEQ(new Decimal('-0.5').toFixed(0), '-0');
    arktest.assertEQ(new Decimal('10.5').toFixed(), '10.5');
    arktest.assertEQ(new Decimal('0.05').toFixed(), '0.05');
    arktest.assertEQ(new Decimal('0.4').toFixed(), '0.4');
    arktest.assertEQ(new Decimal('0.6').toFixed(), '0.6');
    arktest.assertEQ(new Decimal('1.5').toFixed(), '1.5');
    arktest.assertEQ(new Decimal('-1.5').toFixed(), '-1.5');
}

function testToFixedWithRMIsSeven() {
    Decimal.rounding = 7;
    arktest.assertEQ(new Decimal('0.05').toFixed(1), '0.1');
    arktest.assertEQ(new Decimal(0.5).toFixed(0), '1');
    arktest.assertEQ(new Decimal(0.54).toFixed(0), '1');
    arktest.assertEQ(new Decimal(0.55).toFixed(0), '1');
    arktest.assertEQ(new Decimal(0.56).toFixed(0), '1');
    arktest.assertEQ(new Decimal(-0.54).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-0.5).toFixed(0), '-0');
    arktest.assertEQ(new Decimal(-0.56).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-0.5).toFixed(1), '-0.5');
    arktest.assertEQ(new Decimal(1.25).toFixed(1), '1.3');
    arktest.assertEQ(new Decimal(-1.25).toFixed(1), '-1.2');
    arktest.assertEQ(new Decimal(234.20405).toFixed(4), '234.2041');
    arktest.assertEQ(new Decimal(-234.20405).toFixed(4), '-234.2040');
    arktest.assertEQ(new Decimal('234.204050000000000000000000000000006').toFixed(4), '234.2041');
    arktest.assertEQ(new Decimal('-234.20449').toFixed(4), '-234.2045');
    arktest.assertEQ(new Decimal('-234.204050000000000000000000000000006').toFixed(4), '-234.2041');
    arktest.assertEQ(new Decimal(999).toFixed(1), '999.0');
    arktest.assertEQ(new Decimal(999.5).toFixed(0), '1000');
    arktest.assertEQ(new Decimal(-999.5).toFixed(0), '-999');
    arktest.assertEQ(new Decimal(-999.5).toFixed(1), '-999.5');
    arktest.assertEQ(new Decimal('1.000000000000000000005').toFixed(17), '1.00000000000000000');
    arktest.assertEQ(new Decimal('1.000000000000000000005').toFixed(20), '1.00000000000000000001');
    arktest.assertEQ(new Decimal('-1.000000000000000000005').toFixed(17), '-1.00000000000000000');
    arktest.assertEQ(new Decimal('-1.000000000000000000005').toFixed(20), '-1.00000000000000000000');
}

function testToFixedWithRMIsEight() {
    Decimal.rounding = 8;
    arktest.assertEQ(new Decimal('0.05').toFixed(1), '0.0');
    arktest.assertEQ(new Decimal(0.5).toFixed(0), '0');
    arktest.assertEQ(new Decimal(0.54).toFixed(0), '1');
    arktest.assertEQ(new Decimal(0.55).toFixed(0), '1');
    arktest.assertEQ(new Decimal(0.56).toFixed(0), '1');
    arktest.assertEQ(new Decimal(-0.54).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-0.5).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-0.56).toFixed(0), '-1');
    arktest.assertEQ(new Decimal(-0.5).toFixed(1), '-0.5');
    arktest.assertEQ(new Decimal(1.25).toFixed(1), '1.2');
    arktest.assertEQ(new Decimal(-1.25).toFixed(1), '-1.3');
    arktest.assertEQ(new Decimal(234.20405).toFixed(4), '234.2040');
    arktest.assertEQ(new Decimal(-234.20405).toFixed(4), '-234.2041');
    arktest.assertEQ(new Decimal('234.20404999999999999999999999999999').toFixed(4), '234.2040');
    arktest.assertEQ(new Decimal('234.204050000000000000000000000000006').toFixed(4), '234.2041');
    arktest.assertEQ(new Decimal('-234.20449').toFixed(4), '-234.2045');
    arktest.assertEQ(new Decimal('-234.204050000000000000000000000000006').toFixed(4), '-234.2041');
    arktest.assertEQ(new Decimal(999).toFixed(1), '999.0');
    arktest.assertEQ(new Decimal(999.5).toFixed(0), '999');
    arktest.assertEQ(new Decimal(-999.5).toFixed(0), '-1000');
    arktest.assertEQ(new Decimal(-999.5).toFixed(1), '-999.5');
    arktest.assertEQ(new Decimal('1.000000000000000000005').toFixed(17), '1.00000000000000000');
    arktest.assertEQ(new Decimal('1.000000000000000000005').toFixed(20), '1.00000000000000000000');
    arktest.assertEQ(new Decimal('-1.000000000000000000005').toFixed(17), '-1.00000000000000000');
    arktest.assertEQ(new Decimal('-1.000000000000000000005').toFixed(20), '-1.00000000000000000001');
}
