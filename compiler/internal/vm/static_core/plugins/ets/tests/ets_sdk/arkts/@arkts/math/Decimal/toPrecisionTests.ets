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
    const suite = new arktest.ArkTestsuite("Decimal toPrecision API tests");
    suite.addTest("Decimal toPrecision with specific config", testToPrecisionWithSpecificConfig001);
    suite.addTest("Decimal toPrecision with specific config", testToPrecisionWithSpecificConfig002);
    suite.addTest("Decimal toPrecision with specific config", testToPrecisionWithSpecificConfig003);
    suite.addTest("Decimal toPrecision with rounding is 0", testToPrecisionWithRMIsZero001);
    suite.addTest("Decimal toPrecision with rounding is 0", testToPrecisionWithRMIsZero002);
    suite.addTest("Decimal toPrecision with rounding is 0", testToPrecisionWithRMIsZero003);
    suite.addTest("Decimal toPrecision with rounding is 1", testToPrecisionWithRMIsOne001);
    suite.addTest("Decimal toPrecision with rounding is 1", testToPrecisionWithRMIsOne002);
    suite.addTest("Decimal toPrecision with rounding is 2", testToPrecisionWithRMIsTwo001);
    suite.addTest("Decimal toPrecision with rounding is 2", testToPrecisionWithRMIsTwo002);
    suite.addTest("Decimal toPrecision with rounding is 3", testToPrecisionWithRMIsThree001);
    suite.addTest("Decimal toPrecision with rounding is 3", testToPrecisionWithRMIsThree002);
    suite.addTest("Decimal toPrecision with rounding is 4", testToPrecisionWithRMIsFour001);
    suite.addTest("Decimal toPrecision with rounding is 4", testToPrecisionWithRMIsFour002);
    suite.addTest("Decimal toPrecision with rounding is 5", testToPrecisionWithRMIsFive001);
    suite.addTest("Decimal toPrecision with rounding is 5", testToPrecisionWithRMIsFive002);
    suite.addTest("Decimal toPrecision with rounding is 6", testToPrecisionWithRMIsSix001);
    suite.addTest("Decimal toPrecision with rounding is 6", testToPrecisionWithRMIsSix002);
    suite.addTest("Decimal toPrecision with rounding is 6", testToPrecisionWithRMIsSix003);
    return suite.run();
}

function testToPrecisionWithSpecificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 40, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(1), '1e+27');
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(2), '1.2e+27');
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(3), '1.23e+27');
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(4), '1.235e+27');
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(5), '1.2345e+27');
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(6), '1.23450e+27');
    arktest.assertEQ(new Decimal('1.2345e+27').toPrecision(7), '1.234500e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(1), '-1e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(2), '-1.2e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(3), '-1.23e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(4), '-1.235e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(5), '-1.2345e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(6), '-1.23450e+27');
    arktest.assertEQ(new Decimal('-1.2345e+27').toPrecision(7), '-1.234500e+27');
    arktest.assertEQ(new Decimal(7).toPrecision(1), '7');
    arktest.assertEQ(new Decimal(7).toPrecision(2), '7.0');
    arktest.assertEQ(new Decimal(7).toPrecision(3), '7.00');
    arktest.assertEQ(new Decimal(-7).toPrecision(1), '-7');
    arktest.assertEQ(new Decimal(-7).toPrecision(2), '-7.0');
    arktest.assertEQ(new Decimal(-7).toPrecision(3), '-7.00');
    arktest.assertEQ(new Decimal(91).toPrecision(1), '9e+1');
    arktest.assertEQ(new Decimal(91).toPrecision(2), '91');
    arktest.assertEQ(new Decimal(91).toPrecision(3), '91.0');
    arktest.assertEQ(new Decimal(91).toPrecision(4), '91.00');
    arktest.assertEQ(new Decimal(-91).toPrecision(1), '-9e+1');
    arktest.assertEQ(new Decimal(-91).toPrecision(2), '-91');
    arktest.assertEQ(new Decimal(-91).toPrecision(3), '-91.0');
    arktest.assertEQ(new Decimal(-91).toPrecision(4), '-91.00');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(1), '9e+1');
}

function testToPrecisionWithSpecificConfig002() {
    arktest.assertEQ(new Decimal(91.1234).toPrecision(2), '91');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(3), '91.1');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(4), '91.12');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(5), '91.123');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(6), '91.1234');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(7), '91.12340');
    arktest.assertEQ(new Decimal(91.1234).toPrecision(8), '91.123400');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(1), '-9e+1');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(2), '-91');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(3), '-91.1');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(4), '-91.12');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(5), '-91.123');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(6), '-91.1234');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(7), '-91.12340');
    arktest.assertEQ(new Decimal(-91.1234).toPrecision(8), '-91.123400');
    arktest.assertEQ(new Decimal(NaN).toPrecision(1), 'NaN');
    arktest.assertEQ(new Decimal(Infinity).toPrecision(2), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).toPrecision(2), '-Infinity');
    arktest.assertEQ(new Decimal(0.000000555).toPrecision(15), '5.55000000000000e-7');
    arktest.assertEQ(new Decimal(-0.000000555).toPrecision(15), '-5.55000000000000e-7');
    arktest.assertEQ(new Decimal(-0.0000000012345).toPrecision(2), '-1.2e-9');
    arktest.assertEQ(new Decimal(-0.000000012345).toPrecision(2), '-1.2e-8');
    arktest.assertEQ(new Decimal(-0.00000012345).toPrecision(2), '-1.2e-7');
    arktest.assertEQ(new Decimal(123456789).toPrecision(1), '1e+8');
    arktest.assertEQ(new Decimal(123456789).toPrecision(9), '123456789');
    arktest.assertEQ(new Decimal(123456789).toPrecision(8), '1.2345679e+8');
    arktest.assertEQ(new Decimal(123456789).toPrecision(7), '1.234568e+8');
    arktest.assertEQ(new Decimal(-123456789).toPrecision(7), '-1.234568e+8');
}

function testToPrecisionWithSpecificConfig003() {
    arktest.assertEQ(new Decimal(-0.0000012345).toPrecision(2), '-0.0000012');
    arktest.assertEQ(new Decimal(-0.000012345).toPrecision(2), '-0.000012');
    arktest.assertEQ(new Decimal(-0.00012345).toPrecision(2), '-0.00012');
    arktest.assertEQ(new Decimal(-0.0012345).toPrecision(2), '-0.0012');
    arktest.assertEQ(new Decimal(-0.012345).toPrecision(2), '-0.012');
    arktest.assertEQ(new Decimal(-0.12345).toPrecision(2), '-0.12');
    arktest.assertEQ(new Decimal(-1.2345).toPrecision(2), '-1.2');
    arktest.assertEQ(new Decimal(-12.345).toPrecision(2), '-12');
    arktest.assertEQ(new Decimal(-123.45).toPrecision(2), '-1.2e+2');
    arktest.assertEQ(new Decimal(-1234.5).toPrecision(2), '-1.2e+3');
    arktest.assertEQ(new Decimal(-12345).toPrecision(2), '-1.2e+4');
    arktest.assertEQ(new Decimal(-12345.67).toPrecision(4), '-1.235e+4');
    arktest.assertEQ(new Decimal(-12344.67).toPrecision(4), '-1.234e+4');
    arktest.assertEQ(new Decimal(1.25).toPrecision(2), '1.3');
    arktest.assertEQ(new Decimal(1.35).toPrecision(2), '1.4');
}

