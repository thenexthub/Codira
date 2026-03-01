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

function main() {
    const suite = new arktest.ArkTestsuite("Decimal abs API tests");
    suite.addTest('Decimal abs with default config', testAbsWithDefaultConfig001);
    suite.addTest('Decimal abs with default config', testAbsWithDefaultConfig002);
    suite.addTest('Decimal abs with default config', testAbsWithDefaultConfig003);
    suite.addTest('Decimal abs with default config', testAbsWithDefaultConfig004);
    suite.addTest('Decimal abs with toExpNeg and toExpPos is zero', testAbsWithSpecificConfig001);
    suite.addTest('Decimal abs with toExpNeg and toExpPos is zero', testAbsWithSpecificConfig002);
    suite.addTest('Decimal abs with toExpNeg and toExpPos is zero', testAbsWithSpecificConfig003);
    return suite.run();
}

function testAbsWithDefaultConfig001() {
    arktest.assertEQ(new Decimal(0).abs().valueOf(), '0');
    arktest.assertEQ(new Decimal('0').abs().valueOf(), '0');
    arktest.assertEQ(new Decimal(-0).abs().valueOf(), '0');
    arktest.assertEQ(new Decimal('-0').abs().valueOf(), '0');
    arktest.assertEQ(new Decimal(1).abs().valueOf(), '1');
    arktest.assertEQ(new Decimal(-1).abs().valueOf(), '1');
    arktest.assertEQ(new Decimal('-1').abs().valueOf(), '1');
    arktest.assertEQ(new Decimal('0.5').abs().valueOf(), '0.5');
    arktest.assertEQ(new Decimal('-0.5').abs().valueOf(), '0.5');
    arktest.assertEQ(new Decimal(0.1).abs().valueOf(), '0.1');
    arktest.assertEQ(new Decimal(-0.1).abs().valueOf(), '0.1');
    arktest.assertEQ(new Decimal(1.1).abs().valueOf(), '1.1');
    arktest.assertEQ(new Decimal(-1.1).abs().valueOf(), '1.1');
    arktest.assertEQ(new Decimal('1.5').abs().valueOf(), '1.5');
    arktest.assertEQ(new Decimal('-1.5').abs().valueOf(), '1.5');
}

function testAbsWithDefaultConfig002() {
    arktest.assertEQ(new Decimal('-1e-5').abs().valueOf(), '0.00001');
    arktest.assertEQ(new Decimal('-9e9').abs().valueOf(), '9000000000');
    arktest.assertEQ(new Decimal('123456.7891011').abs().valueOf(), '123456.7891011');
    arktest.assertEQ(new Decimal(-123456.7891011).abs().valueOf(), '123456.7891011');
    arktest.assertEQ(new Decimal('99').abs().valueOf(), '99');
    arktest.assertEQ(new Decimal(-99).abs().valueOf(), '99');
    arktest.assertEQ(new Decimal(999.999).abs().valueOf(), '999.999');
    arktest.assertEQ(new Decimal('-999.999').abs().valueOf(), '999.999');
    arktest.assertEQ(new Decimal(new Decimal(-1)).abs().valueOf(), '1');
    arktest.assertEQ(new Decimal(new Decimal('-1')).abs().valueOf(), '1');
    arktest.assertEQ(new Decimal(new Decimal(0.001)).abs().valueOf(), '0.001');
    arktest.assertEQ(new Decimal(new Decimal('-0.001')).abs().valueOf(), '0.001');
}

