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

function main() {
    const suite = new arktest.ArkTestsuite("Decimal valueOf API tests");
    suite.addTest("Decimal valueOf with specific config", testValueOfWithSpecificConfig001);
    suite.addTest("Decimal valueOf with specific config", testValueOfWithSpecificConfig002);
    suite.addTest("Decimal valueOf with specific config", testValueOfWithSpecificConfig003);
    return suite.run();
}

function testValueOfWithSpecificConfig001() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(0).valueOf(), '0');
    arktest.assertEQ(new Decimal('0').valueOf(), '0');
    arktest.assertEQ(new Decimal(NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('Infinity').valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(1).valueOf(), '1');
    arktest.assertEQ(new Decimal(9).valueOf(), '9');
    arktest.assertEQ(new Decimal(90).valueOf(), '90');
    arktest.assertEQ(new Decimal(90.12).valueOf(), '90.12');
    arktest.assertEQ(new Decimal(0.1).valueOf(), '0.1');
    arktest.assertEQ(new Decimal(0.01).valueOf(), '0.01');
    arktest.assertEQ(new Decimal(0.0123).valueOf(), '0.0123');
    arktest.assertEQ(new Decimal('111111111111111111111').valueOf(), '111111111111111111111');
    arktest.assertEQ(new Decimal(0.00001).valueOf(), '0.00001');
}

function testValueOfWithSpecificConfig002() {
    arktest.assertEQ(new Decimal('-0').valueOf(), '-0');
    arktest.assertEQ(new Decimal('-Infinity').valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(-1).valueOf(), '-1');
    arktest.assertEQ(new Decimal(-9).valueOf(), '-9');
    arktest.assertEQ(new Decimal(-90).valueOf(), '-90');
    arktest.assertEQ(new Decimal(-90.12).valueOf(), '-90.12');
    arktest.assertEQ(new Decimal(-0.1).valueOf(), '-0.1');
    arktest.assertEQ(new Decimal(-0.01).valueOf(), '-0.01');
    arktest.assertEQ(new Decimal(-0.0123).valueOf(), '-0.0123');
    arktest.assertEQ(new Decimal('-111111111111111111111').valueOf(), '-111111111111111111111');
}

function testValueOfWithSpecificConfig003() {
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: 0, toExpPos: 0, minE: -9e15, maxE: 9e15 });
    arktest.assertEQ(new Decimal(0.0000001).valueOf(), '1e-7');
    arktest.assertEQ(new Decimal(0.000000123).valueOf(), '1.23e-7');
    arktest.assertEQ(new Decimal(0.000000012).valueOf(), '1.2e-8');
    arktest.assertEQ(new Decimal(-0.0000001).valueOf(), '-1e-7');
    arktest.assertEQ(new Decimal(-0.000000123).valueOf(), '-1.23e-7');
    arktest.assertEQ(new Decimal(-0.000000012).valueOf(), '-1.2e-8');
    arktest.assertEQ(new Decimal('573447902457635.174479825134').valueOf(), '5.73447902457635174479825134e+14');
    arktest.assertEQ(new Decimal('10.7688').valueOf(), '1.07688e+1');
    arktest.assertEQ(new Decimal('3171194102379077141557759899.307946350455841').valueOf(), '3.171194102379077141557759899307946350455841e+27');
    arktest.assertEQ(new Decimal('49243534668981911776986533197425948906.34579').valueOf(), '4.924353466898191177698653319742594890634579e+37');
    arktest.assertEQ(new Decimal('6855582439265693973.28633907445409866949445343654692955').valueOf(), '6.85558243926569397328633907445409866949445343654692955e+18');
    arktest.assertEQ(new Decimal('1').valueOf(), '1e+0');
}
