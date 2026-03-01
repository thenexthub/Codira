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
    return suite.run();
}

function checkLogResult(n: string, base: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLogResult('10000.0000000000000004', 10, '4.01', 3, 2);
    checkLogResult('10000.00000000000004', 10, '4.01', 3, 2);
    checkLogResult('100.000000000000000000001', 10, '2.1', 2, 0);
    checkLogResult('100.00000000000001', 10, '2.1', 2, 0);
    checkLogResult('99999.9999999999999991', 10, '4.999', 4, 1);
    checkLogResult('124050.923004222533485495840', 10, '5.093601', 7, 2);
    checkLogResult('1.258925411794167210423954106395', 10, '0.09999', 4, 1);
    checkLogResult('911191437.48166728043529900000', 10, '8.959609629999999999999999', 25, 3);
    checkLogResult('253.4396732554691740503010363220', 10, '2.403874600001', 13, 2);
}

function testLogWithSpecificConfigPart002() {
    checkLogResult('31809.09321', 2, '14.95715162', 10, 3);
    checkLogResult('21088.306138691278', 2, '14.3641556', 9, 4);
    checkLogResult('21417.99322', 2, '14.386535691235055367', 20, 4);
    checkLogResult('30749.008158228314845157', 2, '14.9', 3, 3);
    checkLogResult('11701.5', 2, '13.51440585840535244680127', 25, 0);
    checkLogResult('31737.6741', 2, '14.954', 5, 2);
    checkLogResult('1688.88816886', 2, '10.7218580867075137099751634', 27, 3);
    checkLogResult('31553.4', 2, '14.945507849063278420302384', 26, 1);
    checkLogResult('28215.19', 2, '14.7841844442', 12, 3);
    checkLogResult('6080.97', 2, '12.57008575', 10, 1);
    checkLogResult('575.881932366571406', 2, '9.16962924962079798', 18, 1);
}
