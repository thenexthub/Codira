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
    const suite = new arktest.ArkTestsuite("Decimal asin API tests");
    suite.addTest("Decimal asin with specific config", testAsinWithSpecificConfigPart001);
    suite.addTest("Decimal asin with specific config", testAsinWithSpecificConfigPart002);
    return suite.run();
}

function checkAsinResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).asin().valueOf(), expected);
}

function testAsinWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkAsinResult('NaN', 40, 4, 'NaN');
    checkAsinResult('1.0000000000000001', 40, 4, 'NaN');
    checkAsinResult('2', 40, 4, 'NaN');
    checkAsinResult('Infinity', 40, 4, 'NaN');
    checkAsinResult('-1.00000000000000000001', 40, 4, 'NaN');
    checkAsinResult('-Infinity', 40, 4, 'NaN');
    checkAsinResult('0', 40, 4, '0');
    checkAsinResult('-0', 40, 4, '-0');
    checkAsinResult('0.365496185994558260858326718937', 10, 5, '0.3741658147');
    checkAsinResult('-0.35641389188', 3, 0, '-0.365');
    checkAsinResult('0.68955841424562468594284036990178287693727890663761450264565153347585586987783918662506831', 7, 6, '0.7608791');
    checkAsinResult('-0.8185571137226394757008542835605396', 1, 1, '-0.9');
    checkAsinResult('0.3909767220307331413111', 5, 3, '0.40169');
    checkAsinResult('0.4973777625990946249943842344354278064648780030532232700989739966171872596183742540844', 7, 0, '0.5205736');
    checkAsinResult('-0.3220586075', 7, 1, '-0.3279031');
    checkAsinResult('-0.4712856183846410563097510893908', 6, 5, '-0.490748');
}

function testAsinWithSpecificConfigPart002() {
    checkAsinResult('0.05838677605449046055272817746189607969418472098217185904914', 2, 6, '0.058');
    checkAsinResult('0.8', 9, 5, '0.927295218');
    checkAsinResult('-0.35', 4, 5, '-0.3576');
    checkAsinResult('0.5', 10, 1, '0.5235987755');
    checkAsinResult('0.12', 3, 1, '0.12');
    checkAsinResult('-0.5434', 8, 3, '-0.57448199');
    checkAsinResult('0.0073911', 6, 1, '0.00739116');
    checkAsinResult('0.6413', 9, 5, '0.696191343');
    checkAsinResult('0.6695', 7, 5, '0.7335355');
    checkAsinResult('-0.73867', 9, 0, '-0.831095123');
    checkAsinResult('-0.734541962948526931', 1, 1, '-0.8');
    checkAsinResult('0.8124222852789144395978469359524044494', 9, 5, '0.94829454');
    checkAsinResult('0.01', 6, 1, '0.0100001');
    checkAsinResult('-0.788421', 4, 1, '-0.9082');
    checkAsinResult('-0.3', 2, 2, '-0.3');
    checkAsinResult('0.6608633389456637408497420998651914114139088612446', 2, 4, '0.72');
    checkAsinResult('0.3931549079701655746240375417718660050381124496506720523996527133505517587998', 23, 1, '0.40406030213304057779811');
    checkAsinResult('-0.587605525487010465245800314956501132921', 13, 3, '-0.6280963940928');
    checkAsinResult('-0.5', 52, 3, '-0.5235987755982988730771072305465838140328615665625177');
}
