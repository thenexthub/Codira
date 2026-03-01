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
    let myTestsuite = new arktest.ArkTestsuite('myTestsuite');
    myTestsuite.addTest('toUpperCaseTest01', toUpperCaseTest01);
    myTestsuite.addTest('toUpperCaseTest02', toUpperCaseTest02);
    myTestsuite.addTest('toUpperCaseTest03', toUpperCaseTest03);
    myTestsuite.addTest('toUpperCaseTest04', toUpperCaseTest04);
    myTestsuite.addTest('toUpperCaseTest05', toUpperCaseTest05);
    myTestsuite.addTest('toUpperCaseTest06', toUpperCaseTest06);
    myTestsuite.addTest('toUpperCaseTest07', toUpperCaseTest07);
    myTestsuite.addTest('toLowerCaseTest01', toLowerCaseTest01);
    myTestsuite.addTest('toLowerCaseTest02', toLowerCaseTest02);
    myTestsuite.addTest('toLowerCaseTest03', toLowerCaseTest03);
    myTestsuite.addTest('toLowerCaseTest04', toLowerCaseTest04);
    myTestsuite.addTest('toLowerCaseTest05', toLowerCaseTest05);
    myTestsuite.addTest('toLowerCaseTest06', toLowerCaseTest06);
    myTestsuite.addTest('toLowerCaseTest07', toLowerCaseTest07);
    myTestsuite.run();
}

function toUpperCaseTest01() {
    let c = c'a'
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c'A')
}

function toUpperCaseTest02() {
    let c = c'z'
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c'Z')
}

function toUpperCaseTest03() {
    let c = c'A'
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c'A')
}

function toUpperCaseTest04() {
    let c = c'Z'
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c'Z')
}

function toUpperCaseTest05() {
    let c = c'0'
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c'0')
}

function toUpperCaseTest06() {
    let c = c'@'
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c'@')
}

function toUpperCaseTest07() {
    let c = c' '
    let cc = c.toUpperCase()
    arktest.assertEQ(cc, c' ')
}

function toLowerCaseTest01() {
    let c = c'a'
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c'a')
}

function toLowerCaseTest02() {
    let c = c'A'
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c'a')
}

function toLowerCaseTest03() {
    let c = c'Z'
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c'z')
}

function toLowerCaseTest04() {
    let c = c'z'
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c'z')
}

function toLowerCaseTest05() {
    let c = c'9'
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c'9')
}

function toLowerCaseTest06() {
    let c = c'@'
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c'@')
}

function toLowerCaseTest07() {
    let c = c' '
    let cc = c.toLowerCase()
    arktest.assertEQ(cc, c' ')
}
