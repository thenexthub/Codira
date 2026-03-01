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
    const suite = new arktest.ArkTestsuite("Decimal cbrt API tests");
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig);
    return suite.run();
}

function checkCbrtResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).cbrt().valueOf(), expected);
}

function testCbrtWithSpecificConfig() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkCbrtResult('-166.616666666', 209, 4, '-5.5026617052342616148242191958679981633756294691968284245117501906609287423436017722548887408126098326860453334063765780606472030703630175042357147586923686464448738079927220693696480524411130124852775767369759');
    checkCbrtResult('-770000', 315, 4, '-91.6565645433022286626092137070830558694163526764780961804072197329043071216124465836059153674866409019818795567376022473570999553232243320951491034642924310116859135403989828713296248110330728429307978818049273063638547762257276709121136983687240301319624468569985024000817994013321153096111049926913537599622929478');
    checkCbrtResult('82632936267840682945343488907.06731470799523118845064687431515', 138, 6, '4355630815.60590669653353707630763857376582544155292301618426362137145407016564135319408858966461324112682532653188194626242452174990441469');
}
