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
    const suite = new arktest.ArkTestsuite("Decimal sign API tests");
    suite.addTest("Decimal sign with default config", testSignWithDefaultConfig);
    return suite.run();
}

function testSignWithDefaultConfig() {
    arktest.assertEQ(Decimal.sign(Infinity), 1);
    arktest.assertEQ(Decimal.sign(-Infinity), -1);
    arktest.assertEQ(Decimal.sign('Infinity'), 1);
    arktest.assertEQ(Decimal.sign('-Infinity'), -1);

    arktest.assertTrue(Number.isNaN(Decimal.sign(NaN)));
    arktest.assertTrue(Number.isNaN(Decimal.sign('NaN')));
    arktest.assertTrue(1 / Decimal.sign('0') === Infinity);
    arktest.assertTrue(1 / Decimal.sign(new Decimal('0')) === Infinity);
    arktest.assertTrue(1 / Decimal.sign('-0') === -Infinity);
    arktest.assertTrue(1 / Decimal.sign(new Decimal('-0')) === -Infinity);

    arktest.assertEQ(Decimal.sign('0'), 0);
    arktest.assertEQ(Decimal.sign('-0'), -0);
    arktest.assertEQ(Decimal.sign('1'), 1);
    arktest.assertEQ(Decimal.sign('-1'), -1);
    arktest.assertEQ(Decimal.sign('9.99'), 1);
    arktest.assertEQ(Decimal.sign('-9.99'), -1);
}
