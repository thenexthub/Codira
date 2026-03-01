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
    const suite = new arktest.ArkTestsuite("Decimal acos API tests");
    suite.addTest("Decimal acos with specific config", testAcosWithSpecificConfigPart001);
    suite.addTest("Decimal acos with specific config", testAcosWithSpecificConfigPart002);
    return suite.run();
}

function checkAcosResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).acos().valueOf(), expected);
}

function testAcosWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkAcosResult('NaN', 40, 4, 'NaN');
    checkAcosResult('Infinity', 40, 4, 'NaN');
    checkAcosResult('-Infinity', 40, 4, 'NaN');
    checkAcosResult('1.0000000000000001', 40, 4, 'NaN');
    checkAcosResult('2', 40, 4, 'NaN');
    checkAcosResult('-1.0000000000000001', 40, 4, 'NaN');
    checkAcosResult('-2', 40, 4, 'NaN');
    checkAcosResult('1', 40, 4, '0');
    checkAcosResult('-1', 40, 4, '3.141592653589793238462643383279502884197');
    checkAcosResult('0', 20, 4, '1.5707963267948966192');
    checkAcosResult('-0', 20, 4, '1.5707963267948966192');
    checkAcosResult('-0.6403', 7, 1, '2.265685');
    checkAcosResult('-0.53', 2, 2, '2.2');
    checkAcosResult('0.4396178420823739355530719404', 10, 0, '1.115623176');
    checkAcosResult('0.496420101927875989152448746035981', 1, 5, '1');
    checkAcosResult('0.1', 2, 0, '1.5');
    checkAcosResult('-0.20878679', 6, 2, '1.78114');
    checkAcosResult('-0.541191262571486130575708261722416873899', 6, 5, '2.14265');
    checkAcosResult('0.5', 7, 1, '1.047197');
}

function testAcosWithSpecificConfigPart002() {
    checkAcosResult('-0.409325859', 10, 2, '1.992511392');
    checkAcosResult('0.551', 5, 6, '0.98723');
    checkAcosResult('0.5', 7, 2, '1.047198');
    checkAcosResult('-0.43419432724217379874620877899386037437997149166291587557428756984642427', 3, 1, '2.01');
    checkAcosResult('0.553', 9, 3, '0.984835716');
    checkAcosResult('0.7809790373261734623688348400648923431479560079817', 6, 1, '0.674564');
    checkAcosResult('0.75824031', 9, 0, '0.710186479');
    checkAcosResult('-0.8', 8, 2, '2.4980916');
    checkAcosResult('0.0799', 3, 5, '1.49');
    checkAcosResult('-0.38393812134692277325859', 4, 1, '1.964');
    checkAcosResult('-0.3907773773285398604336', 1, 1, '1');
    checkAcosResult('-0.69992716948032572772435', 9, 4, '2.34609185');
    checkAcosResult('-0.66158215731029676878636733180177567', 8, 0, '2.2937231');
    checkAcosResult('0.9147368246772431874', 37, 1, '0.415940230269184272745464503534348965');
    checkAcosResult('-0.4553', 27, 1, '2.04350546255828224507689468');
    checkAcosResult('-0.028011903303708804768053896701975443', 56, 1, '1.5988118947274791447123927148057628240666865110752680848');
    checkAcosResult('0.4065777430651635458256706', 14, 6, '1.1520912372989');
    checkAcosResult('-0.78468834561401325', 40, 6, '2.472989536549809027410129361828086088366');
    checkAcosResult('-0.04164046779', 31, 2, '1.61244883758340629332195244831');
}
