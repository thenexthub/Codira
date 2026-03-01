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
    const suite = new arktest.ArkTestsuite("Decimal toNearest API tests");
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    suite.addTest("Decimal toNearest with isZero and isNegative", testToNearestWithIsZeroAndIsNegative);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters001);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters002);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters003);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters004);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters005);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters006);
    suite.addTest("Decimal toNearest with specific config", testToNearestWithTwoParameters007);
    return suite.run();
}

function isMinusZero(n: Decimal) {
    return n.isZero() && n.isNegative();
}

function testToNearestWithIsZeroAndIsNegative() {
    arktest.assertTrue(!isMinusZero(new Decimal(0).toNearest(0)));
    arktest.assertTrue(isMinusZero(new Decimal(-1).toNearest(0)));
    arktest.assertTrue(!isMinusZero(new Decimal(1).toNearest(0)));
    arktest.assertTrue(!isMinusZero(new Decimal(1).toNearest(-0)));
    arktest.assertTrue(!isMinusZero(new Decimal(1).toNearest(-3)));
}

function testToNearestWithTwoParameters001() {
    arktest.assertEQ(new Decimal(NaN).toNearest(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).toNearest(Infinity).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).toNearest(-Infinity).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).toNearest(0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).toNearest(-0).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('9.999e+9000000000000000').toNearest('1e+9000000000000001').valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('9.999e+9000000000000000').toNearest('-1e+9000000000000001').valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-9.999e+9000000000000000').toNearest('1e+9000000000000001').valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('-9.999e+9000000000000000').toNearest('-1e+9000000000000001').valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(123.456).toNearest(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(123.456).toNearest(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(123.456).toNearest(-Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(123.456).toNearest(0).valueOf(), '0');
    arktest.assertEQ(new Decimal(123.456).toNearest('-0').valueOf(), '0');
    arktest.assertEQ(new Decimal(-123.456).toNearest(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-123.456).toNearest(Infinity).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(-123.456).toNearest(-Infinity).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(-123.456).toNearest('-0').valueOf(), '-0');
    arktest.assertEQ(new Decimal(0).toNearest(0).valueOf(), '0');
    arktest.assertEQ(new Decimal(0).toNearest(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(0).toNearest(-Infinity).valueOf(), 'Infinity');
}

function testToNearestWithTwoParameters002() {
    arktest.assertEQ(new Decimal(1).toNearest(-3).valueOf(), '0');
    arktest.assertEQ(new Decimal(1.5).toNearest(-3, 0).valueOf(), '3');
    arktest.assertEQ(new Decimal(-1.5).toNearest(-3, 2).valueOf(), '-3');
    arktest.assertEQ(new Decimal(123.456).toNearest(1).valueOf(), '123');
    arktest.assertEQ(new Decimal(123.456).toNearest(0.1).valueOf(), '123.5');
    arktest.assertEQ(new Decimal(123.456).toNearest(0.01).valueOf(), '123.46');
    arktest.assertEQ(new Decimal(123.456).toNearest(0.001).valueOf(), '123.456');
    arktest.assertEQ(new Decimal(123.456).toNearest(-1).valueOf(), '123');
    arktest.assertEQ(new Decimal(123.456).toNearest(-0.1).valueOf(), '123.5');
    arktest.assertEQ(new Decimal(123.456).toNearest(-0.01).valueOf(), '123.46');
    arktest.assertEQ(new Decimal(123.456).toNearest(-0.001).valueOf(), '123.456');
    arktest.assertEQ(new Decimal(123.456).toNearest('-2').valueOf(), '124');
    arktest.assertEQ(new Decimal(123.456).toNearest('-0.2').valueOf(), '123.4');
    arktest.assertEQ(new Decimal(123.456).toNearest('-0.02').valueOf(), '123.46');
    arktest.assertEQ(new Decimal(123.456).toNearest('-0.002').valueOf(), '123.456');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 4).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(1, 4).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest('1', 5).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(1, 5).valueOf(), '83105511540');
}

