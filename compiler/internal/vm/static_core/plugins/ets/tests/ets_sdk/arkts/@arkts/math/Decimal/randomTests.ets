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

let r: Decimal;
let sd: double;
let maxDigits: int = 100;

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal random API tests");
    suite.addTest("Decimal random with different precision", testRandomWithDiffPr);
    return suite.run();
}

function testRandomWithDiffPr() {
    for (let i = 0; i < 1000; i++) {
        sd = Math.random() * maxDigits + 1 | 0;
        if (Math.random() > 0.5) {
            Decimal.precision = sd;
            r = Decimal.random();
        } else {
            r = Decimal.random(sd);
        }

        arktest.assertTrue(r.greaterThanOrEqualTo(0) &&
                   r.lessThan(1) && r.equals(r) &&
                   r.equals(r.valueOf()));
    }
}
