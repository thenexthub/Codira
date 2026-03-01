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
    const suite = new arktest.ArkTestsuite("Decimal greaterThanOrEqualTo API tests");
    suite.addTest("Decimal greaterThanOrEqualTo with default config", testGreaterThanOrEqualToWithDefaultConfig001);
    suite.addTest("Decimal greaterThanOrEqualTo with default config", testGreaterThanOrEqualToWithDefaultConfig002);
    return suite.run();
}

function testGreaterThanOrEqualToWithDefaultConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertTrue(!new Decimal(1).greaterThanOrEqualTo(1.1));
    arktest.assertTrue(new Decimal('-0.1').greaterThanOrEqualTo(-1));
    arktest.assertTrue(new Decimal(Infinity).greaterThanOrEqualTo(Infinity));
    arktest.assertTrue(!new Decimal('-Infinity').greaterThanOrEqualTo(8));
    arktest.assertTrue(!new Decimal('0.0000000').greaterThanOrEqualTo(0.1));
    arktest.assertTrue(!new Decimal(-0).greaterThanOrEqualTo(0.1));
    arktest.assertTrue(!new Decimal('NaN').greaterThanOrEqualTo(0));
    arktest.assertTrue(new Decimal('-1.234e+2').greaterThanOrEqualTo('-1.234e+3'));
    arktest.assertTrue(!new Decimal('5e-200').greaterThanOrEqualTo(1));
    arktest.assertTrue(new Decimal('0xb').greaterThanOrEqualTo('0x9'));
    arktest.assertTrue(!new Decimal(NaN).greaterThanOrEqualTo(NaN));
    arktest.assertTrue(new Decimal(Infinity).greaterThanOrEqualTo(Infinity));
    arktest.assertTrue(new Decimal(Infinity).greaterThanOrEqualTo(-Infinity));
    arktest.assertTrue(!new Decimal(NaN).greaterThanOrEqualTo(-Infinity));
}

function testGreaterThanOrEqualToWithDefaultConfig002() {
    arktest.assertTrue(new Decimal(-Infinity).greaterThanOrEqualTo(-Infinity));
    arktest.assertTrue(new Decimal('0xa').greaterThanOrEqualTo('0x9'));
    arktest.assertTrue(!new Decimal(0).greaterThanOrEqualTo(NaN));
    arktest.assertTrue(!new Decimal(NaN).greaterThanOrEqualTo(NaN));
    arktest.assertTrue(new Decimal(0).greaterThanOrEqualTo(-Infinity));
    arktest.assertTrue(!new Decimal(0).greaterThanOrEqualTo('Infinity'));
    arktest.assertTrue(!new Decimal(10).greaterThanOrEqualTo(20));
    arktest.assertTrue(new Decimal(1.23001e-2).greaterThanOrEqualTo(1.23e-2));
    arktest.assertTrue(new Decimal(1e-2).greaterThanOrEqualTo(9.999999e-3));
    arktest.assertTrue(new Decimal(1.23001e+2).greaterThanOrEqualTo(1.23e+2));
    arktest.assertTrue(!new Decimal(9.999999e+2).greaterThanOrEqualTo(1e+3));
    arktest.assertTrue(!new Decimal(1.23e-2).greaterThanOrEqualTo(1.23001e-2));
    arktest.assertTrue(!new Decimal(9.999999e-3).greaterThanOrEqualTo(1e-2));
    arktest.assertTrue(!new Decimal(1.23e+2).greaterThanOrEqualTo(1.23001e+2));
    arktest.assertTrue(new Decimal(1e+3).greaterThanOrEqualTo(9.9999999e+2));
}
