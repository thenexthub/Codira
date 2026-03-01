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

let expected: Decimal;

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal sum API tests");
    suite.addTest("Decimal sum with default config that the sum is 0", testSumExpectedZero);
    suite.addTest("Decimal sum with default config that the sum is 10", testSumExpectedTen);
    suite.addTest("Decimal sum with default config that the sum is 600", testSumExpectedSixHundred);
    suite.addTest("Decimal sum with default config that the sum is NaN", testSumExpectedNaN);
    suite.addTest("Decimal sum with default config that the sum is Infinity", testSumExpectedInfinity);
    suite.addTest("Decimal sum with default config that the sum is -Infinity", testSumExpectedNegativeInfinity);
    return suite.run();
}

function testSumExpectedZero() {
    expected = new Decimal(0);
    arktest.assertEQ(Decimal.sum('0').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum('0', new Decimal(0)).valueOf(), expected.valueOf());
}

function testSumExpectedTen() {
    expected = new Decimal(10);
    arktest.assertEQ(Decimal.sum('10').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum('0', new Decimal('10')).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(10, 0).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(0, 0, 0, 0, 0, 0, 10).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(11, -1).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(1, '2', new Decimal(3), new Decimal('4')).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum('9', new Decimal(0.01), 0.99).valueOf(), expected.valueOf());
}

function testSumExpectedSixHundred() {
    expected = new Decimal(600);
    arktest.assertEQ(Decimal.sum(100, 200, 300).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum('100', '200', '300').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(new Decimal(100), new Decimal(200), new Decimal(300)).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(100, '200', new Decimal(300)).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(99.9, 200.05, 300.05).valueOf(), expected.valueOf());
}

function testSumExpectedNaN() {
    expected = new Decimal(NaN);
    arktest.assertEQ(Decimal.sum(NaN).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum('1', NaN).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(100, 200, NaN).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(NaN, 0, '9', new Decimal(0), 11, Infinity).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(0, new Decimal('-Infinity'), '9', new Decimal(NaN), 11).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(4, '-Infinity', 0, '9', new Decimal(0), Infinity, 2).valueOf(), expected.valueOf());
}

function testSumExpectedInfinity() {
    expected = new Decimal(Infinity);
    arktest.assertEQ(Decimal.sum(Infinity).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(1, '1e10000000000000000000000000000000000000000', '4').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(100, 200, 'Infinity').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(0, new Decimal('Infinity'), '9', new Decimal(0), 11).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(0, '9', new Decimal(0), 11, Infinity).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(4, new Decimal(Infinity), 0, '9', new Decimal(0), Infinity, 2).valueOf(), expected.valueOf());
}

function testSumExpectedNegativeInfinity() {
    expected = new Decimal(-Infinity);
    arktest.assertEQ(Decimal.sum(-Infinity).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(1, '-1e10000000000000000000000000000000000000000', '4').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(100, 200, '-Infinity').valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(0, new Decimal('-Infinity'), '9', new Decimal(0), 11).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(0, '9', new Decimal(0), 11, -Infinity).valueOf(), expected.valueOf());
    arktest.assertEQ(Decimal.sum(4, new Decimal(-Infinity), 0, '9', new Decimal(0), -Infinity, 2).valueOf(), expected.valueOf());
}