function testToNearestWithTwoParameters003() {
    Decimal.precision = 3;
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(new Decimal(1), 4).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(1, 4).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(new Decimal('1'), 5).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(1, 5).valueOf(), '83105511540');

    Decimal.precision = 20;
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(-1, 4).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(-1, 4).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest('-1', 5).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(-1, 5).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('-83105511539.5').toNearest(new Decimal(-1), 4).valueOf(), '-83105511540');
    arktest.assertEQ(new Decimal('-83105511539.499999999999999999999999999999').toNearest(1, 4).valueOf(), '-83105511539');
    arktest.assertEQ(new Decimal('-83105511539.5').toNearest(new Decimal('-1'), 5).valueOf(), '-83105511539');
    arktest.assertEQ(new Decimal('-83105511539.5000000000000000000001').toNearest(-1, 5).valueOf(), '-83105511540');
}

function testToNearestWithTwoParameters004() {
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 0).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 1).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 2).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 3).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 4).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 5).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 6).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 7).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.5').toNearest(1, 8).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(1, 1).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(1, 3).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(1, 5).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.499999999999999999999999999999').toNearest(1, 7).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(1, 1).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(1, 3).valueOf(), '83105511539');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(1, 5).valueOf(), '83105511540');
    arktest.assertEQ(new Decimal('83105511539.5000000000000000000001').toNearest(1, 7).valueOf(), '83105511540');
}

function testToNearestWithTwoParameters005() {
    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('3847561.00000749').toNearest(10, 0).valueOf(), '3847570');
    arktest.assertEQ(new Decimal('42835000000000001').toNearest('1e+13', 0).valueOf(), '42840000000000000');
    arktest.assertEQ(new Decimal('42835000000000001').toNearest('1e+13', 1).valueOf(), '42830000000000000');
    arktest.assertEQ(new Decimal('42835000000000000.0002').toNearest('1e+13', 0).valueOf(), '42840000000000000');
    arktest.assertEQ(new Decimal('42835000000000000.0002').toNearest('1e+13', 1).valueOf(), '42830000000000000');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 0).valueOf(), '500');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 1).valueOf(), '400');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 2).valueOf(), '500');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 3).valueOf(), '400');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 4).valueOf(), '400');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 5).valueOf(), '400');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 6).valueOf(), '400');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 7).valueOf(), '400');
    arktest.assertEQ(new Decimal('449.999').toNearest(100, 8).valueOf(), '400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 0).valueOf(), '-500');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 1).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 2).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 3).valueOf(), '-500');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 4).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 5).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 6).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 7).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-449.999').toNearest(100, 8).valueOf(), '-400');
}

function testToNearestWithTwoParameters006() {
    arktest.assertEQ(new Decimal('450').toNearest(100, 0).valueOf(), '500');
    arktest.assertEQ(new Decimal('450').toNearest(100, 1).valueOf(), '400');
    arktest.assertEQ(new Decimal('450').toNearest(100, 2).valueOf(), '500');
    arktest.assertEQ(new Decimal('450').toNearest(100, 3).valueOf(), '400');
    arktest.assertEQ(new Decimal('450').toNearest(100, 4).valueOf(), '500');
    arktest.assertEQ(new Decimal('450').toNearest(100, 5).valueOf(), '400');
    arktest.assertEQ(new Decimal('450').toNearest(100, 6).valueOf(), '400');
    arktest.assertEQ(new Decimal('450').toNearest(100, 7).valueOf(), '500');
    arktest.assertEQ(new Decimal('450').toNearest(100, 8).valueOf(), '400');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 0).valueOf(), '-500');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 1).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 2).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 3).valueOf(), '-500');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 4).valueOf(), '-500');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 5).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 6).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 7).valueOf(), '-400');
    arktest.assertEQ(new Decimal('-450').toNearest(100, 8).valueOf(), '-500');
}

function testToNearestWithTwoParameters007() {
    Decimal.rounding = 0;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '400');
    Decimal.rounding = 2;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 3;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '400');
    Decimal.rounding = 4;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 5;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 6;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 7;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 8;
    arktest.assertEQ(new Decimal('450.001').toNearest(100).valueOf(), '500');
    Decimal.rounding = 0;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
    Decimal.rounding = 1;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-400');
    Decimal.rounding = 2;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-400');
    Decimal.rounding = 3;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
    Decimal.rounding = 4;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
    Decimal.rounding = 5;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
    Decimal.rounding = 6;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
    Decimal.rounding = 7;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
    Decimal.rounding = 8;
    arktest.assertEQ(new Decimal('-450.001').toNearest(100).valueOf(), '-500');
}
