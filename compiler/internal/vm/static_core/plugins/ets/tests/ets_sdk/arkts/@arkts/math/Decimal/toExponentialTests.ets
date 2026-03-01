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
    const suite = new arktest.ArkTestsuite("Decimal toExponential API tests");
    suite.addTest("Decimal toExponential with specific config", testToExponentialWithSpecificConfig001);
    suite.addTest("Decimal toExponential with specific config", testToExponentialWithSpecificConfig002);
    suite.addTest("Decimal toExponential with specific config", testToExponentialWithSpecificConfig003);
    suite.addTest("Decimal toExponential with specific config", testToExponentialWithSpecificConfig004);
    suite.addTest("Decimal toExponential with rounding is 0", testToExponentialWithRMIsZero001);
    suite.addTest("Decimal toExponential with rounding is 0", testToExponentialWithRMIsZero002);
    suite.addTest("Decimal toExponential with rounding is 1", testToExponentialWithRMIsOne001);
    suite.addTest("Decimal toExponential with rounding is 1", testToExponentialWithRMIsOne002);
    suite.addTest("Decimal toExponential with rounding is 2", testToExponentialWithRMIsTwo001);
    suite.addTest("Decimal toExponential with rounding is 2", testToExponentialWithRMIsTwo002);
    suite.addTest("Decimal toExponential with rounding is 3", testToExponentialWithRMIsThree001);
    suite.addTest("Decimal toExponential with rounding is 3", testToExponentialWithRMIsThree002);
    suite.addTest("Decimal toExponential with rounding is 4", testToExponentialWithRMIsFour001);
    suite.addTest("Decimal toExponential with rounding is 4", testToExponentialWithRMIsFour002);
    suite.addTest("Decimal toExponential with rounding is 4", testToExponentialWithRMIsFour003);
    suite.addTest("Decimal toExponential with rounding is 5", testToExponentialWithRMIsFive001);
    suite.addTest("Decimal toExponential with rounding is 5", testToExponentialWithRMIsFive002);
    suite.addTest("Decimal toExponential with rounding is 6", testToExponentialWithRMIsSix001);
    suite.addTest("Decimal toExponential with rounding is 6", testToExponentialWithRMIsSix002);
    return suite.run();
}

function testToExponentialWithSpecificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(1).toExponential(), '1e+0');
    arktest.assertEQ(new Decimal(11).toExponential(), '1.1e+1');
    arktest.assertEQ(new Decimal(112).toExponential(), '1.12e+2');
    arktest.assertEQ(new Decimal(1).toExponential(0), '1e+0');
    arktest.assertEQ(new Decimal(11).toExponential(0), '1e+1');
    arktest.assertEQ(new Decimal(112).toExponential(0), '1e+2');
    arktest.assertEQ(new Decimal(1).toExponential(1), '1.0e+0');
    arktest.assertEQ(new Decimal(11).toExponential(1), '1.1e+1');
    arktest.assertEQ(new Decimal(112).toExponential(1), '1.1e+2');
    arktest.assertEQ(new Decimal(1).toExponential(2), '1.00e+0');
    arktest.assertEQ(new Decimal(11).toExponential(2), '1.10e+1');
    arktest.assertEQ(new Decimal(112).toExponential(2), '1.12e+2');
    arktest.assertEQ(new Decimal(1).toExponential(3), '1.000e+0');
    arktest.assertEQ(new Decimal(11).toExponential(3), '1.100e+1');
    arktest.assertEQ(new Decimal(112).toExponential(3), '1.120e+2');
    arktest.assertEQ(new Decimal(0.1).toExponential(), '1e-1');
    arktest.assertEQ(new Decimal(0.11).toExponential(), '1.1e-1');
    arktest.assertEQ(new Decimal(0.112).toExponential(), '1.12e-1');
    arktest.assertEQ(new Decimal(0.1).toExponential(0), '1e-1');
    arktest.assertEQ(new Decimal(0.11).toExponential(0), '1e-1');
    arktest.assertEQ(new Decimal(0.112).toExponential(0), '1e-1');
}

function testToExponentialWithSpecificConfig002() {
    arktest.assertEQ(new Decimal(0.1).toExponential(1), '1.0e-1');
    arktest.assertEQ(new Decimal(0.11).toExponential(1), '1.1e-1');
    arktest.assertEQ(new Decimal(0.112).toExponential(1), '1.1e-1');
    arktest.assertEQ(new Decimal(0.1).toExponential(2), '1.00e-1');
    arktest.assertEQ(new Decimal(0.11).toExponential(2), '1.10e-1');
    arktest.assertEQ(new Decimal(0.112).toExponential(2), '1.12e-1');
    arktest.assertEQ(new Decimal(0.1).toExponential(3), '1.000e-1');
    arktest.assertEQ(new Decimal(0.11).toExponential(3), '1.100e-1');
    arktest.assertEQ(new Decimal(0.112).toExponential(3), '1.120e-1');
    arktest.assertEQ(new Decimal(-1).toExponential(), '-1e+0');
    arktest.assertEQ(new Decimal(-11).toExponential(), '-1.1e+1');
    arktest.assertEQ(new Decimal(-112).toExponential(), '-1.12e+2');
    arktest.assertEQ(new Decimal(-1).toExponential(0), '-1e+0');
    arktest.assertEQ(new Decimal(-11).toExponential(0), '-1e+1');
    arktest.assertEQ(new Decimal(-112).toExponential(0), '-1e+2');
    arktest.assertEQ(new Decimal(-1).toExponential(1), '-1.0e+0');
    arktest.assertEQ(new Decimal(-11).toExponential(1), '-1.1e+1');
    arktest.assertEQ(new Decimal(-112).toExponential(1), '-1.1e+2');
    arktest.assertEQ(new Decimal(-1).toExponential(2), '-1.00e+0');
    arktest.assertEQ(new Decimal(-11).toExponential(2), '-1.10e+1');
    arktest.assertEQ(new Decimal(-112).toExponential(2), '-1.12e+2');
    arktest.assertEQ(new Decimal(-1).toExponential(3), '-1.000e+0');
}

function testToExponentialWithSpecificConfig003() {
    arktest.assertEQ(new Decimal(-11).toExponential(3), '-1.100e+1');
    arktest.assertEQ(new Decimal(-112).toExponential(3), '-1.120e+2');
    arktest.assertEQ(new Decimal(-0.1).toExponential(), '-1e-1');
    arktest.assertEQ(new Decimal(-0.11).toExponential(), '-1.1e-1');
    arktest.assertEQ(new Decimal(-0.112).toExponential(), '-1.12e-1');
    arktest.assertEQ(new Decimal(-0.1).toExponential(0), '-1e-1');
    arktest.assertEQ(new Decimal(-0.11).toExponential(0), '-1e-1');
    arktest.assertEQ(new Decimal(-0.112).toExponential(0), '-1e-1');
    arktest.assertEQ(new Decimal(-0.1).toExponential(1), '-1.0e-1');
    arktest.assertEQ(new Decimal(-0.11).toExponential(1), '-1.1e-1');
    arktest.assertEQ(new Decimal(-0.112).toExponential(1), '-1.1e-1');
    arktest.assertEQ(new Decimal(-0.1).toExponential(2), '-1.00e-1');
    arktest.assertEQ(new Decimal(-0.11).toExponential(2), '-1.10e-1');
    arktest.assertEQ(new Decimal(-0.112).toExponential(2), '-1.12e-1');
    arktest.assertEQ(new Decimal(-0.1).toExponential(3), '-1.000e-1');
    arktest.assertEQ(new Decimal(-0.11).toExponential(3), '-1.100e-1');
    arktest.assertEQ(new Decimal(-0.112).toExponential(3), '-1.120e-1');
}