function testToPrecisionWithRMIsZero001() {
    Decimal.rounding = 0;
    arktest.assertEQ(new Decimal(9631.01).toPrecision(1, 4), '1e+4');
    arktest.assertEQ(new Decimal(9950095.87).toPrecision(2, 4), '1.0e+7');
    arktest.assertEQ(new Decimal('9.856839969').toPrecision(1, 4), '1e+1');
    arktest.assertEQ(new Decimal('97.504').toPrecision(1, 4), '1e+2');
    arktest.assertEQ(new Decimal(97802.6).toPrecision(1, 4), '1e+5');
    arktest.assertEQ(new Decimal(9.9617).toPrecision(1, 4), '1e+1');
    arktest.assertEQ(new Decimal(989.2).toPrecision(1, 4), '1e+3');
    arktest.assertEQ(new Decimal(99576).toPrecision(2, 4), '1.0e+5');
    arktest.assertEQ(new Decimal('96236483.87').toPrecision(1, 4), '1e+8');
    arktest.assertEQ(new Decimal('-0.00001').toPrecision(1), '-0.00001');
    arktest.assertEQ(new Decimal('-0.00009').toPrecision(8), '-0.000090000000');
    arktest.assertEQ(new Decimal('-0.0000007').toPrecision(1), '-7e-7');
    arktest.assertEQ(new Decimal('68.931683406184761912218250317').toPrecision(15), '68.9316834061848');
    arktest.assertEQ(new Decimal('7860101808970473167417935916.60087069').toPrecision(17), '7.8601018089704732e+27');
    arktest.assertEQ(new Decimal('0.0000000000321445885399803244067719798337437062').toPrecision(42), '3.21445885399803244067719798337437062000000e-11');
    arktest.assertEQ(new Decimal('-8171786349835057630612358814.162756977984').toPrecision(37), '-8171786349835057630612358814.162756978');
    arktest.assertEQ(new Decimal('3340.903970019817086594869184429527413533291595472085').toPrecision(11), '3340.9039701');
    arktest.assertEQ(new Decimal('-7269097658095414435895.916118111573974542730031306').toPrecision(50), '-7269097658095414435895.9161181115739745427300313060');
    arktest.assertEQ(new Decimal('0.00000632206077863').toPrecision(6), '0.00000632207');
    arktest.assertEQ(new Decimal('573').toPrecision(1), '6e+2');
    arktest.assertEQ(new Decimal('0.000000738').toPrecision(2), '7.4e-7');
    arktest.assertEQ(new Decimal('-0.0000005031560306227217140253964236911907612837').toPrecision(7), '-5.031561e-7');
    arktest.assertEQ(new Decimal('-429050053964').toPrecision(4), '-4.291e+11');
    arktest.assertEQ(new Decimal('85131637').toPrecision(4), '8.514e+7');
    arktest.assertEQ(new Decimal('-0.000000003326783057540398442677461').toPrecision(2), '-3.4e-9');
}

function testToPrecisionWithRMIsZero002() {
    arktest.assertEQ(new Decimal('0.00000000000000000006940429596272251146200868514973032594273').toPrecision(17), '6.9404295962722512e-20');
    arktest.assertEQ(new Decimal('-828376248340605120247.15155295013990774586360178257303370779').toPrecision(32), '-828376248340605120247.15155295014');
    arktest.assertEQ(new Decimal('-7982750.6677764682946015520272838914918899297118139169410659').toPrecision(5), '-7.9828e+6');
    arktest.assertEQ(new Decimal('0.007126103937225425278801997738').toPrecision(24), '0.00712610393722542527880200');
    arktest.assertEQ(new Decimal('-56242').toPrecision(2), '-5.7e+4');
    arktest.assertEQ(new Decimal('-8928855203945443164.755136735230293536124112124').toPrecision(37), '-8928855203945443164.755136735230293537');
    arktest.assertEQ(new Decimal('5218572327.98424443372003772604597054153304').toPrecision(12), '5218572327.99');
    arktest.assertEQ(new Decimal('71707870535238750871516796339.59678962573869890935').toPrecision(31), '71707870535238750871516796339.60');
    arktest.assertEQ(new Decimal('88817462.71379822206524285939115943006583441400005007918').toPrecision(24), '88817462.7137982220652429');
    arktest.assertEQ(new Decimal('0.000000003').toPrecision(6), '3.00000e-9');
    arktest.assertEQ(new Decimal('-6.05291095813493573191').toPrecision(4), '-6.053');
    arktest.assertEQ(new Decimal('65163082867698740076').toPrecision(12), '6.51630828677e+19');
    arktest.assertEQ(new Decimal('2483202135696501.60187898870193199949004966876115645').toPrecision(24), '2483202135696501.60187899');
    arktest.assertEQ(new Decimal('0.000000000107650515680635692286894826641576642261').toPrecision(5), '1.0766e-10');
    arktest.assertEQ(new Decimal('642724503819056076.659397077514269963295024012414').toPrecision(42), '642724503819056076.659397077514269963295025');
    arktest.assertEQ(new Decimal('-7119169102619893823635.32141854354').toPrecision(5), '-7.1192e+21');
    arktest.assertEQ(new Decimal('-0.000000067174812556406388291019461146732616998258').toPrecision(31), '-6.717481255640638829101946114674e-8');
    arktest.assertEQ(new Decimal('-12.4197645179995365323309894').toPrecision(10), '-12.41976452');
    arktest.assertEQ(new Decimal('-0.00000000000000065292587801264491162499546440178399210241129').toPrecision(46), '-6.529258780126449116249954644017839921024112900e-16');
    arktest.assertEQ(new Decimal('-441838').toPrecision(7), '-441838.0');
    arktest.assertEQ(new Decimal('0.000000011282852935929493101783925259749957192').toPrecision(16), '1.128285293592950e-8');
    arktest.assertEQ(new Decimal('-86548567').toPrecision(7), '-8.654857e+7');
    arktest.assertEQ(new Decimal('0.00000038883293855303994672627854769926811949').toPrecision(17), '3.8883293855303995e-7');
    arktest.assertEQ(new Decimal('0.00000000000032587').toPrecision(9), '3.25870000e-13');
    arktest.assertEQ(new Decimal('3701031.59037494113').toPrecision(4), '3.702e+6');
    arktest.assertEQ(new Decimal('-3580077435.93682917449675702508371046631533').toPrecision(36), '-3580077435.93682917449675702508371047');
    arktest.assertEQ(new Decimal('-7.4').toPrecision(4), '-7.400');
}

