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
    checkTanResult('0.524217411165998018391262237830297491516856805040463896080592290487405064611025001607311512479709354535407130581948832593516281243820812174844723190311545869854968531247730387537943969800839367585396013641826800875467649274673709974876292276089324234102809573076731030535244688816', 76, 3, '0.5781754114340578960371795632804762026798421163023137415437904934165060267084');
}