function testToExponentialWithSpecificConfig004() {
    arktest.assertEQ(new Decimal(NaN).toExponential(), 'NaN');
    arktest.assertEQ(new Decimal(-NaN).toExponential(2), 'NaN');
    arktest.assertEQ(new Decimal(Infinity).toExponential(), 'Infinity');
    arktest.assertEQ(new Decimal(Infinity).toExponential(10), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).toExponential(0), '-Infinity');
    arktest.assertEQ(new Decimal(-Infinity).toExponential(1), '-Infinity');
    arktest.assertEQ(new Decimal(0).toExponential(), '0e+0');
    arktest.assertEQ(new Decimal(-0).toExponential(), '0e+0');
    arktest.assertEQ(new Decimal(-0.5).toExponential(1), '-5.0e-1');
    arktest.assertEQ(new Decimal(0).toExponential(2), '0.00e+0');
    arktest.assertEQ(new Decimal(11.2356).toExponential(0), '1e+1');
    arktest.assertEQ(new Decimal(11.2356).toExponential(4), '1.1236e+1');
    arktest.assertEQ(new Decimal(0.000112356).toExponential(4), '1.1236e-4');
    arktest.assertEQ(new Decimal(-0.000112356).toExponential(4), '-1.1236e-4');
    arktest.assertEQ(new Decimal(0.000112356).toExponential(), '1.12356e-4');
    arktest.assertEQ(new Decimal(-0.000112356).toExponential(), '-1.12356e-4');
    arktest.assertEQ(new Decimal(0.99976).toExponential(2), '1.00e+0');
    arktest.assertEQ(new Decimal(99.9979).toExponential(2), '1.00e+2');
    arktest.assertEQ(new Decimal('99991.27839').toExponential(2), '1.00e+5');
    arktest.assertEQ(new Decimal('99.999').toExponential(3), '1.000e+2');
    arktest.assertEQ(new Decimal('9999512.8').toExponential(3), '1.000e+7');
    arktest.assertEQ(new Decimal('999702726').toExponential(2), '1.00e+9');
    arktest.assertEQ(new Decimal('999.964717').toExponential(3), '1.000e+3');
}

function testToExponentialWithRMIsZero001() {
    Decimal.rounding = 0;
    arktest.assertEQ(new Decimal('0.00000001').toExponential(), '1e-8');
    arktest.assertEQ(new Decimal('1000.00000001').toExponential(), '1.00000000001e+3');
    arktest.assertEQ(new Decimal('-53453.435435E4').toExponential(), '-5.3453435435e+8');
    arktest.assertEQ(new Decimal('-882546581000927463.34967191957167916942543286').toExponential(40), '-8.8254658100092746334967191957167916942544e+17');
    arktest.assertEQ(new Decimal('-0.00000000479412182855967445061088900853730578349045628396662493370334888944406719979291547717079').toExponential(42), '-4.794121828559674450610889008537305783490457e-9');
    arktest.assertEQ(new Decimal('3614844933096444884855774574994631.0106397808').toExponential(4), '3.6149e+33');
    arktest.assertEQ(new Decimal('0.000000000005582954').toExponential(15), '5.582954000000000e-12');
    arktest.assertEQ(new Decimal('-0.000000000000000000000003887402719918859147748023631511630059257').toExponential(56), '-3.88740271991885914774802363151163005925700000000000000000e-24');
    arktest.assertEQ(new Decimal('-0.00000000000000000006870796458724372772369131903163064352749026131516764202733298056929060151437').toExponential(53), '-6.87079645872437277236913190316306435274902613151676421e-20');
    arktest.assertEQ(new Decimal('381818740872781045337373136215865307.11155405443818235503358935045749888900677769535371296063').toExponential(76), '3.8181874087278104533737313621586530711155405443818235503358935045749888900678e+35');
    arktest.assertEQ(new Decimal('-711375441247784115059.91211858618973289154952986').toExponential(41), '-7.11375441247784115059912118586189732891550e+20');
    arktest.assertEQ(new Decimal('6578282366667302135641813.7249573246362582').toExponential(4), '6.5783e+24');
    arktest.assertEQ(new Decimal('0.00000000000000000006').toExponential(21), '6.000000000000000000000e-20');
    arktest.assertEQ(new Decimal('-53799672107777').toExponential(13), '-5.3799672107777e+13');
    arktest.assertEQ(new Decimal('-0.00000000000000000000006948849870723').toExponential(3), '-6.949e-23');
    arktest.assertEQ(new Decimal('-80735851843167053097578169.623098209399637950843019109979317').toExponential(24), '-8.073585184316705309757817e+25');
    arktest.assertEQ(new Decimal('-0.0000000000042956482047751').toExponential(7), '-4.2956483e-12');
    arktest.assertEQ(new Decimal('-6116215572195144.0801240154580459651167830311633').toExponential(46), '-6.1162155721951440801240154580459651167830311633e+15');
    arktest.assertEQ(new Decimal('-0.000000000000000000007263265230766073544739').toExponential(12), '-7.263265230767e-21');
    arktest.assertEQ(new Decimal('-2301340611570177634.5891814408272260224632').toExponential(25), '-2.3013406115701776345891815e+18');
    arktest.assertEQ(new Decimal('-6029979366232747481609455093247.705001183386474').toExponential(10), '-6.0299793663e+30');
    arktest.assertEQ(new Decimal('-2975443049668038511693.75547178021412').toExponential(11), '-2.97544304967e+21');
    arktest.assertEQ(new Decimal('-41471192639.1600315953295208128538183546').toExponential(16), '-4.1471192639160032e+10');
    arktest.assertEQ(new Decimal('-3612017767852949869824542721.1595027189').toExponential(17), '-3.61201776785294987e+27');
    arktest.assertEQ(new Decimal('-699834940445061151.14676').toExponential(19), '-6.9983494044506115115e+17');
}

function testToExponentialWithRMIsZero002() {
    arktest.assertEQ(new Decimal('-145807003236292450382.86958174').toExponential(22), '-1.4580700323629245038287e+20');
    arktest.assertEQ(new Decimal('-85390930743').toExponential(2), '-8.54e+10');
    arktest.assertEQ(new Decimal('-27152698569707163435').toExponential(15), '-2.715269856970717e+19');
    arktest.assertEQ(new Decimal('-567681003999187989540.627303416332508226276308449233').toExponential(8), '-5.67681004e+20');
    arktest.assertEQ(new Decimal('-2068085084336615438842661921.06985539576218546524301').toExponential(5), '-2.06809e+27');
    arktest.assertEQ(new Decimal('-292273061370427.0122509240087955481845060858420928631').toExponential(23), '-2.92273061370427012250925e+14');
    arktest.assertEQ(new Decimal('-0.0000000000000000433542').toExponential(4), '-4.3355e-17');
    arktest.assertEQ(new Decimal('-3491610942583064798345').toExponential(12), '-3.491610942584e+21');
    arktest.assertEQ(new Decimal('-87019446359851299.8036062002728328').toExponential(24), '-8.701944635985129980360621e+16');
    arktest.assertEQ(new Decimal('-0.000000000486409475991').toExponential(1), '-4.9e-10');
    arktest.assertEQ(new Decimal('-48212433366063403866').toExponential(5), '-4.82125e+19');
    arktest.assertEQ(new Decimal('-0.000000000000000000079559394098236').toExponential(8), '-7.95593941e-20');
    arktest.assertEQ(new Decimal('-0.0000000002005622924388').toExponential(5), '-2.00563e-10');
    arktest.assertEQ(new Decimal('-69777057921142634.3825218243275152606161149381').toExponential(25), '-6.9777057921142634382521825e+16');
    arktest.assertEQ(new Decimal('-842590769172062').toExponential(5), '-8.42591e+14');
    arktest.assertEQ(new Decimal('-6351232644080754054285724566').toExponential(11), '-6.35123264409e+27');
    arktest.assertEQ(new Decimal('-0.00000000000000000000000000055088354925775864958942599785412').toExponential(27), '-5.508835492577586495894259979e-28');
    arktest.assertEQ(new Decimal('-2667451875964').toExponential(9), '-2.667451876e+12');
    arktest.assertEQ(new Decimal('-664446104743236162820999716').toExponential(19), '-6.6444610474323616283e+26');
    arktest.assertEQ(new Decimal('-2419775049242.726').toExponential(12), '-2.419775049243e+12');
    arktest.assertEQ(new Decimal('-0.000000000000000005319').toExponential(2), '-5.32e-18');
    arktest.assertEQ(new Decimal('-0.000000000000000000000000086303035522286938593814060049934').toExponential(11), '-8.63030355223e-26');
    arktest.assertEQ(new Decimal('-250469209819563850.48538613818080285657602717018').toExponential(43), '-2.5046920981956385048538613818080285657602718e+17');
}

