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
    return suite.run();
}

function checkLogResult(n: string, base: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLogResult('128', '2', '7', 1, 0);
    checkLogResult('256', '2', '8', 1, 2);
    checkLogResult('1024', '2', '10', 2, 0);
    checkLogResult('1024', '2', '10', 10, 0);
    checkLogResult('16384', '2', '14', 2, 0);
    checkLogResult('16384', '2', '14', 10, 0);
    checkLogResult('243', '3', '5', 7, 4);
    checkLogResult('243', '9', '2.5', 7, 4);
    checkLogResult('243', '3', '5', 7, 4);
    checkLogResult('243', '9', '2.5', 7, 4);
    checkLogResult('16', '2', '4', 7, 4);
    checkLogResult('32', '4', '2.5', 7, 4);
    checkLogResult('16', '2', '4', 7, 4);
    checkLogResult('32', '4', '2.5', 7, 4);
}

function testLogWithSpecificConfigPart002() {
    checkLogResult('1.2589254117', 10, '0.1', 1, 2);
    checkLogResult('1.023292992', 10, '0.01', 1, 2);
    checkLogResult('1.258925411794167210423954106395800606', 10, '0.1', 1, 4);
    checkLogResult('1.25892541179416721', 10, '0.1', 1, 0);
    checkLogResult('1.258925411', 10, '0.1', 1, 5);
    checkLogResult('1.258925411794167210423954', 10, '0.1', 1, 4);
    checkLogResult('839756321.64088511', '28', '6.16667503', 9, 0);
    checkLogResult('576306512.96177', '985212731.27158', '0.9742', 4, 2);
}

function testLogWithSpecificConfigPart003() {
    checkLogResult('97.65625', '6.25', '2.5', 3, 0);
    checkLogResult('223677472.0384754303304727631735', '26', '5.900904252190486450814', 22, 5);
    checkLogResult('2063000845.3020737243910803079', '35', '6.0324410183892767982149330415', 29, 0);
    checkLogResult('302381977.956021650184952836276441035025875682714942', '2623', '2.4805663226398755020289647999', 29, 6);
    checkLogResult('456870693.58', '238920772.66', '1.0336035877093034523', 21, 4);
    checkLogResult('16', '2', '4', 10, 4);
    checkLogResult('32', '4', '2.5', 10, 1);
    checkLogResult('316.2277660168379331998893544432645719585553021316022642247511089459022980600999502961482777894980004', '10', '2.49999999999999999999', 21, 1);
}
