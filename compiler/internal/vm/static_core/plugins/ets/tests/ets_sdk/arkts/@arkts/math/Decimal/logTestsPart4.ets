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
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfig);
    suite.addTest("Decimal log and static log", testLogAndStaticLog);
    return suite.run();
}

function checkLogResult(n: string, base: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfig() {
    checkLogResult('4573.55560689675', 2, '12.1591004766309775332', 21, 1);
    checkLogResult('24202.85989198517539', 2, '15', 2, 4);
    checkLogResult('18334.9', 2, '14.16230477704721387108079127958', 31, 4);
    checkLogResult('20179.623017', 2, '14.4', 3, 0);
    checkLogResult('8607.97004888426002071', 2, '13.07145734276093159769689946319', 31, 1);
    checkLogResult('27231.463745', 2, '14.732986911725376996874804951679', 32, 3);
    checkLogResult('24325.08', 2, '14.57015693', 10, 0);
    checkLogResult('826.3541073', 2, '9.69', 3, 3);
    checkLogResult('6877.51851488', 2, '12.7476724030697', 15, 3);
    checkLogResult('13510.031', 2, '13.7217433646123774736072103937', 30, 4);
    checkLogResult('559.1647711259', 2, '9.12712965971023632', 18, 1);
    checkLogResult('1262.018796786493279', 2, '10.30151768', 10, 3);
    checkLogResult('31897.9889', 2, '14.9611778475691091525075', 24, 1);
    checkLogResult('24187.818942357666924548', 2, '14.561', 5, 3);
    checkLogResult('7233.846688339241', 2, '12.820547306996872048936910678432', 32, 3);
    checkLogResult('10162.3041', 2, '13.31093992111', 13, 4);
    checkLogResult('9091.859714971663525', 2, '13.1503597085807068872335', 24, 1);
    checkLogResult('16205.492', 2, '13.984195201', 11, 3);
    checkLogResult('17578.3501161869916711', 2, '14.101512046680555', 18, 3);
    checkLogResult('5661.58', 2, '12.466989012642603919950322048', 29, 1);
}

function randInt() {
    return Math.floor(Math.random() * 9e15 / Math.pow(10, Math.random() * 16 | 0));
}

function testLogAndStaticLog() {
    let t1 = Decimal.random();
    let t2 = t1.negate();
    let t3 = randInt();
    let t4 = -t3;
    let t5 = Decimal.random().add(randInt());
    let t6 = t5.negate();

    arktest.assertEQ(new Decimal(0).log(10).valueOf(), Decimal.log(0, 10).valueOf());
    arktest.assertEQ(new Decimal(NaN).log(10).valueOf(), Decimal.log(NaN, 10).valueOf());
    arktest.assertEQ(new Decimal(Infinity).log(10).valueOf(), Decimal.log(Infinity, 10).valueOf());
    arktest.assertEQ(new Decimal(-Infinity).log(10).valueOf(), Decimal.log(-Infinity, 10).valueOf());
    arktest.assertEQ(new Decimal(0.5).log(10).valueOf(), Decimal.log(0.5, 10).valueOf());
    arktest.assertEQ(new Decimal(-0.5).log(10).valueOf(), Decimal.log(-0.5, 10).valueOf());
    arktest.assertEQ(new Decimal(1).log(10).valueOf(), Decimal.log(1, 10).valueOf());
    arktest.assertEQ(new Decimal(-1).log(10).valueOf(), Decimal.log(-1, 10).valueOf());
    arktest.assertEQ(new Decimal(t1).log(10).valueOf(), Decimal.log(t1, 10).valueOf());
    arktest.assertEQ(new Decimal(t2).log(10).valueOf(), Decimal.log(t2, 10).valueOf());
    arktest.assertEQ(new Decimal(t3).log(10).valueOf(), Decimal.log(t3, 10).valueOf());
    arktest.assertEQ(new Decimal(t4).log(10).valueOf(), Decimal.log(t4, 10).valueOf());
    arktest.assertEQ(new Decimal(t5).log(10).valueOf(), Decimal.log(t5, 10).valueOf());
    arktest.assertEQ(new Decimal(t6).log(10).valueOf(), Decimal.log(t6, 10).valueOf());
}
