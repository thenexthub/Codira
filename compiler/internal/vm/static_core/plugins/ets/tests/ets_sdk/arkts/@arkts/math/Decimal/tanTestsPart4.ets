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
    const suite = new arktest.ArkTestsuite("Decimal tan API tests");
    suite.addTest("Decimal tan with specific config", testTanWithSpecificConfig001);
    return suite.run();
}

function checkTanResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).tan().valueOf(), expected);
}

function testTanWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkTanResult('0.5592', 74, 1, '0.625835640923790139625307834841776614566589496394316390500913130105766275');
    checkTanResult('6661489439437775791.114329793186996632507954406585491', 17, 5, '-1.5847845023422794');
    checkTanResult('4391137181720764819571909798888888888888888888', 86, 1, '-0.8685635418477878118403689813703497084174127945629019783498553011616589497613259649227');
    checkTanResult('300000000000000000000000000000000000000000000000000000000000000000000000000200000000400300000000000000000000000000000.0000000000000000000000000000000000000000000000000003', 122, 3, '-1.5903229154055637520787252214804110216069360069379104488574199058901135519171083962076619886567087892277814852998414939069');
}
