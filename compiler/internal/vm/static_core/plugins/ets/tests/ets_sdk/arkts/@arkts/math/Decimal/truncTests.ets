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
    const suite = new arktest.ArkTestsuite("Decimal trunc with default config");
    suite.addTest("Decimal trunc with default config", testTruncWithSepcificConfig001);
    suite.addTest("Decimal trunc with default config", testTruncWithSepcificConfig002);
    suite.addTest("Decimal trunc with default config", testTruncWithSepcificConfig003);
    suite.addTest("Decimal trunc with default config", testTruncWithSepcificConfig004);
    suite.addTest("Decimal trunc with default config", testTruncWithSepcificConfig005);
    suite.addTest("Decimal trunc with default config", testTruncWithSepcificConfig006);
    return suite.run();
}

function testTruncWithSepcificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -1e3, toExpPos: 1e3, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(0).trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal(Infinity).trunc().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).trunc().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(NaN).trunc().valueOf(), 'NaN');
}

function testTruncWithSepcificConfig002() {
    arktest.assertEQ(new Decimal(0.1).trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.49999999999999994').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal(0.5).trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal(0.7).trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal(-0.1).trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0.49999999999999994').trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal(-0.5).trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal(-0.7).trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal(1).trunc().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.1).trunc().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.5).trunc().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.7).trunc().valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).trunc().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.1).trunc().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.5).trunc().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.7).trunc().valueOf(), '-1');
}

function testTruncWithSepcificConfig003() {
    arktest.assertEQ(new Decimal('1.9999999999').trunc().valueOf(), '1');
    arktest.assertEQ(new Decimal('-1.9999999999').trunc().valueOf(), '-1');
    arktest.assertEQ(new Decimal('-2075364.364286541923').trunc().valueOf(), '-2075364');
    arktest.assertEQ(new Decimal('60593539780450631').trunc().valueOf(), '60593539780450631');
    arktest.assertEQ(new Decimal('65937898671515').trunc().valueOf(), '65937898671515');
    arktest.assertEQ(new Decimal('-39719494751819198566798.578').trunc().valueOf(), '-39719494751819198566798');
    arktest.assertEQ(new Decimal('92627382695288166556.8683774524284866028260448205069').trunc().valueOf(), '92627382695288166556');
    arktest.assertEQ(new Decimal('-881574').trunc().valueOf(), '-881574');
    arktest.assertEQ(new Decimal('-3633239209.654526163275621746013315304191073405508491056').trunc().valueOf(), '-3633239209');
    arktest.assertEQ(new Decimal('-23970335459820625362').trunc().valueOf(), '-23970335459820625362');
    arktest.assertEQ(new Decimal('131869457416154038').trunc().valueOf(), '131869457416154038');
    arktest.assertEQ(new Decimal('-4542227860.9511298545226').trunc().valueOf(), '-4542227860');
    arktest.assertEQ(new Decimal('2416872281.963955669484225137349193306323379254936827').trunc().valueOf(), '2416872281');
    arktest.assertEQ(new Decimal('-757684868752087594264588207655.27838048392835556').trunc().valueOf(), '-757684868752087594264588207655');
    arktest.assertEQ(new Decimal('-438798503526.2317623894721299587561697').trunc().valueOf(), '-438798503526');
    arktest.assertEQ(new Decimal('801625782231888715214665').trunc().valueOf(), '801625782231888715214665');
    arktest.assertEQ(new Decimal('327765350218284325239839632046.91682741746683081459605386').trunc().valueOf(), '327765350218284325239839632046');
    arktest.assertEQ(new Decimal('-7469045007691432294.362757245').trunc().valueOf(), '-7469045007691432294');
    arktest.assertEQ(new Decimal('8365540212937142194319515218789.4106658678537421977827').trunc().valueOf(), '8365540212937142194319515218789');
    arktest.assertEQ(new Decimal('-14108.495051214515').trunc().valueOf(), '-14108');
    arktest.assertEQ(new Decimal('49104501.10055989379655329194309526150310568683504206945625').trunc().valueOf(), '49104501');
    arktest.assertEQ(new Decimal('131370406.330005158136313262837556068534122953').trunc().valueOf(), '131370406');
    arktest.assertEQ(new Decimal('-689.6944252229740521128820354989299283').trunc().valueOf(), '-689');
    arktest.assertEQ(new Decimal('73441822178.572653').trunc().valueOf(), '73441822178');
}

