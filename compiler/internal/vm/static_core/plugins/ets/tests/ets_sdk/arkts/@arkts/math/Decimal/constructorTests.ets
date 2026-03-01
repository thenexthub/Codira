/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

import { Decimal } from "@arkts.math.Decimal";

function checkDecimalProperties(digits: double[], exponent: double, sign: double, n: double | string) {
    let v = new Decimal(n);
    arktest.assertEQ(v.d!.length, digits.length);
    for (let i = 0; i < digits.length; i++) {
        arktest.assertEQ(v.d![i], digits[i]);
    }
    arktest.assertEQ(v.e, exponent);
    arktest.assertEQ(v.s, sign);
}

function config() {
    Decimal.precision = 40;
    Decimal.rounding = 4;
    Decimal.toExpNeg = -9e15;
    Decimal.toExpPos = 9e15;
    Decimal.maxE = 9e15;
    Decimal.minE = -9e15;
}

function testCreateDecimalWithNumber001() {
    checkDecimalProperties([0], 0, 1, 0);
    checkDecimalProperties([1], 0, -1, -1);
    checkDecimalProperties([10], 1, -1, -10);
}

function testCreateDecimalWithNumber002() {
    checkDecimalProperties([1], 0, 1, 1);
    checkDecimalProperties([10], 1, 1, 10);
    checkDecimalProperties([100], 2, 1, 100);
    checkDecimalProperties([1000], 3, 1, 1000);
    checkDecimalProperties([10000], 4, 1, 10000);
    checkDecimalProperties([100000], 5, 1, 100000);
    checkDecimalProperties([1000000], 6, 1, 1000000);
}

function testCreateDecimalWithNumber003() {
    checkDecimalProperties([1], 7, 1, 10000000);
    checkDecimalProperties([10], 8, 1, 100000000);
    checkDecimalProperties([100], 9, 1, 1000000000);
    checkDecimalProperties([1000], 10, 1, 10000000000);
    checkDecimalProperties([10000], 11, 1, 100000000000);
    checkDecimalProperties([100000], 12, 1, 1000000000000);
    checkDecimalProperties([1000000], 13, 1, 10000000000000);
}

function testCreateDecimalWithNumber004() {
    checkDecimalProperties([1], 14, -1, -100000000000000);
    checkDecimalProperties([10], 15, -1, -1000000000000000);
    checkDecimalProperties([1000000], -1, 1, 1e-1);
    checkDecimalProperties([100000], -2, -1, -1e-2);
    checkDecimalProperties([10000], -3, 1, 1e-3);
    checkDecimalProperties([1000], -4, -1, -1e-4);
    checkDecimalProperties([100], -5, 1, 1e-5);
    checkDecimalProperties([10], -6, -1, -1e-6);
    checkDecimalProperties([1], -7, 1, 1e-7);
}

function testCreateDecimalWithNumber005() {
    checkDecimalProperties([1000000], -8, 1, 1e-8);
    checkDecimalProperties([100000], -9, -1, -1e-9);
    checkDecimalProperties([10000], -10, 1, 1e-10);
    checkDecimalProperties([1000], -11, -1, -1e-11);
    checkDecimalProperties([100], -12, 1, 1e-12);
    checkDecimalProperties([10], -13, -1, -1e-13);
    checkDecimalProperties([1], -14, 1, 1e-14);
}

function testCreateDecimalWithNumber006() {
    checkDecimalProperties([1000000], -15, 1, 1e-15);
    checkDecimalProperties([100000], -16, -1, -1e-16);
    checkDecimalProperties([10000], -17, 1, 1e-17);
    checkDecimalProperties([1000], -18, -1, -1e-18);
    checkDecimalProperties([100], -19, 1, 1e-19);
    checkDecimalProperties([10], -20, -1, -1e-20);
    checkDecimalProperties([1], -21, 1, 1e-21);
}

function testCreateDecimalWithString001() {
    checkDecimalProperties([9], 0, 1, '9');
    checkDecimalProperties([99], 1, -1, '-99');
    checkDecimalProperties([999], 2, 1, '999');
    checkDecimalProperties([9999], 3, -1, '-9999');
    checkDecimalProperties([99999], 4, 1, '99999');
    checkDecimalProperties([999999], 5, -1, '-999999');
    checkDecimalProperties([9999999], 6, 1, '9999999');
}

function testCreateDecimalWithString002() {
    checkDecimalProperties([9, 9999999], 7, -1, '-99999999');
    checkDecimalProperties([99, 9999999], 8, 1, '999999999');
    checkDecimalProperties([999, 9999999], 9, -1, '-9999999999');
    checkDecimalProperties([9999, 9999999], 10, 1, '99999999999');
    checkDecimalProperties([99999, 9999999], 11, -1, '-999999999999');
    checkDecimalProperties([999999, 9999999], 12, 1, '9999999999999');
    checkDecimalProperties([9999999, 9999999], 13, -1, '-99999999999999');
}

