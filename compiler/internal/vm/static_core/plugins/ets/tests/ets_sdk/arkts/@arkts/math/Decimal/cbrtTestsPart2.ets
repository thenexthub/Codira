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
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig001);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig002);
    return suite.run();
}

function checkCbrtResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).cbrt().valueOf(), expected);
}

function testCbrtWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkCbrtResult('-61298.9118623440767849615617799359637', 197, 2, '-39.429165763027824633368109569426988335873352574747430934500984379971077138739905025020214349026555314495729845811145173351353942674905845843718751365519784088965133478757694847415073443325596801746');
    checkCbrtResult('-2667296564625933848.771563268738963983215162122262769664228', 16, 0, '-1386831.726818274');
    checkCbrtResult('-0.334827956215', 18, 4, '-0.694396042925351186');
    checkCbrtResult('-7500005000000.0400000000000300010000000000000000000000000000000005000080000000000000000000000000000000000000000000000000000000000002', 148, 3, '-19574.34255569632077437589146736430354513462358173952556661853843109637110903043841920141950812963359142265525718451865996670282229623898813004424264');
}

function testCbrtWithSpecificConfig002() {
    checkCbrtResult('-0.448468', 65, 3, '-0.76543882436933301547484358146042454236821782800684597996385314496');
    checkCbrtResult('4456480839.850118829380687717773702938945780108504222', 186, 0, '1645.62426023819366824168925970611179098042486858393864888187997539499903266584393283173483354076296462532577165369465744086271078047145002431391610101290550337528381582739390745330190718');
    checkCbrtResult('-96609.9150016783612444151212219171469160528', 240, 2, '-45.8853342870476892493411403085005845531506513402973065908688567063670288324040964632169186092910604211131726914840870130664139857417136966852726525679016120128473243314724524836298854193278510304781073613018919609658048462067078117541267917');
}
