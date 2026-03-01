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
    const suite = new arktest.ArkTestsuite("Decimal isPositive API tests");
    suite.addTest("Decimal isPositive with default config", testIsPositiveWithDefaultConfig);
    suite.addTest("Decimal isPositive with random number", testIsPositiveWithRandomNumber);
    return suite.run();
}

function testIsPositiveWithDefaultConfig() {
    arktest.assertTrue(new Decimal(0).isPositive());
    arktest.assertTrue(new Decimal(Infinity).isPositive());
    arktest.assertTrue(new Decimal(0.5).isPositive());
    arktest.assertTrue(new Decimal(1).isPositive());
    arktest.assertFalse(new Decimal(NaN).isPositive());
    arktest.assertFalse(new Decimal(-Infinity).isPositive());
    arktest.assertFalse(new Decimal(-0.5).isPositive());
    arktest.assertFalse(new Decimal(-1).isPositive());
}

function randInt() {
    return Math.floor(Math.random() * 9e15 / Math.pow(10, Math.random() * 16 | 0));
}

function testIsPositiveWithRandomNumber() {
    for (let i = 0; i < 50; i++) {
        let ran = randInt();
        if (ran == 0) {
            arktest.assertTrue(new Decimal(ran).isPositive());
        } else if (ran > 0) {
            arktest.assertTrue(new Decimal(ran).isPositive());
            arktest.assertFalse(new Decimal(-ran).isPositive());
        } else {
            arktest.assertTrue(new Decimal(-ran).isPositive());
            arktest.assertFalse(new Decimal(ran).isPositive());
        }
    }
}
