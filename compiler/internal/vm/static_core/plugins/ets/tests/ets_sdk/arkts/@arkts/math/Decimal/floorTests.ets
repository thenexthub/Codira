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
    const suite = new arktest.ArkTestsuite("Decimal floor API tests");
    suite.addTest("Decimal floor with default config", testFloorWithSpecificConfig001);
    suite.addTest("Decimal floor with default config", testFloorWithSpecificConfig002);
    suite.addTest("Decimal floor with default config", testFloorWithSpecificConfig003);
    suite.addTest("Decimal floor with default config", testFloorWithSpecificConfig004);
    suite.addTest("Decimal floor with default config", testFloorWithSpecificConfig005);
    return suite.run();
}

function testFloorWithSpecificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -1e3, toExpPos: 1e3, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(0).floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal(Infinity).floor().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).floor().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(NaN).floor().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(0.1).floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.49999999999999994').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal(0.5).floor().valueOf(), '0');
    arktest.assertEQ(new Decimal(0.7).floor().valueOf(), '0');
    arktest.assertEQ(new Decimal(-0.1).floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal('-0.49999999999999994').floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-0.5).floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-0.7).floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal(1).floor().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.1).floor().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.5).floor().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.7).floor().valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.1).floor().valueOf(), '-2');
    arktest.assertEQ(new Decimal(-1.5).floor().valueOf(), '-2');
    arktest.assertEQ(new Decimal(-1.7).floor().valueOf(), '-2');
}

function testFloorWithSpecificConfig002() {
    arktest.assertEQ(new Decimal('1.0000000000000000000000001').floor().valueOf(), '1');
    arktest.assertEQ(new Decimal(0.999999999999).floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('9.999999999999').floor().valueOf(), '9');
    arktest.assertEQ(new Decimal(123456.789).floor().valueOf(), '123456');
    arktest.assertEQ(new Decimal('-1.0000000000000000000000001').floor().valueOf(), '-2');
    arktest.assertEQ(new Decimal(-0.999999999999).floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal('-9.999999999999').floor().valueOf(), '-10');
    arktest.assertEQ(new Decimal(-123456.789).floor().valueOf(), '-123457');
    arktest.assertEQ(new Decimal('-2075364.364286541923').floor().valueOf(), '-2075365');
    arktest.assertEQ(new Decimal('60593539780450631').floor().valueOf(), '60593539780450631');
    arktest.assertEQ(new Decimal('65937898671515').floor().valueOf(), '65937898671515');
    arktest.assertEQ(new Decimal('-39719494751819198566798.578').floor().valueOf(), '-39719494751819198566799');
    arktest.assertEQ(new Decimal('92627382695288166556.8683774524284866028260448205069').floor().valueOf(), '92627382695288166556');
    arktest.assertEQ(new Decimal('-881574').floor().valueOf(), '-881574');
    arktest.assertEQ(new Decimal('-3633239209.654526163275621746013315304191073405508491056').floor().valueOf(), '-3633239210', );
    arktest.assertEQ(new Decimal('-23970335459820625362').floor().valueOf(), '-23970335459820625362');
    arktest.assertEQ(new Decimal('131869457416154038').floor().valueOf(), '131869457416154038');
    arktest.assertEQ(new Decimal('-2685').floor().valueOf(), '-2685');
    arktest.assertEQ(new Decimal('-4542227860.9511298545226').floor().valueOf(), '-4542227861');
}

