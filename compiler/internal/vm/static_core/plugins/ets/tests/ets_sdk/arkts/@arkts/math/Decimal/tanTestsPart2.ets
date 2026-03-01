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
    const suite = new arktest.ArkTestsuite("Decimal tan API tests");
    suite.addTest("Decimal tan with specific config", testTanWithSpecificConfig001);
    suite.addTest("Decimal tan with specific config", testTanWithSpecificConfig002);
    return suite.run();
}

function checkTanResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).tan().valueOf(), expected);
}

function testTanWithSpecificConfig001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkTanResult('562', 10, 0, '-0.3594685138');
    checkTanResult('6433373233.39333336638333334', 5, 4, '-1.383');
    checkTanResult('233333333331549444444444.09076555555555555', 10, 4, '-5.305366526');
    checkTanResult('40', 9, 2, '-1.11721493');
    checkTanResult('906287052558597992196230844623', 10, 1, '0.3528425701');
    checkTanResult('900000', 4, 6, '-0.3334');
    checkTanResult('57777777777.88888888887777777428970596211366', 13, 1, '4.620958871418');
    checkTanResult('33109173612614237', 15, 1, '0.697054002938309');
    checkTanResult('446571', 8, 4, '-0.11299979');
    checkTanResult('55.33', 25, 1, '-2.721513818553435443556001');
    checkTanResult('0.6874', 3, 4, '0.821');
}

function testTanWithSpecificConfig002() {
    checkTanResult('1', 23, 0, '1.557407724654902230507');
    checkTanResult('78887888588.8', 15, 5, '-0.0613791279629461');
    checkTanResult('49999999999999999999999999999999999999999999999999999999.399999999999999999992702', 15, 2, '-4.12257063808152');
    checkTanResult('49999999999999999999999999999999999999999999999999999.99999999999999999999999999999', 38, 6, '-0.41507238166911582158673379470559517419');
    checkTanResult('0.00000391163258651029507727', 31, 2, '0.000003911632586530245537172208314767');
    checkTanResult('0.17183171313', 36, 4, '0.173543103883274147884533120728007237');
    checkTanResult('861646.5732966967557082', 34, 2, '-2.464947656728891301675656852697406');
    checkTanResult('47777777777777777.410171', 105, 5, '-13.74058038972407219477409398839850580956304359987902889186211451339313466403485175336849944968942741891');
    checkTanResult('0.390071731702448005162815294909486174474947036240529240590971978418532990165699316733956931596883519677657915509998387416505531185920438999174869181240110048120035540672577', 63, 1, '0.411138769556421473902517663320961167226706740351370710111315144');
    checkTanResult('23174769554', 70, 3, '0.2212756420749658903960662285747816321880538429001022336777294070264195');
}
