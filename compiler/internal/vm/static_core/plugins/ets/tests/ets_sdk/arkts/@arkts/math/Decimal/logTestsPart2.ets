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
    checkLogResult('2464.348361986885121671329250344224', 10, '3.3917021', 18, 1);
    checkLogResult('2464.348361986885121671329250344224', 10, '3.39170210000000001', 18, 0);
    checkLogResult('10000.0000000000004', 10, '4.01', 3, 2);
    checkLogResult('10000.00000000004', 10, '4.01', 3, 2);
    checkLogResult('100.0000000000001', 10, '2.1', 2, 0);
    checkLogResult('99999.99999999999991', 10, '4.999', 4, 1);
    checkLogResult('99999.9999999999991', 10, '4.999', 4, 1);
    checkLogResult('99999.99999991', 10, '4.999', 4, 1);

    checkLogResult('6.626757835589191227753975149737456562020794782', 10, '0.8213011002743699999999999999999999999999999', 43, 1);
    checkLogResult('4.20732041199815040736678139715312481859825562145776045079', 10, '0.6240055873352599999999999999999999999999999999999999', 52, 3);
    checkLogResult('64513410281785809574142282220919135969.8537876292904158501590880', 10, '37.80964999999999999999', 22, 1);
    checkLogResult('33.51145738694771448172942314968136067036971739113975569076629', 10, '1.5251933153717162999999999999999999999999999999999999999', 56, 3);
    checkLogResult('10232.9299228075413096627', 10, '4.009999999999999', 16, 1);
    checkLogResult('1.258925411794167210423954106395', 10, '0.099999999999999999999999999999723814', 35, 0);
    checkLogResult('1.29891281037500', 10, '0.11357', 5, 1);
    checkLogResult('16.399137225681149762104868844', 10, '1.21482099999999999999999', 24, 3);
    checkLogResult('0.01', 10, '-2', 17, 3);
    checkLogResult('0.0000000001', 10, '-10', 4, 2);
    checkLogResult('0.00001', 10, '-5', 35, 3);
    checkLogResult('0.00000001', 10, '-8', 24, 2);
    checkLogResult('0.0000100000000000010000005060000000000800030000000400908', 10, '-4.99', 3, 1);
}

function testLogWithSpecificConfigPart002() {
    checkLogResult('94143178827', '3486784401', '1.15', 3, 0);
    checkLogResult('15625', '3125', '1.2', 2, 3);
    checkLogResult('3', '3486784401', '0.05', 1, 8);
    checkLogResult('268435456', '1048576', '1.4', 2, 3);
    checkLogResult('25', '9765625', '0.2', 1, 7);
    checkLogResult('524288', '256', '2.375', 4, 8);
    checkLogResult('177147', '81', '2.75', 3, 5);
    checkLogResult('531441', '59049', '1.2', 2, 8);
    checkLogResult('387420489', '59049', '1.8', 2, 6);
    checkLogResult('16384', '65536', '0.875', 3, 6);
    checkLogResult('31381059609', '59049', '2.2', 2, 5);
    checkLogResult('8589934592', '65536', '2.0625', 5, 3);
    checkLogResult('33554432', '256', '3.125', 4, 3);
    checkLogResult('4503599627370496', '65536', '3.25', 3, 3);
    checkLogResult('68630377364883', '59049', '2.9', 2, 3);
    checkLogResult('68630377364883', '847288609443', '1.16', 3, 5);
    checkLogResult('16', '1125899906842624', '0.08', 1, 2);
    checkLogResult('3814697265625', '390625', '2.25', 3, 8);
    checkLogResult('8', '4294967296', '0.09375', 4, 1);
}