function testToExponentialWithRMIsOne001() {
    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('-0.0E-0').toExponential(), '0e+0');
    arktest.assertEQ(new Decimal('-2856376.815219143184897347685012382222462687620998915470135915511363444').toExponential(60), '-2.856376815219143184897347685012382222462687620998915470135915e+6');
    arktest.assertEQ(new Decimal('0.000000000000000000000007757').toExponential(5), '7.75700e-24');
    arktest.assertEQ(new Decimal('0.7').toExponential(1), '7.0e-1');
    arktest.assertEQ(new Decimal('52109749078977455423107465583658126637').toExponential(34), '5.2109749078977455423107465583658126e+37');
    arktest.assertEQ(new Decimal('0.00000000000000000000000000003631093819552528994444977110063007461579154042777868294').toExponential(57), '3.631093819552528994444977110063007461579154042777868294000e-29');
    arktest.assertEQ(new Decimal('-989393786.042588804219191').toExponential(15), '-9.893937860425888e+8');
    arktest.assertEQ(new Decimal('8797804362260746751563912625017414439944006.5804807').toExponential(16), '8.7978043622607467e+42');
    arktest.assertEQ(new Decimal('-0.000000465617027643946026213823955447791862428108248596086901464075785390015').toExponential(19), '-4.6561702764394602621e-7');
    arktest.assertEQ(new Decimal('-254277048.224290221559692488430240765024783').toExponential(33), '-2.542770482242902215596924884302407e+8');
    arktest.assertEQ(new Decimal('0.000000027').toExponential(8), '2.70000000e-8');
    arktest.assertEQ(new Decimal('-80291821891769794.408790934252924453237503615825249362166').toExponential(37), '-8.0291821891769794408790934252924453237e+16');
    arktest.assertEQ(new Decimal('-0.0000000000000008052959230040573585458547716514262').toExponential(26), '-8.05295923004057358545854771e-16');
    arktest.assertEQ(new Decimal('-0.00000000000000000000278675879025858093817787290334306').toExponential(6), '-2.786758e-21');
    arktest.assertEQ(new Decimal('-801608356247372258038.538246876417776604065270622886204812876').toExponential(43), '-8.0160835624737225803853824687641777660406527e+20');
    arktest.assertEQ(new Decimal('-7284905488799914469461919177.08975892527524').toExponential(34), '-7.2849054887999144694619191770897589e+27');
    arktest.assertEQ(new Decimal('-0.00000000000000007586908').toExponential(3), '-7.586e-17');
    arktest.assertEQ(new Decimal('-595081509336365806742.496029416739842548642249').toExponential(40), '-5.9508150933636580674249602941673984254864e+20');
    arktest.assertEQ(new Decimal('-0.000000000000000003526911897770082481187').toExponential(9), '-3.526911897e-18');
    arktest.assertEQ(new Decimal('-0.0000000000000000000005774729035676859').toExponential(3), '-5.774e-22');
    arktest.assertEQ(new Decimal('-0.00000000000064700957007714124190210074383').toExponential(25), '-6.4700957007714124190210074e-13');
    arktest.assertEQ(new Decimal('-5610492566512449795573').toExponential(6), '-5.610492e+21');
    arktest.assertEQ(new Decimal('-601556443593022914280678').toExponential(3), '-6.015e+23');
}

function testToExponentialWithRMIsOne002() {
    arktest.assertEQ(new Decimal('-606733615533.448288878').toExponential(13), '-6.0673361553344e+11');
    arktest.assertEQ(new Decimal('-315617199368461055533962323.071668327669249').toExponential(1), '-3.1e+26');
    arktest.assertEQ(new Decimal('-9139107951210456203234346').toExponential(22), '-9.1391079512104562032343e+24');
    arktest.assertEQ(new Decimal('-2044198307917443182711').toExponential(4), '-2.0441e+21');
    arktest.assertEQ(new Decimal('-821283723216249535240085.606500821783973097233814324').toExponential(44), '-8.21283723216249535240085606500821783973097233e+23');
    arktest.assertEQ(new Decimal('-637540984314799.4').toExponential(3), '-6.375e+14');
    arktest.assertEQ(new Decimal('-217797482005219478530856429744.7268928676963181').toExponential(32), '-2.17797482005219478530856429744726e+29');
    arktest.assertEQ(new Decimal('-395476721391').toExponential(4), '-3.9547e+11');
    arktest.assertEQ(new Decimal('-6892798573971046301111').toExponential(4), '-6.8927e+21');
    arktest.assertEQ(new Decimal('-0.000000000006338421414029165389261335065112712777').toExponential(20), '-6.33842141402916538926e-12');
    arktest.assertEQ(new Decimal('-0.000000000000000000000000000004572725511159166').toExponential(4), '-4.5727e-30');
    arktest.assertEQ(new Decimal('-0.000000000000000078847457779026882221249217577974').toExponential(31), '-7.8847457779026882221249217577974e-17');
    arktest.assertEQ(new Decimal('-2649162316402.649271824').toExponential(17), '-2.64916231640264927e+12');
    arktest.assertEQ(new Decimal('-17360440496948254515028685124.37795415803082546457797184294').toExponential(8), '-1.73604404e+28');
    arktest.assertEQ(new Decimal('-86802249856236148.11694273469092873').toExponential(12), '-8.680224985623e+16');
    arktest.assertEQ(new Decimal('-0.00000000000000000043859841576346037715462713764211635').toExponential(1), '-4.3e-19');
    arktest.assertEQ(new Decimal('-0.000000000076886753538909815914171710501337139').toExponential(26), '-7.68867535389098159141717105e-11');
    arktest.assertEQ(new Decimal('-5243250386110905059283.894223250016067979080420266').toExponential(35), '-5.24325038611090505928389422325001606e+21');
    arktest.assertEQ(new Decimal('-0.0000000000000000000013874592057586367688528204069850262406').toExponential(2), '-1.38e-21');
    arktest.assertEQ(new Decimal('-7308601949094508589445770.5820741094106150373221910779').toExponential(42), '-7.308601949094508589445770582074109410615037e+24');
    arktest.assertEQ(new Decimal('-32638405387645.3309565877781780222317335852159983').toExponential(4), '-3.2638e+13');
    arktest.assertEQ(new Decimal('-35545473744809471901929.118320651505996237856336054914').toExponential(41), '-3.55454737448094719019291183206515059962378e+22');
    arktest.assertEQ(new Decimal('-0.00000000005390624225279268530907215395611').toExponential(13), '-5.3906242252792e-11');
    arktest.assertEQ(new Decimal('-8867608738112131.050787').toExponential(20), '-8.86760873811213105078e+15');
    arktest.assertEQ(new Decimal('-0.00000000000000000000004781292548355671480462711435866243551').toExponential(14), '-4.78129254835567e-23');
    arktest.assertEQ(new Decimal('-0.00000000000000000064694208834502691835879021438795583630205').toExponential(40), '-6.4694208834502691835879021438795583630205e-19');
}

