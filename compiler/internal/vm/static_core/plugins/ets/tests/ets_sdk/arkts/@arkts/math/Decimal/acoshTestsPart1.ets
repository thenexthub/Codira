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
    const suite = new arktest.ArkTestsuite("Decimal acosh API tests");
    suite.addTest("Decimal acosh with specific config", testAcoshhWithSpecificConfig);
    return suite.run();
}

function checkAcoshResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).acosh().valueOf(), expected);
}

function testAcoshhWithSpecificConfig() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkAcoshResult('NaN', 40, 4, 'NaN');
    checkAcoshResult('0.9999999999999999999999999', 40, 4, 'NaN');
    checkAcoshResult('0', 40, 4, 'NaN');
    checkAcoshResult('-1', 40, 4, 'NaN');
    checkAcoshResult('-Infinity', 40, 4, 'NaN');
    checkAcoshResult('1', 40, 4, '0');
    checkAcoshResult('Infinity', 40, 4, 'Infinity');

    checkAcoshResult('3.3', 2, 4, '1.9');
    checkAcoshResult('4608888.8834', 7, 1, '16.03664');
    checkAcoshResult('145005612624726799', 7, 1, '40.20869');
    checkAcoshResult('644444444487588842435356055860541441083823652904922222222222222', 2, 6, '150');
    checkAcoshResult('9.28', 1, 5, '3');
    checkAcoshResult('76', 10, 0, '5.023837236');
}