function testToPrecisionWithRMIsZero003() {
    arktest.assertEQ(new Decimal('109519523263844229810.067657779734413280795410968892638').toPrecision(24), '109519523263844229810.068');
    arktest.assertEQ(new Decimal('-509247322311590671954830.8684766061855').toPrecision(35), '-509247322311590671954830.86847660619');
    arktest.assertEQ(new Decimal('0.00000000075518638430980800496570562671727889997').toPrecision(35), '7.5518638430980800496570562671727890e-10');
    arktest.assertEQ(new Decimal('-5056721600639122835615986051.4688319428182').toPrecision(43), '-5056721600639122835615986051.468831942818200');
    arktest.assertEQ(new Decimal('-0.000000000000000179614686112555178588617182925146').toPrecision(43), '-1.796146861125551785886171829251460000000000e-16');
    arktest.assertEQ(new Decimal('599').toPrecision(2), '6.0e+2');
    arktest.assertEQ(new Decimal('0.00000000000000076199293').toPrecision(7), '7.619930e-16');
    arktest.assertEQ(new Decimal('834668.237012103815961019258574789273273342').toPrecision(25), '834668.2370121038159610193');
    arktest.assertEQ(new Decimal('-392251395952329649490767912.240768552138247705202732').toPrecision(24), '-3.92251395952329649490768e+26');
    arktest.assertEQ(new Decimal('-47504099413385554632166.50972492550706').toPrecision(27), '-47504099413385554632166.5098');
}

function testToPrecisionWithRMIsOne001() {
    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('-1336106841').toPrecision(2, 0), '-1.4e+9');
    arktest.assertEQ(new Decimal('-2244450.2134814273335262397290334104071203538487453309626146').toPrecision(23, 0), '-2244450.2134814273335263');
    arktest.assertEQ(new Decimal('873625255363763952428129881990.679929486040461455296118489').toPrecision(3, 0), '8.74e+29');
    arktest.assertEQ(new Decimal('-1.8545354973317961318592328878502252820666161607740183').toPrecision(30, 0), '-1.85453549733179613185923288786');
    arktest.assertEQ(new Decimal('431.71506519265522010949747887049').toPrecision(13, 0), '431.7150651927');
    arktest.assertEQ(new Decimal('-8606297211156287.5252002375256362382564355963505470716151').toPrecision(30, 0), '-8606297211156287.52520023752564');
    arktest.assertEQ(new Decimal('-8463488970828351722405003.220603').toPrecision(11, 0), '-8.4634889709e+24');
    arktest.assertEQ(new Decimal('-844789036.52397268892').toPrecision(16), '-844789036.5239726');
    arktest.assertEQ(new Decimal('-5056.206290127678787491852732096790643060542').toPrecision(42), '-5056.20629012767878749185273209679064306054');
    arktest.assertEQ(new Decimal('-0.32875191313148737635018598702989525').toPrecision(37), '-0.3287519131314873763501859870298952500');
    arktest.assertEQ(new Decimal('-60729764').toPrecision(8), '-60729764');
    arktest.assertEQ(new Decimal('-0.00000000000007622481594531380999826456196664586').toPrecision(4), '-7.622e-14');
    arktest.assertEQ(new Decimal('-4686402261639729535.7363244924747488').toPrecision(31), '-4686402261639729535.736324492474');
    arktest.assertEQ(new Decimal('-2').toPrecision(2), '-2.0');
    arktest.assertEQ(new Decimal('-13801188035233586637950193108.135925743814734511256495').toPrecision(55), '-13801188035233586637950193108.13592574381473451125649500');
    arktest.assertEQ(new Decimal('0.00008073275871498397993').toPrecision(24), '0.0000807327587149839799300000');
    arktest.assertEQ(new Decimal('-0.00000006').toPrecision(7), '-6.000000e-8');
    arktest.assertEQ(new Decimal('-383574993535').toPrecision(9), '-3.83574993e+11');
    arktest.assertEQ(new Decimal('0.000000000000076987').toPrecision(17), '7.6987000000000000e-14');
    arktest.assertEQ(new Decimal('80928866804.6112050947427973864826014844575374353').toPrecision(27), '80928866804.6112050947427973');
    arktest.assertEQ(new Decimal('-0.0073014067221009206110062377503733').toPrecision(6), '-0.00730140');
    arktest.assertEQ(new Decimal('2721047738841604910360884862459.4086993273252009015').toPrecision(27), '2.72104773884160491036088486e+30');
    arktest.assertEQ(new Decimal('30087807819177335941398228.1424107931203').toPrecision(19), '3.008780781917733594e+25');
    arktest.assertEQ(new Decimal('-0.00000000000000000013152892077961366915825014697229779786776').toPrecision(60), '-1.31528920779613669158250146972297797867760000000000000000000e-19');
    arktest.assertEQ(new Decimal('-858982311008.257025719798657844609315293821').toPrecision(2), '-8.5e+11');
    arktest.assertEQ(new Decimal('-0.0000000000036312827608449878').toPrecision(5), '-3.6312e-12');
    arktest.assertEQ(new Decimal('-0.006').toPrecision(5), '-0.0060000');
    arktest.assertEQ(new Decimal('-465727983501322687372765').toPrecision(6), '-4.65727e+23');
}

function testToPrecisionWithRMIsOne002() {
    arktest.assertEQ(new Decimal('-0.000003323316666280366035430077076052').toPrecision(18), '-0.00000332331666628036603');
    arktest.assertEQ(new Decimal('0.00000003533702791135712510338001418872124').toPrecision(7), '3.533702e-8');
    arktest.assertEQ(new Decimal('-0.0434').toPrecision(4), '-0.04340');
    arktest.assertEQ(new Decimal('-597340.2785660690868585878522362354706741').toPrecision(36), '-597340.278566069086858587852236235470');
    arktest.assertEQ(new Decimal('0.00000006').toPrecision(4), '6.000e-8');
    arktest.assertEQ(new Decimal('-0.00000000000000000036243233591127762966338166').toPrecision(19), '-3.624323359112776296e-19');
    arktest.assertEQ(new Decimal('-3731378568692042924197.15400334142251496795634388').toPrecision(25), '-3731378568692042924197.154');
    arktest.assertEQ(new Decimal('-68249040894032065692.62771690318493').toPrecision(22), '-68249040894032065692.62');
    arktest.assertEQ(new Decimal('8786096722661914.89732851188880184891692993684242690315').toPrecision(24), '8786096722661914.89732851');
    arktest.assertEQ(new Decimal('-0.00000000000184133215362813472644863729').toPrecision(38), '-1.8413321536281347264486372900000000000e-12');
    arktest.assertEQ(new Decimal('0.0000000040395827543504045').toPrecision(2), '4.0e-9');
    arktest.assertEQ(new Decimal('-29427119846374896').toPrecision(5), '-2.9427e+16');
    arktest.assertEQ(new Decimal('-917760614.45404359204911454').toPrecision(10), '-917760614.4');
    arktest.assertEQ(new Decimal('89427').toPrecision(1), '8e+4');
    arktest.assertEQ(new Decimal('0.0000092032398813435695382866726').toPrecision(27), '0.00000920323988134356953828667260');
    arktest.assertEQ(new Decimal('82068995955708118').toPrecision(2), '8.2e+16');
    arktest.assertEQ(new Decimal('335195944828445911672446409.3379497158141').toPrecision(12), '3.35195944828e+26');
    arktest.assertEQ(new Decimal('-0.00000000389774891030223957363124620581272897758735065471').toPrecision(12), '-3.89774891030e-9');
    arktest.assertEQ(new Decimal('-4').toPrecision(1), '-4');
    arktest.assertEQ(new Decimal('8').toPrecision(1), '8');
    arktest.assertEQ(new Decimal('0.000000000141172955693912934219137966').toPrecision(33), '1.41172955693912934219137966000000e-10');
    arktest.assertEQ(new Decimal('92148111958857').toPrecision(6), '9.21481e+13');
    arktest.assertEQ(new Decimal('-0.0000000000000000058599759784328539').toPrecision(16), '-5.859975978432853e-18');
    arktest.assertEQ(new Decimal('-72').toPrecision(3), '-72.0');
    arktest.assertEQ(new Decimal('3785098751297.89299119509940797071574729867819252140059').toPrecision(38), '3785098751297.8929911950994079707157472');
    arktest.assertEQ(new Decimal('43893416753778361.297703358127215475077814').toPrecision(3), '4.38e+16');
    arktest.assertEQ(new Decimal('-33110.2909623520267070846514').toPrecision(10), '-33110.29096');
    arktest.assertEQ(new Decimal('-74.3830525178488270772048643629212191403649548392158614').toPrecision(43), '-74.38305251784882707720486436292121914036495');
}