function testToExponentialWithRMIsTwo001() {
    Decimal.rounding = 2;
    arktest.assertEQ(new Decimal('0E0000000000').toExponential(), '0e+0');
    arktest.assertEQ(new Decimal('-0E01').toExponential(), '0e+0');
    arktest.assertEQ(new Decimal('-0E00000000001').toExponential(2), '0.00e+0');
    arktest.assertEQ(new Decimal('0.0000000000000304656552536921453451651664421156').toExponential(31), '3.0465655253692145345165166442116e-14');
    arktest.assertEQ(new Decimal('90573943842008592406279.60854292331338139428664197890720339655043720040907662489784').toExponential(61), '9.0573943842008592406279608542923313381394286641978907203396551e+22');
    arktest.assertEQ(new Decimal('-0.117181502970008783734855040984899').toExponential(35), '-1.17181502970008783734855040984899000e-1');
    arktest.assertEQ(new Decimal('-0.00000000000000052886056528317233012115396784629214632').toExponential(8), '-5.28860565e-16');
    arktest.assertEQ(new Decimal('0.0000000000000000064114675970838738').toExponential(19), '6.4114675970838738000e-18');
    arktest.assertEQ(new Decimal('0.00000000000000000008').toExponential(20), '8.00000000000000000000e-20');
    arktest.assertEQ(new Decimal('2740000645782887717230785.977111035204503916681178023040781520856250236336811781347278').toExponential(77), '2.74000064578288771723078597711103520450391668117802304078152085625023633681179e+24');
    arktest.assertEQ(new Decimal('0.00000000000000081936742669491704846805837777816457628').toExponential(37), '8.1936742669491704846805837777816457628e-16');
    arktest.assertEQ(new Decimal('-721574484716710.00141299844961546').toExponential(7), '-7.2157448e+14');
    arktest.assertEQ(new Decimal('-0.00000000000000532180746470365').toExponential(21), '-5.321807464703650000000e-15');
    arktest.assertEQ(new Decimal('-4449471658582621135143349142.228707647170080816912435271162').toExponential(3), '-4.449e+27');
    arktest.assertEQ(new Decimal('-49229158213139196237.584').toExponential(21), '-4.922915821313919623758e+19');
    arktest.assertEQ(new Decimal('-0.000000000000069966682257740984029052').toExponential(15), '-6.996668225774098e-14');
    arktest.assertEQ(new Decimal('-868560391748.2213394261601242479516861829472792').toExponential(34), '-8.6856039174822133942616012424795168e+11');
    arktest.assertEQ(new Decimal('-8461810373307862460504').toExponential(3), '-8.461e+21');
    arktest.assertEQ(new Decimal('-38987166277031946258244119.67718').toExponential(27), '-3.898716627703194625824411967e+25');
    arktest.assertEQ(new Decimal('-282193549675582402670759843.23655').toExponential(12), '-2.821935496755e+26');
    arktest.assertEQ(new Decimal('-0.0000000000000000000003491662482987').toExponential(2), '-3.49e-22');
    arktest.assertEQ(new Decimal('-0.0000000000000336211177857623161536645733316587527475522615').toExponential(27), '-3.362111778576231615366457333e-14');
    arktest.assertEQ(new Decimal('-0.00000000000059933412636903331').toExponential(4), '-5.9933e-13');
    arktest.assertEQ(new Decimal('-277927721100404435781172100113.4136636412460458083951').toExponential(8), '-2.77927721e+29');
    arktest.assertEQ(new Decimal('-0.0000000001876833722329987477942').toExponential(12), '-1.876833722329e-10');
    arktest.assertEQ(new Decimal('-653341175209856').toExponential(1), '-6.5e+14');
}

function testToExponentialWithRMIsTwo002() {
    arktest.assertEQ(new Decimal('-862729184017386.7961').toExponential(18), '-8.627291840173867961e+14');
    arktest.assertEQ(new Decimal('-0.00000000003913745716559766839130121802935022889').toExponential(28), '-3.9137457165597668391301218029e-11');
    arktest.assertEQ(new Decimal('-89532775488').toExponential(2), '-8.95e+10');
    arktest.assertEQ(new Decimal('-0.000000000000000021395541875015568986238771696').toExponential(22), '-2.1395541875015568986238e-17');
    arktest.assertEQ(new Decimal('-0.00000000000498575853353890809143399546448630559732119628509').toExponential(44), '-4.98575853353890809143399546448630559732119628e-12');
    arktest.assertEQ(new Decimal('-89989591559494822').toExponential(2), '-8.99e+16');
    arktest.assertEQ(new Decimal('-34934632714180035424463').toExponential(8), '-3.49346327e+22');
    arktest.assertEQ(new Decimal('-0.00000000000003569953760575390545759785014980652333323889116').toExponential(22), '-3.5699537605753905457597e-14');
    arktest.assertEQ(new Decimal('-2989253688034.9975618286212199904979534461637613').toExponential(22), '-2.9892536880349975618286e+12');
    arktest.assertEQ(new Decimal('-30438391921.790494961888803732171').toExponential(20), '-3.04383919217904949618e+10');
    arktest.assertEQ(new Decimal('-823241154405701456').toExponential(9), '-8.232411544e+17');
    arktest.assertEQ(new Decimal('-0.00000000000000058091512269904640168152354').toExponential(21), '-5.809151226990464016815e-16');
    arktest.assertEQ(new Decimal('-85220423973269.324312660179132118').toExponential(18), '-8.522042397326932431e+13');
    arktest.assertEQ(new Decimal('-0.000000000000000000000752109428925015').toExponential(7), '-7.5210942e-22');
    arktest.assertEQ(new Decimal('-520183214495439298725191.09').toExponential(13), '-5.2018321449543e+23');
    arktest.assertEQ(new Decimal('-6040840454537113956292.68198016245611021901815486929628647').toExponential(44), '-6.04084045453711395629268198016245611021901815e+21');
    arktest.assertEQ(new Decimal('-0.00000000000014954781789967551381259345443436747983317353423').toExponential(36), '-1.495478178996755138125934544343674798e-13');
    arktest.assertEQ(new Decimal('-68814844975107335.241512452206302822599853065465371507616758').toExponential(48), '-6.881484497510733524151245220630282259985306546537e+16');
    arktest.assertEQ(new Decimal('-0.00000000000004712138901995619').toExponential(13), '-4.7121389019956e-14');
}

