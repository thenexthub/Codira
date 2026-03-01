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

function main() {
    let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
    myTestsuite.addTest("testDecimal", testDecimal);
    myTestsuite.addTest("testFullDigit", testFullDigit);
    myTestsuite.addTest("testScientific", testScientific);
    myTestsuite.run();
}

function testDecimal() {
    let num1: Double = 0.1;
    let num2: Double = 0.01;
    let num3: Double = 0.001;
    let num4: Double = 0.0001;
    let num5: Double = 0.00001;
    let num6: Double = 0.000001;
    let num7: Double = 0.0000001;
    let num8: Double = 0.00000001;

    const str1 = num1.toPrecision(1);
    const str2 = num2.toPrecision(2);
    const str3 = num3.toPrecision(3);
    const str4 = num4.toPrecision(4);
    const str5 = num5.toPrecision(5);
    const str6 = num6.toPrecision(6);
    const str7 = num7.toPrecision(7);
    const str8 = num8.toPrecision(8);

    arktest.assertEQ(str1, "0.1");
    arktest.assertEQ(str2, "0.010");
    arktest.assertEQ(str3, "0.00100");
    arktest.assertEQ(str4, "0.0001000");
    arktest.assertEQ(str5, "0.000010000");
    arktest.assertEQ(str6, "0.00000100000");
    arktest.assertEQ(str7, "1.000000e-7");
    arktest.assertEQ(str8, "1.0000000e-8");
}

function testFullDigit() {
    let num1: Double = 1;
    let num2: Double = 12;
    let num3: Double = 123;
    let num4: Double = 1234;
    let num5: Double = 12345;
    let num6: Double = 123456;
    let num7: Double = 1234567;
    let num8: Double = 12345678;

    const str1 = num1.toPrecision(1);
    const str2 = num2.toPrecision(2);
    const str3 = num3.toPrecision(3);
    const str4 = num4.toPrecision(4);
    const str5 = num5.toPrecision(5);
    const str6 = num6.toPrecision(6);
    const str7 = num7.toPrecision(7);
    const str8 = num8.toPrecision(8);

    arktest.assertEQ(str1, "1");
    arktest.assertEQ(str2, "12");
    arktest.assertEQ(str3, "123");
    arktest.assertEQ(str4, "1234");
    arktest.assertEQ(str5, "12345");
    arktest.assertEQ(str6, "123456");
    arktest.assertEQ(str7, "1234567");
    arktest.assertEQ(str8, "12345678");
}

function testScientific() {
    let num: Double = 1234567890;

    const str1 = num.toPrecision(1);
    const str2 = num.toPrecision(2);
    const str3 = num.toPrecision(3);
    const str4 = num.toPrecision(4);
    const str5 = num.toPrecision(5);
    const str6 = num.toPrecision(6);
    const str7 = num.toPrecision(7);
    const str8 = num.toPrecision(8);
    const str9 = num.toPrecision(9);
    const str10 = num.toPrecision(10);
    const str11 = num.toPrecision(11);
    const str12 = num.toPrecision(12);

    arktest.assertEQ(str1, "1e+9");
    arktest.assertEQ(str2, "1.2e+9");
    arktest.assertEQ(str3, "1.23e+9");
    arktest.assertEQ(str4, "1.235e+9");
    arktest.assertEQ(str5, "1.2346e+9");
    arktest.assertEQ(str6, "1.23457e+9");
    arktest.assertEQ(str7, "1.234568e+9");
    arktest.assertEQ(str8, "1.2345679e+9");
    arktest.assertEQ(str9, "1.23456789e+9");
    arktest.assertEQ(str10, "1234567890");
    arktest.assertEQ(str11, "1234567890.0");
    arktest.assertEQ(str12, "1234567890.00");
}