function testToPrecisionWithRMIsTwo001() {
    Decimal.rounding = 2;
    arktest.assertEQ(new Decimal('5805164734299168659.6173113885173384955443').toPrecision(3, 1), '5.80e+18');
    arktest.assertEQ(new Decimal('-1.719875889271327133154458155573493605566221534').toPrecision(16, 1), '-1.719875889271327');
    arktest.assertEQ(new Decimal('113.672129563441659725876055771857758675550104070419635029').toPrecision(12, 1), '113.672129563');
    arktest.assertEQ(new Decimal('-77950052814622081084397.91853869253589242574').toPrecision(24, 1), '-77950052814622081084397.9');
    arktest.assertEQ(new Decimal('4531069852787151785292512309.2901993579425172826443679877').toPrecision(9, 1), '4.53106985e+27');
    arktest.assertEQ(new Decimal('45285.2460896131694164407978407142422013937').toPrecision(32, 1), '45285.246089613169416440797840714');
    arktest.assertEQ(new Decimal('307760226411464.73332680798632996332324381779707').toPrecision(31, 1), '307760226411464.7333268079863299');
    arktest.assertEQ(new Decimal('-0.0300921721159558').toPrecision(3), '-0.0300');
    arktest.assertEQ(new Decimal('65317841202.2094985937177227348012464402154').toPrecision(34), '65317841202.20949859371772273480125');
    arktest.assertEQ(new Decimal('-892315754952021994731329589682.1894180393920044085713').toPrecision(14), '-8.9231575495202e+29');
    arktest.assertEQ(new Decimal('-0.0000000280756792028583066').toPrecision(11), '-2.8075679202e-8');
    arktest.assertEQ(new Decimal('9714558552').toPrecision(6), '9.71456e+9');
    arktest.assertEQ(new Decimal('0.00000000029514099281').toPrecision(11), '2.9514099281e-10');
    arktest.assertEQ(new Decimal('-124459985101107').toPrecision(6), '-1.24459e+14');
    arktest.assertEQ(new Decimal('0.0000734657394154607815562372').toPrecision(30), '0.0000734657394154607815562372000000');
    arktest.assertEQ(new Decimal('1787195303539715').toPrecision(15), '1.78719530353972e+15');
    arktest.assertEQ(new Decimal('-2861102528').toPrecision(2), '-2.8e+9');
    arktest.assertEQ(new Decimal('-0.00000000874480375581').toPrecision(15), '-8.74480375581000e-9');
    arktest.assertEQ(new Decimal('-1792404726015427380.248150830448457643618022').toPrecision(43), '-1792404726015427380.248150830448457643618022');
    arktest.assertEQ(new Decimal('-678437320202616518.22201579122092864').toPrecision(34), '-678437320202616518.2220157912209286');
    arktest.assertEQ(new Decimal('-0.000000019373049152157802208097998096558936746196727718').toPrecision(46), '-1.937304915215780220809799809655893674619672771e-8');
    arktest.assertEQ(new Decimal('824172.15863347130174103086069960571').toPrecision(26), '824172.15863347130174103087');
    arktest.assertEQ(new Decimal('0.00000000190040714061724').toPrecision(18), '1.90040714061724000e-9');
    arktest.assertEQ(new Decimal('-1634488249956745498.58311123049258868631623840423306').toPrecision(24), '-1634488249956745498.58311');
    arktest.assertEQ(new Decimal('0.0000019600923098540334001755857361187871270117098').toPrecision(47), '0.0000019600923098540334001755857361187871270117098000');
    arktest.assertEQ(new Decimal('83829').toPrecision(4), '8.383e+4');
    arktest.assertEQ(new Decimal('284330612033786406376718').toPrecision(19), '2.843306120337864064e+23');
    arktest.assertEQ(new Decimal('1862340943682995.08270612464203237562317928642459').toPrecision(6), '1.86235e+15');
}

function testToPrecisionWithRMIsTwo002() {
    arktest.assertEQ(new Decimal('-23195312138083').toPrecision(3), '-2.31e+13');
    arktest.assertEQ(new Decimal('5450236028274773541895.65198933808968167192289601277').toPrecision(7), '5.450237e+21');
    arktest.assertEQ(new Decimal('-0.00897641974940807545386111786545972434475187220274239581167').toPrecision(31), '-0.008976419749408075453861117865459');
    arktest.assertEQ(new Decimal('-761181660548661030.25539542029').toPrecision(20), '-761181660548661030.25');
    arktest.assertEQ(new Decimal('-1844205.936199580689273072905714475263817').toPrecision(15), '-1844205.93619958');
    arktest.assertEQ(new Decimal('4842.77906784902805070438222238898372327092242428134814721').toPrecision(42), '4842.77906784902805070438222238898372327093');
    arktest.assertEQ(new Decimal('-416119895344562950350397179').toPrecision(25), '-4.161198953445629503503971e+26');
    arktest.assertEQ(new Decimal('10836').toPrecision(4), '1.084e+4');
    arktest.assertEQ(new Decimal('0.0000000871081704218174598654542083').toPrecision(30), '8.71081704218174598654542083000e-8');
    arktest.assertEQ(new Decimal('7913968291641940848703040206324645237.8515176490912667096').toPrecision(8), '7.9139683e+36');
    arktest.assertEQ(new Decimal('-0.000008').toPrecision(1), '-0.000008');
    arktest.assertEQ(new Decimal('83660085624983922907621996804192921.3992927').toPrecision(11), '8.3660085625e+34');
    arktest.assertEQ(new Decimal('0.000006980263007423150706324065130475391').toPrecision(10), '0.000006980263008');
    arktest.assertEQ(new Decimal('-31348084528321454060964445534333629317.69561497283830023').toPrecision(55), '-31348084528321454060964445534333629317.69561497283830023');
    arktest.assertEQ(new Decimal('-2417953792643886.3485495754363678888681996409674308643').toPrecision(53), '-2417953792643886.3485495754363678888681996409674308643');
    arktest.assertEQ(new Decimal('3982592').toPrecision(2), '4.0e+6');
    arktest.assertEQ(new Decimal('-2092315.0150297222').toPrecision(19), '-2092315.015029722200');
    arktest.assertEQ(new Decimal('-364992136844916.909223894931280218350055327754935').toPrecision(22), '-364992136844916.9092238');
    arktest.assertEQ(new Decimal('8333642861002789136219873').toPrecision(3), '8.34e+24');
    arktest.assertEQ(new Decimal('760088371794122.3380234188299740029832128019574765416').toPrecision(14), '7.6008837179413e+14');
    arktest.assertEQ(new Decimal('-6655726127').toPrecision(11), '-6655726127.0');
}