function testToExponentialWithRMIsThree001() {
    Decimal.rounding = 3;
    arktest.assertEQ(new Decimal('-999999999.000000009e-0').toExponential(), '-9.99999999000000009e+8');
    arktest.assertEQ(new Decimal('-3997644229032512204527047.63278376060858663250289320247532594416986984981431156065660613').toExponential(59), '-3.99764422903251220452704763278376060858663250289320247532595e+24');
    arktest.assertEQ(new Decimal('5534083545686.157907280686578717428772').toExponential(36), '5.534083545686157907280686578717428772e+12');
    arktest.assertEQ(new Decimal('0.000000005').toExponential(8), '5.00000000e-9');
    arktest.assertEQ(new Decimal('-408363116583051').toExponential(14), '-4.08363116583051e+14');
    arktest.assertEQ(new Decimal('92782304156342969452.738186255580532649103987374718221928871873054827841260470670536425').toExponential(24), '9.278230415634296945273818e+19');
    arktest.assertEQ(new Decimal('-0.000000000001087325089986030854546619968259691229662152159029641023997866843605032534351388775075').toExponential(23), '-1.08732508998603085454662e-12');
    arktest.assertEQ(new Decimal('352888045173776066886981811418233.218955856086').toExponential(22), '3.5288804517377606688698e+32');
    arktest.assertEQ(new Decimal('43218878143887755.42593887518334667202').toExponential(17), '4.32188781438877554e+16');
    arktest.assertEQ(new Decimal('-815').toExponential(2), '-8.15e+2');
    arktest.assertEQ(new Decimal('1515077312590223222.678749527895871363186918977679783240817218232896076765321818445939718165').toExponential(27), '1.515077312590223222678749527e+18');
    arktest.assertEQ(new Decimal('-805381864216645365099.17032729912932814374101821').toExponential(43), '-8.0538186421664536509917032729912932814374102e+20');
    arktest.assertEQ(new Decimal('-343670973010020.990473804391071456587732173').toExponential(22), '-3.4367097301002099047381e+14');
    arktest.assertEQ(new Decimal('-0.0000000000053420288504').toExponential(4), '-5.3421e-12');
    arktest.assertEQ(new Decimal('-263200517731973006983184.60341959097016190770542276').toExponential(7), '-2.6320052e+23');
    arktest.assertEQ(new Decimal('-0.000000000044673422483').toExponential(1), '-4.5e-11');
    arktest.assertEQ(new Decimal('-0.00000000000000007232463101115829118145025733451800457178').toExponential(33), '-7.232463101115829118145025733451801e-17');
    arktest.assertEQ(new Decimal('-11832010004415476244854.5914170978206041022038489').toExponential(44), '-1.18320100044154762448545914170978206041022039e+22');
    arktest.assertEQ(new Decimal('-7745237371276392645710.0521930569226728841707200771').toExponential(21), '-7.745237371276392645711e+21');
    arktest.assertEQ(new Decimal('-0.000000000443155950005325569564213010993378905').toExponential(21), '-4.431559500053255695643e-10');
    arktest.assertEQ(new Decimal('-0.000000000000000000000002443').toExponential(1), '-2.5e-24');
    arktest.assertEQ(new Decimal('-0.0000000000500502702843902395839120312700550362154137').toExponential(39), '-5.005027028439023958391203127005503621542e-11');
    arktest.assertEQ(new Decimal('-0.00000000000000000000064535253779342133343665123283565').toExponential(21), '-6.453525377934213334367e-22');
    arktest.assertEQ(new Decimal('-45594370326121.71862685098237352845979966987').toExponential(31), '-4.5594370326121718626850982373529e+13');
    arktest.assertEQ(new Decimal('-17088248121660259').toExponential(3), '-1.709e+16');
}

function testToExponentialWithRMIsThree002() {
    arktest.assertEQ(new Decimal('-39047581533864712.6574405').toExponential(16), '-3.9047581533864713e+16');
    arktest.assertEQ(new Decimal('-0.000000000000000020880420127397564274443250271135').toExponential(8), '-2.08804202e-17');
    arktest.assertEQ(new Decimal('-6801694635944774.65568900821692503508025').toExponential(33), '-6.801694635944774655689008216925036e+15');
    arktest.assertEQ(new Decimal('-87691286374104240967.93180059373367907299683816381677816389').toExponential(31), '-8.7691286374104240967931800593734e+19');
    arktest.assertEQ(new Decimal('-0.000000000000000000000000000028022577317152384526775320012').toExponential(18), '-2.802257731715238453e-29');
    arktest.assertEQ(new Decimal('-44704405768781565005877.813010169083').toExponential(4), '-4.4705e+22');
    arktest.assertEQ(new Decimal('-0.00000000041737490849648644923105632500267064').toExponential(20), '-4.17374908496486449232e-10');
    arktest.assertEQ(new Decimal('-0.00000000022706134122862417334386435').toExponential(4), '-2.2707e-10');
    arktest.assertEQ(new Decimal('-0.0000000000000000000000028543100839983854161').toExponential(5), '-2.85432e-24');
    arktest.assertEQ(new Decimal('-5791889489461.643555240257').toExponential(8), '-5.79188949e+12');
    arktest.assertEQ(new Decimal('-7459701910718662.03421293892346992893463534702').toExponential(2), '-7.46e+15');
    arktest.assertEQ(new Decimal('-10535086280628995915087428.2423609320023833125322801559606').toExponential(13), '-1.0535086280629e+25');
    arktest.assertEQ(new Decimal('-2907441265164718836710598468491.31550321772').toExponential(22), '-2.9074412651647188367106e+30');
    arktest.assertEQ(new Decimal('-5010945976711327691648509517.2305').toExponential(21), '-5.010945976711327691649e+27');
    arktest.assertEQ(new Decimal('-0.0000000000000000000886339602133865324283362544').toExponential(16), '-8.8633960213386533e-20');
    arktest.assertEQ(new Decimal('-31891844834898211661.45273071401566483780434051217').toExponential(40), '-3.1891844834898211661452730714015664837805e+19');
    arktest.assertEQ(new Decimal('-50833809760143655338432298825.264367948359').toExponential(33), '-5.083380976014365533843229882526437e+28');
    arktest.assertEQ(new Decimal('-0.000000000000000678534987604148025611184').toExponential(1), '-6.8e-16');
    arktest.assertEQ(new Decimal('-7838656097386639584904346062976.9346038436').toExponential(1), '-7.9e+30');
    arktest.assertEQ(new Decimal('-630535780834495012856').toExponential(8), '-6.30535781e+20');
    arktest.assertEQ(new Decimal('-0.00000000000000000000000000000966289400023904753107633012').toExponential(3), '-9.663e-30');
    arktest.assertEQ(new Decimal('-2315198482308.7361348').toExponential(12), '-2.315198482309e+12');
    arktest.assertEQ(new Decimal('-8158235289415958939').toExponential(12), '-8.158235289416e+18');
    arktest.assertEQ(new Decimal('-416188905174043169336992.063606399885828326245241437').toExponential(46), '-4.1618890517404316933699206360639988582832624525e+23');
    arktest.assertEQ(new Decimal('-5975507169818339908394.41709631281058258352209').toExponential(29), '-5.97550716981833990839441709632e+21');
    arktest.assertEQ(new Decimal('-0.000000000000000006337122571683959413228').toExponential(4), '-6.3372e-18');
    arktest.assertEQ(new Decimal('-8918908714500548003.38400978696756078013348').toExponential(7), '-8.9189088e+18');
}

