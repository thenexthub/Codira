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
    checkSinResult('22011131111011111.111111611113111111111151111111', 143, 2, '0.82582504036277799386306063085803210583251158969990606609364360685569588545519071481543672724620118406694191888115120286393881609546697317692404');
    checkSinResult('996270725099515169352424536636186062915113219400094989.8763797268889422850038402633796294758036260533902551191769915343780424028900449342752548782035', 46, 2, '0.6613706114081017074779805460666900787572253475');
    checkSinResult('0.780360750628373', 37, 5, '0.7035358359376557390803090830882458906');
}