function testToPrecisionWithRMIsThree001() {
    Decimal.rounding = 3;
    arktest.assertEQ(new Decimal('-172186735288586033321621121024.11240623').toPrecision(11, 2), '-1.7218673528e+29');
    arktest.assertEQ(new Decimal('-33197729862068219255677464974').toPrecision(3, 2), '-3.31e+28');
    arktest.assertEQ(new Decimal('-483519132605694848658321267839.23575134378118945659616358').toPrecision(10, 2), '-4.835191326e+29');
    arktest.assertEQ(new Decimal('7.24882150443803').toPrecision(2, 2), '7.3');
    arktest.assertEQ(new Decimal('-89186640077683569.4070614276736450982125609').toPrecision(29, 2), '-89186640077683569.407061427673');
    arktest.assertEQ(new Decimal('-49379651041268.548293').toPrecision(15, 2), '-49379651041268.5');
    arktest.assertEQ(new Decimal('-7685054.17489171951660508194254495141726065698575306365447451').toPrecision(21, 2), '-7685054.17489171951660');
    arktest.assertEQ(new Decimal('-39449414270333.925852213834759031494508489474').toPrecision(26), '-39449414270333.925852213835');
    arktest.assertEQ(new Decimal('-7.50437989975503711836768').toPrecision(12), '-7.50437989976');
    arktest.assertEQ(new Decimal('-0.00000430397576').toPrecision(16), '-0.000004303975760000000');
    arktest.assertEQ(new Decimal('-16040233916257241895.9765063397398857').toPrecision(34), '-16040233916257241895.97650633973989');
    arktest.assertEQ(new Decimal('-7438.9287248601393818639176907606').toPrecision(20), '-7438.9287248601393819');
    arktest.assertEQ(new Decimal('0.000000985746558429876825600458537705318327799').toPrecision(13), '9.857465584298e-7');
    arktest.assertEQ(new Decimal('532637.90959835472848504665779583159203905641996').toPrecision(37), '532637.9095983547284850466577958315920');
    arktest.assertEQ(new Decimal('-1404166952915258058306475434520.7856110230505157').toPrecision(12), '-1.40416695292e+30');
    arktest.assertEQ(new Decimal('60346876.6670832429026869255506808488').toPrecision(36), '60346876.6670832429026869255506808488');
    arktest.assertEQ(new Decimal('-252466132238128405832984').toPrecision(9), '-2.52466133e+23');
    arktest.assertEQ(new Decimal('55').toPrecision(2), '55');
    arktest.assertEQ(new Decimal('-63075151.9624657765163257922531779394931714').toPrecision(41), '-63075151.962465776516325792253177939493172');
    arktest.assertEQ(new Decimal('741146113404361548.543142388').toPrecision(7), '7.411461e+17');
    arktest.assertEQ(new Decimal('-58835755359067474972692072494278983.6314961114191480012916').toPrecision(36), '-58835755359067474972692072494278983.7');
    arktest.assertEQ(new Decimal('-333').toPrecision(3), '-333');
    arktest.assertEQ(new Decimal('72470760481059').toPrecision(6), '7.24707e+13');
    arktest.assertEQ(new Decimal('39232618.151351544223399576553545438981252').toPrecision(36), '39232618.1513515442233995765535454389');
    arktest.assertEQ(new Decimal('-357994').toPrecision(1), '-4e+5');
    arktest.assertEQ(new Decimal('-18904.11335233460016293296574557643545512393801643609213933').toPrecision(3), '-1.90e+4');
    arktest.assertEQ(new Decimal('-6585152111956929.9243094771233289848761842728287629').toPrecision(52), '-6585152111956929.924309477123328984876184272828762900');
    arktest.assertEQ(new Decimal('0.0000004505328').toPrecision(4), '4.505e-7');
    arktest.assertEQ(new Decimal('-24125965461845662271').toPrecision(14), '-2.4125965461846e+19');
}

function testToPrecisionWithRMIsThree002() {
    arktest.assertEQ(new Decimal('4826731373891127996812671510065700.871947701').toPrecision(9), '4.82673137e+33');
    arktest.assertEQ(new Decimal('-6621278.1120573461544975284970826524341806671316100080257485').toPrecision(8), '-6621278.2');
    arktest.assertEQ(new Decimal('-0.00000018015392869565386634525164264799463344376205007391').toPrecision(56), '-1.8015392869565386634525164264799463344376205007391000000e-7');
    arktest.assertEQ(new Decimal('-0.00026465463574439280006655492609887592672292239588307259').toPrecision(35), '-0.00026465463574439280006655492609887593');
    arktest.assertEQ(new Decimal('4.8781522898830009076096556452567').toPrecision(18), '4.87815228988300090');
    arktest.assertEQ(new Decimal('-511071171995240827790778012016163902').toPrecision(32), '-5.1107117199524082779077801201617e+35');
    arktest.assertEQ(new Decimal('147342425157068905574.390834406').toPrecision(20), '1.4734242515706890557e+20');
    arktest.assertEQ(new Decimal('-4019325091848890817268596991.8152').toPrecision(34), '-4019325091848890817268596991.815200');
    arktest.assertEQ(new Decimal('384715413967421').toPrecision(2), '3.8e+14');
    arktest.assertEQ(new Decimal('7483444.498791364040133403947480439118040376737700653').toPrecision(9), '7483444.49');
    arktest.assertEQ(new Decimal('-594538312.625485172379').toPrecision(13), '-594538312.6255');
    arktest.assertEQ(new Decimal('0.00753').toPrecision(6), '0.00753000');
    arktest.assertEQ(new Decimal('81440148247675.27449603492606125135884').toPrecision(11), '8.1440148247e+13');
    arktest.assertEQ(new Decimal('8444003009300239495556').toPrecision(13), '8.444003009300e+21');
    arktest.assertEQ(new Decimal('2308.1529840912558574923966042774800185916972327325289261').toPrecision(53), '2308.1529840912558574923966042774800185916972327325289');
    arktest.assertEQ(new Decimal('2674.698673623').toPrecision(3), '2.67e+3');
    arktest.assertEQ(new Decimal('-2828191361802874708546255368471.80800005766').toPrecision(27), '-2.82819136180287470854625537e+30');
    arktest.assertEQ(new Decimal('518250411').toPrecision(9), '518250411');
    arktest.assertEQ(new Decimal('32661.9135347256259375001777960775509').toPrecision(2), '3.2e+4');
    arktest.assertEQ(new Decimal('29.153476022164169919735054013077734177').toPrecision(22), '29.15347602216416991973');
    arktest.assertEQ(new Decimal('-4611285536613066107912600830385').toPrecision(19), '-4.611285536613066108e+30');
    arktest.assertEQ(new Decimal('-51774110.070514498902397536020716707114309435632036586').toPrecision(48), '-51774110.0705144989023975360207167071143094356321');
    arktest.assertEQ(new Decimal('-11969053.9052').toPrecision(10), '-11969053.91');
}

