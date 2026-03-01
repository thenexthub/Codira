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
    const suite = new arktest.ArkTestsuite("Decimal atan API tests");
    suite.addTest("Decimal atan with specific config", testAtanWithSpecificConfigPart001);
    return suite.run();
}

function checkAtanResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).atan().valueOf(), expected);
}

function testAtanWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkAtanResult('NaN', 40, 4, 'NaN');
    checkAtanResult('0', 40, 4, '0');
    checkAtanResult('-0', 40, 4, '-0');
    checkAtanResult('Infinity', 20, 4, '1.5707963267948966192');
    checkAtanResult('-Infinity', 20, 4, '-1.5707963267948966192');

    checkAtanResult('0.454786854', 2, 1, '0.42');
    checkAtanResult('392.8825877961099786091', 2, 1, '1.5');
    checkAtanResult('90.02', 9, 1, '1.55968814');
    checkAtanResult('0.22618819', 1, 6, '0.2');
    checkAtanResult('2.104', 3, 3, '1.12');
    checkAtanResult('0.4', 2, 0, '0.39');
    checkAtanResult('700000', 7, 5, '1.570795');
    checkAtanResult('0.1034373855738', 4, 0, '0.1031');
    checkAtanResult('2888822222222222.77777777888888888887945111', 4, 2, '1.571');
    checkAtanResult('0.7163035', 5, 0, '0.62159');
    checkAtanResult('31986483109634719887104428745', 4, 1, '1.57');
    checkAtanResult('66099.99999090960999899', 9, 2, '1.5707812');
    checkAtanResult('1453.5793000200494316368218639720487955117970374751', 8, 3, '1.5701083');
    checkAtanResult('25', 10, 1, '1.530817639');
    checkAtanResult('0.00083', 2, 5, '0.00083');
    checkAtanResult('15469795946533333333333', 3, 2, '1.58');
    checkAtanResult('0.0471789911257911619518474101710506908660018025802862437747', 10, 4, '0.04714403323');
    checkAtanResult('2436496471267622480734244943816819264461333', 6, 4, '1.5708');
    checkAtanResult('397.86', 6, 3, '1.56828');
    checkAtanResult('23350774.374402222222222222', 5, 2, '1.5708');
    checkAtanResult('6.4999999895999999999999989901', 3, 2, '1.42');
    checkAtanResult('0.2021040932658758909350221378184270710381308788971', 25, 0, '0.1994179053851243775408929');
    checkAtanResult('37322947047433501470414286593213511', 3, 4, '1.57');
    checkAtanResult('0.2', 30, 5, '0.197395559849880758370049765195');
    checkAtanResult('68205', 73, 0, '1.570781665113202779409754819994955472047433041976705657000777636779446534');
    checkAtanResult('546276727353824444.00000000000000000004444444444444444', 10, 1, '1.570796326');
    checkAtanResult('0.702767582631380812', 59, 3, '0.6125809883518849916298401561171422315736485425008362936336');
    checkAtanResult('77.2782608490479012', 63, 5, '1.55785679919770947051742907742015525136935668142627146470943833');
    checkAtanResult('3782579384288161800008711705384065591', 64, 1, '1.570796326794896619231321691639751441834214834998463624232552952');
    checkAtanResult('353878', 38, 3, '1.5707935009622650418565673617028447836');
}
