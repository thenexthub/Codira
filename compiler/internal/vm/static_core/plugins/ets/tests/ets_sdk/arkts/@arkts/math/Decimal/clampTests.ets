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
    const suite = new arktest.ArkTestsuite("Decimal clamp API tests");
    suite.addTest("Decimal clamp with default config", testClampWithDefaultConfig001);
    suite.addTest("Decimal clamp with default config", testClampWithDefaultConfig002);
    return suite.run();
}

function testClampWithDefaultConfig001() {
    arktest.assertEQ( new Decimal('-0').clamp('0', '0').valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').clamp('-0', '0').valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').clamp('0', '-0').valueOf(), '-0');
    arktest.assertEQ(new Decimal('-0').clamp('-0', '-0').valueOf(), '-0');
    arktest.assertEQ(new Decimal('0').clamp('0', '0').valueOf(), '0');
    arktest.assertEQ(new Decimal('0').clamp('-0', '0').valueOf(), '0');
    arktest.assertEQ(new Decimal('0').clamp('0', '-0').valueOf(), '0');
    arktest.assertEQ(new Decimal('0').clamp('-0', '-0').valueOf(), '0');
    arktest.assertEQ(new Decimal(0).clamp(0, 1).valueOf(), '0');
    arktest.assertEQ(new Decimal(-1).clamp(0, 1).valueOf(), '0');
    arktest.assertEQ(new Decimal(-2).clamp(0, 1).valueOf(), '0');
    arktest.assertEQ(new Decimal(1).clamp(0, 1).valueOf(), '1');
    arktest.assertEQ(new Decimal(2).clamp(0, 1).valueOf(), '1');
}

function testClampWithDefaultConfig002() {
    arktest.assertEQ(new Decimal(1).clamp(1, 1).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).clamp(1, 1).valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).clamp(-1, 1).valueOf(), '-1');
    arktest.assertEQ(new Decimal(2).clamp(1, 2).valueOf(), '2');
    arktest.assertEQ(new Decimal(3).clamp(1, 2).valueOf(), '2');
    arktest.assertEQ(new Decimal(1).clamp(0, 1).valueOf(), '1');
    arktest.assertEQ(new Decimal(2).clamp(0, 1).valueOf(), '1');
    arktest.assertEQ(new Decimal(Infinity).clamp(0, 1).valueOf(), '1');
    arktest.assertEQ(new Decimal(0).clamp(-Infinity, 0).valueOf(), '0');
    arktest.assertEQ(new Decimal(-Infinity).clamp(0, 1).valueOf(), '0');
    arktest.assertEQ(new Decimal(-Infinity).clamp(-Infinity, Infinity).valueOf(), '-Infinity');
    arktest.assertEQ(new Decimal(Infinity).clamp(-Infinity, Infinity).valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(0).clamp(1, Infinity).valueOf(), '1');
    arktest.assertEQ(new Decimal(0).clamp(NaN, 1).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(0).clamp(0, NaN).valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(NaN).clamp( 0, 1).valueOf(), 'NaN');
}
