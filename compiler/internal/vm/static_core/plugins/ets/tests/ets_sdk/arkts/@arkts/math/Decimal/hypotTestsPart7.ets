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
    checkHypotResult('19999999999999999999999999999999.919999999999999999999', '-95670705629746263190920599520452475212900072593749902459133558626394349560336068069498100861613875411', 24, 6, '95670705629746263190920600000000000000000000000000000000000000000000000000000000000000000000000000000');
    checkHypotResult('19333333333.33313333333333', '-307058050750', 19, 4, '307666092230.143791');
    checkHypotResult('-0.00001175529004148059889839195560772120687431522218537528816456748920635963047412762382567334499074596992570830887179621917584255805343012605460010989845894713567382571024', '59.8600988774488317444932', 22, 1, '59.86009887744998599287');
    checkHypotResult('-0.1098958603115877986399807307790849607335424187223724689574921530969578406825061416021390586394788741395795498746592063682753064315701974886', '-52222222222222262283923266176757777', 76, 5, '52222222222222262283923266176757777.00000000000000000000000000000000000011563');
    checkHypotResult('-17118682794143106193517128.496346472179213795', '19999999999999999997919330.00231406838630646574151545822388888888888888888', 38, 5, '26325829533112365449334045.441841092728');
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