function testTruncWithSepcificConfig004() {
    arktest.assertEQ(new Decimal('-2329.42655772223486531483602927572548264457').trunc().valueOf(), '-2329');
    arktest.assertEQ(new Decimal('-834103872107533086').trunc().valueOf(), '-834103872107533086');
    arktest.assertEQ(new Decimal('-1501493189970435.74866616700317').trunc().valueOf(), '-1501493189970435');
    arktest.assertEQ(new Decimal('70591.2244675522123484658978887').trunc().valueOf(), '70591');
    arktest.assertEQ(new Decimal('4446128540401735117.435836700611264749985822486641350492901').trunc().valueOf(), '4446128540401735117');
    arktest.assertEQ(new Decimal('-597273').trunc().valueOf(), '-597273');
    arktest.assertEQ(new Decimal('729117').trunc().valueOf(), '729117');
    arktest.assertEQ(new Decimal('-6581532150677269472829.38194951340848938896000325718062365494').trunc().valueOf(), '-6581532150677269472829');
    arktest.assertEQ(new Decimal('-131279182164804751.430589952021038264').trunc().valueOf(), '-131279182164804751');
    arktest.assertEQ(new Decimal('2949426983040959.8911208825380208568451907').trunc().valueOf(), '2949426983040959');
    arktest.assertEQ(new Decimal('25166.125888418871654557352055849116604612621573251770362').trunc().valueOf(), '25166');
    arktest.assertEQ(new Decimal('4560569286495.98300685103599898554605198').trunc().valueOf(), '4560569286495');
    arktest.assertEQ(new Decimal('13.763105480576616251068323541559825687').trunc().valueOf(), '13');
    arktest.assertEQ(new Decimal('9050999219306.7846946346757664893036971777').trunc().valueOf(), '9050999219306');
    arktest.assertEQ(new Decimal('20962819101135667464733349383.8959025798517496777183').trunc().valueOf(), '20962819101135667464733349383');
    arktest.assertEQ(new Decimal('4125789711001606948191.4707575965791242737346836').trunc().valueOf(), '4125789711001606948191');
    arktest.assertEQ(new Decimal('-6935501.294727166142750626019282').trunc().valueOf(), '-6935501');
    arktest.assertEQ(new Decimal('-1.518418076611593764852321765899').trunc().valueOf(), '-1');
    arktest.assertEQ(new Decimal('6912783515683955988122411164548.393').trunc().valueOf(), '6912783515683955988122411164548');
    arktest.assertEQ(new Decimal('657.0353902852').trunc().valueOf(), '657');
    arktest.assertEQ(new Decimal('0.00000000000000000000000017921822306362413915').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('1483059355427939255846407887.011361095342689876').trunc().valueOf(), '1483059355427939255846407887');
    arktest.assertEQ(new Decimal('8551283060956479352.5707396').trunc().valueOf(), '8551283060956479352');
    arktest.assertEQ(new Decimal('0.000000000000000000000000019904267').trunc().valueOf(), '0');
}

function testTruncWithSepcificConfig005() {
    arktest.assertEQ(new Decimal('321978830777554620127500539.339278568133088682532238002577').trunc().valueOf(), '321978830777554620127500539');
    arktest.assertEQ(new Decimal('2073.532654804291079327244387978249477171032485250998396').trunc().valueOf(), '2073');
    arktest.assertEQ(new Decimal('677676305591.2').trunc().valueOf(), '677676305591');
    arktest.assertEQ(new Decimal('0.0000000000006').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('39181479479778357').trunc().valueOf(), '39181479479778357');
    arktest.assertEQ(new Decimal('0.00000000000000000087964700066672916651').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('896').trunc().valueOf(), '896');
    arktest.assertEQ(new Decimal('115083055948552475').trunc().valueOf(), '115083055948552475');
    arktest.assertEQ(new Decimal('9105942082143427451223').trunc().valueOf(), '9105942082143427451223');
    arktest.assertEQ(new Decimal('0.0000000000000009').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.00000000000000000000004').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000250427721966583680168028884692015623739').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000000000001585613219016120158734661293405081934').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000000090358252973411013592234').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('276312604693909858427.21965306055697011390137926559').trunc().valueOf(), '276312604693909858427');
    arktest.assertEQ(new Decimal('0.0000252').trunc().valueOf(), '0');
}

function testTruncWithSepcificConfig006() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -100, toExpPos: 100, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(-1e308).trunc().valueOf(), '-1e+308');
    arktest.assertEQ(new Decimal('2.1e308').trunc().valueOf(), '2.1e+308');
    arktest.assertEQ(new Decimal('-1e-999').trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal('1e-999').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('1e-9000000000000000').trunc().valueOf(), '0');
    arktest.assertEQ(new Decimal('-1e-9000000000000000').trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal('-9.9e-9000000000000001').trunc().valueOf(), '-0');
    arktest.assertEQ(new Decimal('9.999999e+9000000000000000').trunc().valueOf(), '9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('-9.999999e+9000000000000000').trunc().valueOf(), '-9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('1E9000000000000001').trunc().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-1e+9000000000000001').trunc().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('5.5879983320336874473209567979e+287894365').trunc().valueOf(), '5.5879983320336874473209567979e+287894365');
    arktest.assertEQ(new Decimal('-5.5879983320336874473209567979e+287894365').trunc().valueOf(), '-5.5879983320336874473209567979e+287894365');
}
