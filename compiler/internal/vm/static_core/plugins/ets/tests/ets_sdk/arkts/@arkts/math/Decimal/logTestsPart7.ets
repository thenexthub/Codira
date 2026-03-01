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
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart002);
    return suite.run();
}

function checkLogResult(n: string, base: string | double, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLogResult('684413372.332', '749444030.62', '0.99556', 5, 4);
    checkLogResult('1276559129.76358811', '1814329747.19301894', '0.983510102095361604388', 21, 4);
    checkLogResult('470873324.56', '770017206.95', '0.975963952980122531477453931545461086248352', 42, 4);
    checkLogResult('142843622.855', '188030025.676', '0.985573716314165', 15, 4);
    checkLogResult('208762187.506204', '15673510.715596', '1.1563', 5, 4);
    checkLogResult('1066260899.1963', '954219284.761', '1.005369396783858165862954752482856604', 37, 4);
    checkLogResult('98615189.15', '75483684.05', '1.0147363402964731399253', 23, 4);
    checkLogResult('134306349.93018997', '262971762.95484809', '0.965342550919082621945239', 24, 4);
    checkLogResult('964681161.089224', '1910911588.814815', '0.9680153968863558918522522557796148', 34, 4);
    checkLogResult('9303669', '272208139', '0.8262', 4, 4);
    checkLogResult('388804210', '196979048', '1.035603565223696855965', 22, 4);
    checkLogResult('699589959.2322617', '574032511.7854473', '1.0098079347111332288609', 23, 4);
    checkLogResult('100575245.36', '172874206.82', '0.971443699412905370317336892965778', 33, 4);
    checkLogResult('188632711.8541175', '1056627336.0975408', '0.9170754305183363941127042', 25, 4);
    checkLogResult('267522787.94', '528716571.79', '0.966083390988836341228896', 24, 4);
    checkLogResult('145509306.43395', '472783713.04935', '0.941003844701466585568051857', 28, 4);
    checkLogResult('991525965.6381098', '609527830.0476525', '1.024053580832128', 16, 4);
    checkLogResult('1023653880.6218838', '953120602.1428507', '1.00345303146', 13, 4);
    checkLogResult('55755796.19', '1330531177.01', '0.84899920538009273', 17, 4);
    checkLogResult('334096229.1342503', '563056758.6770503', '0.97409528', 8, 4);
}

function testLogWithSpecificConfigPart002() {
    checkLogResult('9635164', '231514430', '0.834932623823994616103829175346875687708', 39, 4);
    checkLogResult('131654133.157309973', '115412751.259558256', '1.007092396906741330059871530698890891053443', 43, 4);
    checkLogResult('26880.2432276408875624', 2, '14.7142585720457255', 19, 3);
    checkLogResult('18216.8140929585641372', 2, '14.152983050314836771855701', 26, 1);
    checkLogResult('28062.73494235358182', 2, '14.776367997755111083362495', 26, 0);
    checkLogResult('7408.82842447993', 2, '12.8550297084583087071', 21, 1);
    checkLogResult('395.067', 2, '8.62595353', 9, 3);
    checkLogResult('27442.6587462411378', 2, '14.74414', 7, 0);
    checkLogResult('29259.23925137426', 2, '14.83660463902', 13, 1);
}
