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
    checkHypotResult('-306555555555555555555555055555555555555555555555555555559555555555555555555555555555555555555555555555555595555555555555555555555555555555555555555555525555555555555555555555555555555.5555555555', '0.15666773914679673976142669430328125908359330664846715765297103124831482667541495', 15, 3, '306555555555555000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
    checkHypotResult('-5734991.83220402694035812590544522858837047993475498775680667010453187699606727653014592756227908536357219129844096200521', '99399999.699', 55, 0, '99565305.56110721338952690556051338361260372609898394192');
    checkHypotResult('-0.585223', '9742275700501', 25, 1, '9742275700501');
    checkHypotResult('-817', '2706343113769608709474153626083514693465686442858333333333333333', 54, 2, '2706343113769608709474153626083514693465686442858333340000000000');
    checkHypotResult('521999999999199999929999999999999999.99999995999999','-6777777777777777777777777.777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777770777777775', 54, 2, '521999999999199999930044002175866866.547023411749520706');
}
