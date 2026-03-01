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
    suite.addTest("Decimal cbrt with default config", testCbrtAndStaticCbrt);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig001);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig002);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig003);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig004);
    return suite.run();
}

function checkCbrtResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).cbrt().valueOf(), expected);
}

function testCbrtAndStaticCbrt() {
    arktest.assertEQ(new Decimal(0).cbrt().valueOf(), Decimal.cbrt(0).valueOf());
    arktest.assertEQ(new Decimal(NaN).cbrt().valueOf(), Decimal.cbrt(NaN).valueOf());
    arktest.assertEQ(new Decimal(Infinity).cbrt().valueOf(), Decimal.cbrt(Infinity).valueOf());
    arktest.assertEQ(new Decimal(-Infinity).cbrt().valueOf(), Decimal.cbrt(-Infinity).valueOf());
    arktest.assertEQ(new Decimal(0.5).cbrt().valueOf(), Decimal.cbrt(0.5).valueOf());
    arktest.assertEQ(new Decimal(-0.5).cbrt().valueOf(), Decimal.cbrt(-0.5).valueOf());
    arktest.assertEQ(new Decimal(1).cbrt().valueOf(), Decimal.cbrt(1).valueOf());
    arktest.assertEQ(new Decimal(-1).cbrt().valueOf(), Decimal.cbrt(-1).valueOf());
}

function testCbrtWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkCbrtResult('NaN', 40, 4, 'NaN');
    checkCbrtResult('0', 40, 4, '0');
    checkCbrtResult('-0', 40, 4, '-0');
    checkCbrtResult('Infinity', 40, 4, 'Infinity');
    checkCbrtResult('-Infinity', 40, 4, '-Infinity');
    checkCbrtResult('1', 40, 4, '1');
    checkCbrtResult('-1', 40, 4, '-1');
}

function testCbrtWithSpecificConfig002() {
    checkCbrtResult('8', 40, 4, '2');
    checkCbrtResult('-8', 40, 4, '-2');
    checkCbrtResult('-4', 5, 2, '-1.5874');
    checkCbrtResult('-0.8667', 7, 0, '-0.9534318');
    checkCbrtResult('-0.0000000071', 9, 2, '-0.00192199734');
    checkCbrtResult('-21791364.26687703933853824404550506158463693222', 3, 1, '-279');
    checkCbrtResult('36', 9, 6, '3.30192725');
}

function testCbrtWithSpecificConfig003() {
    checkCbrtResult('0.898', 7, 0, '0.9647737');
    checkCbrtResult('-94444444444444444.444444444444444444444444444444444444444444414404441444441', 8, 2, '-455399.06');
    checkCbrtResult('-3782929120857330517139808', 5, 2, '-155810000');
    checkCbrtResult('966666666692107298099458326175656566655348300467903965479870253722013788135743211111111111111111111.22222', 1, 5, '1000000000000000000000000000000000');
    checkCbrtResult('16.84', 7, 4, '2.563189');
    checkCbrtResult('-58918', 2, 3, '-39');
}

function testCbrtWithSpecificConfig004() {
    checkCbrtResult('-74535634133038239268820777237286', 2, 4, '-42000000000');
    checkCbrtResult('6888888888.66666666666602248', 3, 5, '1900');
    checkCbrtResult('608785700000000000000000100000000000000000000.000000500020900040006', 1, 5, '800000000000000');
    checkCbrtResult('-3834369', 8, 0, '-156.51813');
    checkCbrtResult('4345752', 8, 0, '163.18804');
    checkCbrtResult('5000300.5', 5, 5, '171');
    checkCbrtResult('0.73', 7, 3, '0.9004113');
}
