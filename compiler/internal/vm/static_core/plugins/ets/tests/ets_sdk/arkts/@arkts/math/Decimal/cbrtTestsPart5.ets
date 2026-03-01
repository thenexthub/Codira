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
    const suite = new arktest.ArkTestsuite("Decimal cbrt API tests");
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig001);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig002);
    return suite.run();
}

function checkCbrtResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).cbrt().valueOf(), expected);
}

function testCbrtWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkCbrtResult('0.612', 20, 3, '0.84901847487755466985');
    checkCbrtResult('-26403122.886392512193592', 9, 3, '-297.772851');
    checkCbrtResult('-15047905.6442424463707664865226022703526330746497470178339129', 10, 2, '-246.8834739');
    checkCbrtResult('-0.57216409476227961', 122, 4, '-0.83018242216788557900092187018458431268949382089457981268783675303907114542359515145805957769873268594828033331460924645988');
}

function testCbrtWithSpecificConfig002() {
    checkCbrtResult('-58892300900.0000037', 72, 3, '-3890.62620353612987584901759020572729952315745798457018660549639374283766');
    checkCbrtResult('4312692950776.434730495181946194732352621783', 134, 0, '16277.3179326290103123151730291347428666068760542903239801418389563715424057253795836596796134792716104557634024961949417454938847683');
    checkCbrtResult('0.772144893677647945980591', 32, 4, '0.91741591091195652167447475342283');
    checkCbrtResult('-6404156859301838811126951723297929248851328010128081790499875', 82, 4, '-185703741377679767357.3800976029196648819860800202650959861930855922060702625367479');
}
