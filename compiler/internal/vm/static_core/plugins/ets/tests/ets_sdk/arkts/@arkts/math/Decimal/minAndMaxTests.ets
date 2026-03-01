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
    suite.addTest('Decimal min and max with default config', testMinAndMaxWithDefaultConfig002);
    suite.addTest('Decimal min and max with default config', testMinAndMaxWithDefaultConfig003);
    suite.addTest('Decimal min and max with default config', testMinAndMaxWithDefaultConfig004);
    return suite.run();
}

function checkResultIsNaN(...n: Value[]) {
    arktest.assertTrue(Number.isNaN(Decimal.max(...n).toNumber()));
    arktest.assertTrue(Number.isNaN(Decimal.min(...n).toNumber()));
}

function checkResultValues(min: double | string, max: double | string, ...n: Value[]) {
    arktest.assertEQ(new Decimal(min).valueOf(), Decimal.min(...n).valueOf());
    arktest.assertEQ(new Decimal(max).valueOf(), Decimal.max(...n).valueOf());
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
    Decimal.set({ precision: 20, rounding: 4, toExpNeg: -7, toExpPos: 21, minE: -9e15, maxE: 9e15 });
    checkResultIsNaN(NaN);
    checkResultIsNaN(-2, 0, -1, NaN);
    checkResultIsNaN(-2, NaN, 0, -1);
    checkResultIsNaN(NaN, -2, 0, -1);
    checkResultIsNaN(NaN, -2, 0, -1);
    checkResultIsNaN(-2, 0, -1, new Decimal(NaN));
    checkResultIsNaN(-2, 0, -1, new Decimal(NaN));
    checkResultIsNaN(Infinity, -2, 'NaN', 0, -1, -Infinity);
    checkResultIsNaN('NaN', Infinity, -2, 0, -1, -Infinity);
    checkResultIsNaN(Infinity, -2, NaN, 0, -1, -Infinity);
    checkResultValues(0, 0, 0, 0, 0);
    checkResultValues(-2, Infinity, -2, 0, -1, Infinity);
    checkResultValues(-Infinity, 0, -2, 0, -1, -Infinity);
    checkResultValues(-Infinity, Infinity, -Infinity, -2, 0, -1, Infinity);
    checkResultValues(-Infinity, Infinity, Infinity, -2, 0, -1, -Infinity);
    checkResultValues(-Infinity, Infinity, -Infinity, -2, 0, new Decimal(Infinity));
}

function testMinAndMaxWithDefaultConfig002() {
    checkResultValues(-2, 0, -2, 0, -1);
    checkResultValues(-2, 0, -2, -1, 0);
    checkResultValues(-2, 0, 0, -2, -1);
    checkResultValues(-2, 0, 0, -1, -2);
    checkResultValues(-2, 0, -1, -2, 0);
    checkResultValues(-2, 0, -1, 0, -2);
    checkResultValues(-1, 1, -1, 0, 1);
    checkResultValues(-1, 1, -1, 1, 0);
    checkResultValues(-1, 1, 0, -1, 1);
    checkResultValues(-1, 1, 0, 1, -1);
    checkResultValues(-1, 1, 1, -1, 0);
    checkResultValues(-1, 1, 1, 0, -1);
    checkResultValues(0, 2, 0, 1, 2);
    checkResultValues(0, 2, 0, 2, 1);
    checkResultValues(0, 2, 1, 0, 2);
    checkResultValues(0, 2, 1, 2, 0);
    checkResultValues(0, 2, 2, 1, 0);
    checkResultValues(0, 2, 2, 0, 1);
    checkResultValues(-1, 1, '-1', 0, new Decimal(1));
    checkResultValues(-1, 1, '-1', new Decimal(1));
    checkResultValues(-1, 1, 0, '-1', new Decimal(1));
    checkResultValues(0, 1, 0, new Decimal(1));
    checkResultValues(1, 1, new Decimal(1));
    checkResultValues(-1, -1, new Decimal(-1));
}

function testMinAndMaxWithDefaultConfig003() {
    checkResultValues(0.0009999, 0.0010001, 0.001, 0.0009999, 0.0010001);
    checkResultValues(-0.0010001, -0.0009999, -0.001, -0.0009999, -0.0010001);
    checkResultValues(-0.000001, 999.001, 2, -0, '1e-9000000000000000', 324.32423423, -0.000001, '999.001', 10);
    checkResultValues('-9.99999e+9000000000000000', Infinity, 10, '-9.99999e+9000000000000000', new Decimal(Infinity), '9.99999e+9000000000000000', 0);
    checkResultValues('-9.999999e+9000000000000000', '1.01e+9000000000000000', '-9.99998e+9000000000000000', '-9.999999e+9000000000000000', '9e+8999999999999999', '1.01e+9000000000000000', 1e+300);
    checkResultValues(1, Infinity, 1, '1e+9000000000000001', 1e200);
    checkResultValues(-Infinity, 1, 1, '-1e+9000000000000001', -1e200);
    checkResultValues(0, 1, 1, '1e-9000000000000001', 1e-200);
    checkResultValues(-3, 3, 1, '2', 3, '-1', -2, '-3');
}

function testMinAndMaxWithDefaultConfig004() {
    checkResultCompareZero(new Decimal("-0"), new Decimal(0), "-0", 0, "-0");
    checkResultCompareZero(new Decimal("-0"), new Decimal(0), 0, "-0", 0);
    checkResultCompareZero(new Decimal("-0"), new Decimal("0"), "-0", 0, "-0");
    checkResultCompareZero(new Decimal("-0"), new Decimal("0"), 0, "-0", 0);
}
