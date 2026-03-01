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
    const suite = new arktest.ArkTestsuite("Decimal isFiniteEtc API tests");
    suite.addTest("Decimal isFinite and isNaN with constructor form 1;", testWithConstructorfrom1);
    suite.addTest("Decimal isFinite and isNaN with constructor form '-0.1';", testWithConstructorfromString0_1);
    suite.addTest("Decimal isFinite and isNaN with constructor form Infinity;", testWithConstructorfromInfinity);
    suite.addTest("Decimal isFinite and isNaN with constructor form '-Infinity';", testWithConstructorfromStringNegativeInfinity);
    suite.addTest("Decimal isFinite and isNaN with constructor form '0.0000000';", testWithConstructorfromString0_0000000);
    suite.addTest("Decimal isFinite and isNaN with constructor form -0;", testWithConstructorfromNegativeZero);
    suite.addTest("Decimal isFinite and isNaN with constructor form 'NaN';", testWithConstructorfromStringNaN);
    suite.addTest("Decimal isFinite and isNaN with constructor form '-1.234e+2';", testWithConstructorfromStringNegative1_234e_2);
    suite.addTest("Decimal isFinite and isNaN with constructor form '5e-200';", testWithConstructorfromString5e_200);
    suite.addTest("Decimal isFinite and isNaN with constructor form '1';", testWithConstructorfromString1);
    return suite.run();
}

function testWithConstructorfrom1() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal(1);
    arktest.assertTrue(n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(!n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(n.isInteger());
}

function testWithConstructorfromString0_1() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal('-0.1');
    arktest.assertTrue(n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(!n.isInteger());
}

function testWithConstructorfromInfinity() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal(Infinity);
    arktest.assertTrue(!n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(!n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(!n.isInteger());
}

function testWithConstructorfromStringNegativeInfinity() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal('-Infinity');
    arktest.assertTrue(!n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(!n.isInteger());
}

function testWithConstructorfromString0_0000000() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal('0.0000000');
    arktest.assertTrue(n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(!n.isNegative());
    arktest.assertTrue(n.isZero());
    arktest.assertTrue(n.isInteger());
}

function testWithConstructorfromNegativeZero() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal(-0);
    arktest.assertTrue(n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(n.isZero());
    arktest.assertTrue(n.isInteger());
}

function testWithConstructorfromStringNaN() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal('NaN');
    arktest.assertTrue(!n.isFinite());
    arktest.assertTrue(n.isNaN());
    arktest.assertTrue(!n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(!n.isInteger());
}

function testWithConstructorfromStringNegative1_234e_2() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal('-1.234e+2');
    arktest.assertTrue(n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(!n.isInteger());
}

function testWithConstructorfromString5e_200() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    let n = new Decimal('5e-200');
    arktest.assertTrue(n.isFinite());
    arktest.assertTrue(!n.isNaN());
    arktest.assertTrue(!n.isNegative());
    arktest.assertTrue(!n.isZero());
    arktest.assertTrue(!n.isInteger());
}

function testWithConstructorfromString1() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertTrue(!new Decimal('1.0000000000000000000001').isInteger());
    arktest.assertTrue(!new Decimal('0.999999999999999999999').isInteger());
    arktest.assertTrue(new Decimal('4e4').isInteger());
    arktest.assertTrue(new Decimal('-4e4').isInteger());
}
