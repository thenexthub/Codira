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
    const suite = new arktest.ArkTestsuite("Decimal greaterThan API tests");
    suite.addTest("Decimal greaterThan with default config", testGreaterThanWithDefaultConfig);
    return suite.run();
}

function testGreaterThanWithDefaultConfig() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    arktest.assertTrue(new Decimal(1).greaterThan(0.99999));
    arktest.assertTrue(!new Decimal('-0.1').greaterThan(-0.1));
    arktest.assertTrue(new Decimal(Infinity).greaterThan('9e999'));
    arktest.assertTrue(!new Decimal('-Infinity').greaterThan(-Infinity));
    arktest.assertTrue(new Decimal('0.0000000').greaterThan(-0.000001));
    arktest.assertTrue(new Decimal(-0).greaterThan(-1));
    arktest.assertTrue(!new Decimal('NaN').greaterThan(0));
    arktest.assertTrue(new Decimal('-1.234e+2').greaterThan('-0xff'));
    arktest.assertTrue(new Decimal('5e-200').greaterThan(5e-201));
    arktest.assertTrue(!new Decimal(NaN).greaterThan(NaN));
    arktest.assertTrue(!new Decimal(10).greaterThan(10));
    arktest.assertTrue(!new Decimal(2).greaterThan(10));
    arktest.assertTrue(new Decimal(1.23001e-2).greaterThan(1.23e-2));
    arktest.assertTrue(new Decimal(1e-2).greaterThan(9.999999e-3));
    arktest.assertTrue(new Decimal(1.23001e+2).greaterThan(1.23e+2));
    arktest.assertTrue(!new Decimal(9.999999e+2).greaterThan(1e+3));
    arktest.assertTrue(!new Decimal(1.23e-2).greaterThan(1.23001e-2));
    arktest.assertTrue(!new Decimal(9.999999e-3).greaterThan(1e-2));
    arktest.assertTrue(!new Decimal(1.23e+2).greaterThan(1.23001e+2));
    arktest.assertTrue(new Decimal(1e+3).greaterThan(9.9999999e+2));
}
