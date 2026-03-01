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
    checkHypotResult('1', 'NaN', 20, 4, 'NaN');
    checkHypotResult('NaN', '1', 20, 4, 'NaN');
    checkHypotResult('Infinity', 'NaN', 20, 4, 'Infinity');
    checkHypotResult('NaN', '-Infinity', 20, 4, 'Infinity');
    checkHypotResult('Infinity', '-9', 20, 4, 'Infinity');
    checkHypotResult('9', 'Infinity', 20, 4, 'Infinity');
    checkHypotResult('9', '-Infinity', 20, 4, 'Infinity');
    checkHypotResult('0', '0', 20, 4, '0');
    checkHypotResult('0', '-0', 20, 4, '0');
    checkHypotResult('-0', '-0', 20, 4, '0');
}

function testHypot002() {
    checkHypotResult('-73859972487135996812.3348849386076204815086183608621433996131', '-40900275', 92, 4, '73859972487135996812.334896262958542538634842711692934521304033086425417371740907212824629036');
    checkHypotResult('87777821928367383886207001990043726894046439498880958101', '13100000', 53, 2, '87777821928367383886207001990043726894046439498880959000');
    checkHypotResult('-19.8731062176484', '-631970.6', 60, 0, '631970.600312467344715323508497215861344293021711284138304041');
    checkHypotResult('861123450449582222', '-3268941330032700177909698138482', 73, 1, '3268941330032700177909698251903.062363755594001549793838456512462423760753');
    checkHypotResult('-1.759', '-0.1666933060407184714285777825813947232316520181597980844428', 8, 3, '1.7668807');
    checkHypotResult('0.1852451289933636289285652675346736246965605', '-48506.827382527105658688770367877324273166861983575641312454026244237512829473955620223096782936193504746', 48, 2, '48506.8273828808265642535390473175215096921677017');
    checkHypotResult('-8', '-5482493793', 53, 2, '5482493793.0000000058367599140481142689669647662254134');
    checkHypotResult('4111111111111111.11111111111611110111171111811111111611', '-0.7801745669150266383906952786894096943691775116027', 26, 1, '4111111111111111.1111111111');
    checkHypotResult('-55102601.0381', '-5160591', 93, 5, '55343729.0091054507509667515051408897750996134016431347788775107555545851579689419350413225929');
    checkHypotResult('183', '0.6', 22, 3, '183.0009836039139981977');
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
    suite.addTest("Decimal hypot", testHypot002);
    config();
    return suite.run();
}