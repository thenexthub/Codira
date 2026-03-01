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

function main(): int {
    let mathTestsuite = new arktest.ArkTestsuite("mathTest")
    mathTestsuite.addTest("logTest01", logTest01)
    mathTestsuite.addTest("logTest02", logTest02)
    mathTestsuite.addTest("logTest03", logTest03)
    mathTestsuite.addTest("logTest04", logTest04)
    mathTestsuite.addTest("expmTest01", expmTest01)
    mathTestsuite.addTest("expmTest02", expmTest02)
    mathTestsuite.addTest("expmTest03", expmTest03)
    mathTestsuite.addTest("expmTest04", expmTest04)
    return mathTestsuite.run()
}


function logTest01() {
    const result: double = log1p(1.0);
    const expected: double = Math.log(2.0);
    arktest.assertEQ(result, expected);
}

function logTest02() {
    const result: double = log1p(10.0);
    const expected: double = Math.log(11.0);
    arktest.assertEQ(result, expected);
}

function logTest03() {
    const result: float = log1p(1.0f);
    const expected: float = Math.log(2.0f);
    arktest.assertEQ(result, expected);
}

function logTest04() {
    const result: float = log1p(10.0f);
    const expected: float = Math.log(11.0f);
    arktest.assertEQ(result, expected);
}

function expmTest01() {
    const result: double = expm1(1.0);
    const expected: double = Math.exp(1.0) - 1;
    arktest.assertEQ(result, expected);
}

function expmTest02() {
    const result: double = expm1(10.0);
    const expected: double = Math.exp(10.0) - 1;
    arktest.assertEQ(result, expected);
}

function expmTest03() {
    const result: float = expm1(1.0f);
    const expected: float = Math.exp(1.0f) - 1;
    arktest.assertEQ(result, expected);
}

function expmTest04() {
    const result: float = expm1(10.0f);
    const expected: float = Math.exp(10.0f) - 1;
    arktest.assertEQ(result, expected);
}