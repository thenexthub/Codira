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
    checkAcoshResult('28099999969999999491999992999999999999.9999999999999939697', 4, 1, '86.92');
    checkAcoshResult('585287364', 7, 6, '20.88076');
    checkAcoshResult('7', 3, 1, '2.63');
    checkAcoshResult('557921.15371560132973597030139633138927112983', 9, 2, '13.9251202');
    checkAcoshResult('222222270476', 7, 1, '26.82009');
    checkAcoshResult('9925', 4, 2, '9.896');
    checkAcoshResult('51156203358371111111111111111111.88888888888888185816094145162589', 5, 0, '73.706');
    checkAcoshResult('920.33', 5, 5, '7.5179');
    checkAcoshResult('398.29', 4, 0, '6.681');
    checkAcoshResult('300000000000000000000000', 8, 4, '54.751217');
    checkAcoshResult('2077', 8, 2, '8.331827');
    checkAcoshResult('466660000000000000000000.666663813047981223086678888888888888', 5, 2, '55.194');
    checkAcoshResult('1481.596575', 2, 0, '8');
    checkAcoshResult('51843625827420648103724841829104208766358144', 6, 4, '101.35');
    checkAcoshResult('66.49', 6, 6, '4.89014');
    checkAcoshResult('30564048384', 13, 1, '24.83623744632');
    checkAcoshResult('7', 48, 1, '2.63391579384963341725009269461593688805396394293');
}
