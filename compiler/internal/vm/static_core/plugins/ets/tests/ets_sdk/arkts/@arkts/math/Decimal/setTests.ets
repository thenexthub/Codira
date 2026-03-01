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

import { Decimal, DecimalConfig } from "@arkts.math.Decimal";

const MAX_DIGITS = 1e9;
const EXP_LIMIT = 9e15;

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal set API tests");

    suite.addTest("Decimal set by get", testSetByGet);
    suite.addTest("Decimal set with precision", testWithPrecision);
    suite.addTest("Decimal set with rounding", testSetWithRounding);
    suite.addTest("Decimal set with toExpNeg", testSetWithToExpNeg);
    suite.addTest("Decimal set with toExpPos", testSetWithToExpPos);
    suite.addTest("Decimal set with maxE", testSetWithMaxE);
    suite.addTest("Decimal set with minE", testSetWithMinE);
    suite.addTest("Decimal set with crypto", testSetWithCrypto);
    suite.addTest("Decimal set with modulo", testSetWithModulo);
    suite.addTest("Decimal set with defaults", testSetWithDefaults);
    suite.addTest("Decimal set with partial update and reset", testSetPartialUpdateAndReset);
    suite.addTest("Decimal set retains unspecified properties", testSetRetainsUnspecifiedProperties);

    return suite.run();
}

function tPrecision(expected: double, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.precision, expected);
}

function tRounding(expected: int, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.rounding, expected);
}

function tToExpNeg(expected: double, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.toExpNeg, expected);
}

function tToExpPos(expected: double, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.toExpPos, expected);
}

function tMaxE(expected: double, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.maxE, expected);
}

function tMinE(expected: double, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.minE, expected);
}

function tCrypto(expected: boolean, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.crypto, expected);
}

function tModulo(expected: int, obj: DecimalConfig) {
    Decimal.set(obj);
    arktest.assertEQ(Decimal.modulo, expected);
}

function testSetByGet() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15, crypto: false, modulo: 1 });
    arktest.assertEQ(Decimal.precision, 20);
    arktest.assertEQ(Decimal.rounding, 4);
    arktest.assertEQ(Decimal.toExpNeg, -7);
    arktest.assertEQ(Decimal.toExpPos, 21);
    arktest.assertEQ(Decimal.minE, -9e15);
    arktest.assertEQ(Decimal.maxE, 9e15);
    arktest.assertFalse(Decimal.crypto);
    arktest.assertEQ(Decimal.modulo, 1);

    Decimal.set({ precision: 40, rounding : 4, toExpNeg: -1000, toExpPos: 1000, minE: -1e9, maxE: 1e9, modulo: 4 });
    arktest.assertEQ(Decimal.precision, 40);
    arktest.assertEQ(Decimal.rounding, 4);
    arktest.assertEQ(Decimal.toExpNeg, -1000);
    arktest.assertEQ(Decimal.toExpPos, 1000);
    arktest.assertEQ(Decimal.minE, -1e9);
    arktest.assertEQ(Decimal.maxE, 1e9);
    arktest.assertEQ(Decimal.modulo, 4);

    Decimal.set({ toExpNeg: -7, toExpPos: 21, minE: -324, maxE: 308 });
    arktest.assertEQ(Decimal.toExpNeg, -7);
    arktest.assertEQ(Decimal.toExpPos, 21);
    arktest.assertEQ(Decimal.minE, -324);
    arktest.assertEQ(Decimal.maxE, 308);
}

function testWithPrecision() {
    tPrecision(1, { precision: 1 });
    tPrecision(20, { precision: 20 });
    tPrecision(300000, { precision: 300000 });
    tPrecision(4e+8, { precision: 4e8 });
    tPrecision(1e9, { precision: 1e9 });
    tPrecision(MAX_DIGITS, { precision: MAX_DIGITS });
}

function testSetWithRounding() {
    tRounding(0, { rounding: 0 });
    tRounding(1, { rounding: 1 });
    tRounding(2, { rounding: 2 });
    tRounding(3, { rounding: 3 });
    tRounding(4, { rounding: 4 });
    tRounding(5, { rounding: 5 });
    tRounding(6, { rounding: 6 });
    tRounding(7, { rounding: 7 });
    tRounding(8, { rounding: 8 });
}

function testSetWithToExpNeg() {
    tToExpNeg(0, { toExpNeg: 0});
    tToExpNeg(-1, { toExpNeg: -1});
    tToExpNeg(-999, { toExpNeg: -999});
    tToExpNeg(-5675367, { toExpNeg: -5675367});
    tToExpNeg(-98770170790791, { toExpNeg: -98770170790791});
    tToExpNeg(-EXP_LIMIT, { toExpNeg: -EXP_LIMIT});
}

function testSetWithToExpPos() {
    tToExpPos(0, { toExpPos: 0 });
    tToExpPos(1, { toExpPos: 1 });
    tToExpPos(999, { toExpPos: 999 });
    tToExpPos(5675367, { toExpPos: 5675367 });
    tToExpPos(98770170790791, { toExpPos: 98770170790791 });
    tToExpPos(EXP_LIMIT, { toExpPos: EXP_LIMIT });
}