function testToExponentialWithRMIsFour001() {
    Decimal.rounding = 4;
    arktest.assertEQ(new Decimal('-0.00000000000000000000000000000000000000500223911660588892717870293065633642').toExponential(30), '-5.002239116605888927178702930656e-39');
    arktest.assertEQ(new Decimal('-852292947230244775434968241532.494643593912804433318745222587246680109833509655450267792446').toExponential(20), '-8.52292947230244775435e+29');
    arktest.assertEQ(new Decimal('-61169514510.8673382').toExponential(13), '-6.1169514510867e+10');
    arktest.assertEQ(new Decimal('-8057457635273076761707597221751692660178316952146').toExponential(47), '-8.05745763527307676170759722175169266017831695215e+48');
    arktest.assertEQ(new Decimal('-49235721020.9847017846898652687600227388412980598816').toExponential(12), '-4.923572102098e+10');
    arktest.assertEQ(new Decimal('-798134166171502711774690607651594491039629').toExponential(33), '-7.981341661715027117746906076515945e+41');
    arktest.assertEQ(new Decimal('-0.008').toExponential(2), '-8.00e-3');
    arktest.assertEQ(new Decimal('0.000000000000008517466793430899278197016892').toExponential(39), '8.517466793430899278197016892000000000000e-15');
    arktest.assertEQ(new Decimal('-3.0322935124071923328711934463341802038').toExponential(9), '-3.032293512e+0');
    arktest.assertEQ(new Decimal('-0.00000000000000000002606829044034893056789087713239958101382673852').toExponential(53), '-2.60682904403489305678908771323995810138267385200000000e-20');
    arktest.assertEQ(new Decimal('-393581692727398036652.850960055902271').toExponential(15), '-3.935816927273980e+20');
    arktest.assertEQ(new Decimal('-859021525010507210136559039003.689834129033952321238').toExponential(13), '-8.5902152501051e+29');
    arktest.assertEQ(new Decimal('-0.00000000000000000000000000000724490826045045451271534').toExponential(5), '-7.24491e-30');
    arktest.assertEQ(new Decimal('-8494807028534919397498922.15049193806567151361656033246').toExponential(52), '-8.4948070285349193974989221504919380656715136165603325e+24');
    arktest.assertEQ(new Decimal('-0.00000000000000006329523959626011114164').toExponential(10), '-6.3295239596e-17');
    arktest.assertEQ(new Decimal('-3172569235260846783669130724638.711').toExponential(10), '-3.1725692353e+30');
    arktest.assertEQ(new Decimal('-406572707673.336570352310681187663765').toExponential(9), '-4.065727077e+11');
    arktest.assertEQ(new Decimal('-6828838692499980755.7424722315549682855987375899188309581152').toExponential(32), '-6.82883869249998075574247223155497e+18');
    arktest.assertEQ(new Decimal('-2561444004270452149437863.38354535663028539').toExponential(26), '-2.56144400427045214943786338e+24');
    arktest.assertEQ(new Decimal('-497637439956044400125498.8682100590602459937304614141772').toExponential(26), '-4.97637439956044400125498868e+23');
    arktest.assertEQ(new Decimal('-430789192919870282274653450614.349564081').toExponential(30), '-4.307891929198702822746534506143e+29');
    arktest.assertEQ(new Decimal('-0.00000000000000000000000000855367295711812079').toExponential(2), '-8.55e-27');
    arktest.assertEQ(new Decimal('-790612526329.410459220189562').toExponential(3), '-7.906e+11');
    arktest.assertEQ(new Decimal('-0.00000000000000000000031841363').toExponential(7), '-3.1841363e-22');
}

function testToExponentialWithRMIsFour002() {
    arktest.assertEQ(new Decimal('-620680493048450055389.3227069760888437941041').toExponential(16), '-6.2068049304845006e+20');
    arktest.assertEQ(new Decimal('-8480947614295114807.320148688').toExponential(7), '-8.4809476e+18');
    arktest.assertEQ(new Decimal('-228798857073425585542366.399034916953775').toExponential(18), '-2.287988570734255855e+23');
    arktest.assertEQ(new Decimal('-8148647139762925073276.1644862403206980851079').toExponential(36), '-8.148647139762925073276164486240320698e+21');
    arktest.assertEQ(new Decimal('-0.0000000000068764313878566475604352570287089535238582267443').toExponential(17), '-6.87643138785664756e-12');
    arktest.assertEQ(new Decimal('-3709586618852569033.55141868').toExponential(6), '-3.709587e+18');
    arktest.assertEQ(new Decimal('-68086794224433270564431694468.814537646575833889824621540849').toExponential(10), '-6.8086794224e+28');
    arktest.assertEQ(new Decimal('-49663010851788946007').toExponential(12), '-4.966301085179e+19');
    arktest.assertEQ(new Decimal('-534439184068052811184219234.494113670484623394').toExponential(32), '-5.34439184068052811184219234494114e+26');
    arktest.assertEQ(new Decimal('-27987324119455299').toExponential(9), '-2.798732412e+16');
    arktest.assertEQ(new Decimal('-1554430791885961.956863404519493346081223').toExponential(18), '-1.554430791885961957e+15');
    arktest.assertEQ(new Decimal('-6906190838220750039778836.289105048686876596').toExponential(20), '-6.90619083822075003978e+24');
    arktest.assertEQ(new Decimal('-1108034176809.7705783154').toExponential(21), '-1.108034176809770578315e+12');
    arktest.assertEQ(new Decimal('-14266566332440117777110.63461224926682073525873105').toExponential(2), '-1.43e+22');
    arktest.assertEQ(new Decimal('-91477543307040.916791223').toExponential(2), '-9.15e+13');
    arktest.assertEQ(new Decimal('-110010856476508992391958436.9355559264588205214557001854').toExponential(4), '-1.1001e+26');
    arktest.assertEQ(new Decimal('-12148027447349021').toExponential(1), '-1.2e+16');
    arktest.assertEQ(new Decimal('-44268551660889.40880208546489742632181832780494').toExponential(1), '-4.4e+13');
    arktest.assertEQ(new Decimal('-86205892033855548408.169086865949596390775').toExponential(23), '-8.62058920338555484081691e+19');
    arktest.assertEQ(new Decimal('-0.00000000000051876025261394172').toExponential(1), '-5.2e-13');
    arktest.assertEQ(new Decimal('-0.0000000000488063953404884862027221562057786242658496407473').toExponential(47), '-4.88063953404884862027221562057786242658496407473e-11');
    arktest.assertEQ(new Decimal('-5254530327311322805.9528217').toExponential(3), '-5.255e+18');
    arktest.assertEQ(new Decimal('-0.0000000000646304880039951167486').toExponential(16), '-6.4630488003995117e-11');
    arktest.assertEQ(new Decimal('-0.0000000000000008638990742870608').toExponential(12), '-8.638990742871e-16');
    arktest.assertEQ(new Decimal('-1578177500205.60815944470062002898187').toExponential(26), '-1.57817750020560815944470062e+12');
    arktest.assertEQ(new Decimal('-0.00000000000000000000000000365583845930939004226367940618').toExponential(22), '-3.6558384593093900422637e-27');
}

