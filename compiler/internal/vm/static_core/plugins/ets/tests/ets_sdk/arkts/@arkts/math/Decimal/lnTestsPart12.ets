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
    const suite = new arktest.ArkTestsuite("Decimal ln API tests part 12");
    suite.addTest("Decimal ln with specific config", testLnWithSpecificConfigPart001);
    suite.addTest("Decimal ln with specific config", testLnWithSpecificConfigPart002);
    return suite.run();
}

function checkLnResult(n: string, expected: string, sd: double, rm: int) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).ln().valueOf(), expected);
}

function testLnWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: 0, toExpPos: 0, minE: -9e15, maxE: 9e15 });
    checkLnResult('3.62838465078525030E+7741738753458588', '2e+16', 1, 2);
    checkLnResult('7E+2084142509237', '4.79891547344626727113852205192e+12', 30, 1);
    checkLnResult('6.632399246416402141258204555971393162485E+5282951327214421', '1.216444497305703653967296294053184676613657e+16', 44, 5);
    checkLnResult('9.3160920091238143684430426114185923861195316789176E+9009721765428', '2.0745e+13', 5, 1);
    checkLnResult('9.3685E+670410418', '1.543677e+9', 8, 5);
    checkLnResult('2.5346294534658525082446802192095498605896343566597E+4300272406899978', '9.9017431399415154341560132628634710568864551e+15', 44, 0);
    checkLnResult('4.646038E+85', '1.97255748e+2', 9, 5);
    checkLnResult('7.11299819E+7082753304', '1.63e+10', 3, 1);
}

function testLnWithSpecificConfigPart002() {
    checkLnResult('7.60429765973931070947601E+223343106', '5.14266508527302571586964653405e+8', 30, 0);
    checkLnResult('1.259774278348886756714714853102551787716753309044316124949E+712621', '1.6408707224870705769939774744309009702195e+6', 41, 1);
    checkLnResult('6.0261907885269058826333443505093996174E+6436400131', '1.482035899598163792393092381789529257871165943e+10', 46, 3);
    checkLnResult('2.30437658671337063317129505E+933', '2.15e+3', 4, 0);
    checkLnResult('3.513354973190022585498661637765E+7375148', '1.69819070999962638165306055501e+7', 30, 3);
    checkLnResult('3.59522279140903230506716479623E+86968785461810', '2.002530289601622667750807006104599119026e+14', 40, 1);
    checkLnResult('2.323805246716532118808402603969727E+9227297604', '2.12466e+10', 6, 6);
    checkLnResult('4.19231E-7592313503421', '-1.748194789431315881762449482356139169e+13', 37, 1);
}