function testAbsWithDefaultConfig003() {
    arktest.assertEQ(new Decimal(Infinity).abs().valueOf(),'Infinity');
    arktest.assertEQ(new Decimal(-Infinity).abs().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('Infinity').abs().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-Infinity').abs().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal(NaN).abs().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal(-NaN).abs().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('NaN').abs().valueOf(), 'NaN');
    arktest.assertEQ(new Decimal('-NaN').abs().valueOf(), 'NaN');
}

function testAbsWithDefaultConfig004() {
    arktest.assertEQ(new Decimal('11.121').abs().valueOf(), '11.121');
    arktest.assertEQ(new Decimal('-0.023842').abs().valueOf(), '0.023842');
    arktest.assertEQ(new Decimal('-1.19').abs().valueOf(), '1.19');
    arktest.assertEQ(new Decimal('-0.00000000009622').abs().valueOf(), '9.622e-11');
    arktest.assertEQ(new Decimal('-0.000000000509').abs().valueOf(), '5.09e-10');
    arktest.assertEQ(new Decimal('3838.2').abs().valueOf(), '3838.2');
    arktest.assertEQ(new Decimal('127.0').abs().valueOf(), '127');
    arktest.assertEQ(new Decimal('4.23073').abs().valueOf(), '4.23073');
    arktest.assertEQ(new Decimal('-2.5469').abs().valueOf(), '2.5469');
    arktest.assertEQ(new Decimal('-29949').abs().valueOf(), '29949');
    arktest.assertEQ(new Decimal('-277.10').abs().valueOf(), '277.1');
    arktest.assertEQ(new Decimal('-0.00000000000000497898').abs().valueOf(), '4.97898e-15');
    arktest.assertEQ(new Decimal('53.456').abs().valueOf(), '53.456');
    arktest.assertEQ(new Decimal('-100564').abs().valueOf(), '100564');
    arktest.assertEQ(new Decimal('-12431.9').abs().valueOf(), '12431.9');
    arktest.assertEQ(new Decimal('-97633.7').abs().valueOf(), '97633.7');
    arktest.assertEQ(new Decimal('220').abs().valueOf(), '220');
    arktest.assertEQ(new Decimal('18.720').abs().valueOf(), '18.72');
    arktest.assertEQ(new Decimal('-2817').abs().valueOf(), '2817');
    arktest.assertEQ(new Decimal('-44535').abs().valueOf(), '44535');
}

function testAbsWithSpecificConfig001() {
    Decimal.set({ toExpNeg: 0, toExpPos: 0 });
    arktest.assertEQ(new Decimal('-5.2452468128e+1').abs().valueOf(), '5.2452468128e+1');
    arktest.assertEQ(new Decimal('1.41525905257189365008396e+16').abs().valueOf(), '1.41525905257189365008396e+16');
    arktest.assertEQ(new Decimal('2.743068083928e+11').abs().valueOf(), '2.743068083928e+11');
    arktest.assertEQ(new Decimal('-1.52993064722314247378724599e+26').abs().valueOf(), '1.52993064722314247378724599e+26');
    arktest.assertEQ(new Decimal('3.7205576746e+10').abs().valueOf(), '3.7205576746e+10');
    arktest.assertEQ(new Decimal('-2.663e-10').abs().valueOf(),'2.663e-10');
    arktest.assertEQ(new Decimal('-1.26574209965030360615518e+17').abs().valueOf(), '1.26574209965030360615518e+17');
    arktest.assertEQ(new Decimal('1.052e+3').abs().valueOf(),'1.052e+3');
    arktest.assertEQ(new Decimal('-4.452945872502e+6').abs().valueOf(), '4.452945872502e+6');
    arktest.assertEQ(new Decimal('2.95732460816619226e+13').abs().valueOf(), '2.95732460816619226e+13');
    arktest.assertEQ(new Decimal('-1.1923100194288654481424e+18').abs().valueOf(), '1.1923100194288654481424e+18');
    arktest.assertEQ(new Decimal('8.99315449050893705e+6').abs().valueOf(), '8.99315449050893705e+6');
    arktest.assertEQ(new Decimal('5.200726538434486963e+8').abs().valueOf(),'5.200726538434486963e+8');
    arktest.assertEQ(new Decimal('1.182618278949368566264898065e+18').abs().valueOf(), '1.182618278949368566264898065e+18');
    arktest.assertEQ(new Decimal('-3.815873266712e-20').abs().valueOf(), '3.815873266712e-20');
    arktest.assertEQ(new Decimal('-1.316675370382742615e+6').abs().valueOf(), '1.316675370382742615e+6');
    arktest.assertEQ(new Decimal('-2.1032502e+6').abs().valueOf(), '2.1032502e+6');
    arktest.assertEQ(new Decimal('1.8e+1').abs().valueOf(), '1.8e+1');
}

function testAbsWithSpecificConfig002() {
    arktest.assertEQ(new Decimal('1.033525906631680944018544811261e-13').abs().valueOf(), '1.033525906631680944018544811261e-13');
    arktest.assertEQ(new Decimal('-1.102361746443461856816e+14').abs().valueOf(), '1.102361746443461856816e+14');
    arktest.assertEQ(new Decimal('8.595358491143959e+1').abs().valueOf(),'8.595358491143959e+1');
    arktest.assertEQ(new Decimal('1.226806049797304683867e-18').abs().valueOf(), '1.226806049797304683867e-18');
    arktest.assertEQ(new Decimal('-5e+0').abs().valueOf(), '5e+0');
    arktest.assertEQ(new Decimal('-1.091168788407093537887970016e+15').abs().valueOf(), '1.091168788407093537887970016e+15');
    arktest.assertEQ(new Decimal('3.87166413612272027e+12').abs().valueOf(), '3.87166413612272027e+12');
    arktest.assertEQ(new Decimal('1.411514e+5').abs().valueOf(), '1.411514e+5');
    arktest.assertEQ(new Decimal('1.0053454672509859631996e+22').abs().valueOf(), '1.0053454672509859631996e+22');
    arktest.assertEQ(new Decimal('6.9265714e+0').abs().valueOf(), '6.9265714e+0');
    arktest.assertEQ(new Decimal('1.04627709e+4').abs().valueOf(), '1.04627709e+4');
    arktest.assertEQ(new Decimal('2.285650225267766689304972e+5').abs().valueOf(), '2.285650225267766689304972e+5');
    arktest.assertEQ(new Decimal('4.5790517211306242e+7').abs().valueOf(), '4.5790517211306242e+7');
    arktest.assertEQ(new Decimal('-3.0033340092338313923473428e+16').abs().valueOf(), '3.0033340092338313923473428e+16');
    arktest.assertEQ(new Decimal('-2.83879929283797623e+1').abs().valueOf(), '2.83879929283797623e+1');
    arktest.assertEQ(new Decimal('4.5266377717178121183759377414e-5').abs().valueOf(), '4.5266377717178121183759377414e-5');
    arktest.assertEQ(new Decimal('-5.3781e+4').abs().valueOf(), '5.3781e+4');
    arktest.assertEQ(new Decimal('-6.722035208213298413522819127e-18').abs().valueOf(), '6.722035208213298413522819127e-18');
    arktest.assertEQ(new Decimal('-3.02865707828281230987116e+23').abs().valueOf(), '3.02865707828281230987116e+23');
}

function testAbsWithSpecificConfig003() {
    arktest.assertEQ(new Decimal('1e-9000000000000000').abs().valueOf(), '1e-9000000000000000');
    arktest.assertEQ(new Decimal('-1e-9000000000000000').abs().valueOf(), '1e-9000000000000000');
    arktest.assertEQ(new Decimal('-9.9e-9000000000000001').abs().valueOf(), '0e+0');
    arktest.assertEQ(new Decimal('9.999999e+9000000000000000').abs().valueOf(), '9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('-9.999999e+9000000000000000').abs().valueOf(), '9.999999e+9000000000000000');
    arktest.assertEQ(new Decimal('1E9000000000000001').abs().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-1e+9000000000000001').abs().valueOf(), 'Infinity');
    arktest.assertEQ(new Decimal('-5.5879983320336874473209567979e+287894365').abs().valueOf(), '5.5879983320336874473209567979e+287894365');
}