function testToExponentialWithRMIsFour003() {
    arktest.assertEQ(new Decimal('-2.03361945085664524115397653636144859').toExponential(24), '-2.033619450856645241153977e+0');
    arktest.assertEQ(new Decimal('112955590.0430616').toExponential(3), '1.130e+8');
    arktest.assertEQ(new Decimal('-21366468193.419876852426155614364269').toExponential(34), '-2.1366468193419876852426155614364269e+10');
    arktest.assertEQ(new Decimal('58208661.56595661515285734890860077163').toExponential(20), '5.82086615659566151529e+7');
    arktest.assertEQ(new Decimal('9161580.937281742611120838868847823478250167882379624').toExponential(22), '9.1615809372817426111208e+6');
    arktest.assertEQ(new Decimal('38.97650690106116419699490320634490920742414').toExponential(19), '3.8976506901061164197e+1');
    arktest.assertEQ(new Decimal('9099491.4931570087194607344641722310103895224905').toExponential(37), '9.0994914931570087194607344641722310104e+6');
    arktest.assertEQ(new Decimal('605633').toExponential(2), '6.06e+5');
    arktest.assertEQ(new Decimal('26.9999747904737055189921170044987').toExponential(25), '2.6999974790473705518992117e+1');
    arktest.assertEQ(new Decimal('6710880.136172156342982663450743452').toExponential(13), '6.7108801361722e+6');
    arktest.assertEQ(new Decimal('-8').toExponential(1), '-8.0e+0');
    arktest.assertEQ(new Decimal('0.03').toExponential(3), '3.000e-2');
    arktest.assertEQ(new Decimal('-469').toExponential(1), '-4.7e+2');
    arktest.assertEQ(new Decimal('-6.3').toExponential(4), '-6.3000e+0');
    arktest.assertEQ(new Decimal('-542').toExponential(1), '-5.4e+2');
    arktest.assertEQ(new Decimal('-5.2').toExponential(4), '-5.2000e+0');
    arktest.assertEQ(new Decimal('-0.09').toExponential(5), '-9.00000e-2');
    arktest.assertEQ(new Decimal('-0.31').toExponential(4), '-3.1000e-1');
    arktest.assertEQ(new Decimal('-436').toExponential(1), '-4.4e+2');
    arktest.assertEQ(new Decimal('-3').toExponential(2), '-3.00e+0');
    arktest.assertEQ(new Decimal('-0.05').toExponential(2), '-5.00e-2');
    arktest.assertEQ(new Decimal('0.01').toExponential(2), '1.00e-2');
    arktest.assertEQ(new Decimal('123').toExponential(), '1.23e+2');
    arktest.assertEQ(new Decimal('123').toExponential(0), '1e+2');
    arktest.assertEQ(new Decimal('123').toExponential(-0), '1e+2');
}

function testToExponentialWithRMIsFive001() {
    Decimal.rounding = 5;
    arktest.assertEQ(new Decimal('495474614815842191683004449862568813538.573064401156').toExponential(14), '4.95474614815842e+38');
    arktest.assertEQ(new Decimal('-89667567079038139').toExponential(16), '-8.9667567079038139e+16');
    arktest.assertEQ(new Decimal('-703').toExponential(1), '-7.0e+2');
    arktest.assertEQ(new Decimal('-2624861185343559570287214983819906').toExponential(4), '-2.6249e+33');
    arktest.assertEQ(new Decimal('-0.0000000000000000000000000000006510119186347371697501169416839709631422185436811698613').toExponential(84), '-6.510119186347371697501169416839709631422185436811698613000000000000000000000000000000e-31');
    arktest.assertEQ(new Decimal('7729').toExponential(2), '7.73e+3');
    arktest.assertEQ(new Decimal('14393.781011009257793117531801548751').toExponential(31), '1.4393781011009257793117531801549e+4');
    arktest.assertEQ(new Decimal('8404542').toExponential(1), '8.4e+6');
    arktest.assertEQ(new Decimal('0.00000000000000000000000000000000008471284625267663009248667391059202502589207637435209861233007389').toExponential(78), '8.471284625267663009248667391059202502589207637435209861233007389000000000000000e-35');
    arktest.assertEQ(new Decimal('-52607929722701509263909039511536.9266822991').toExponential(14), '-5.26079297227015e+31');
    arktest.assertEQ(new Decimal('-4635506008570035.51411914120562163394396594237358863897062').toExponential(35), '-4.63550600857003551411914120562163394e+15');
    arktest.assertEQ(new Decimal('-7821956340648233876718910043.4751303552919130625101491').toExponential(52), '-7.8219563406482338767189100434751303552919130625101491e+27');
    arktest.assertEQ(new Decimal('-697718409782854734').toExponential(9), '-6.977184098e+17');
    arktest.assertEQ(new Decimal('-8092701222454628.9934935902179330839653799891168').toExponential(1), '-8.1e+15');
    arktest.assertEQ(new Decimal('-3872944373744596.91569188472997336351132980366520033057011287').toExponential(30), '-3.872944373744596915691884729973e+15');
    arktest.assertEQ(new Decimal('-138967565295.146055555208419143848718279114979831585').toExponential(6), '-1.389676e+11');
    arktest.assertEQ(new Decimal('-22183169931309038822.22612').toExponential(21), '-2.218316993130903882223e+19');
    arktest.assertEQ(new Decimal('-0.000000000000000000000000337080930401566').toExponential(9), '-3.370809304e-25');
    arktest.assertEQ(new Decimal('-61503417721509415792.24703').toExponential(4), '-6.1503e+19');
    arktest.assertEQ(new Decimal('-0.00000000000000000000031365713378439345').toExponential(8), '-3.13657134e-22');
    arktest.assertEQ(new Decimal('-0.000000000187981').toExponential(1), '-1.9e-10');
}

function testToExponentialWithRMIsFive002() {
    arktest.assertEQ(new Decimal('-25965083537144256779700497237.5841327343962292316215149169').toExponential(27), '-2.596508353714425677970049724e+28');
    arktest.assertEQ(new Decimal('-415145454574.827760411230810117491706171981266892178').toExponential(36), '-4.151454545748277604112308101174917062e+11');
    arktest.assertEQ(new Decimal('-0.000000000000000001319061308619561567664259803361817').toExponential(1), '-1.3e-18');
    arktest.assertEQ(new Decimal('-1529485448704655315921667').toExponential(19), '-1.5294854487046553159e+24');
    arktest.assertEQ(new Decimal('-0.000000000000193654876547081437655834005383101033507988').toExponential(40), '-1.9365487654708143765583400538310103350799e-13');
    arktest.assertEQ(new Decimal('-38812825927635742702751547.353').toExponential(26), '-3.88128259276357427027515474e+25');
    arktest.assertEQ(new Decimal('-0.00000000005645254749041555173742897362182099811344').toExponential(29), '-5.64525474904155517374289736218e-11');
    arktest.assertEQ(new Decimal('-89496338575500640913143.0087734467744538').toExponential(35), '-8.94963385755006409131430087734467745e+22');
    arktest.assertEQ(new Decimal('-375517319017640249').toExponential(16), '-3.7551731901764025e+17');
    arktest.assertEQ(new Decimal('-0.00000000000000076019214974360137746140339586742455753').toExponential(6), '-7.601921e-16');
    arktest.assertEQ(new Decimal('-693501087055377288564').toExponential(11), '-6.93501087055e+20');
    arktest.assertEQ(new Decimal('-1283656440009563292695670.575360580373829197017512').toExponential(15), '-1.283656440009563e+24');
    arktest.assertEQ(new Decimal('-49556505932168.7211084603').toExponential(7), '-4.9556506e+13');
    arktest.assertEQ(new Decimal('-813358458894586332533196788.490201803951456991010654609646').toExponential(12), '-8.133584588946e+26');
    arktest.assertEQ(new Decimal('-38242072955850210158724').toExponential(9), '-3.824207296e+22');
    arktest.assertEQ(new Decimal('-0.00000000000421680868317080291').toExponential(7), '-4.2168087e-12');
    arktest.assertEQ(new Decimal('-7152812829336253.782723153403637377960530795').toExponential(9), '-7.152812829e+15');
    arktest.assertEQ(new Decimal('-80469635248612874.5712104436').toExponential(19), '-8.0469635248612874571e+16');
    arktest.assertEQ(new Decimal('-272654995401.8643349550392803783934819148125595437353472547').toExponential(27), '-2.726549954018643349550392804e+11');
    arktest.assertEQ(new Decimal('-2477986360297097033217143442370.539404').toExponential(24), '-2.477986360297097033217143e+30');
    arktest.assertEQ(new Decimal('-2762055540757162').toExponential(10), '-2.7620555408e+15');
    arktest.assertEQ(new Decimal('-50436788962').toExponential(3), '-5.044e+10');
    arktest.assertEQ(new Decimal('-151176171306898543.9270094279657616389483779413616294465635').toExponential(35), '-1.51176171306898543927009427965761639e+17');
    arktest.assertEQ(new Decimal('-177876313221062362.01').toExponential(17), '-1.77876313221062362e+17');
    arktest.assertEQ(new Decimal('-42803336471574.430066253616').toExponential(23), '-4.28033364715744300662536e+13');
    arktest.assertEQ(new Decimal('-0.00000000000060527568964627046163209582').toExponential(3), '-6.053e-13');
    arktest.assertEQ(new Decimal('-39447068214322315.685949701607748760885392781169754754427622').toExponential(34), '-3.9447068214322315685949701607748761e+16');
    arktest.assertEQ(new Decimal('-4762036655865520.285').toExponential(17), '-4.76203665586552028e+15');
    arktest.assertEQ(new Decimal('-7442141482296791204320219.2472305303585223494415').toExponential(36), '-7.442141482296791204320219247230530359e+24');
}

