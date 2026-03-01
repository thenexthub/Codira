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
    const suite = new arktest.ArkTestsuite("Decimal asinh API tests");
    suite.addTest("Decimal asinh with specific config", testAsinhWithSpecificConfigPart001);
    suite.addTest("Decimal asinh with specific config", testAsinhWithSpecificConfigPart002);
    return suite.run();
}

function checkAsinhResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).asinh().valueOf(), expected);
}

function testAsinhWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkAsinhResult('NaN', 40, 4, 'NaN');
    checkAsinhResult('Infinity', 40, 4, 'Infinity');
    checkAsinhResult('-Infinity', 40, 4, '-Infinity');
    checkAsinhResult('0', 40, 4, '0');
    checkAsinhResult('-0', 40, 4, '-0');

    checkAsinhResult('3.1444197343', 4, 3, '1.863');
    checkAsinhResult('-7546164.80347805747707434691023987685', 5, 2, '-16.529');
    checkAsinhResult('-0.15139', 9, 1, '-0.150817601');
    checkAsinhResult('7649', 8, 2, '9.6354774');
    checkAsinhResult('9522664768544403839474412593333.0574738', 8, 6, '72.024375');
    checkAsinhResult('7979240.25859873', 3, 3, '16.5');
    checkAsinhResult('268', 5, 2, '6.2842');
    checkAsinhResult('-0.036', 2, 5, '-0.036');
    checkAsinhResult('0.4807', 9, 3, '0.4638831');
}

function testAsinhWithSpecificConfigPart002() {
    checkAsinhResult('0.229', 10, 4, '0.2270443104');
    checkAsinhResult('2.00400000074005', 7, 0, '1.445423');
    checkAsinhResult('200000000000000.00004161', 6, 5, '33.6225');
    checkAsinhResult('-58888888888888791888.6', 3, 0, '-46.3');
    checkAsinhResult('-6031221', 3, 4, '-16.3');
    checkAsinhResult('-640.3942', 7, 5, '-7.155232');
    checkAsinhResult('14', 10, 4, '3.333477587');
    checkAsinhResult('4', 2, 0, '2.1');
    checkAsinhResult('0.03', 7, 0, '0.02999551');
    checkAsinhResult('90', 1, 4, '5');
    checkAsinhResult('-882.444444444144444444494444440464444444494940444444244444', 5, 4, '-7.4758');
    checkAsinhResult('-8', 13, 5, '-2.776472280724');
    checkAsinhResult('-86000000000000000000.000000000000000000000005', 70, 0, '-46.59402615070627535463115294058075590852999710911467932978308270954662');
    checkAsinhResult('-376.89', 48, 4, '-6.62510230823208343124289061224985909335679702244');
}
