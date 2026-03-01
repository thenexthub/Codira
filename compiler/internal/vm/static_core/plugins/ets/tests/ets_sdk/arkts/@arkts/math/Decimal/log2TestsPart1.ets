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
    const suite = new arktest.ArkTestsuite("Decimal log2 API tests");
    suite.addTest("Decimal log2 with specific config", testLog2WithSpecificConfig);
    suite.addTest("Decimal log2 with for", testLog2WithFor);
    return suite.run();
}

function checkLog2Result(n: string, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(2).valueOf(), expected);
}

function testLog2WithSpecificConfig() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLog2Result('NaN', 'NaN', 40, 4);
    checkLog2Result('-1e-234', 'NaN', 40, 4);
    checkLog2Result('0', '-Infinity', 40, 4);
    checkLog2Result('-0', '-Infinity', 40, 4);
    checkLog2Result('1', '0', 40, 4);
    checkLog2Result('Infinity', 'Infinity', 40, 4);
}

function testLog2WithFor() {
    for (let i = 0; i < 30; i++) {
        checkLog2Result(String(Math.pow(2, i)), String(i), 40, 4);
    }
}