function testToPrecisionWithRMIsFour001() {
    Decimal.rounding = 4;
    arktest.assertEQ(new Decimal('687378946204028.408158998985701430935094').toPrecision(30, 3), '687378946204028.408158998985701');
    arktest.assertEQ(new Decimal('42.4523909443358871476552683504968536100051').toPrecision(5, 3), '42.452');
    arktest.assertEQ(new Decimal('-22771061110217019663705702.44170142085171219649140996').toPrecision(40, 3), '-22771061110217019663705702.44170142085172');
    arktest.assertEQ(new Decimal('-1470.6403099740161675122356986295857257144815364').toPrecision(34, 3), '-1470.640309974016167512235698629586');
    arktest.assertEQ(new Decimal('-1110227398804733429555663947.06619').toPrecision(7, 3), '-1.110228e+27');
    arktest.assertEQ(new Decimal('-648982371105405071851661301').toPrecision(11, 3), '-6.4898237111e+26');
    arktest.assertEQ(new Decimal('-4641197449469148.658850361201902222').toPrecision(31, 3), '-4641197449469148.658850361201903');
    arktest.assertEQ(new Decimal('79053003797878062.6454954').toPrecision(13), '7.905300379788e+16');
    arktest.assertEQ(new Decimal('-0.00000000000068349').toPrecision(12), '-6.83490000000e-13');
    arktest.assertEQ(new Decimal('-62760641815.690849736612012010742308663').toPrecision(28), '-62760641815.69084973661201201');
    arktest.assertEQ(new Decimal('0.000704496313').toPrecision(3), '0.000704');
    arktest.assertEQ(new Decimal('82926865286287.88523573683428608303107210630792996432').toPrecision(51), '82926865286287.8852357368342860830310721063079299643', );
    arktest.assertEQ(new Decimal('-0.00032388272393900301214220090248744799603424908').toPrecision(29), '-0.00032388272393900301214220090249');
    arktest.assertEQ(new Decimal('8621641486938.4837308885005093571508566552428700982454').toPrecision(2), '8.6e+12');
    arktest.assertEQ(new Decimal('2').toPrecision(1), '2');
    arktest.assertEQ(new Decimal('146414401170525590746.047955203899370771105088').toPrecision(20), '1.4641440117052559075e+20');
    arktest.assertEQ(new Decimal('3511.925583').toPrecision(10), '3511.925583');
    arktest.assertEQ(new Decimal('2861824.253079699095727765750377038689').toPrecision(22), '2861824.253079699095728');
    arktest.assertEQ(new Decimal('-39400977564.548924098664431671700066962').toPrecision(10), '-3.940097756e+10');
    arktest.assertEQ(new Decimal('-888').toPrecision(3), '-888');
    arktest.assertEQ(new Decimal('-0.00030210612521372498814172125610432438685').toPrecision(30), '-0.000302106125213724988141721256104');
    arktest.assertEQ(new Decimal('6943.480455255531561580965042850266831249032130818358478956').toPrecision(32), '6943.4804552555315615809650428503');
    arktest.assertEQ(new Decimal('3365678.3397481381125085749').toPrecision(7), '3365678');
    arktest.assertEQ(new Decimal('-53943374313769567458.386865325').toPrecision(11), '-5.3943374314e+19');
    arktest.assertEQ(new Decimal('-6.6788050922551015054225285214704948993825429849797925563674').toPrecision(51), '-6.67880509225510150542252852147049489938254298497979');
    arktest.assertEQ(new Decimal('1364240644139816224.60228356028').toPrecision(6), '1.36424e+18');
    arktest.assertEQ(new Decimal('141023647795041672538410.84935693266374259666015274447').toPrecision(19), '1.410236477950416725e+23');
    arktest.assertEQ(new Decimal('-802.81776500697712984253334522').toPrecision(9), '-802.817765');
    arktest.assertEQ(new Decimal('-5.27621072242469066889626007535503721885096').toPrecision(40), '-5.276210722424690668896260075355037218851');
    arktest.assertEQ(new Decimal('-0.0008742095689707877849902027926289294748756775668387').toPrecision(15), '-0.000874209568970788');
}

function testToPrecisionWithRMIsFour002() {
    arktest.assertEQ(new Decimal('0.09205383316200189249855864903410820435666385119723209239').toPrecision(14), '0.092053833162002');
    arktest.assertEQ(new Decimal('0.0000000000000706562983181282092835675843980510112').toPrecision(17), '7.0656298318128209e-14');
    arktest.assertEQ(new Decimal('-8665115168521166587').toPrecision(18), '-8.66511516852116659e+18');
    arktest.assertEQ(new Decimal('33490648463534229842937.79268276945692333064632966129475').toPrecision(11), '3.3490648464e+22');
    arktest.assertEQ(new Decimal('-39041587174692569176.827407061541838942655371389185').toPrecision(37), '-39041587174692569176.82740706154183894');
    arktest.assertEQ(new Decimal('-3834').toPrecision(5), '-3834.0');
    arktest.assertEQ(new Decimal('-0.00891238264481441877626863').toPrecision(25), '-0.008912382644814418776268630');
    arktest.assertEQ(new Decimal('-206119').toPrecision(2), '-2.1e+5');
    arktest.assertEQ(new Decimal('0.000000048334').toPrecision(12), '4.83340000000e-8');
    arktest.assertEQ(new Decimal('0.000000000000000000318519653367523052').toPrecision(31), '3.185196533675230520000000000000e-19');
    arktest.assertEQ(new Decimal('0.000000000000000060431217298488095562718496137220939447806').toPrecision(56), '6.0431217298488095562718496137220939447806000000000000000e-17');
}