function testSetWithMaxE() {
    tMaxE(0, { maxE: 0 });
    tMaxE(1, { maxE: 1 });
    tMaxE(999, { maxE: 999 });
    tMaxE(5675367, { maxE: 5675367 });
    tMaxE(98770170790791, { maxE: 98770170790791 });
    tMaxE(EXP_LIMIT, { maxE: EXP_LIMIT });
}

function testSetWithMinE() {
    tMinE(0, { minE: 0 });
    tMinE(-1, { minE: -1 });
    tMinE(-999, { minE: -999 });
    tMinE(-5675367, { minE: -5675367 });
    tMinE(-98770170790791, { minE: -98770170790791 });
    tMinE(-EXP_LIMIT, { minE: -EXP_LIMIT });
}

function testSetWithCrypto() {
    tCrypto(false, { crypto: false });
    arktest.assertFalse(Decimal.crypto);
}

function testSetWithModulo() {
    tModulo(0, { modulo: 0 });
    tModulo(1, { modulo: 1 });
    tModulo(2, { modulo: 2 });
    tModulo(3, { modulo: 3 });
    tModulo(4, { modulo: 4 });
    tModulo(5, { modulo: 5 });
    tModulo(6, { modulo: 6 });
    tModulo(7, { modulo: 7 });
    tModulo(8, { modulo: 8 });
    tModulo(9, { modulo: 9 });
}

function testSetWithDefaults() {
    Decimal.set({ precision: 100, rounding: 2, toExpNeg: -100, toExpPos: 200 });
    arktest.assertEQ(Decimal.precision, 100);

    Decimal.set({ defaults: true });
    arktest.assertEQ(Decimal.precision, 20);
    arktest.assertEQ(Decimal.rounding, 4);
    arktest.assertEQ(Decimal.toExpNeg, -7);
    arktest.assertEQ(Decimal.toExpPos, 21);

    Decimal.set({ rounding: 3 });
    Decimal.set({ precision: 50, defaults: true });
    arktest.assertEQ(Decimal.precision, 50);
    arktest.assertEQ(Decimal.rounding, 4);
}

function testSetPartialUpdateAndReset() {
    Decimal.set({ precision: 70, rounding: 6, toExpNeg: -10, toExpPos: 20,
        minE: -1e9, maxE: 1e9, modulo: 3 });
    arktest.assertEQ(Decimal.precision, 70);
    arktest.assertEQ(Decimal.rounding, 6);
    arktest.assertEQ(Decimal.toExpNeg, -10);
    arktest.assertEQ(Decimal.toExpPos, 20);
    arktest.assertEQ(Decimal.minE, -1e9);
    arktest.assertEQ(Decimal.maxE, 1e9);
    arktest.assertEQ(Decimal.modulo, 3);

    Decimal.set({ precision: 50, rounding: 7, toExpNeg: -20 });
    arktest.assertEQ(Decimal.precision, 50);
    arktest.assertEQ(Decimal.rounding, 7);
    arktest.assertEQ(Decimal.toExpNeg, -20);
    arktest.assertEQ(Decimal.toExpPos, 20);
    arktest.assertEQ(Decimal.minE, -1e9);
    arktest.assertEQ(Decimal.maxE, 1e9);
    arktest.assertEQ(Decimal.modulo, 3);
    arktest.assertFalse(Decimal.crypto);

    Decimal.set({ toExpPos: 21 });
    arktest.assertEQ(Decimal.precision, 50);
    arktest.assertEQ(Decimal.rounding, 7);
    arktest.assertEQ(Decimal.toExpNeg, -20);

    Decimal.set({ rounding: 3, defaults: true });
    arktest.assertEQ(Decimal.rounding, 3);
    arktest.assertEQ(Decimal.precision, 20);
    arktest.assertEQ(Decimal.toExpNeg, -7);
}

function testSetRetainsUnspecifiedProperties() {
    Decimal.set({ precision: 55, rounding: 5, toExpNeg: -10, modulo: 6 });

    arktest.assertEQ(Decimal.precision, 55);
    arktest.assertEQ(Decimal.rounding, 5);
    arktest.assertEQ(Decimal.toExpNeg, -10);
    arktest.assertEQ(Decimal.modulo, 6);

    Decimal.set({ rounding: 6, toExpNeg: -16, modulo: 8 });
    arktest.assertEQ(Decimal.precision, 55);
    arktest.assertEQ(Decimal.rounding, 6);
    arktest.assertEQ(Decimal.toExpNeg, -16);
    arktest.assertEQ(Decimal.modulo, 8);

    Decimal.set({ precision: 17 });
    arktest.assertEQ(Decimal.precision, 17);
    arktest.assertEQ(Decimal.rounding, 6);
    arktest.assertEQ(Decimal.toExpNeg, -16);
    arktest.assertEQ(Decimal.modulo, 8);
}
