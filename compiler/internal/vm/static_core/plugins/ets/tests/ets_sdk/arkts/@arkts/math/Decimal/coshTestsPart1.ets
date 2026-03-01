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
    const suite = new arktest.ArkTestsuite("Decimal cosh API tests");
    suite.addTest("Decimal cosh with specific config", testCoshWithSpecificConfigPart001);
    suite.addTest("Decimal cosh with specific config", testCoshWithSpecificConfigPart002);
    suite.addTest("Decimal cosh with specific config", testCoshWithSpecificConfigPart003);
    return suite.run();
}

function checkCoshResult(n: string, pr: double, rm: int, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).cosh().valueOf(), expected);
}

function testCoshWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkCoshResult('NaN', 40, 4, 'NaN');
    checkCoshResult('0', 20, 4, '1');
    checkCoshResult('-0', 20, 4, '1');
    checkCoshResult('Infinity', 40, 4, 'Infinity');
    checkCoshResult('-Infinity', 40, 4, 'Infinity');

    checkCoshResult('0.7356369431260767888795717715533948998814', 4, 2, '1.284');
    checkCoshResult('0.32350325607614', 8, 2, '1.0527852');
    checkCoshResult('0.39', 10, 3, '1.077018834');
    checkCoshResult('-0.4085', 1, 3, '1');
    checkCoshResult('-0.8353225329554414512103907831003554564', 10, 1, '1.369646055');
    checkCoshResult('0.822765764301886756212368544717371789849053470184', 1, 2, '2');
    checkCoshResult('-0.608417', 2, 3, '1.1');
    checkCoshResult('-0.3135167132332845', 4, 1, '1.049');
    checkCoshResult('-0.3872949', 5, 6, '1.0759');
    checkCoshResult('-0.69', 5, 2, '1.2477');
    checkCoshResult('-0.6767082617749', 8, 6, '1.2378392');
    checkCoshResult('-0.8808', 8, 5, '1.4136402');
    checkCoshResult('0.49600945701356094984359748395247735156888479861846043140472804537060894678', 6, 5, '1.12556');
    checkCoshResult('0.046', 8, 6, '1.0010582');
    checkCoshResult('0.9873815', 5, 3, '1.5283');
    checkCoshResult('0.8784', 10, 5, '1.411246223');
    checkCoshResult('0.7', 7, 3, '1.255169');
    checkCoshResult('-0.503676971304', 2, 4, '1.1');
    checkCoshResult('-0.52', 9, 0, '1.1382741');
    checkCoshResult('0.335', 5, 3, '1.0566');
    checkCoshResult('0.54503134222707683066822814099005615624557424312284166822346112986570184867578297341372133058', 8, 4, '1.152243');
}

function testCoshWithSpecificConfigPart002() {
    checkCoshResult('0.140170026295214447428649742547694034916', 3, 3, '1');
    checkCoshResult('0.9749', 8, 1, '1.514066');
    checkCoshResult('0.45175', 1, 6, '1');
    checkCoshResult('-0.519253467128781061573541633704', 2, 4, '1.1');
    checkCoshResult('-0.406127947', 8, 4, '1.0836098');
    checkCoshResult('0.9879237062663941347586862695316948803970665245072293680153692819868', 1, 6, '2');
    checkCoshResult('0.93872862', 6, 2, '1.47393');
    checkCoshResult('-0.3444', 2, 1, '1');
    checkCoshResult('0.08916117467145186539045806046188938916876705938558151882', 3, 4, '1');
    checkCoshResult('-0.416', 5, 0, '1.0878');
    checkCoshResult('0.96395116729510741473428', 7, 2, '1.50171');
    checkCoshResult('0.99122709843935', 6, 6, '1.53283');
    checkCoshResult('-0.982458257490853554631770530367962387675296749498661808', 4, 2, '1.523');
    checkCoshResult('-0.499', 9, 1, '1.12710543');
    checkCoshResult('0.2', 10, 4, '1.020066756');
    checkCoshResult('0.32898510410686170443107121609290058087988034054', 1, 2, '2');
    checkCoshResult('-0.5054805141', 4, 0, '1.131');
    checkCoshResult('0.905453421072444252766729099661770268735489121', 5, 2, '1.4388');
    checkCoshResult('0.8', 8, 4, '1.3374349');
    checkCoshResult('-0.92256', 9, 6, '1.45661127');
    checkCoshResult('-0.3', 3, 1, '1.04');
    checkCoshResult('-0.8022', 6, 0, '1.3394');
    checkCoshResult('0.280279125313444767164013539810445145153395088331', 9, 6, '1.039536');
    checkCoshResult('0.922', 9, 0, '1.4560184');
}

function testCoshWithSpecificConfigPart003() {
    checkCoshResult('-0.3831639', 2, 3, '1');
    checkCoshResult('-0.116822455409380771652804478024770757014', 9, 2, '1.00683151');
    checkCoshResult('-0.6271139128871161107740496', 6, 6, '1.20317');
    checkCoshResult('-0.365006', 6, 4, '1.06736');
    checkCoshResult('0.2851921', 4, 0, '1.041');
    checkCoshResult('0.8341406563142720368', 9, 6, '1.36854088');
    checkCoshResult('0.91', 4, 3, '1.443');
    checkCoshResult('-0.578095765576', 3, 2, '1.18');
    checkCoshResult('-0.5523113', 1, 2, '2');
    checkCoshResult('0.8820775602347211771265107133401152251168847255764663', 3, 1, '1.41');
    checkCoshResult('0.6', 6, 6, '1.18547');
    checkCoshResult('0.9519214874557153045994771268316705218216663716829234702503703405', 4, 4, '1.488');
    checkCoshResult('-0.3218817', 3, 2, '1.06');
    checkCoshResult('0.44351821336668159678862896314345092', 3, 4, '1.1');
    checkCoshResult('0.3049381299213785', 53, 1, '1.0468550263953477301259281776879206575932575051887101');
    checkCoshResult('0.26099', 10, 5, '1.034251653');
    checkCoshResult('-0.144401442746890126', 19, 4, '1.010444017455030066');
    checkCoshResult('-0.5714572371177476181814955346898484074259146899385956872902543373645059', 73, 5, '1.167773824213016730263000463935976580681778400516250742892039174797873742');
    checkCoshResult('-0.86979236712642165283', 49, 1, '1.402726925102945094675128817370785304534247058512');
    checkCoshResult('-0.9152145856049272208', 69, 0, '1.44887088581511840041990778954419353285403899090899859306282037305997');
    checkCoshResult('0.387335806024227799329046192853758455983606', 5, 2, '1.076');
    checkCoshResult('0.388975556466', 66, 5, '1.07660966083264696675091150003451244766269524553188412387790834976');
    checkCoshResult('0.33160669064983873525', 31, 6, '1.055487176550956129534144973298');
    checkCoshResult('-0.28019', 70, 5, '1.039510693535834131464819444254660971197184738177259681537805828091364');
    checkCoshResult('0.0343866155482285230161896002936333091757205795137', 120, 5, '1.0005912779235089895769422420654318600732389756527372429232171774523704748559241986434711131587505267979139253999862614');
}
