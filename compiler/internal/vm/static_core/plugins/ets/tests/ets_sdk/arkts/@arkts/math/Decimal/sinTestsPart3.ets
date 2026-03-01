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
    const suite = new arktest.ArkTestsuite("Decimal sin API tests");
    suite.addTest("Decimal sin with specific config", testSinWithSpecificConfig);
    return suite.run();
}

function checkSinResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).sin().valueOf(), expected);
}

function testSinWithSpecificConfig() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkSinResult('8719985534823285.8450271997', 5, 4, '-0.43915');
    checkSinResult('0.9528405', 9, 0, '0.815064492');
    checkSinResult('3007.440843', 1, 3, '-0.9');
    checkSinResult('8821000', 9, 3, '-0.999820738');
    checkSinResult('0.7244', 2, 5, '0.66');
}