function testFloorWithSpecificConfig003() {
    arktest.assertEQ(new Decimal('-834103872107533086').floor().valueOf(), '-834103872107533086');
    arktest.assertEQ(new Decimal('-1501493189970435.74866616700317').floor().valueOf(), '-1501493189970436');
    arktest.assertEQ(new Decimal('70591.2244675522123484658978887').floor().valueOf(), '70591');
    arktest.assertEQ(new Decimal('4446128540401735117.435836700611264749985822486641350492901').floor().valueOf(), '4446128540401735117');
    arktest.assertEQ(new Decimal('-597273').floor().valueOf(), '-597273');
    arktest.assertEQ(new Decimal('729117.5').floor().valueOf(), '729117');
    arktest.assertEQ(new Decimal('-729117.001').floor().valueOf(), '-729118');
    arktest.assertEQ(new Decimal('4803729546823170064608098091').floor().valueOf(), '4803729546823170064608098091');
    arktest.assertEQ(new Decimal('-6581532150677269472829.38194951340848938896000325718062365494').floor().valueOf(), '-6581532150677269472830');
    arktest.assertEQ(new Decimal('2949426983040959.8911208825380208568451907').floor().valueOf(), '2949426983040959');
    arktest.assertEQ(new Decimal('25166.125888418871654557352055849116604612621573251770362').floor().valueOf(), '25166');
    arktest.assertEQ(new Decimal('4560569286495.98300685103599898554605198').floor().valueOf(), '4560569286495');
    arktest.assertEQ(new Decimal('13.763105480576616251068323541559825687').floor().valueOf(), '13');
    arktest.assertEQ(new Decimal('9050999219306.7846946346757664893036971777').floor().valueOf(), '9050999219306');
    arktest.assertEQ(new Decimal('39900924.00000005').floor().valueOf(), '39900924');
    arktest.assertEQ(new Decimal('115911043168452445').floor().valueOf(), '115911043168452445');
    arktest.assertEQ(new Decimal('20962819101135667464733349383.8959025798517496777183').floor().valueOf(), '20962819101135667464733349383');
    arktest.assertEQ(new Decimal('4125789711001606948191.4707575965791242737346836').floor().valueOf(), '4125789711001606948191');
    arktest.assertEQ(new Decimal('-6935501.294727166142750626019282').floor().valueOf(), '-6935502');
    arktest.assertEQ(new Decimal('-1.518418076611593764852321765899').floor().valueOf(), '-2');
    arktest.assertEQ(new Decimal('-35416').floor().valueOf(), '-35416');
    arktest.assertEQ(new Decimal('6912783515683955988122411164548.393').floor().valueOf(), '6912783515683955988122411164548');
    arktest.assertEQ(new Decimal('657.0353902852').floor().valueOf(), '657');
    arktest.assertEQ(new Decimal('0.00000000000000000000000017921822306362413915').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('1483059355427939255846407887.011361095342689876').floor().valueOf(), '1483059355427939255846407887');
    arktest.assertEQ(new Decimal('7.722e+999999999999999').floor().valueOf(), '7.722e+999999999999999');
    arktest.assertEQ(new Decimal('7722').floor().valueOf(), '7722');
}

function testFloorWithSpecificConfig004() {
    arktest.assertEQ(new Decimal('0.00000005').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('8551283060956479352.5707396').floor().valueOf(), '8551283060956479352');
    arktest.assertEQ(new Decimal('0.000000000000000000000000019904267').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('321978830777554620127500539.339278568133088682532238002577').floor().valueOf(), '321978830777554620127500539');
    arktest.assertEQ(new Decimal('2073.532654804291079327244387978249477171032485250998396').floor().valueOf(), '2073');
    arktest.assertEQ(new Decimal('677676305591.2').floor().valueOf(), '677676305591');
    arktest.assertEQ(new Decimal('39181479479778357').floor().valueOf(), '39181479479778357');
    arktest.assertEQ(new Decimal('0.00000000000000000087964700066672916651').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('115083055948552475').floor().valueOf(), '115083055948552475');
    arktest.assertEQ(new Decimal('9105942082143427451223').floor().valueOf(), '9105942082143427451223');
    arktest.assertEQ(new Decimal('0.00000000000000000000004').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000250427721966583680168028884692015623739').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000000000001585613219016120158734661293405081934').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.00009').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('0.000000090358252973411013592234').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('276312604693909858427.21965306055697011390137926559').floor().valueOf(), '276312604693909858427');
    arktest.assertEQ(new Decimal('0.0000252').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('1.9999999999').floor().valueOf(), '1');
}

function testFloorWithSpecificConfig005() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -100, toExpPos: 100, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal("-1e-308").floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1e308).floor().valueOf(), '-1e+308');
    arktest.assertEQ(new Decimal("-1e308").floor().valueOf(), '-1e+308');
    arktest.assertEQ(new Decimal('2.1e308').floor().valueOf(), '2.1e+308');
    arktest.assertEQ(new Decimal('-1e-999').floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal('1e-999').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('1e-9000000000000000').floor().valueOf(), '0');
    arktest.assertEQ(new Decimal('-1e-9000000000000000').floor().valueOf(), '-1');
    arktest.assertEQ(new Decimal('-9.9e-9000000000000001').floor().valueOf(), '-0');
    arktest.assertEQ(new Decimal('9.999999e+9000000000000000').floor().valueOf(), '9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('-9.999999e+9000000000000000').floor().valueOf(), '-9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('1E9000000000000001').floor().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-1e+9000000000000001').floor().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('5.5879983320336874473209567979e+287894365').floor().valueOf(), '5.5879983320336874473209567979e+287894365');
    arktest.assertEQ(new Decimal('-5.5879983320336874473209567979e+287894365').floor().valueOf(), '-5.5879983320336874473209567979e+287894365');
}