function testToPrecisionWithRMIsFive001() {
    Decimal.rounding = 5;
    arktest.assertEQ(new Decimal('-786589693451258754942279859.3834').toPrecision(1, 4), '-8e+26');
    arktest.assertEQ(new Decimal('-26').toPrecision(3, 4), '-26.0');
    arktest.assertEQ(new Decimal('-846222672789.2087639320702375427266333530942524245').toPrecision(10, 4), '-8.462226728e+11');
    arktest.assertEQ(new Decimal('-0.0000004019666978288041783154210868').toPrecision(1, 4), '-4e-7');
    arktest.assertEQ(new Decimal('-315609.775843992').toPrecision(15, 4), '-315609.775843992');
    arktest.assertEQ(new Decimal('-3318880945').toPrecision(4, 4), '-3.319e+9');
    arktest.assertEQ(new Decimal('-6.2847').toPrecision(1, 4), '-6');
    arktest.assertEQ(new Decimal('-1408003897645960.648499616456').toPrecision(28), '-1408003897645960.648499616456');
    arktest.assertEQ(new Decimal('-1').toPrecision(2), '-1.0');
    arktest.assertEQ(new Decimal('-827860423543649').toPrecision(3), '-8.28e+14');
    arktest.assertEQ(new Decimal('0.00054398953021585321711560388889590290139888').toPrecision(29), '0.00054398953021585321711560388890');
    arktest.assertEQ(new Decimal('-0.000000004408792').toPrecision(4), '-4.409e-9');
    arktest.assertEQ(new Decimal('0.0000000004').toPrecision(5), '4.0000e-10');
    arktest.assertEQ(new Decimal('34001779327925905').toPrecision(3), '3.40e+16');
    arktest.assertEQ(new Decimal('-90332622851356543193546536340366547').toPrecision(3), '-9.03e+34');
    arktest.assertEQ(new Decimal('-45320100856429143.39155209710530673318222777').toPrecision(5), '-4.5320e+16');
    arktest.assertEQ(new Decimal('3618328715720583671291544414202').toPrecision(4), '3.618e+30');
    arktest.assertEQ(new Decimal('-1003.61139687804673322250551').toPrecision(9), '-1003.61140');
    arktest.assertEQ(new Decimal('-8139415035028632370.38736602659835').toPrecision(24), '-8139415035028632370.38737');
    arktest.assertEQ(new Decimal('83198058').toPrecision(1), '8e+7');
    arktest.assertEQ(new Decimal('-799491603856548').toPrecision(6), '-7.99492e+14');
    arktest.assertEQ(new Decimal('444').toPrecision(3), '444');
    arktest.assertEQ(new Decimal('0.0000061325826693823067791292255878336353793864046451956723').toPrecision(12), '0.00000613258266938');
    arktest.assertEQ(new Decimal('-554696279951718746537611.26040029508470430208572833137315').toPrecision(29), '-554696279951718746537611.26040');
    arktest.assertEQ(new Decimal('446.189185820662709163412845035853873').toPrecision(3), '446');
    arktest.assertEQ(new Decimal('22873128187827109553471831451.06623850866672688842662473').toPrecision(40), '22873128187827109553471831451.06623850867');
    arktest.assertEQ(new Decimal('880389').toPrecision(1), '9e+5');
}

function testToPrecisionWithRMIsFive002() {
    arktest.assertEQ(new Decimal('-67516118890844443.625641').toPrecision(14), '-6.7516118890844e+16');
    arktest.assertEQ(new Decimal('-0.36107158435820101656696353075596201902674001080619510849').toPrecision(14), '-0.36107158435820');
    arktest.assertEQ(new Decimal('8.958386374640407364828679985365339921820421370157246').toPrecision(19), '8.958386374640407365');
    arktest.assertEQ(new Decimal('257').toPrecision(1), '3e+2');
    arktest.assertEQ(new Decimal('-1904659739878060478.113131137688927604413210083841').toPrecision(13), '-1.904659739878e+18');
    arktest.assertEQ(new Decimal('-0.00006271421732891589577305487292334').toPrecision(6), '-0.0000627142');
    arktest.assertEQ(new Decimal('331054103').toPrecision(7), '3.310541e+8');
    arktest.assertEQ(new Decimal('-179388600781592577147651.2641684828762234473').toPrecision(7), '-1.793886e+23');
    arktest.assertEQ(new Decimal('0.00046').toPrecision(4), '0.0004600');
    arktest.assertEQ(new Decimal('-2906505321975413509885').toPrecision(2), '-2.9e+21');
    arktest.assertEQ(new Decimal('86415.9473950557683374').toPrecision(13), '86415.94739506');
    arktest.assertEQ(new Decimal('6.7304135909152').toPrecision(7), '6.730414');
    arktest.assertEQ(new Decimal('-503236749968584').toPrecision(7), '-5.032367e+14');
    arktest.assertEQ(new Decimal('-502416820138682162877178622610283').toPrecision(23), '-5.0241682013868216287718e+32');
    arktest.assertEQ(new Decimal('-0.0552606118984074172116684879479087').toPrecision(33), '-0.0552606118984074172116684879479087');
    arktest.assertEQ(new Decimal('91017414629852252476380368766.47117955844005').toPrecision(32), '91017414629852252476380368766.471');
    arktest.assertEQ(new Decimal('28586.32124747000107561236523943').toPrecision(16), '28586.32124747000');
    arktest.assertEQ(new Decimal('0.0000019356655453225341951305040536808235510260170838860718').toPrecision(22), '0.000001935665545322534195131');
    arktest.assertEQ(new Decimal('7.803563246406851025').toPrecision(2), '7.8');
    arktest.assertEQ(new Decimal('-489914223627882382434323457.50920109688497974624541155867073').toPrecision(24), '-4.89914223627882382434323e+26');
    arktest.assertEQ(new Decimal('384718796891211107').toPrecision(18), '384718796891211107');
    arktest.assertEQ(new Decimal('42510.7400230989797123019438399853496258').toPrecision(25), '42510.74002309897971230194');
    arktest.assertEQ(new Decimal('-738804895894').toPrecision(4), '-7.388e+11');
    arktest.assertEQ(new Decimal('-0.0000005').toPrecision(8), '-5.0000000e-7');
}

