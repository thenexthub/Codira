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
    checkHypotResult('0.4519719010451812700381881242155284703426402593258190987', '92346327717009585478189395590602.34545881237122995324639951758026247635570631069180557087', 55, 4, '92346327717009585478189395590602.3454588123712299532464');
    checkHypotResult('69398002688216703269792743800286307271116693445111111111111111', '-211393667179764005100083390018873694', 100, 0, '69398002688216703269792743800286307271116693445111111433074862.06409526630410784634920186057594444197');
    checkHypotResult('0.465596788868', '-9431769081', 29, 3, '9431769081.0000000000114920312');
    checkHypotResult('-587359.66395', '-0.49568003707000085674015751654640980033', 13, 5, '587359.6639502');
    checkHypotResult('-0.059956811106228018570509499080322651692489744383276282735', '0.7728191', 54, 5, '0.775141393890713275121528253795513914835075182802874717');
    checkHypotResult('-41720000000020000000000000000000000000000000000000000700000000000000600000004000000001000000000000060000000000.00080000000000000006', '-46835015.87548144758089751035240583513', 34, 6, '41720000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000');
    checkHypotResult('0.6', '-91.5250722217761303', 63, 0, '91.5270388748665715095625471128327141160527733302883072128489811');
}
