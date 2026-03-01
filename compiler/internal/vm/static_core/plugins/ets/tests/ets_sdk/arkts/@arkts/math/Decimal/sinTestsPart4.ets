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
    checkSinResult('333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333.33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333335', 171, 3, '0.999861104212610844432827649084650490692662683912806434297051730891484017620891490814751729807506108594451118885277110368312661669230716203945478379798471281336247356157603');
}
