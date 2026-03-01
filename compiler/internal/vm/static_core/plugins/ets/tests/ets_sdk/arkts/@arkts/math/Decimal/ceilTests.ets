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
    const suite = new arktest.ArkTestsuite("Decimal ceil with default config");
    suite.addTest("Decimal ceil with default config", testCeilWithSepcificConfig001);
    suite.addTest("Decimal ceil with default config", testCeilWithSepcificConfig002);
    suite.addTest("Decimal ceil with default config", testCeilWithSepcificConfig003);
    suite.addTest("Decimal ceil with default config", testCeilWithSepcificConfig004);
    return suite.run();
}

function testCeilWithSepcificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -100, toExpPos: 100, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(0).ceil().valueOf(), '0');
    arktest.assertEQ(new Decimal(Infinity).ceil().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).ceil().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(NaN).ceil().valueOf(), 'NaN');
}

function testCeilWithSepcificConfig002() {
    arktest.assertEQ(new Decimal(0.1).ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal('0.4999999999999999').ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal(0.5).ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal(0.7).ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal(-0.1).ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0.4999999999999999').ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal(-0.5).ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal(-0.7).ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal(1).ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal(1.1).ceil().valueOf(), '2');
    arktest.assertEQ(new Decimal(1.5).ceil().valueOf(), '2');
    arktest.assertEQ(new Decimal(1.7).ceil().valueOf(), '2');
    arktest.assertEQ(new Decimal(-1).ceil().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.1).ceil().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.5).ceil().valueOf(), '-1');
    arktest.assertEQ(new Decimal(-1.7).ceil().valueOf(), '-1');
}

function testCeilWithSepcificConfig003() {
    arktest.assertEQ(new Decimal('-0.999').ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal('1.0000000000000000001').ceil().valueOf(), '2');
    arktest.assertEQ(new Decimal('1.0000000000000000000000000000000000000000000000000000000000000000000000000000001').ceil().valueOf(), '2');
    arktest.assertEQ(new Decimal('1e-9000000000000000').ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal('-1e-9000000000000000').ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal('-9.9e-9000000000000001').ceil().valueOf(), '-0');
    arktest.assertEQ(new Decimal('9.999999e+9000000000000000').ceil().valueOf(), '9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('-9.999999e+9000000000000000').ceil().valueOf(), '-9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('-1e+9000000000000001').ceil().valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('5.5879983320336874473209567979e+287894365').ceil().valueOf(), '5.5879983320336874473209567979e+287894365');
    arktest.assertEQ(new Decimal('-5.5879983320336874473209567979e+287894365').ceil().valueOf(), '-5.5879983320336874473209567979e+287894365');
}

function testCeilWithSepcificConfig004() {
    arktest.assertEQ(new Decimal('-2075364.364286541923').ceil().valueOf(), '-2075364');
    arktest.assertEQ(new Decimal('60593539780450631').ceil().valueOf(), '60593539780450631');
    arktest.assertEQ(new Decimal('65937898671515').ceil().valueOf(), '65937898671515');
    arktest.assertEQ(new Decimal('-39719494751819198566798.578').ceil().valueOf(), '-39719494751819198566798');
    arktest.assertEQ(new Decimal('92627382695288166556.8683774524284866028260448205069').ceil().valueOf(), '92627382695288166557');
    arktest.assertEQ(new Decimal('-881574').ceil().valueOf(), '-881574');
    arktest.assertEQ(new Decimal('-3633239209.654526163275621746013315304191073405508491056').ceil().valueOf(), '-3633239209');
    arktest.assertEQ(new Decimal('321978830777554620127500539.339278568133088682532238002577').ceil().valueOf(), '321978830777554620127500540');
    arktest.assertEQ(new Decimal('2073.532654804291079327244387978249477171032485250998396').ceil().valueOf(), '2074');
    arktest.assertEQ(new Decimal('677676305591.2').ceil().valueOf(), '677676305592');
    arktest.assertEQ(new Decimal('0.0000000000006').ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal('39181479479778357').ceil().valueOf(), '39181479479778357');
    arktest.assertEQ(new Decimal('0.00000000000000000087964700066672916651').ceil().valueOf(), '1');
    arktest.assertEQ(new Decimal('896').ceil().valueOf(), '896');
    arktest.assertEQ(new Decimal('115083055948552475').ceil().valueOf(), '115083055948552475');
    arktest.assertEQ(new Decimal('9105942082143427451223').ceil().valueOf(), '9105942082143427451223');
    arktest.assertEQ(new Decimal('276312604693909858427.21965306055697011390137926559').ceil().valueOf(), '276312604693909858428');
    arktest.assertEQ(new Decimal('0.0000252').ceil().valueOf(), '1');
}
