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
    const suite = new arktest.ArkTestsuite("Decimal log API tests");
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart001);
    return suite.run();
}

function checkLogResult(n: string, base: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLogResult('22876792454961', '59049', '2.8', 2, 2);
    checkLogResult('32', '33554432', '0.2', 1, 2);
    checkLogResult('16', '1125899906842624', '0.08', 1, 2);
    checkLogResult('16777216', '1024', '2.4', 2, 2);
    checkLogResult('31381059609', '3486784401', '1.1', 2, 4);
    checkLogResult('131072', '16', '4.25', 3, 7);
    checkLogResult('17179869184', '65536', '2.125', 4, 2);
    checkLogResult('131072', '32', '3.4', 2, 5);
    checkLogResult('31381059609', '6561', '2.75', 3, 4);
    checkLogResult('1162261467', '81', '4.75', 3, 2);
    checkLogResult('5', '152587890625', '0.0625', 3, 8);
    checkLogResult('4', '1024', '0.2', 1, 2);
    checkLogResult('268435456', '1048576', '1.4', 2, 8);
    checkLogResult('456870693.58', '238920772.66', '1.0336035877093034523', 21, 4);
    checkLogResult('575547956.8582', '824684975.3545', '0.98248076', 8, 4);
    checkLogResult('82275648.874341603', '959190115.624130088', '0.88124641544168894893181429200832363', 35, 4);
    checkLogResult('74257343.4', '743703514.4', '0.88720377341908842250463392057841865999040289364224', 50, 4);
    checkLogResult('617556576.22', '1390349767.37', '0.96145220002205342499', 20, 4);
    checkLogResult('385659206.402956', '306197094.245356', '1.0118079926535367225661814147003237994862', 41, 4);
    checkLogResult('1739848017', '139741504', '1.134455757605027173760473871049514546484', 40, 4);
}