function testToPrecisionWithRMIsSix001() {
    Decimal.rounding = 6;
    arktest.assertEQ(new Decimal('42334337596496149636254.4926162509306406461').toPrecision(23, 5), '42334337596496149636254');
    arktest.assertEQ(new Decimal('-7246374971.34279698356').toPrecision(1, 5), '-7e+9');
    arktest.assertEQ(new Decimal('71516263932998764871838469072.280115355524').toPrecision(29, 5), '71516263932998764871838469072');
    arktest.assertEQ(new Decimal('71257489.59952274151690076187021820922744').toPrecision(36, 5), '71257489.5995227415169007618702182092');
    arktest.assertEQ(new Decimal('268492834.77041').toPrecision(9, 5), '268492835');
    arktest.assertEQ(new Decimal('50325.551277778107847798801525').toPrecision(26, 5), '50325.551277778107847798802');
    arktest.assertEQ(new Decimal('-528930398665449048343281311623.69686').toPrecision(10, 5), '-5.289303987e+29');
    arktest.assertEQ(new Decimal('0.08').toPrecision(4), '0.08000');
    arktest.assertEQ(new Decimal('-4513243388120382069815.8508153058993058875').toPrecision(5), '-4.5132e+21');
    arktest.assertEQ(new Decimal('-73549.2594630551663822238').toPrecision(5), '-73549');
    arktest.assertEQ(new Decimal('127586800472892289589088296800.6').toPrecision(25), '1.275868004728922895890883e+29');
    arktest.assertEQ(new Decimal('-0.0003715444034899460421534099962225699').toPrecision(37), '-0.0003715444034899460421534099962225699000');
    arktest.assertEQ(new Decimal('-6962556526511822306135536').toPrecision(11), '-6.9625565265e+24');
    arktest.assertEQ(new Decimal('16758370364138.915293525076269061228714877').toPrecision(12), '1.67583703641e+13');
    arktest.assertEQ(new Decimal('-173594.950640855535151767073139475349181096310921699').toPrecision(50), '-173594.95064085553515176707313947534918109631092170');
    arktest.assertEQ(new Decimal('-69503965525000308384.151383').toPrecision(11), '-6.9503965525e+19');
    arktest.assertEQ(new Decimal('441122486054080817112').toPrecision(7), '4.411225e+20');
    arktest.assertEQ(new Decimal('24670440647835969376423717700462.39').toPrecision(28), '2.467044064783596937642371770e+31');
    arktest.assertEQ(new Decimal('3971189754948164565361634.8039734590476326224193520402091769').toPrecision(20), '3.9711897549481645654e+24');
    arktest.assertEQ(new Decimal('-1475761320868963235919.64499841336073105746686372924161').toPrecision(14), '-1.4757613208690e+21');
    arktest.assertEQ(new Decimal('91683083887068.61911461351134520171343337804061135').toPrecision(21), '91683083887068.6191146');
    arktest.assertEQ(new Decimal('-7923074181102822.5778').toPrecision(19), '-7923074181102822.578');
    arktest.assertEQ(new Decimal('-0.000000068').toPrecision(4), '-6.800e-8');
    arktest.assertEQ(new Decimal('-0.00000000025795467108146').toPrecision(21), '-2.57954671081460000000e-10');
    arktest.assertEQ(new Decimal('0.000000005535291197169667611325365189624523452').toPrecision(11), '5.5352911972e-9');
    arktest.assertEQ(new Decimal('604883577').toPrecision(8), '6.0488358e+8');
}

function testToPrecisionWithRMIsSix002() {
    arktest.assertEQ(new Decimal('-0.00000000757553501363609536678641245355').toPrecision(10), '-7.575535014e-9');
    arktest.assertEQ(new Decimal('0.00000000075470679605789002306444877998602723').toPrecision(25), '7.547067960578900230644488e-10');
    arktest.assertEQ(new Decimal('-3645614567625.4').toPrecision(12), '-3.64561456763e+12');
    arktest.assertEQ(new Decimal('0.0000009').toPrecision(2), '9.0e-7');
    arktest.assertEQ(new Decimal('687').toPrecision(1), '7e+2');
    arktest.assertEQ(new Decimal('517277827334839.8174848543680868015165926618').toPrecision(31), '517277827334839.8174848543680868');
    arktest.assertEQ(new Decimal('655.46270361324473194').toPrecision(1), '7e+2');
    arktest.assertEQ(new Decimal('1632131488313153.497374248234935731568').toPrecision(36), '1632131488313153.49737424823493573157');
    arktest.assertEQ(new Decimal('274068317992.5998880719845028748169734442394151076').toPrecision(40), '274068317992.5998880719845028748169734442');
    arktest.assertEQ(new Decimal('-0.00000000706025531009734073').toPrecision(4), '-7.060e-9');
    arktest.assertEQ(new Decimal('0.0044439457493').toPrecision(4), '0.004444');
    arktest.assertEQ(new Decimal('72482770689153111154104782082.022764082943227214833851').toPrecision(32), '72482770689153111154104782082.023');
    arktest.assertEQ(new Decimal('5913069403607279420613864.152').toPrecision(20), '5.9130694036072794206e+24');
    arktest.assertEQ(new Decimal('843384561300245347961437.96592523791').toPrecision(27), '843384561300245347961437.966');
    arktest.assertEQ(new Decimal('0.000003519882128251').toPrecision(20), '0.0000035198821282510000000');
    arktest.assertEQ(new Decimal('-0.00000000100371560130267706870096885251').toPrecision(24), '-1.00371560130267706870097e-9');
    arktest.assertEQ(new Decimal('17504218.49703016415913667026121376499').toPrecision(15), '17504218.4970302');
    arktest.assertEQ(new Decimal('-0.000000005169058703').toPrecision(1), '-5e-9');
    arktest.assertEQ(new Decimal('69228032455').toPrecision(10), '6.922803246e+10');
    arktest.assertEQ(new Decimal('-16').toPrecision(2), '-16');
}

function testToPrecisionWithRMIsSix003() {
    arktest.assertEQ(new Decimal('-13551475134681927071279391508441439066206.58705380600075').toPrecision(28), '-1.355147513468192707127939151e+40');
    arktest.assertEQ(new Decimal('81670324.1197758695212865075629796973196504241126').toPrecision(18), '81670324.1197758695');
    arktest.assertEQ(new Decimal('0.00004797485174640366805332660647').toPrecision(1), '0.00005');
    arktest.assertEQ(new Decimal('-0.0000000004864397594461335282648538530108953965681345').toPrecision(10), '-4.864397594e-10');
    arktest.assertEQ(new Decimal('47694105.23125322528167211284521303').toPrecision(15), '47694105.2312532');
    arktest.assertEQ(new Decimal('-496210618135432953927871636.779236').toPrecision(10), '-4.962106181e+26');
    arktest.assertEQ(new Decimal('12800030559497062236641930592334626609.7332').toPrecision(23), '1.2800030559497062236642e+37');
    arktest.assertEQ(new Decimal('-574830783.6689168903917696583746514637433390929').toPrecision(10), '-574830783.7');
    arktest.assertEQ(new Decimal('5969.43108619905746956015212970904111744101').toPrecision(19), '5969.431086199057470');
    arktest.assertEQ(new Decimal('-4814.32904953003285').toPrecision(2), '-4.8e+3');
    arktest.assertEQ(new Decimal('42973001760252134').toPrecision(4), '4.297e+16');
    arktest.assertEQ(new Decimal('-5762846.590152347665179652381407653797146356303622218259885').toPrecision(5), '-5.7628e+6');
    arktest.assertEQ(new Decimal('904864662232032.16061240181031792729165740314293194205879163').toPrecision(48), '904864662232032.160612401810317927291657403142932');
    arktest.assertEQ(new Decimal('798923115068265241915.537619430376605').toPrecision(5), '7.9892e+20');
    arktest.assertEQ(new Decimal('-8.97759349384000643427096282979').toPrecision(18), '-8.97759349384000643');
    arktest.assertEQ(new Decimal('12.345e1').toPrecision(), '123.45');
}
