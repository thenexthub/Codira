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
    const suite = new arktest.ArkTestsuite("Decimal log API tests");
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart001);
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart002);
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart003);
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart004);
    return suite.run();
}

function checkLogResult(n: string, base: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLogResult('0', '10', '-Infinity', 40, 4);
    checkLogResult('-0', '10', '-Infinity', 40, 4);
    checkLogResult('1', '10', '0', 40, 4);
    checkLogResult('-1', '10', 'NaN', 40, 4);
    checkLogResult('Infinity', '10', 'Infinity', 40, 4);
    checkLogResult('Infinity', '0', 'NaN', 40, 4);
    checkLogResult('-Infinity', 'Infinity', 'NaN', 40, 4);
    checkLogResult('NaN', '10', 'NaN', 40, 4);
}

function testLogWithSpecificConfigPart002() {
    checkLogResult('1', '0', 'NaN', 40, 4);
    checkLogResult('10', '0', 'NaN', 40, 4);
    checkLogResult('10', '-0', 'NaN', 40, 4);
    checkLogResult('10', '1', 'NaN', 40, 4);
    checkLogResult('10', '-1', 'NaN', 40, 4);
    checkLogResult('10', 'Infinity', 'NaN', 40, 4);
    checkLogResult('10', '-Infinity', 'NaN', 40, 4);
    checkLogResult('10', 'NaN', 'NaN', 40, 4);
    checkLogResult('-1', '-1', 'NaN', 40, 4);
    checkLogResult('0', '0', 'NaN', 40, 4);
}

function testLogWithSpecificConfigPart003() {
    checkLogResult('7625597484987', '59049', '2.7', 2, 5);
    checkLogResult('839756321.64088511', '28', '6.16667503', 9, 0);
    checkLogResult('94143178827', '3486784401', '1.15', 3, 0);
    checkLogResult('243', '9', '3', 1, 4);
    checkLogResult('512', '16', '2.25', 7, 0);
    checkLogResult('512', '16', '2.25', 7, 2);
    checkLogResult('512', '16', '2.2', 2, 5);
    checkLogResult('512', '16', '2.2', 2, 6);
}

function testLogWithSpecificConfigPart004() {
    checkLogResult('16', '2', '4', 7, 0);
    checkLogResult('16', '2', '4', 7, 2);
    checkLogResult('243', '3', '5', 7, 1);
    checkLogResult('243', '9', '2.5', 7, 1);
    checkLogResult('243', '3', '5', 7, 3);
    checkLogResult('243', '9', '2.5', 7, 3);
    checkLogResult('32', '4', '2.5', 7, 0);
    checkLogResult('32', '4', '2.5', 7, 2);
    checkLogResult('4', '2', '2', 1, 2);
    checkLogResult('8', '2', '3', 2, 0);
    checkLogResult('16', '2', '4', 2, 0);
    checkLogResult('32', '2', '5', 3, 0);
    checkLogResult('64', '2', '6', 3, 0);
    checkLogResult('64', '2', '6', 2, 0);
    checkLogResult('64', '2', '6', 1, 2);
}
