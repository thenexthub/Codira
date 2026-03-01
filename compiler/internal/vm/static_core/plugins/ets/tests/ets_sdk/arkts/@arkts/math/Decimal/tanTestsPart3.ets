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
    const suite = new arktest.ArkTestsuite("Decimal tan API tests");
    suite.addTest("Decimal tan with specific config", testTanWithSpecificConfig);
    return suite.run();
}

function checkTanResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).tan().valueOf(), expected);
}

function testTanWithSpecificConfig() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkTanResult('1147231', 8, 4, '-0.097582634');
    checkTanResult('0.000553763344', 27, 4, '0.000553763400604559119913346525');
    checkTanResult('68', 24, 1, '-2.04008159801594615389349');
    checkTanResult('97415551555.55555655655655554555555855555', 38, 6, '0.63497857004739965579501336949790595112');
    checkTanResult('3052744161971196989887333333333333333', 15, 0, '2.3893665412711');
    checkTanResult('20083.4596122', 37, 0, '-0.9171727446291034528773430723329592848');
    checkTanResult('9200', 30, 3, '6.43843719089952501300840146253');
    checkTanResult('323646192595.4845', 127, 6, '-0.3747612268270038347175054875779625030989108207803221587812055629899198693615407534041949990189331417585285760850333254670480789');
}
