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

function checkHypotResult(a: string, b: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(Decimal.hypot(a, b).valueOf(), expected);
}

function testHypot001() {
    checkHypotResult('-0.55', '-9000000.000000000000000000000000000000000000000000000000000000000000000000000000000009', 62, 1, '9000000.0000000168055555555555398651834705075738799986293162621');
    checkHypotResult('52222222222227378067480110037255168691979179742111111111', '-867069820.333078943531805179108670644728376893549909696519412570032111919516568762799', 7, 2, '52222230000000000000000000000000000000000000000000000000');
    checkHypotResult('-8.0898982865397951925970316274645291728205275567128', '-0.0000945902', 28, 6, '8.089898287092787678956284349');
    checkHypotResult('-0.09771882084', '-8', 41, 2, '8.000596788236885020873363221344096704335');
    checkHypotResult('-10.11111111545', '-0.306711465294651568375461540673418284608909142237604855', 98, 2, '10.11576195409518993106696772029199668325921193685867908705991646038726063827463162781415087647978');
}

function config() {
    Decimal.precision = 40;
    Decimal.rounding = 4;
    Decimal.toExpNeg = -9e15;
    Decimal.toExpPos = 9e15;
    Decimal.minE = -9e15;
    Decimal.maxE = 9e15;
}

function main() {
    const suite = new arktest.ArkTestsuite("Decimal hypot API tests");
    suite.addTest("Decimal hypot", testHypot001);
    config();
    return suite.run();
}