function testCreateDecimalWithString003() {
    checkDecimalProperties([9, 9999999, 9999999], 14, 1, '999999999999999');
    checkDecimalProperties([99, 9999999, 9999999], 15, -1, '-9999999999999999');
    checkDecimalProperties([999, 9999999, 9999999], 16, 1, '99999999999999999');
    checkDecimalProperties([9999, 9999999, 9999999], 17, -1, '-999999999999999999');
    checkDecimalProperties([99999, 9999999, 9999999], 18, 1, '9999999999999999999');
    checkDecimalProperties([999999, 9999999, 9999999], 19, -1, '-99999999999999999999');
    checkDecimalProperties([9999999, 9999999, 9999999], 20, 1, '999999999999999999999');
}

function testCreateDecimalWithBinary001() {
    arktest.assertEQ(new Decimal('0b0').valueOf(), '0');
    arktest.assertEQ(new Decimal('0B0').valueOf(), '0');
    arktest.assertEQ(new Decimal('-0b101').valueOf(), '-5');
    arktest.assertEQ(new Decimal('+0b101').valueOf(), '5');
    arktest.assertEQ(new Decimal('0b1.1').valueOf(), '1.5');
    arktest.assertEQ(new Decimal('-0b1.1').valueOf(), '-1.5');
}

function testCreateDecimalWithBinary002() {
    arktest.assertEQ(new Decimal('0b100011100000101.00').valueOf(), '18181');
    arktest.assertEQ(new Decimal('-0b1100.10').valueOf(), '-12.5');
    arktest.assertEQ(new Decimal('0b1010011111101000000.10').valueOf(), '343872.5');
    arktest.assertEQ(new Decimal('-0b101001000.010010').valueOf(), '-328.28125');
    arktest.assertEQ(new Decimal('-0b1010011011110011111.0010010100000000010').valueOf(), '-341919.144535064697265625');
    arktest.assertEQ(new Decimal('0b1100001.000110101101010110000').valueOf(), '97.10482025146484375');
    arktest.assertEQ(new Decimal('-0b11101100001010010.01101').valueOf(), '-120914.40625');
    arktest.assertEQ(new Decimal('0b1101101100000111101001111111010001111010111011001010100101001001011101001').valueOf(), '8080777260861123367657');
}

function testCreateDecimalWithOctal() {
    arktest.assertEQ(new Decimal('0o10').valueOf(), '8');
    arktest.assertEQ(new Decimal('-0O010.4').valueOf(), '-8.5');
    arktest.assertEQ(new Decimal('+0O010.4').valueOf(), '8.5');
    arktest.assertEQ(new Decimal('-0o1000000.00000001').valueOf(), '-262144.000000059604644775390625');
    arktest.assertEQ(new Decimal('0o10250053005734.31').valueOf(), '572315667420.390625');
}
function testCreateDecimalWithHex() {
    arktest.assertEQ(new Decimal('0x00001').valueOf(), '1');
    arktest.assertEQ(new Decimal('0xff').valueOf(), '255');
    arktest.assertEQ(new Decimal('-0Xf.8').valueOf(), '-15.5');
    arktest.assertEQ(new Decimal('+0Xf.8').valueOf(), '15.5');
    arktest.assertEQ(new Decimal('-0x1000000.00000001').valueOf(), '-16777216.00000000023283064365386962890625');
    arktest.assertEQ(new Decimal('0xc16de7aa5bf90c3755ef4dea45e982b351b6e00cd25a82dcfe0646abb').valueOf(), '325927753012307620476767402981591827744994693483231017778102969592507');
}

function testCreateDecimalWithNaN() {
    arktest.assertEQ(new Decimal(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('+NaN').valueOf(), 'NaN');
}

function testCreateDecimalWithInfinity() {
    arktest.assertEQ(new Decimal(Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('Infinity').valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-Infinity').valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal('+Infinity').valueOf(), 'Infinity');
}

function main() {
    const suite = new arktest.ArkTestsuite("Decimal Constructor API tests");
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber001);
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber002);
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber003);
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber004);
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber005);
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber006);
    suite.addTest("Decimal constructor with string", testCreateDecimalWithString001);
    suite.addTest("Decimal constructor with string", testCreateDecimalWithString002);
    suite.addTest("Decimal constructor with string", testCreateDecimalWithString003);
    suite.addTest("Decimal constructor with binary", testCreateDecimalWithBinary001);
    suite.addTest("Decimal constructor with binary", testCreateDecimalWithBinary002);
    suite.addTest("Decimal constructor with octal", testCreateDecimalWithOctal);
    suite.addTest("Decimal constructor with hex", testCreateDecimalWithHex);
    suite.addTest("Decimal constructor with NaN", testCreateDecimalWithNaN);
    suite.addTest("Decimal constructor with Infinity", testCreateDecimalWithInfinity);
    config();
    return suite.run();
}