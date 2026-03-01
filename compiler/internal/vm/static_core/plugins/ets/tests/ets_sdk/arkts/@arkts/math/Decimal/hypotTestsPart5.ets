/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

import { Decimal } from "@arkts.math.Decimal";

function main() {
    const suite = new arktest.ArkTestsuite("Decimal hypot API tests");
    suite.addTest("Decimal hypot", testHypot001);
    return suite.run();
}

function checkHypotResult(a: string, b: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(Decimal.hypot(a, b).valueOf(), expected);
}

function testHypot001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkHypotResult('30000.6', '511111111111145686553087261381.999991267342292294981021024064788686134890483114665388298481703', 51, 2, '511111111111145686553087261381.999991267342292294982');
    checkHypotResult('72926343.726877231000712798686463998173070827809981833334964558342793176683041794308836271', '-82222222222221937444', 49, 2, '82222222222221937444.00003234071924617295968730272');
    checkHypotResult('-242915822316177763142341053724.3195904798466883740740090079450439', '-299999999999.999999999999999999995999999999999999999999999999990999994', 51, 2, '242915822316177763142341053724.319590665096045830207');
}
