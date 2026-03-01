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

function main() {
    const suite = new arktest.ArkTestsuite("Decimal equals API tests");
    suite.addTest("Decimal equals with default config", testEqualsWithDefaultConfig001);
    suite.addTest("Decimal equals with default config", testEqualsWithDefaultConfig002);
    return suite.run();
}

function testEqualsWithDefaultConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertTrue(new Decimal(1).equals(new Decimal(1)));
    arktest.assertTrue(new Decimal(1).equals(1));
    arktest.assertTrue(new Decimal(1).equals('1.0'));
    arktest.assertTrue(new Decimal(1).equals('1.00'));
    arktest.assertTrue(new Decimal(1).equals('1.000'));
    arktest.assertTrue(new Decimal(1).equals('1.0000'));
    arktest.assertTrue(new Decimal(1).equals('1.00000'));
    arktest.assertTrue(new Decimal(1).equals('1.000000'));
    arktest.assertTrue(new Decimal(1).equals(new Decimal(1)));
    arktest.assertTrue(new Decimal(1).equals('0x1'));
    arktest.assertTrue(new Decimal(1).equals('0o1'));
    arktest.assertTrue(new Decimal(1).equals('0b1'));
    arktest.assertTrue(!new Decimal('-0.1').equals(0.1));
    arktest.assertTrue(new Decimal(Infinity).equals('Infinity'));
    arktest.assertTrue(new Decimal(Infinity).equals(Infinity));
    arktest.assertTrue(!new Decimal('-Infinity').equals(Infinity));
    arktest.assertTrue(new Decimal('-Infinity').equals(-Infinity));
    arktest.assertTrue(new Decimal('0.0000000').equals(-0));
    arktest.assertTrue(new Decimal(-0).equals('0.000'));
    arktest.assertTrue(!new Decimal('NaN').equals(NaN));
}

function testEqualsWithDefaultConfig002() {
    arktest.assertTrue(!new Decimal('NaN').equals(Infinity));
    arktest.assertTrue(new Decimal('-1.234e+2').equals(-123.4));
    arktest.assertTrue(new Decimal('5e-200').equals(5e-200));
    arktest.assertTrue(new Decimal('1').equals(new Decimal('1')));
    arktest.assertTrue(new Decimal('1').equals(new Decimal('1').toString()));
    arktest.assertTrue(new Decimal('1').equals(new Decimal('1').toString()));
    arktest.assertTrue(new Decimal('1').equals(new Decimal('1').valueOf()));
    arktest.assertTrue(new Decimal('1').equals(1));
    arktest.assertTrue(new Decimal('1').equals('1e+0'));
    arktest.assertTrue(!new Decimal('1').equals(-1));
    arktest.assertTrue(!new Decimal('1').equals(0.1));
    arktest.assertTrue(!new Decimal(NaN).equals(0));
    arktest.assertTrue(!new Decimal(Infinity).equals(0));
    arktest.assertTrue(!new Decimal(0.1).equals(0));
    arktest.assertTrue(!new Decimal(1e9 + 1).equals(1e9));
    arktest.assertTrue(!new Decimal(1e9 - 1).equals(1e9));
    arktest.assertTrue(new Decimal(1e9 + 1).equals(1e9 + 1));
    arktest.assertTrue(new Decimal(1).equals(1));
    arktest.assertTrue(!new Decimal(1).equals(-1));
    arktest.assertTrue(!new Decimal(NaN).equals(NaN));
    arktest.assertTrue(!new Decimal('NaN').equals('NaN'));
}
