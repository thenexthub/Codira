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

import { Decimal, Value } from '@arkts.math.Decimal';

function main(): int {
    const suite = new arktest.ArkTestsuite('Decimal min and max API tests');
    suite.addTest('Decimal min and max with default config', testMinAndMaxWithDefaultConfig001);
    return suite.run();
}

function checkResultCompareZero(min: Decimal, max: Decimal, ...n: Value[]) {
    let minDecimal = Decimal.min(...n);
    let maxDecimal = Decimal.max(...n);
    arktest.assertEQ(min.valueOf(), minDecimal.valueOf());
    arktest.assertEQ(min.s, minDecimal.s);
    arktest.assertEQ(max.valueOf(), maxDecimal.valueOf());
    arktest.assertEQ(max.s, maxDecimal.s);
}

function testMinAndMaxWithDefaultConfig001() {
    checkResultCompareZero(new Decimal(-0.0), new Decimal("0"), 0.0, -0.0, 0.0);
    checkResultCompareZero(new Decimal(-0.0), new Decimal(0.0), 0.0, -0.0, 0.0);
}