function testToExponentialWithRMIsSix001() {
    Decimal.rounding = 6;
    arktest.assertEQ(new Decimal('-4350270750116411997402439304498892819').toExponential(13), '-4.3502707501164e+36');
    arktest.assertEQ(new Decimal('0.0000000000000000000094520280724178734152').toExponential(1), '9.5e-21');
    arktest.assertEQ(new Decimal('13963118675055417278567601269341861.725007220074421462599465604772727067224824374190703237660781').toExponential(83), '1.39631186750554172785676012693418617250072200744214625994656047727270672248243741907e+34');
    arktest.assertEQ(new Decimal('0.00000000000000000000000005944657036540768164877637239177740419063920648').toExponential(7), '5.9446570e-26');
    arktest.assertEQ(new Decimal('0.000000000007').toExponential(5), '7.00000e-12');
    arktest.assertEQ(new Decimal('-287060740776209.3950381715').toExponential(2), '-2.87e+14');
    arktest.assertEQ(new Decimal('3411740542875509328514044').toExponential(18), '3.411740542875509329e+24');
    arktest.assertEQ(new Decimal('-0.000000000000000000000000000062023511273868704611839583').toExponential(47), '-6.20235112738687046118395830000000000000000000000e-29');
    arktest.assertEQ(new Decimal('29434913012157027662.686313539671733652865549279174').toExponential(44), '2.94349130121570276626863135396717336528655493e+19');
    arktest.assertEQ(new Decimal('0.000000000401255076512828067130306533670644537831678294548').toExponential(38), '4.01255076512828067130306533670644537832e-10');
    arktest.assertEQ(new Decimal('-542773064444.317654960431120452254700391693837992').toExponential(13), '-5.4277306444432e+11');
    arktest.assertEQ(new Decimal('-4355706886680889557797360814401.536556745674646509159280626').toExponential(30), '-4.355706886680889557797360814402e+30');
    arktest.assertEQ(new Decimal('-0.00000000000000128978312277001609181774216296380783932').toExponential(2), '-1.29e-15');
    arktest.assertEQ(new Decimal('-10588973816292989768709129.1767038708798755780352204').toExponential(19), '-1.0588973816292989769e+25');
    arktest.assertEQ(new Decimal('-32105695962.8803639621').toExponential(9), '-3.210569596e+10');
    arktest.assertEQ(new Decimal('-71850427017374468136068271495.87').toExponential(29), '-7.18504270173744681360682714959e+28');
    arktest.assertEQ(new Decimal('-461568214282826906622.7773895179987062919071922').toExponential(39), '-4.615682142828269066227773895179987062919e+20');
    arktest.assertEQ(new Decimal('-13864477517287.15552607265').toExponential(22), '-1.3864477517287155526073e+13');
    arktest.assertEQ(new Decimal('-67931200280922.72252141789646787475433427482').toExponential(9), '-6.793120028e+13');
    arktest.assertEQ(new Decimal('-0.000000000000000008074975073002274636799975').toExponential(3), '-8.075e-18');
}

function testToExponentialWithRMIsSix002() {
    arktest.assertEQ(new Decimal('-8360228691054180854419062.530687032074820667001120752628').toExponential(45), '-8.360228691054180854419062530687032074820667001e+24');
    arktest.assertEQ(new Decimal('-0.000000000003076395676041719403521594').toExponential(22), '-3.0763956760417194035216e-12');
    arktest.assertEQ(new Decimal('-25288383009460922631988717.84659997837058450749').toExponential(7), '-2.5288383e+25');
    arktest.assertEQ(new Decimal('-455418519247311560996997520087.98189').toExponential(9), '-4.554185192e+29');
    arktest.assertEQ(new Decimal('-913517537232.413846739726417').toExponential(24), '-9.135175372324138467397264e+11');
    arktest.assertEQ(new Decimal('-0.0000000008257259383044471855222900534859251889332388855848').toExponential(48), '-8.257259383044471855222900534859251889332388855848e-10');
    arktest.assertEQ(new Decimal('-0.000000000000765159726845092270720405167100094').toExponential(18), '-7.651597268450922707e-13');
    arktest.assertEQ(new Decimal('-895201176395099451377549961.34870447').toExponential(18), '-8.952011763950994514e+26');
    arktest.assertEQ(new Decimal('-0.00000000027395479569618982298152060567357').toExponential(31), '-2.7395479569618982298152060567357e-10');
    arktest.assertEQ(new Decimal('-1311514517004533788414313').toExponential(23), '-1.31151451700453378841431e+24');
    arktest.assertEQ(new Decimal('-0.0000000005915297930316863890707686339684395').toExponential(18), '-5.915297930316863891e-10');
    arktest.assertEQ(new Decimal('-0.0000000000000000000000000014487033279693402845128265141859').toExponential(3), '-1.449e-27');
    arktest.assertEQ(new Decimal('-36919550406.826974442743517918128').toExponential(1), '-3.7e+10');
    arktest.assertEQ(new Decimal('-39453476889403.824996317791066388653').toExponential(33), '-3.945347688940382499631779106638865e+13');
    arktest.assertEQ(new Decimal('-0.0000000000000000000000000000854770378842608635356').toExponential(6), '-8.547704e-29');
    arktest.assertEQ(new Decimal('-37618296325402619735777629.467812385256281737441412').toExponential(2), '-3.76e+25');
    arktest.assertEQ(new Decimal('-80310660863986235667567286452').toExponential(15), '-8.031066086398624e+28');
    arktest.assertEQ(new Decimal('-0.000000000000000040382762560881354955896694823328777602811').toExponential(18), '-4.038276256088135496e-17');
    arktest.assertEQ(new Decimal('-17717357474086086837250852').toExponential(17), '-1.77173574740860868e+25');
    arktest.assertEQ(new Decimal('-1421967648805122645888').toExponential(9), '-1.421967649e+21');
    arktest.assertEQ(new Decimal('-469485715327').toExponential(1), '-4.7e+11');
    arktest.assertEQ(new Decimal('-73722232915604550.75681748682810527006882666313809409').toExponential(39), '-7.372223291560455075681748682810527006883e+16');
    arktest.assertEQ(new Decimal('-895393963565598').toExponential(10), '-8.9539396357e+14');
    arktest.assertEQ(new Decimal('-81464610385.48021722504148014045579178726').toExponential(29), '-8.14646103854802172250414801405e+10');
    arktest.assertEQ(new Decimal('-1205341573442.5581371841633131879').toExponential(16), '-1.2053415734425581e+12');
    arktest.assertEQ(new Decimal('-83521417686104613359610131317.0854966756043001041619492').toExponential(44), '-8.35214176861046133596101313170854966756043001e+28');
}
